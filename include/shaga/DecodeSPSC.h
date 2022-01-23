/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2022, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_DecodeSPSC
#define HEAD_shaga_DecodeSPSC

#include "common.h"

namespace shaga
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  DecodeSPSC  /////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class DataInterface = SPSCDataDynAlloc, typename std::enable_if_t<std::is_base_of<SPSCDataInterface, DataInterface>::value, int> = 0>
	class DecodeSPSC
	{
		private:
			#ifdef SHAGA_THREADING
				std::atomic<uint_fast32_t> _err_count {0};
			#else
				volatile uint_fast32_t _err_count {0};
			#endif // SHAGA_THREADING

			std::weak_ptr<StringSPSC> _err_spsc;
			bool _throw_at_error {false};

			#ifdef OS_LINUX
				int _eventfd {-1};
				SHARED_SOCKET _event_sock;
				uint64_t _eventfd_write_val {1};
				uint64_t _eventfd_read_val {0};
			#endif // OS_LINUX

		protected:
			const uint_fast32_t _max_packet_size;
			const uint_fast32_t _num_packets;
			std::string _name;

			std::vector<std::unique_ptr<DataInterface>> _data;
			DataInterface* _curdata {nullptr};

			#ifdef SHAGA_THREADING
				std::atomic<uint_fast32_t> _pos_read {0};
				std::atomic<uint_fast32_t> _pos_write {0};
			#else
				volatile uint_fast32_t _pos_read {0};
				volatile uint_fast32_t _pos_write {0};
			#endif // SHAGA_THREADING

			#ifdef OS_LINUX
			virtual bool read_eventfd (void) final
			{
				const ssize_t sze = ::read (this->_eventfd, &_eventfd_read_val, sizeof (_eventfd_read_val));
				if (sze < 0) {
					if (errno == EWOULDBLOCK) {
						return false;
					}
					cThrow ("{}: Error reading from notice eventfd: {}"sv, this->_name, strerror (errno));
				}
				else if (sze != sizeof (_eventfd_read_val)) {
					cThrow ("{}: Error reading from notice eventfd: Unknown error"sv, this->_name);
				}

				return true;
			}
			#endif // OS_LINUX

			virtual void nonfatal_error (const std::string_view buf) final
			{
				#ifdef SHAGA_THREADING
					_err_count.fetch_add (1, std::memory_order::memory_order_relaxed);
				#else
					++_err_count;
				#endif // SHAGA_THREADING

				if (true == _throw_at_error) {
					cThrow ("{}: {}"sv, _name, buf);
				}

				try {
					auto ptr = _err_spsc.lock ();
					if (nullptr != ptr) {
						ptr->push_back (_name + ": "s + std::string (buf));
					}
				}
				catch (...) {
					/* Ignore the exception at this point */
				}
			}

			virtual void push (void) final
			{
				#ifdef SHAGA_THREADING
					_SHAGA_SPSC_D_RING (next, _pos_write.load (std::memory_order_relaxed));

					if (next == _pos_read.load (std::memory_order_acquire)) {
						cThrow ("{}: Ring full"sv, _name);
					}

					_pos_write.store (next, std::memory_order_release);
				#else
					_SHAGA_SPSC_D_RING (next, _pos_write);

					if (next == _pos_read) {
						cThrow ("{}: Ring full"sv, _name);
					}

					_pos_write = next;
				#endif // SHAGA_THREADING

				#ifdef OS_LINUX
				if (const ssize_t ret = ::write (this->_eventfd, &_eventfd_write_val, sizeof (_eventfd_write_val)); ret < 0) {
					cThrow ("{}: Error writing to eventfd: {}"sv, _name, strerror (errno));
				}
				else if (ret != sizeof (_eventfd_write_val)) {
					cThrow ("{}: Error writing to eventfd: Unknown error"sv, _name);
				}
				#endif // OS_LINUX

				_curdata = _data[next].get ();
			}

			virtual void _push_buffer (const uint8_t *const buffer, uint_fast32_t offset, const uint_fast32_t len) = 0;

		public:
			DecodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				_max_packet_size (max_packet_size),
				_num_packets (num_packets)
			{
				_name.assign (typeid (*this).name ());

				if (_num_packets < 2) {
					cThrow ("{}: Ring size must be at least 2"sv, _name);
				}

				_data.reserve (_num_packets);
				for (uint_fast32_t i = 0; i < num_packets; i++) {
					_data.push_back (std::make_unique<DataInterface> (_max_packet_size));
				}
				_curdata = _data.front ().get ();

				#ifdef OS_LINUX
				/* This eventfd will work as a SEMAPHORE, so every push will increase counter by one and every read will decrease it. */
				this->_eventfd = eventfd (0, EFD_NONBLOCK | EFD_SEMAPHORE);
				if (this->_eventfd < 0) {
					cThrow ("{}: Unable to init eventfd: {}"sv, _name, strerror (errno));
				}
				this->_event_sock = std::make_shared<ShSocket> (this->_eventfd);
				#endif // OS_LINUX
			}

			virtual ~DecodeSPSC ()
			{
				#ifdef OS_LINUX
				this->_eventfd = -1;
				this->_event_sock.reset ();
				#endif // OS_LINUX

				_curdata = nullptr;
				_data.clear ();
			}

			/* Disable copy and assignment */
			DecodeSPSC (const DecodeSPSC &) = delete;
			DecodeSPSC& operator= (const DecodeSPSC &) = delete;

			virtual void set_name (const std::string_view name) final
			{
				_name.assign (name);
			}

			virtual void throw_at_nonfatal_error (const bool enabled) final
			{
				_throw_at_error = enabled;
			}

			virtual void set_error_spsc (std::weak_ptr<StringSPSC> err_spsc) final
			{
				_err_spsc = err_spsc;
			}

			virtual uint_fast32_t get_err_count (void) const final
			{
				#ifdef SHAGA_THREADING
					return _err_count.load (std::memory_order::memory_order_relaxed);
				#else
					return _err_count;
				#endif // SHAGA_THREADING
			}

			virtual uint_fast32_t get_err_count_reset (void) final
			{
				#ifdef SHAGA_THREADING
					return _err_count.exchange (0, std::memory_order::memory_order_acq_rel);
				#else
					return std::exchange (_err_count, 0);
				#endif // SHAGA_THREADING
			}

			#ifdef OS_LINUX
			virtual int get_eventfd (void) const final
			{
				return this->_eventfd;
			}

			virtual SHARED_SOCKET get_eventfd_shared_socket (void) const final
			{
				return this->_event_sock;
			}
			#endif // OS_LINUX

			virtual bool pop_buffer (std::string &out) final
			{
				#ifdef OS_LINUX
				if (this->read_eventfd () == false) {
					return false;
				}
				#endif // OS_LINUX

				#ifdef SHAGA_THREADING
					const uint_fast32_t now = this->_pos_read.load (std::memory_order_relaxed);
					if (now == this->_pos_write.load (std::memory_order_acquire)) {
						#ifdef OS_LINUX
						cThrow ("{}: Internal error: eventfd test passed but atomic position didn't"sv, this->_name);
						#endif // OS_LINUX
						return false;
					}
				#else
					const uint_fast32_t now = this->_pos_read;
					if (now == this->_pos_write) {
						#ifdef OS_LINUX
						cThrow ("{}: Internal error: eventfd test passed but read/write position didn't"sv, this->_name);
						#endif // OS_LINUX
						return false;
					}
				#endif // SHAGA_THREADING

				out.assign (reinterpret_cast<const char *> (this->_data[now]->buffer), this->_data[now]->size ());
				this->_data[now]->free ();

				_SHAGA_SPSC_D_RING (next, now);

				#ifdef SHAGA_THREADING
					this->_pos_read.store (next, std::memory_order_release);
				#else
					this->_pos_read = next;
				#endif // SHAGA_THREADING

				return true;
			}

			virtual bool pop_buffer (void) final
			{
				#ifdef OS_LINUX
				if (this->read_eventfd () == false) {
					return false;
				}
				#endif // OS_LINUX

				#ifdef SHAGA_THREADING
					const uint_fast32_t now = this->_pos_read.load (std::memory_order_relaxed);
					if (now == this->_pos_write.load (std::memory_order_acquire)) {
						#ifdef OS_LINUX
						cThrow ("{}: Internal error: eventfd test passed but atomic position didn't"sv, this->_name);
						#endif // OS_LINUX
						return false;
					}
				#else
					const uint_fast32_t now = this->_pos_read;
					if (now == this->_pos_write) {
						#ifdef OS_LINUX
						cThrow ("{}: Internal error: eventfd test passed but read/write position didn't"sv, this->_name);
						#endif // OS_LINUX
						return false;
					}
				#endif // SHAGA_THREADING

				this->_data[now]->free ();

				_SHAGA_SPSC_D_RING (next, now);

				#ifdef SHAGA_THREADING
					this->_pos_read.store (next, std::memory_order_release);
				#else
					this->_pos_read = next;
				#endif // SHAGA_THREADING

				return true;
			}

			virtual std::optional<std::string_view> peek_buffer (void) final
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t now = this->_pos_read.load (std::memory_order_relaxed);
					if (now == this->_pos_write.load (std::memory_order_acquire)) {
						return std::nullopt;
					}
				#else
					const uint_fast32_t now = this->_pos_read;
					if (now == this->_pos_write) {
						return std::nullopt;
					}
				#endif // SHAGA_THREADING

				return std::optional<std::string_view> {std::in_place, reinterpret_cast<const char *> (this->_data[now]->buffer), this->_data[now]->size ()};
			}

			virtual void push_buffer (const void *const buffer, const uint_fast32_t offset, const uint_fast32_t len) final
			{
				this->_push_buffer (reinterpret_cast<const uint8_t *> (buffer), offset, len);
			}

			virtual void push_buffer (const void *const buffer, const uint_fast32_t len) final
			{
				this->_push_buffer (reinterpret_cast<const uint8_t *> (buffer), 0, len);
			}

			virtual void push_buffer (const std::string_view buffer) final
			{
				this->_push_buffer (reinterpret_cast<const uint8_t *> (buffer.data ()), 0, buffer.size ());
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  SimpleNewLineDecodeSPSC  ////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class SimpleNewLineDecodeSPSC : public DecodeSPSC<T>
	{
		private:
			const bool _skip_empty_lines {true};

			virtual void _push_buffer (const uint8_t *const buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				for (;offset < len; ++offset) {
					if ('\r' == buffer[offset]) {
						continue;
					}
					else if ('\n' == buffer[offset]) {
						if (false == _skip_empty_lines || false == this->_curdata->empty ()) {
							this->push ();

							this->_curdata->zero_size ();
							this->_curdata->alloc ();
						}
					}
					else {
						this->_curdata->buffer[this->_curdata->inc_size ()] = buffer[offset];
					}
				}
			}

		public:
			SimpleNewLineDecodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets, const bool skip_empty_lines = true) :
				DecodeSPSC<T> (max_packet_size + 1, num_packets),
				_skip_empty_lines (skip_empty_lines)
			{
				this->_curdata->zero_size ();
				this->_curdata->alloc ();
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Simple8DecodeSPSC  //////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class Simple8DecodeSPSC : public DecodeSPSC<T>
	{
		private:
			const uint8_t _stx;
			const uint8_t _etx;
			const uint8_t _ntx;

			bool _got_stx {false};
			bool _escape_next_char {false};

			bool _has_crc8 {false};
			uint8_t _crc8_val {0};

			virtual bool check (void)
			{
				if (true == _has_crc8) {
					this->_curdata->dec_size ();
					if (_crc8_val != this->_curdata->buffer[this->_curdata->size ()]) {
						/* CRC failed */
						this->nonfatal_error ("CRC mismatch"sv);
						return false;
					}
				}

				return true;
			}

			virtual void _push_buffer (const uint8_t *const buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				for (;offset < len; ++offset) {
					if (_stx == buffer[offset]) {
						if (true == _got_stx) {
							this->nonfatal_error ("STX without ETX"sv);
						}
						_escape_next_char = false;
						this->_curdata->zero_size ();
						this->_curdata->alloc ();
						_crc8_val = _stx;
						_got_stx = true;
					}
					else if (false == _got_stx) {
						/* Ignore this data, because there was no STX */
					}
					else if (_etx == buffer[offset]) {
						if (true == _escape_next_char) {
							this->nonfatal_error ("ETX instantly after NTX"sv);
						}
						else {
							if (this->check () == true) {
								this->push ();
							}
						}
						_got_stx = false;
					}
					else if (_ntx == buffer[offset]) {
						if (true == _escape_next_char) {
							this->nonfatal_error ("NTX instantly after NTX"sv);
							_got_stx = false;
						}
						else {
							_escape_next_char = true;
						}
					}
					else {
						if (true == _has_crc8 && this->_curdata->size () > 0) {
							_crc8_val = CRC::_crc8_dallas_table[this->_curdata->buffer[this->_curdata->size () - 1] ^ _crc8_val];
						}

						if (true == _escape_next_char) {
							_escape_next_char = false;
							this->_curdata->buffer[this->_curdata->inc_size ()] = buffer[offset] ^ 0x80;
						}
						else {
							this->_curdata->buffer[this->_curdata->inc_size ()] = buffer[offset];
						}
					}
				}
			}

		public:
			Simple8DecodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets, const std::array<uint8_t, 3> control_bytes) :
				DecodeSPSC<T> (max_packet_size + 1, num_packets),
				_stx (control_bytes[0]),
				_etx (control_bytes[1]),
				_ntx (control_bytes[2])
			{
				if ((_stx & 0x7F) == (_etx & 0x7F) || (_stx & 0x7F) == (_ntx & 0x7F) || (_etx & 0x7F) == (_ntx & 0x7F)) {
					cThrow ("Control bytes must differ in lower 7 bits"sv);
				}
			}

			virtual void has_crc8 (const bool enabled) final
			{
				_has_crc8 = enabled;
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Simple16DecodeSPSC  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class Simple16DecodeSPSC : public DecodeSPSC<T>
	{
		private:
			const std::array<uint8_t,2> _stx;

			#ifdef SHAGA_THREADING
				std::atomic<uint_fast32_t> _ignored_bytes {0};
			#else
				volatile uint_fast32_t _ignored_bytes {0};
			#endif // SHAGA_THREADING

			uint_fast8_t _got_stx {0};

			bool _has_crc16 {false};
			uint_fast16_t _crc16_val {0};
			uint_fast16_t _crc16_startval {0};

			uint_fast32_t _remaining_len {UINT32_MAX};

			virtual bool check (void)
			{
				try {
					if (true == _has_crc16) {
						this->_curdata->dec2_size ();

						if (0 != _crc16_val) {
							/* CRC-16 failed */
							this->nonfatal_error ("CRC-16 mismatch"sv);
							return false;
						}
					}

					return true;
				}
				catch (...) {
					this->nonfatal_error ("Check failed"sv);
					return false;
				}
			}

			virtual void _push_buffer (const uint8_t *const buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				for (;offset < len; ++offset) {
					if (0 == _got_stx) {
						if (_stx[0] == buffer[offset]) {
							_got_stx = 1;
						}
						else {
							#ifdef SHAGA_THREADING
								_ignored_bytes.fetch_add (1, std::memory_order::memory_order_relaxed);
							#else
								_ignored_bytes += 1;
							#endif // SHAGA_THREADING
						}
					}
					else if (1 == _got_stx) {
						if (_stx[1] == buffer[offset]) {
							_got_stx = 2;
						}
						else {
							/* Expected second STX character, return to the beginning */
							_got_stx = 0;
							#ifdef SHAGA_THREADING
								_ignored_bytes.fetch_add (2, std::memory_order::memory_order_relaxed);
							#else
								_ignored_bytes += 2;
							#endif // SHAGA_THREADING
						}
					}
					else if (2 == _got_stx) {
						/* Low length byte */
						_remaining_len = buffer[offset];
						_got_stx = 3;

						_crc16_val = _crc16_startval;
						_crc16_val = (_crc16_val >> 8) ^ CRC::_crc16_modbus_table[(_crc16_val ^ static_cast<uint_fast16_t> (buffer[offset])) & 0xff];
					}
					else if (3 == _got_stx) {
						/* High length byte */
						_remaining_len |= (buffer[offset] << 8);

						if (_remaining_len > this->_max_packet_size) {
							this->nonfatal_error ("Packet size larger than allocated size, skipping..."sv);
							_got_stx = 0;
							_remaining_len = UINT32_MAX;

							#ifdef SHAGA_THREADING
								_ignored_bytes.fetch_add (4, std::memory_order::memory_order_relaxed);
							#else
								_ignored_bytes += 4;
							#endif // SHAGA_THREADING
						}
						else {
							_got_stx = 4;

							_crc16_val = (_crc16_val >> 8) ^ CRC::_crc16_modbus_table[(_crc16_val ^ static_cast<uint_fast16_t> (buffer[offset])) & 0xff];

							this->_curdata->zero_size ();
							this->_curdata->alloc (_remaining_len);
						}
					}
					else {
						/* If we have at least 2 bytes already, use it for crc calculation. */
						if (true == _has_crc16) {
							_crc16_val = (_crc16_val >> 8) ^ CRC::_crc16_modbus_table[(_crc16_val ^ static_cast<uint_fast16_t> (buffer[offset])) & 0xff];
						}

						/* This is part of the message */
						this->_curdata->buffer[this->_curdata->inc_size ()] = buffer[offset];

						if (_remaining_len > 0) {
							_remaining_len --;
						}
					}

					if (4 == _got_stx && 0 == _remaining_len) {
						/* We got all data we expected */
						if (true == this->check ()) {
							this->push ();
						}
						_got_stx = 0;
						_remaining_len = UINT32_MAX;
					}
				}
			}

		public:
			Simple16DecodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets, const std::array<uint8_t, 2> start_sequence) :
				DecodeSPSC<T> (max_packet_size + 2, num_packets),
				_stx (start_sequence)
			{ }

			virtual void has_crc16 (const bool enabled, const bool include_stx = false, const uint_fast16_t startval = UINT16_MAX) final
			{
				_has_crc16 = enabled;

				if (false == include_stx) {
					_crc16_startval = startval;
				}
				else {
					uint_fast16_t crc16_val = startval;
					crc16_val = (crc16_val >> 8) ^ CRC::_crc16_modbus_table[(crc16_val ^ static_cast<uint_fast16_t> (_stx[0])) & 0xff];
					crc16_val = (crc16_val >> 8) ^ CRC::_crc16_modbus_table[(crc16_val ^ static_cast<uint_fast16_t> (_stx[1])) & 0xff];
					_crc16_startval = crc16_val;
				}
			}

			virtual uint_fast32_t get_ignored_bytes (void) const
			{
				#ifdef SHAGA_THREADING
					return _ignored_bytes.load (std::memory_order::memory_order_relaxed);
				#else
					return _ignored_bytes;
				#endif // SHAGA_THREADING
			}

			virtual uint_fast32_t get_ignored_bytes_reset (void)
			{
				#ifdef SHAGA_THREADING
					return _ignored_bytes.exchange (0, std::memory_order::memory_order_acq_rel);
				#else
					return std::exchange (_ignored_bytes, 0);
				#endif // SHAGA_THREADING
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  PacketDecodeSPSC  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class PacketDecodeSPSC : public DecodeSPSC<T>
	{
		private:
			static const constexpr char _magic[2] {5, 23};
			static const constexpr uint_fast32_t _crc_max_len {16};
			static const constexpr uint8_t _crc_start_val {19};

			uint8_t _crc8_val {0};
			uint8_t _crc8_expected {0};

			uint_fast32_t _remaining_data {0};
			uint_fast32_t _remaining_crc {0};
			uint_fast32_t _got_header {0};

			std::string _size_buf;

			virtual void _push_buffer (const uint8_t *const buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				for (;offset < len; ++offset) {
					if (_remaining_crc > 0) {
						_crc8_val = CRC::_crc8_dallas_table[buffer[offset] ^ _crc8_val];
						--_remaining_crc;

						if (0 == _remaining_crc && _crc8_val != _crc8_expected) {
							this->nonfatal_error ("CRC mismatch"sv);
							_got_header = 0;
							_remaining_crc = 0;
							_remaining_data = 0;
						}
					}

					if (_remaining_data > 0) {
						this->_curdata->buffer[this->_curdata->inc_size ()] = buffer[offset];
						--_remaining_data;

						if (0 == _remaining_data) {
							if (_crc8_val != _crc8_expected) {
								this->nonfatal_error ("CRC mismatch"sv);
							}
							else {
								this->push ();
							}
							_got_header = 0;
							_remaining_crc = 0;
						}
					}
					else if (0 == _got_header) {
						/* _magic[0] */
						if (buffer[offset] == _magic[0]) {
							_got_header = 1;
						}
					}
					else if (1 == _got_header) {
						/* _magic[1] */
						if (buffer[offset] == _magic[1]) {
							_got_header = 2;
						}
					}
					else if (2 == _got_header) {
						/* This is CRC byte. From this moment on, calculate CRC from up to _crc_max_len bytes. */
						_got_header = 3;
						_crc8_expected = buffer[offset];
						_crc8_val = _crc_start_val;
						_remaining_crc = _crc_max_len;
					}
					else if (3 == _got_header) {
						/* 1. size byte */
						_got_header = 4;
						_size_buf.resize (3, 0x00);
						_size_buf[0] = buffer[offset];
					}
					else if (4 == _got_header) {
						/* 2. size byte */
						_got_header = 5;
						_size_buf[1] = buffer[offset];
					}
					else if (5 == _got_header) {
						/* 3. size byte */
						_got_header = 0;
						_size_buf[2] = buffer[offset];

						_remaining_data = BIN::to_uint24 (_size_buf);
						this->_curdata->alloc (_remaining_data);
						this->_curdata->zero_size ();
					}
					else {
						cThrow ("Fell over the edge of the world"sv);
					}
				}
			}

		public:
			PacketDecodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				DecodeSPSC<T> (max_packet_size, num_packets)
			{ }
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  SeqPacketDecodeSPSC  ////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class SeqPacketDecodeSPSC : public DecodeSPSC<T>
	{
		private:
			virtual void _push_buffer (const uint8_t *const buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				const uint_fast32_t sze = len - offset;

				if (sze < 3) {
					this->nonfatal_error ("Message too short"sv);
					return;
				}

				uint_fast32_t data_length {0};

				try {
					const std::string_view tbuf (reinterpret_cast <const char *> (buffer + offset), 3);
					data_length = BIN::to_uint24 (tbuf);
				}
				catch (const std::exception &e) {
					this->nonfatal_error (e.what ());
					return;
				}

				if ((data_length + 3) != sze) {
					this->nonfatal_error ("Message length mismatch"sv);
					return;
				}

				if (data_length > this->_max_packet_size) {
					this->nonfatal_error ("Message too long"sv);
					return;
				}

				this->_curdata->alloc (data_length);
				this->_curdata->set_size (data_length);

				::memcpy (this->_curdata->buffer, buffer + offset + 3, data_length);

				this->push ();
			}

		public:
			SeqPacketDecodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				DecodeSPSC<T> (max_packet_size, num_packets)
			{ }
	};
}

#endif // HEAD_shaga_DecodeSPSC

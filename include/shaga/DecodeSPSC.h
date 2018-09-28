/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_DecodeSPSC
#define HEAD_shaga_DecodeSPSC

#include "common.h"

namespace shaga {
#define RING(x) { if ((x) >= this->_num_packets) { (x) = 0; } }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  DecodeSPSC  /////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class DecodeSPSC
	{
		private:
			#ifdef SHAGA_THREADING
				std::atomic<int> _err_count {0};
			#else
				int _err_count {0};
			#endif // SHAGA_THREADING

			std::weak_ptr<StringSPSC> _err_spsc;
			bool _throw_at_error {false};

			#ifdef OS_LINUX
			int _eventfd {-1};
			SHARED_SOCKET _event_sock;
			#endif // OS_LINUX

			uint64_t _eventfd_write_val {1};
			uint64_t _eventfd_read_val {0};

		protected:
			const uint_fast32_t _max_packet_size;
			const uint_fast32_t _num_packets;
			std::string _name;

			typedef T value_type;
			std::vector<std::unique_ptr<value_type>> _data;
			value_type* _curdata;

			#ifdef SHAGA_THREADING
				std::atomic<uint_fast32_t> _pos_read {0};
				std::atomic<uint_fast32_t> _pos_write {0};
			#else
				uint_fast32_t _pos_read {0};
				uint_fast32_t _pos_write {0};
			#endif // SHAGA_THREADING

			virtual bool read_eventfd (void) final
			{
				#ifdef OS_LINUX
				const ssize_t sze = ::read (this->_eventfd, &_eventfd_read_val, sizeof (_eventfd_read_val));
				if (sze < 0) {
					if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
						return false;
					}
					cThrow ("%s: Error reading from notice eventfd: %s", this->_name.c_str (), strerror (errno));
				}
				#endif // OS_LINUX

				return true;
			}

			virtual void nonfatal_error (const char *buf) final
			{
				#ifdef SHAGA_THREADING
					_err_count.fetch_add (1);
				#else
					_err_count++;
				#endif // SHAGA_THREADING

				if (true == _throw_at_error) {
					cThrow ("%s: %s", _name.c_str (), buf);
				}

				try {
					auto ptr = _err_spsc.lock ();
					if (nullptr != ptr) {
						ptr->push_back (_name + ": " + std::string (buf));
					}
				}
				catch (...) {
					/* Ignore the exception at this point */
				}
			}

			virtual void push (void) final
			{
				#ifdef SHAGA_THREADING
					uint_fast32_t next = _pos_write.load (std::memory_order_relaxed) + 1;
					RING (next);

					if (next == _pos_read.load (std::memory_order_acquire)) {
						cThrow ("%s: Ring full", _name.c_str ());
					}

					_pos_write.store (next, std::memory_order_release);
				#else
					uint_fast32_t next = _pos_write + 1;
					RING (next);

					if (next == _pos_read) {
						cThrow ("%s: Ring full", _name.c_str ());
					}

					_pos_write = next;
				#endif // SHAGA_THREADING

				#ifdef OS_LINUX
				if (::write (_eventfd, &_eventfd_write_val, sizeof (_eventfd_write_val)) < 0) {
					cThrow ("%s: Error writing to eventfd: %s", _name.c_str (), strerror (errno));
				}
				#endif // OS_LINUX

				_curdata = _data[next].get ();
			}

		public:
			DecodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				_max_packet_size (max_packet_size),
				_num_packets (num_packets)
			{
				if (_num_packets < 2) {
					cThrow ("%s: Ring size must be at least 2", _name.c_str ());
				}

				_data.reserve (_num_packets);
				for (uint_fast32_t i = 0; i < num_packets; i++) {
					_data.push_back (std::make_unique<T> (_max_packet_size));
				}
				_curdata = _data.front ().get ();

				#ifdef OS_LINUX
				/* This eventfd will work as a SEMAPHORE, so every push will increase counter by one and every read will decrease it. */
				_eventfd = eventfd (0, EFD_NONBLOCK | EFD_SEMAPHORE);
				if (_eventfd < 0) {
					cThrow ("%s: Unable to init eventfd: %s", _name.c_str (), strerror (errno));
				}
				_event_sock = std::make_shared<ShSocket> (_eventfd);
				#endif // OS_LINUX

				_name.assign (typeid (*this).name ());
			}

			virtual ~DecodeSPSC ()
			{
				#ifdef OS_LINUX
				_eventfd = -1;
				_event_sock.reset ();
				#endif // OS_LINUX

				_curdata = nullptr;
				_data.clear ();
			}

			/* Disable copy and assignment */
			DecodeSPSC (const DecodeSPSC &) = delete;
			DecodeSPSC& operator= (const DecodeSPSC &) = delete;

			virtual void set_name (const std::string &name) final
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

			virtual int get_err_count (void) const final
			{
				#ifdef SHAGA_THREADING
					return _err_count.load ();
				#else
					return _err_count;
				#endif // SHAGA_THREADING
			}

			virtual int get_err_count_reset (void) final
			{
				#ifdef SHAGA_THREADING
					return _err_count.exchange (0);
				#else
					return std::exchange (_err_count, 0);
				#endif // SHAGA_THREADING
			}

			#ifdef OS_LINUX
			virtual int get_eventfd (void) const final
			{
				return _eventfd;
			}

			virtual SHARED_SOCKET get_event_socket (void) final
			{
				return _event_sock;
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
						cThrow ("%s: Internal error: eventfd test passed but atomic position didn't", this->_name.c_str ());
						#endif // OS_LINUX
						return false;
					}
				#else
					const uint_fast32_t now = this->_pos_read;
					if (now == this->_pos_write) {
						#ifdef OS_LINUX
						cThrow ("%s: Internal error: eventfd test passed but read/write position didn't", this->_name.c_str ());
						#endif // OS_LINUX
						return false;
					}
				#endif // SHAGA_THREADING

				out.assign (reinterpret_cast<const char *> (this->_data[now]->buffer), this->_data[now]->size ());
				this->_data[now]->free ();

				uint_fast32_t next = now + 1;
				RING (next);

				#ifdef SHAGA_THREADING
					this->_pos_read.store (next, std::memory_order_release);
				#else
					this->_pos_read = next;
				#endif // SHAGA_THREADING

				return true;
			}

			virtual void push_buffer (const uint8_t *buffer, uint_fast32_t offset, const uint_fast32_t len) = 0;
			virtual void push_buffer (const std::string &buffer) = 0;
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Uart8DecodeSPSC  ////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class Uart8DecodeSPSC : public DecodeSPSC<T>
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
						this->nonfatal_error ("CRC mismatch");
						return false;
					}
				}

				return true;
			}

		public:
			Uart8DecodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets, const uint8_t stx, const uint8_t etx, const uint8_t ntx) :
				DecodeSPSC<T> (max_packet_size + 1, num_packets),
				_stx (stx),
				_etx (etx),
				_ntx (ntx)
			{ }

			virtual void has_crc8 (const bool enabled) final
			{
				_has_crc8 = enabled;
			}

			virtual void push_buffer (const uint8_t *buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				for (;offset < len; ++offset) {
					if (_stx == buffer[offset]) {
						if (true == _got_stx) {
							this->nonfatal_error ("STX without ETX");
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
							this->nonfatal_error ("ETX instantly after NTX");
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
							this->nonfatal_error ("NTX instantly after NTX");
							_got_stx = false;
						}
						else {
							_escape_next_char = true;
						}
					}
					else {
						if (true == _has_crc8 && this->_curdata->size () > 0) {
							_crc8_val = _crc8_dallas_table[this->_curdata->buffer[this->_curdata->size () - 1] ^ _crc8_val];
						}

						if (true == _escape_next_char) {
							_escape_next_char = false;
							this->_curdata->buffer[this->_curdata->inc_size ()] = buffer[offset] & (~0x80);
						}
						else {
							this->_curdata->buffer[this->_curdata->inc_size ()] = buffer[offset];
						}
					}
				}
			}

			virtual void push_buffer (const std::string &buffer) override final
			{
				this->push_buffer (reinterpret_cast<const uint8_t *> (buffer.data ()), 0, buffer.size ());
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Uart16DecodeSPSC  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class Uart16DecodeSPSC : public DecodeSPSC<T>
	{
		private:
			const std::array<uint8_t,2> _stx;

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
							this->nonfatal_error ("CRC-16 mismatch");
							return false;
						}
					}

					return true;
				}
				catch (...) {
					this->nonfatal_error ("Check failed");
					return false;
				}
			}

		public:
			Uart16DecodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets, const std::array<uint8_t,2> stx) :
				DecodeSPSC<T> (max_packet_size + 2, num_packets),
				_stx (stx)
			{ }

			virtual void has_crc16 (const bool enabled, const bool include_stx = false, const uint_fast16_t startval = UINT16_MAX) final
			{
				_has_crc16 = enabled;

				if (false == include_stx) {
					_crc16_startval = startval;
				}
				else {
					uint_fast16_t crc16_val = startval;
					crc16_val = (crc16_val >> 8) ^ _crc16_modbus_table[(crc16_val ^ static_cast<uint_fast16_t> (_stx[0])) & 0xff];
					crc16_val = (crc16_val >> 8) ^ _crc16_modbus_table[(crc16_val ^ static_cast<uint_fast16_t> (_stx[1])) & 0xff];
					_crc16_startval = crc16_val;
				}
			}

			virtual void push_buffer (const uint8_t *buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				for (;offset < len; ++offset) {
					if (0 == _got_stx) {
						if (_stx[0] == buffer[offset]) {
							_got_stx = 1;
						}
					}
					else if (1 == _got_stx) {
						if (_stx[1] == buffer[offset]) {
							_got_stx = 2;
						}
						else {
							/* Expected second STX character, return to the beginning */
							_got_stx = 0;
						}
					}
					else if (2 == _got_stx) {
						/* Low length byte */
						_remaining_len = buffer[offset];
						_got_stx = 3;

						_crc16_val = _crc16_startval;
						_crc16_val = (_crc16_val >> 8) ^ _crc16_modbus_table[(_crc16_val ^ static_cast<uint_fast16_t> (buffer[offset])) & 0xff];
					}
					else if (3 == _got_stx) {
						/* High length byte */
						_remaining_len |= (buffer[offset] << 8);

						if (_remaining_len > this->_max_packet_size) {
							this->nonfatal_error ("Packet size larger than allocated size, skipping...");
							_got_stx = 0;
							_remaining_len = UINT32_MAX;
						}
						else {
							_got_stx = 4;

							_crc16_val = (_crc16_val >> 8) ^ _crc16_modbus_table[(_crc16_val ^ static_cast<uint_fast16_t> (buffer[offset])) & 0xff];

							this->_curdata->zero_size ();
							this->_curdata->alloc (_remaining_len);
						}
					}
					else {
						/* If we have at least 2 bytes already, use it for crc calculation. */
						if (true == _has_crc16) {
							_crc16_val = (_crc16_val >> 8) ^ _crc16_modbus_table[(_crc16_val ^ static_cast<uint_fast16_t> (buffer[offset])) & 0xff];
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

			virtual void push_buffer (const std::string &buffer) override final
			{
				this->push_buffer (reinterpret_cast<const uint8_t *> (buffer.data ()), 0, buffer.size ());
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  PacketDecodeSPSC  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class PacketDecodeSPSC : public DecodeSPSC<T>
	{
		private:
			const char _magic[2] {5, 23};
			const uint_fast32_t _crc_max_len {16};
			const uint8_t _crc_start_val {19};

			uint8_t _crc8_val {0};
			uint8_t _crc8_expected {0};

			uint_fast32_t _remaining_data {0};
			uint_fast32_t _remaining_crc {0};
			uint_fast32_t _got_header {0};

			std::string _size_buf;

		public:
			PacketDecodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				DecodeSPSC<T> (max_packet_size, num_packets)
			{ }

			virtual void push_buffer (const uint8_t *buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				for (;offset < len; ++offset) {
					if (_remaining_crc > 0) {
						_crc8_val = _crc8_dallas_table[buffer[offset] ^ _crc8_val];
						--_remaining_crc;

						if (0 == _remaining_crc && _crc8_val != _crc8_expected) {
							this->nonfatal_error ("CRC mismatch");
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
								this->nonfatal_error ("CRC mismatch");
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
						cThrow ("Fell over the edge of the world");
					}
				}
			}

			virtual void push_buffer (const std::string &buffer) override final
			{
				this->push_buffer (reinterpret_cast<const uint8_t *> (buffer.data ()), 0, buffer.size ());
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  SeqPacketDecodeSPSC  ////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class SeqPacketDecodeSPSC : public DecodeSPSC<T>
	{
		public:
			SeqPacketDecodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				DecodeSPSC<T> (max_packet_size, num_packets)
			{ }

			virtual void push_buffer (const uint8_t *buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				const uint_fast32_t sze = len - offset;

				if (sze < 3) {
					this->nonfatal_error ("Message too short");
					return;
				}

				uint_fast32_t data_length = 0;

				try {
					const std::string tbuf (reinterpret_cast <const char *> (buffer + offset), 3);
					data_length = BIN::to_uint24 (tbuf);
				}
				catch (const std::exception &e) {
					this->nonfatal_error (e.what ());
					return;
				}

				if ((data_length + 3) != sze) {
					this->nonfatal_error ("Message length mismatch");
					return;
				}

				if (data_length > this->_max_packet_size) {
					this->nonfatal_error ("Message too long");
					return;
				}

				this->_curdata->alloc (data_length);
				this->_curdata->set_size (data_length);

				::memcpy (this->_curdata->buffer, buffer + offset + 3, data_length);

				this->push ();
			}

			virtual void push_buffer (const std::string &buffer) override final
			{
				this->push_buffer (reinterpret_cast<const uint8_t *> (buffer.data ()), 0, buffer.size ());
			}
	};

#undef RING
}

#endif // HEAD_shaga_DecodeSPSC

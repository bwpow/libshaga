/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_EncodeSPSC
#define HEAD_shaga_EncodeSPSC

#include "common.h"

namespace shaga {
#define RING(x) { if ((x) >= this->_num_packets) { (x) = 0; } }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  EncodeSPSC  /////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class EncodeSPSC
	{
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
				std::atomic<uint_fast32_t> _stored_bytes {0};
			#else
				volatile uint_fast32_t _pos_read {0};
				volatile uint_fast32_t _pos_write {0};
				volatile uint_fast32_t _stored_bytes {0};
			#endif // SHAGA_THREADING

			virtual void push (void) final
			{
				#ifdef SHAGA_THREADING
					uint_fast32_t next = _pos_write.load (std::memory_order::memory_order_relaxed) + 1;
					RING (next);

					if (next == _pos_read.load (std::memory_order::memory_order_acquire)) {
						cThrow ("{}: Ring full"sv, _name);
					}

					_pos_write.store (next, std::memory_order::memory_order_relaxed);
					_stored_bytes.fetch_add (_curdata->size (), std::memory_order::memory_order_seq_cst);
				#else
					uint_fast32_t next = _pos_write + 1;
					RING (next);

					if (next == _pos_read) {
						cThrow ("{}: Ring full"sv, _name);
					}

					_pos_write = next;
					_stored_bytes += _curdata->size ();
				#endif // SHAGA_THREADING

				_curdata = _data[next].get ();
			}

			virtual void _push_buffer (const uint8_t *const buffer, uint_fast32_t offset, const uint_fast32_t len) = 0;

		public:
			EncodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				_max_packet_size (max_packet_size),
				_num_packets (num_packets)
			{
				this->_name.assign (typeid (*this).name ());

				if (_num_packets < 2) {
					cThrow ("{}: Ring size must be at least 2"sv, _name);
				}

				this->_data.reserve (_num_packets);
				for (uint_fast32_t i = 0; i < _num_packets; i++) {
					this->_data.push_back (std::make_unique<T> (_max_packet_size));
				}
				this->_curdata = this->_data.front ().get ();
			}

			virtual ~EncodeSPSC ()
			{
				this->_curdata = nullptr;
				this->_data.clear ();
			}

			/* Disable copy and assignment */
			EncodeSPSC (const EncodeSPSC &) = delete;
			EncodeSPSC& operator= (const EncodeSPSC &) = delete;

			virtual void set_name (const std::string_view name) final
			{
				this->_name.assign (name);
			}

			virtual uint_fast32_t get_stored_bytes (void) const final
			{
				#ifdef SHAGA_THREADING
					return this->_stored_bytes.load (std::memory_order_relaxed);
				#else
					return this->_stored_bytes;
				#endif // SHAGA_THREADING
			}

			virtual bool empty (void) const final
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t now_read = this->_pos_read.load (std::memory_order_relaxed);
					const uint_fast32_t now_write = this->_pos_write.load (std::memory_order_acquire);
				#else
					const uint_fast32_t now_read = this->_pos_read;
					const uint_fast32_t now_write = this->_pos_write;
				#endif // SHAGA_THREADING

				return (now_read == now_write);
			}

			virtual void clear (void) = 0;
			virtual uint_fast32_t fill_front_buffer (char *outbuffer, uint_fast32_t len) = 0;
			virtual void move_front_buffer (uint_fast32_t len) = 0;

			virtual void push_buffer (const uint8_t *const buffer, uint_fast32_t offset, const uint_fast32_t len) final
			{
				this->_push_buffer (buffer, offset, len);
			}

			virtual void push_buffer (const std::string_view buffer) final
			{
				this->_push_buffer (reinterpret_cast<const uint8_t *> (buffer.data ()), 0, buffer.size ());
			}

			virtual void push_buffer (const uint8_t *const buffer, const uint_fast32_t len) final
			{
				this->_push_buffer (buffer, 0, len);
			}

			virtual void push_buffer (const char *const buffer, uint_fast32_t offset, const uint_fast32_t len) final
			{
				this->_push_buffer (reinterpret_cast<const uint8_t *> (buffer), offset, len);
			}

			virtual void push_buffer (const char *const buffer, const uint_fast32_t len) final
			{
				this->_push_buffer (reinterpret_cast<const uint8_t *> (buffer), 0, len);
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  ContStreamEncodeSPSC  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class ContStreamEncodeSPSC : public EncodeSPSC<T>
	{
		private:
			uint_fast32_t _read_offset {0};

		public:
			ContStreamEncodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				EncodeSPSC<T> (max_packet_size, num_packets)
			{ }

			virtual void clear (void) override final
			{
				#ifdef SHAGA_THREADING
					uint_fast32_t now_read = this->_pos_read.load (std::memory_order_relaxed);
					const uint_fast32_t now_write = this->_pos_write.load (std::memory_order_acquire);
				#else
					uint_fast32_t now_read = this->_pos_read;
					const uint_fast32_t now_write = this->_pos_write;
				#endif // SHAGA_THREADING

				while (now_read != now_write) {
					this->_data[now_read]->free ();
					++now_read;
					RING (now_read);
				}

				_read_offset = 0;
				#ifdef SHAGA_THREADING
					this->_pos_read.store (now_read, std::memory_order::memory_order_relaxed);
					this->_stored_bytes.store (0, std::memory_order::memory_order_release);
				#else
					this->_pos_read = now_read;
					this->_stored_bytes = 0;
				#endif // SHAGA_THREADING
			}

			virtual uint_fast32_t fill_front_buffer (char *outbuffer, uint_fast32_t len) override final
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t stored_bytes = this->_stored_bytes.load (std::memory_order_relaxed);

					if (len > stored_bytes) {
						len = stored_bytes;
					}
				#else
					if (len > this->_stored_bytes) {
						len = this->_stored_bytes;
					}
				#endif // SHAGA_THREADING

				if (0 == len) {
					return 0;
				}

				#ifdef SHAGA_THREADING
					uint_fast32_t now_read = this->_pos_read.load (std::memory_order_relaxed);
					const uint_fast32_t now_write = this->_pos_write.load (std::memory_order_acquire);
				#else
					uint_fast32_t now_read = this->_pos_read;
					const uint_fast32_t now_write = this->_pos_write;
				#endif // SHAGA_THREADING

				if (now_write == now_read) {
					return 0;
				}

				uint_fast32_t read_offset = _read_offset;
				uint_fast32_t remaining;
				uint_fast32_t offset = 0;

				while (now_read != now_write) {
					remaining = this->_data[now_read]->size () - read_offset;
					if (remaining >= len) {
						/* We got enought data */

						::memcpy (outbuffer + offset, this->_data[now_read]->buffer + read_offset, len);
						offset += len;
						break;
					}
					else {
						::memcpy (outbuffer + offset, this->_data[now_read]->buffer + read_offset, remaining);
						offset += remaining;
						len -= remaining;

						++now_read;
						RING (now_read);
						read_offset = 0;
					}
				}

				return offset;
			}

			virtual void move_front_buffer (uint_fast32_t len) override final
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t stored_bytes = this->_stored_bytes.load (std::memory_order_relaxed);

					if (len > stored_bytes) {
						cThrow ("{}: Unable to move front buffer. Destination too far."sv, this->_name);
					}
				#else
					if (len > this->_stored_bytes) {
						cThrow ("{}: Unable to move front buffer. Destination too far."sv, this->_name);
					}
				#endif // SHAGA_THREADING

				if (0 == len) {
					return;
				}

				#ifdef SHAGA_THREADING
					uint_fast32_t now_read = this->_pos_read.load (std::memory_order_relaxed);
					const uint_fast32_t now_write = this->_pos_write.load (std::memory_order_acquire);
				#else
					uint_fast32_t now_read = this->_pos_read;
					const uint_fast32_t now_write = this->_pos_write;
				#endif // SHAGA_THREADING

				uint_fast32_t read_offset = _read_offset;
				uint_fast32_t remaining;
				const uint_fast32_t orig_len = len;

				while (true) {
					if (now_read == now_write) {
						cThrow ("{}: Unable to move front buffer. Destination too far."sv, this->_name);
					}

					remaining = this->_data[now_read]->size () - read_offset;
					if (remaining == len) {
						this->_data[now_read]->free ();
						++now_read;
						RING (now_read);
						read_offset = 0;
						break;
					}
					else if (remaining > len) {
						/* We got enought data */
						read_offset += len;
						break;
					}
					else {
						len -= remaining;
						this->_data[now_read]->free ();
						++now_read;
						RING (now_read);
						read_offset = 0;
					}
				}

				_read_offset = read_offset;
				#ifdef SHAGA_THREADING
					this->_pos_read.store (now_read, std::memory_order::memory_order_relaxed);
					this->_stored_bytes.fetch_sub (orig_len, std::memory_order::memory_order_seq_cst);
				#else
					this->_pos_read = now_read;
					this->_stored_bytes -= orig_len;
				#endif // SHAGA_THREADING
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  SeqStreamEncodeSPSC  ////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class SeqStreamEncodeSPSC : public EncodeSPSC<T>
	{
		public:
			SeqStreamEncodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				EncodeSPSC<T> (max_packet_size, num_packets)
			{ }

			virtual void clear (void) override final
			{
				#ifdef SHAGA_THREADING
					uint_fast32_t now_read = this->_pos_read.load (std::memory_order_relaxed);
					const uint_fast32_t now_write = this->_pos_write.load (std::memory_order_acquire);
				#else
					uint_fast32_t now_read = this->_pos_read;
					const uint_fast32_t now_write = this->_pos_write;
				#endif // SHAGA_THREADING

				while (now_read != now_write) {
					this->_data[now_read]->free ();
					++now_read;
					RING (now_read);
				}

				#ifdef SHAGA_THREADING
					this->_pos_read.store (now_read, std::memory_order::memory_order_relaxed);
					this->_stored_bytes.store (0, std::memory_order::memory_order_release);
				#else
					this->_pos_read = now_read;
					this->_stored_bytes = 0;
				#endif // SHAGA_THREADING
			}

			virtual uint_fast32_t fill_front_buffer (char *outbuffer, uint_fast32_t len) override final
			{
				if (0 == len) {
					return 0;
				}

				#ifdef SHAGA_THREADING
					uint_fast32_t now_read = this->_pos_read.load (std::memory_order::memory_order_relaxed);
					const uint_fast32_t now_write = this->_pos_write.load (std::memory_order::memory_order_acquire);
				#else
					uint_fast32_t now_read = this->_pos_read;
					const uint_fast32_t now_write = this->_pos_write;
				#endif // SHAGA_THREADING

				if (now_write == now_read) {
					return 0;
				}

				if (len < this->_data[now_read]->size ()) {
					cThrow ("Supplied buffer too short for data"sv);
				}

				::memcpy (outbuffer, this->_data[now_read]->buffer, this->_data[now_read]->size ());

				return this->_data[now_read]->size ();
			}

			virtual void move_front_buffer (uint_fast32_t len) override final
			{
				if (0 == len) {
					return;
				}

				#ifdef SHAGA_THREADING
					uint_fast32_t now_read = this->_pos_read.load (std::memory_order::memory_order_relaxed);
					const uint_fast32_t now_write = this->_pos_write.load (std::memory_order::memory_order_acquire);
				#else
					uint_fast32_t now_read = this->_pos_read;
					const uint_fast32_t now_write = this->_pos_write;
				#endif // SHAGA_THREADING

				if (now_read == now_write) {
					cThrow ("{}: Unable to move front buffer. Destination too far."sv, this->_name);
				}

				if (this->_data[now_read]->size () != len) {
					cThrow ("{}: Supplied wrong move length."sv, this->_name);
				}

				this->_data[now_read]->free ();
				++now_read;
				RING (now_read);

				#ifdef SHAGA_THREADING
					this->_pos_read.store (now_read, std::memory_order::memory_order_relaxed);
					this->_stored_bytes.fetch_sub (len, std::memory_order::memory_order_seq_cst);
				#else
					this->_pos_read = now_read;
					this->_stored_bytes -= len;
				#endif // SHAGA_THREADING
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Uart8EncodeSPSC  ////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	class Uart8EncodeSPSC : public ContStreamEncodeSPSC<T>
	{
		private:
			const uint8_t _stx;
			const uint8_t _etx;
			const uint8_t _ntx;

			bool _has_crc8 {false};

			virtual void _push_buffer (const uint8_t *const buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				uint8_t crc8_val = _stx;

				this->_curdata->alloc ();
				this->_curdata->zero_size ();
				this->_curdata->buffer[this->_curdata->inc_size ()] = _stx;

				for (;offset < len; ++offset) {
					crc8_val = CRC::_crc8_dallas_table[buffer[offset] ^ crc8_val];

					if (_stx == buffer[offset] || _etx == buffer[offset] || _ntx == buffer[offset]) {
						this->_curdata->buffer[this->_curdata->inc_size ()] = _ntx;
						this->_curdata->buffer[this->_curdata->inc_size ()] = buffer[offset] | 0x80;
					}
					else {
						this->_curdata->buffer[this->_curdata->inc_size ()] = buffer[offset];
					}
				}

				if (true == _has_crc8) {
					if (_stx == crc8_val || _etx == crc8_val || _ntx == crc8_val) {
						this->_curdata->buffer[this->_curdata->inc_size ()] = _ntx;
						this->_curdata->buffer[this->_curdata->inc_size ()] = crc8_val | 0x80;
					}
					else {
						this->_curdata->buffer[this->_curdata->inc_size ()] = crc8_val;
					}
				}

				this->_curdata->buffer[this->_curdata->inc_size ()] = _etx;

				this->push ();
			}

		public:
			/* Result can have every byte escaped (max_packet_size * 2) plus STX, ETX and escaped CRC (+4) */
			Uart8EncodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets, const uint8_t stx, const uint8_t etx, const uint8_t ntx) :
				ContStreamEncodeSPSC<T> ((max_packet_size * 2) + 4, num_packets),
				_stx (stx),
				_etx (etx),
				_ntx (ntx)
			{ }

			virtual void has_crc8 (const bool enabled)
			{
				_has_crc8 = enabled;
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Uart16EncodeSPSC  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	class Uart16EncodeSPSC : public ContStreamEncodeSPSC<T>
	{
		private:
			const std::array<uint8_t,2> _stx;

			bool _has_crc16 {false};
			uint_fast16_t _crc16_startval {0};

			virtual void _push_buffer (const uint8_t *const buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				/* STX is not part of CRC-16 */
				uint_fast16_t crc16_val = _crc16_startval;

				const uint_fast32_t real_len = (len - offset) + (_has_crc16 ? 2 : 0);

				this->_curdata->zero_size ();
				this->_curdata->alloc (real_len + 4);

				/* STX two bytes */
				this->_curdata->buffer[this->_curdata->inc_size ()] = _stx[0];
				this->_curdata->buffer[this->_curdata->inc_size ()] = _stx[1];

				/* Length encoded in little endian */
				this->_curdata->buffer[this->_curdata->inc_size ()] = real_len & 0xff;
				this->_curdata->buffer[this->_curdata->inc_size ()] = (real_len >> 8) & 0xff;

				if (true == _has_crc16) {
					crc16_val = (crc16_val >> 8) ^ CRC::_crc16_modbus_table[(crc16_val ^ static_cast<uint_fast16_t> (this->_curdata->buffer[this->_curdata->size () - 2])) & 0xff];
					crc16_val = (crc16_val >> 8) ^ CRC::_crc16_modbus_table[(crc16_val ^ static_cast<uint_fast16_t> (this->_curdata->buffer[this->_curdata->size () - 1])) & 0xff];
				}

				for (;offset < len; ++offset) {
					if (true == _has_crc16) {
						crc16_val = (crc16_val >> 8) ^ CRC::_crc16_modbus_table[(crc16_val ^ static_cast<uint_fast16_t> (buffer[offset])) & 0xff];
					}

					this->_curdata->buffer[this->_curdata->inc_size ()] = buffer[offset];
				}

				if (true == _has_crc16) {
					/* Always little endian */
					this->_curdata->buffer[this->_curdata->inc_size ()] = crc16_val & 0xff;
					this->_curdata->buffer[this->_curdata->inc_size ()] = (crc16_val >> 8) & 0xff;
				}

				this->push ();
			}

		public:
			/* Result can have max_packet_size plus STX (2 bytes) + LEN (2 bytes) + CRC (2 bytes) */
			Uart16EncodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets, const std::array<uint8_t,2> stx) :
				ContStreamEncodeSPSC<T> (max_packet_size + 6, num_packets),
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
					crc16_val = (crc16_val >> 8) ^ CRC::_crc16_modbus_table[(crc16_val ^ static_cast<uint_fast16_t> (_stx[0])) & 0xff];
					crc16_val = (crc16_val >> 8) ^ CRC::_crc16_modbus_table[(crc16_val ^ static_cast<uint_fast16_t> (_stx[1])) & 0xff];
					_crc16_startval = crc16_val;
				}
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  PacketEncodeSPSC  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class PacketEncodeSPSC : public ContStreamEncodeSPSC<T>
	{
		private:
			static const constexpr char _magic[2] {5, 23};
			static const constexpr uint_fast32_t _crc_max_len {16};
			static const constexpr uint8_t _crc_start_val {19};
			static const constexpr size_t _header_size {6};

			std::string header;

			virtual void _push_buffer (const uint8_t *const buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				const uint_fast32_t sze = len - offset;

				if (0 == sze) {
					return;
				}

				header.resize (0);
				header.append (_magic, sizeof (_magic));

				/* CRC will be placed here */
				BIN::from_uint8 (0, header);

				/* Data length */
				BIN::from_uint24 (sze, header);

				if (_header_size != header.size ()) {
					cThrow ("Header has incorrect size"sv);
				}

				if ((sze + _header_size) > this->_max_packet_size) {
					cThrow ("Buffer too long"sv);
				}

				this->_curdata->alloc (sze + _header_size);
				this->_curdata->set_size (sze + _header_size);

				::memcpy (this->_curdata->buffer, header.data (), _header_size);
				::memcpy (this->_curdata->buffer + _header_size, buffer + offset, sze);

				this->_curdata->buffer[2] = CRC::crc8_dallas (reinterpret_cast<const char *> (this->_curdata->buffer + 3), std::min (this->_curdata->size () - 3, _crc_max_len), _crc_start_val);

				this->push ();
			}

		public:
			/* 16-bit magic + 24-bit size + 8-bit crc + data */
			PacketEncodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				ContStreamEncodeSPSC<T> (max_packet_size + 6, num_packets)
			{
				header.reserve (_header_size);
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  SeqPacketEncodeSPSC  ////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class SeqPacketEncodeSPSC : public SeqStreamEncodeSPSC<T>
	{
		private:
			static const constexpr size_t _header_size {3};
			std::string header;

			virtual void _push_buffer (const uint8_t *const buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				const uint_fast32_t sze = len - offset;

				if (0 == sze) {
					return;
				}

				header.resize (0);
				BIN::from_uint24 (sze, header);

				if (_header_size != header.size ()) {
					cThrow ("Header has incorrect size"sv);
				}

				if ((sze + _header_size) > this->_max_packet_size) {
					cThrow ("Buffer too long"sv);
				}

				this->_curdata->alloc (sze + _header_size);
				this->_curdata->set_size (sze + _header_size);

				::memcpy (this->_curdata->buffer, header.data (), _header_size);
				::memcpy (this->_curdata->buffer + _header_size, buffer + offset, sze);

				this->push ();
			}

		public:
			/* Result can have max_packet_size plus 3 bytes to store size (24-bit) */
			SeqPacketEncodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				SeqStreamEncodeSPSC<T> (max_packet_size + 3, num_packets)
			{
				header.reserve (_header_size);
			}
	};

#undef RING
}

#endif // HEAD_shaga_EncodeSPSC

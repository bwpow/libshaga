/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

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
		private:
			int _eventfd {-1};
			uint64_t _eventfd_write_val {0};
			uint64_t _eventfd_read_val {0};

		protected:
			const uint_fast32_t _max_packet_size;
			const uint_fast32_t _num_packets;
			std::string _name;

			typedef T value_type;

			std::vector<std::unique_ptr<value_type>> _data;
			value_type* _curdata;

			std::atomic<uint_fast32_t> _pos_read {0};
			std::atomic<uint_fast32_t> _pos_write {0};

			virtual uint_fast32_t read_eventfd (void) final
			{
				_eventfd_read_val = 0;

				#ifdef OS_LINUX
				const ssize_t sze = ::read (_eventfd, &_eventfd_read_val, sizeof (_eventfd_read_val));
				if (sze < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
					cThrow ("%s: Error reading from notice eventfd: %s", this->_name.c_str (), strerror (errno));
				}
				#endif // OS_LINUX

				return static_cast<uint_fast32_t> (_eventfd_read_val);
			}

			virtual void push (void) final
			{
				uint_fast32_t next = _pos_write.load (std::memory_order_relaxed) + 1;
				RING (next);

				if (next == _pos_read.load (std::memory_order_acquire)) {
					cThrow ("%s: Ring full", _name.c_str ());
				}

				_pos_write.store (next, std::memory_order_release);

				#ifdef OS_LINUX
				_eventfd_write_val = _curdata->size ();
				if (::write (_eventfd, &_eventfd_write_val, sizeof (_eventfd_write_val)) < 0) {
					cThrow ("%s: Error writing to eventfd: %s", _name.c_str (), strerror (errno));
				}
				#endif // OS_LINUX

				_curdata = _data[next].get ();
			}

		public:
			EncodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
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
				_eventfd = eventfd (0, EFD_NONBLOCK);
				if (_eventfd < 0) {
					cThrow ("%s: Unable to init eventfd: %s", _name.c_str (), strerror (errno));
				}
				#endif // OS_LINUX

				_name.assign (typeid (*this).name ());
			}

			virtual ~EncodeSPSC ()
			{
				if (_eventfd >= 0) {
					::close (_eventfd);
					_eventfd = -1;
				}

				_curdata = nullptr;
				_data.clear ();
			}

			/* Disable copy and assignment */
			EncodeSPSC (const EncodeSPSC &) = delete;
			EncodeSPSC& operator= (const EncodeSPSC &) = delete;

			virtual void set_name (const std::string &name) final
			{
				_name.assign (name);
			}

			virtual int get_eventfd (void) const final
			{
				return _eventfd;
			}

			virtual bool empty (void) const final
			{
				const uint_fast32_t now_read = _pos_read.load (std::memory_order_relaxed);
				const uint_fast32_t now_write = _pos_write.load (std::memory_order_acquire);
				return (now_read == now_write);
			}

			virtual void clear (void) = 0;
			virtual uint_fast32_t fill_front_buffer (char *outbuffer, uint_fast32_t len) = 0;
			virtual void move_front_buffer (uint_fast32_t len) = 0;

			virtual void push_buffer (const uint8_t *buffer, uint_fast32_t offset, const uint_fast32_t len) = 0;
			virtual void push_buffer (const std::string &buffer) = 0;
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  ContStreamEncodeSPSC  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class ContStreamEncodeSPSC : public EncodeSPSC<T>
	{
		private:
			uint_fast32_t _read_offset {0};
			uint_fast32_t _remaining_total {0};

		public:
			ContStreamEncodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				EncodeSPSC<T> (max_packet_size, num_packets)
			{ }

			virtual void clear (void) override final
			{
				#ifdef OS_LINUX
					this->read_eventfd ();
					_remaining_total = 0;
				#endif // OS_LINUX

				uint_fast32_t now_read = this->_pos_read.load (std::memory_order_relaxed);
				const uint_fast32_t now_write = this->_pos_write.load (std::memory_order_acquire);

				while (now_read != now_write) {
					this->_data[now_read]->free ();
					++now_read;
					RING (now_read);
				}

				_read_offset = 0;
				this->_pos_read.store (now_read, std::memory_order_release);
			}

			virtual uint_fast32_t fill_front_buffer (char *outbuffer, uint_fast32_t len) override final
			{
				#ifdef OS_LINUX
					_remaining_total += this->read_eventfd ();

					if (len > _remaining_total) {
						len = _remaining_total;
					}
				#endif // OS_LINUX

				if (0 == len) {
					return 0;
				}

				uint_fast32_t now_read = this->_pos_read.load (std::memory_order_relaxed);
				const uint_fast32_t now_write = this->_pos_write.load (std::memory_order_acquire);
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
				#ifdef OS_LINUX
					_remaining_total += this->read_eventfd ();

					if (len > _remaining_total) {
						cThrow ("%s: Unable to move front buffer. Destination too far.", this->_name.c_str ());
					}

					_remaining_total -= len;
				#endif // OS_LINUX

				if (0 == len) {
					return;
				}

				uint_fast32_t now_read = this->_pos_read.load (std::memory_order_relaxed);
				const uint_fast32_t now_write = this->_pos_write.load (std::memory_order_acquire);

				uint_fast32_t read_offset = _read_offset;
				uint_fast32_t remaining;

				while (true) {
					if (now_read == now_write) {
						cThrow ("%s: Unable to move front buffer. Destination too far.", this->_name.c_str ());
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
				this->_pos_read.store (now_read, std::memory_order_release);
			}

			#ifdef OS_LINUX
			uint_fast32_t get_remaining_total (void) const
			{
				return _remaining_total;
			}
			#endif // OS_LINUX

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
				#ifdef OS_LINUX
				this->read_eventfd ();
				#endif // OS_LINUX

				uint_fast32_t now_read = this->_pos_read.load (std::memory_order_relaxed);
				const uint_fast32_t now_write = this->_pos_write.load (std::memory_order_acquire);

				while (now_read != now_write) {
					this->_data[now_read]->free ();
					++now_read;
					RING (now_read);
				}

				this->_pos_read.store (now_read, std::memory_order_release);
			}

			virtual uint_fast32_t fill_front_buffer (char *outbuffer, uint_fast32_t len) override final
			{
				#ifdef OS_LINUX
				this->read_eventfd ();
				#endif // OS_LINUX

				if (0 == len) {
					return 0;
				}

				uint_fast32_t now_read = this->_pos_read.load (std::memory_order_relaxed);
				const uint_fast32_t now_write = this->_pos_write.load (std::memory_order_acquire);
				if (now_write == now_read) {
					return 0;
				}

				if (len < this->_data[now_read]->size ()) {
					cThrow ("Supplied buffer too short for data");
				}

				::memcpy (outbuffer, this->_data[now_read]->buffer, this->_data[now_read]->size ());

				return this->_data[now_read]->size ();
			}

			virtual void move_front_buffer (uint_fast32_t len) override final
			{
				#ifdef OS_LINUX
				this->read_eventfd ();
				#endif // OS_LINUX

				if (0 == len) {
					return;
				}

				uint_fast32_t now_read = this->_pos_read.load (std::memory_order_relaxed);
				const uint_fast32_t now_write = this->_pos_write.load (std::memory_order_acquire);

				if (now_read == now_write) {
					cThrow ("%s: Unable to move front buffer. Destination too far.", this->_name.c_str ());
				}

				if (this->_data[now_read]->size () != len) {
					cThrow ("%s: Supplied wrong move length.", this->_name.c_str ());
				}

				this->_data[now_read]->free ();
				++now_read;
				RING (now_read);

				this->_pos_read.store (now_read, std::memory_order_release);
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  UartEncodeSPSC  /////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	class UartEncodeSPSC : public ContStreamEncodeSPSC<T>
	{
		private:
			const uint8_t _stx;
			const uint8_t _etx;
			const uint8_t _ntx;

			bool _has_crc8 {false};

		public:
			/* Result can have every byte escaped (max_packet_size * 2) plus STX, ETX and escaped CRC (+4) */
			UartEncodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets, const uint8_t stx, const uint8_t etx, const uint8_t ntx) :
				ContStreamEncodeSPSC<T> ((max_packet_size * 2) + 4, num_packets),
				_stx (stx),
				_etx (etx),
				_ntx (ntx)
			{ }

			virtual void has_crc8 (const bool enabled)
			{
				_has_crc8 = enabled;
			}

			virtual void push_buffer (const uint8_t *buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				uint8_t crc8_val = _stx;

				this->_curdata->alloc ();
				this->_curdata->zero_size ();
				this->_curdata->buffer[this->_curdata->inc_size ()] = _stx;

				for (;offset < len; ++offset) {
					crc8_val = _crc8_table[buffer[offset] ^ crc8_val];

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

			virtual void push_buffer (const std::string &buffer) override final
			{
				this->push_buffer (reinterpret_cast<const uint8_t *> (buffer.data ()), 0, buffer.size ());
			}

	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  PacketEncodeSPSC  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class PacketEncodeSPSC : public ContStreamEncodeSPSC<T>
	{
		private:
			const char _magic[2] {5, 23};
			const uint_fast32_t _crc_max_len {16};
			const uint8_t _crc_start_val {19};

			std::string tbuf;

		public:
			/* 16-bit magic + 24-bit size + 8-bit crc + data */
			PacketEncodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				ContStreamEncodeSPSC<T> (max_packet_size + 6, num_packets)
			{
				tbuf.reserve (6);
			}

			virtual void push_buffer (const uint8_t *buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				const uint_fast32_t sze = len - offset;

				if (0 == sze) {
					return;
				}

				tbuf.resize (0);
				tbuf.append (_magic, sizeof (_magic));

				/* CRC will be placed here */
				BIN::from_uint8 (0, tbuf);

				BIN::from_uint24 (sze, tbuf);

				if ((sze + tbuf.size ()) > this->_max_packet_size) {
					cThrow ("Buffer too long");
				}

				this->_curdata->alloc (sze + tbuf.size ());
				this->_curdata->set_size (sze + tbuf.size ());

				::memcpy (this->_curdata->buffer, tbuf.data (), tbuf.size ());
				::memcpy (this->_curdata->buffer + (tbuf.size ()), buffer + offset, sze);

				this->_curdata->buffer[2] = CRC::crc8 (reinterpret_cast<const char *> (this->_curdata->buffer + 3), std::min (this->_curdata->size () - 3, _crc_max_len), _crc_start_val);

				this->push ();
			}

			virtual void push_buffer (const std::string &buffer) override final
			{
				this->push_buffer (reinterpret_cast<const uint8_t *> (buffer.data ()), 0, buffer.size ());
			}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  SeqPacketEncodeSPSC  ////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T> class SeqPacketEncodeSPSC : public SeqStreamEncodeSPSC<T>
	{
		private:
			std::string tbuf;

		public:
			/* Result can have max_packet_size plus 3 bytes to store size (24-bit) */
			SeqPacketEncodeSPSC (const uint_fast32_t max_packet_size, const uint_fast32_t num_packets) :
				SeqStreamEncodeSPSC<T> (max_packet_size + 3, num_packets)
			{
				tbuf.reserve (3);
			}

			virtual void push_buffer (const uint8_t *buffer, uint_fast32_t offset, const uint_fast32_t len) override final
			{
				const uint_fast32_t sze = len - offset;

				if (0 == sze) {
					return;
				}

				tbuf.resize (0);
				BIN::from_uint24 (sze, tbuf);

				if ((sze + tbuf.size ()) > this->_max_packet_size) {
					cThrow ("Buffer too long");
				}

				this->_curdata->alloc (sze + tbuf.size ());
				this->_curdata->set_size (sze + tbuf.size ());

				::memcpy (this->_curdata->buffer, tbuf.data (), tbuf.size ());
				::memcpy (this->_curdata->buffer + (tbuf.size ()), buffer + offset, sze);

				this->push ();
			}

			virtual void push_buffer (const std::string &buffer) override final
			{
				this->push_buffer (reinterpret_cast<const uint8_t *> (buffer.data ()), 0, buffer.size ());
			}
	};

#undef RING
}

#endif // HEAD_shaga_EncodeSPSC

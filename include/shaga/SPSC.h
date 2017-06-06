/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_SPSC
#define HEAD_shaga_SPSC

#include "common.h"

namespace shaga {

#define RING(x) { if ((x) >= _size) { (x) = 0; } }

	template<class T>
	class SPSC {
		private:
			typedef T value_type;

			const uint_fast32_t _size;
			std::atomic<uint_fast32_t> _pos_read;
			std::atomic<uint_fast32_t> _pos_write;

			T* const _data;

		public:
			explicit SPSC (const uint_fast32_t sze) : _size (sze), _pos_read (0), _pos_write (0), _data (static_cast<T*> (::malloc (sizeof (T) * sze)))
			{
				if (sze < 2) {
					cThrow ("Ring size must be at least 2");
				}

				if (_data == nullptr) {
					cThrow ("Ring data allocation failed");
				}
			}

			~SPSC ()
			{
				uint_fast32_t now = _pos_read.load (std::memory_order_consume);
				const uint_fast32_t last = _pos_write.load (std::memory_order_consume);
				while (now != last) {
					_data[now++].~T();
					RING (now);
				}

				::free(_data);
			}

			/* Disable copy and assignment */
			SPSC (const SPSC &) = delete;
			SPSC& operator= (const SPSC &) = delete;

			bool is_lock_free (void) const
			{
				return _pos_read.is_lock_free () && _pos_write.is_lock_free ();
			}

			bool push_back (const T &entry)
			{
				const uint_fast32_t now = _pos_write.load (std::memory_order_relaxed);
				uint_fast32_t next = now + 1;
				RING (next);

				if (next == _pos_read.load (std::memory_order_acquire)) {
					return false;
				}

				new (&_data[now]) T(entry);
				_pos_write.store (next, std::memory_order_release);

				return true;
			}

			bool push_back (T &&entry)
			{
				const uint_fast32_t now = _pos_write.load (std::memory_order_relaxed);
				uint_fast32_t next = now + 1;
				RING (next);

				if (next == _pos_read.load (std::memory_order_acquire)) {
					return false;
				}

				new (&_data[now]) T();
				_data[now] = std::move (entry);
				_pos_write.store (next, std::memory_order_release);

				return true;
			}

			template<class ...Args>
			bool emplace_back (Args&&... args)
			{
				const uint_fast32_t now = _pos_write.load (std::memory_order_relaxed);
				uint_fast32_t next = now + 1;
				RING (next);

				if (next == _pos_read.load (std::memory_order_acquire)) {
					return false;
				}

				new (&_data[now]) T(std::forward<Args>(args)...);
				_pos_write.store (next, std::memory_order_release);

				return true;
			}

			bool pop_front (T &entry)
			{
				const uint_fast32_t now = _pos_read.load (std::memory_order_relaxed);
				if (now == _pos_write.load (std::memory_order_acquire)) {
					return false;
				}

				uint_fast32_t next = now + 1;
				RING (next);

				entry = std::move (_data[now]);
				_data[now].~T();

				_pos_read.store (next, std::memory_order_release);
				return true;
			}

			bool pop_front (void)
			{
				const uint_fast32_t now = _pos_read.load (std::memory_order_relaxed);
				if (now == _pos_write.load (std::memory_order_acquire)) {
					return false;
				}

				uint_fast32_t next = now + 1;
				RING (next);

				_data[now].~T();

				_pos_read.store (next, std::memory_order_release);
				return true;
			}

			T* front (void)
			{
				const uint_fast32_t now = _pos_read.load (std::memory_order_relaxed);
				if (now == _pos_write.load (std::memory_order_acquire)) {
					return nullptr;
				}

				return &_data[now];
			}

			void clear (void)
			{
				while (pop_front () == true);
			}

			bool empty (void) const
			{
				return _pos_read.load (std::memory_order_consume) == _pos_write.load (std::memory_order_consume);
			}

			bool full (void) const
			{
				uint_fast32_t next = _pos_write.load (std::memory_order_consume) + 1;
				RING (next);
				return (next == _pos_read.load (std::memory_order_consume));
			}
	};

	template<class T>
	class PaSPSC {
		private:
			typedef T value_type;

			const uint_fast32_t _size;
			std::atomic<uint_fast32_t> _pos_read;
			std::atomic<uint_fast32_t> _pos_write;

			T* const _data;

		public:
			template<class ...Args>
			explicit PaSPSC (const uint_fast32_t sze, Args&&... args) : _size (sze), _pos_read (0), _pos_write (0), _data (static_cast<T*> (::malloc (sizeof (T) * sze)))
			{
				if (sze < 2) {
					cThrow ("Ring size must be at least 2");
				}

				if (_data == nullptr) {
					cThrow ("Ring data allocation failed");
				}

				for (uint_fast32_t now = 0; now < _size; ++now) {
					new (&_data[now]) T(std::forward<Args>(args)...);
				}
			}

			~PaSPSC ()
			{
				for (uint_fast32_t now = 0; now < _size; ++now) {
					_data[now].~T();
				}

				::free(_data);
			}

			bool is_lock_free (void) const
			{
				return _pos_read.is_lock_free () && _pos_write.is_lock_free ();
			}

			T& back (void)
			{
				const uint_fast32_t now = _pos_write.load (std::memory_order_relaxed);
				uint_fast32_t next = now + 1;
				RING (next);

				if (next == _pos_read.load (std::memory_order_acquire)) {
					cThrow ("Ring full");
				}

				return _data[now];
			}

			T& push_back (void)
			{
				const uint_fast32_t now = _pos_write.load (std::memory_order_relaxed);
				uint_fast32_t next = now + 1;
				RING (next);

				if (next == _pos_read.load (std::memory_order_acquire)) {
					cThrow ("Ring full");
				}

				_pos_write.store (next, std::memory_order_release);

				return _data[next];
			}

			T& front (void)
			{
				const uint_fast32_t now = _pos_read.load (std::memory_order_relaxed);
				if (now == _pos_write.load (std::memory_order_acquire)) {
					cThrow ("Ring empty");
				}

				return _data[now];
			}

			void pop_front (void)
			{
				const uint_fast32_t now = _pos_read.load (std::memory_order_relaxed);
				if (now == _pos_write.load (std::memory_order_acquire)) {
					cThrow ("Ring empty");
				}

				uint_fast32_t next = now + 1;
				RING (next);

				_pos_read.store (next, std::memory_order_release);
			}

			void clear (void)
			{
				while (pop_front () == true);
			}

			bool empty (void) const
			{
				return _pos_read.load (std::memory_order_consume) == _pos_write.load (std::memory_order_consume);
			}

			bool full (void) const
			{
				uint_fast32_t next = _pos_write.load (std::memory_order_consume) + 1;
				RING (next);
				return (next == _pos_read.load (std::memory_order_consume));
			}

			/* Disable default copy constructors */
			PaSPSC (const PaSPSC &) = delete;
			PaSPSC& operator= (const PaSPSC &) = delete;
	};

#undef RING

	typedef SPSC<std::string> StringSPSC;
}

#endif // HEAD_shaga_SPSC

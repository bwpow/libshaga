/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_SPSC
#define HEAD_shaga_SPSC

#include "common.h"

namespace shaga {
#define RING(x) if ((x) >= _size) { (x) = 0; }

	template<class T = std::string> class SPSC
	{
		private:
			const uint_fast32_t _size;

			#ifdef SHAGA_THREADING
				std::atomic<uint_fast32_t> _pos_read {0};
				std::atomic<uint_fast32_t> _pos_write {0};
			#else
				volatile uint_fast32_t _pos_read {0};
				volatile uint_fast32_t _pos_write {0};
			#endif // SHAGA_THREADING

			T* const _data;

		public:
			explicit SPSC (const uint_fast32_t sze) :
				_size (sze),
				_data (static_cast<T*> (::malloc (sizeof (T) * sze)))
			{
				if (sze < 2) {
					cThrow ("Ring size must be at least 2"sv);
				}

				if (_data == nullptr) {
					cThrow ("Ring data allocation failed"sv);
				}
			}

			~SPSC ()
			{
				#ifdef SHAGA_THREADING
					uint_fast32_t now = _pos_read.load (std::memory_order_relaxed);
					const uint_fast32_t last = _pos_write.load (std::memory_order_acquire);
				#else
					uint_fast32_t now = _pos_read;
					const uint_fast32_t last = _pos_write;
				#endif // SHAGA_THREADING

				while (now != last) {
					_data[now++].~T();
					RING (now);
				}

				::free(_data);
			}

			bool is_lock_free (void) const
			{
				#ifdef SHAGA_THREADING
					return _pos_read.is_lock_free () && _pos_write.is_lock_free ();
				#else
					return true;
				#endif // SHAGA_THREADING
			}

			bool push_back (const T &entry)
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t now = _pos_write.load (std::memory_order_relaxed);
				#else
					const uint_fast32_t now = _pos_write;
				#endif // SHAGA_THREADING

				uint_fast32_t next = now + 1;
				RING (next);

				#ifdef SHAGA_THREADING
				if (next == _pos_read.load (std::memory_order_acquire)) {
				#else
				if (next == _pos_read) {
				#endif // SHAGA_THREADING
					return false;
				}

				new (&_data[now]) T(entry);

				#ifdef SHAGA_THREADING
					_pos_write.store (next, std::memory_order_release);
				#else
					_pos_write = next;
				#endif // SHAGA_THREADING

				return true;
			}

			bool push_back (T &&entry)
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t now = _pos_write.load (std::memory_order_relaxed);
				#else
					const uint_fast32_t now = _pos_write;
				#endif // SHAGA_THREADING

				uint_fast32_t next = now + 1;
				RING (next);

				#ifdef SHAGA_THREADING
				if (next == _pos_read.load (std::memory_order_acquire)) {
				#else
				if (next == _pos_read) {
				#endif // SHAGA_THREADING
					return false;
				}

				new (&_data[now]) T();
				_data[now] = std::move (entry);

				#ifdef SHAGA_THREADING
					_pos_write.store (next, std::memory_order_release);
				#else
					_pos_write = next;
				#endif // SHAGA_THREADING

				return true;
			}

			template<class ...Args>
			bool emplace_back (Args&&... args)
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t now = _pos_write.load (std::memory_order_relaxed);
				#else
					const uint_fast32_t now = _pos_write;
				#endif // SHAGA_THREADING

				uint_fast32_t next = now + 1;
				RING (next);

				#ifdef SHAGA_THREADING
				if (next == _pos_read.load (std::memory_order_acquire)) {
				#else
				if (next == _pos_read) {
				#endif // SHAGA_THREADING
					return false;
				}

				new (&_data[now]) T(std::forward<Args>(args)...);

				#ifdef SHAGA_THREADING
					_pos_write.store (next, std::memory_order_release);
				#else
					_pos_write = next;
				#endif // SHAGA_THREADING

				return true;
			}

			bool pop_front (T &entry)
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t now = _pos_read.load (std::memory_order_relaxed);
					if (now == _pos_write.load (std::memory_order_acquire)) {
						return false;
					}
				#else
					const uint_fast32_t now = _pos_read;
					if (now == _pos_write) {
						return false;
					}
				#endif // SHAGA_THREADING

				uint_fast32_t next = now + 1;
				RING (next);

				entry = std::move (_data[now]);
				_data[now].~T();

				#ifdef SHAGA_THREADING
					_pos_read.store (next, std::memory_order_release);
				#else
					_pos_read = next;
				#endif // SHAGA_THREADING
				return true;
			}

			bool pop_front (void)
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t now = _pos_read.load (std::memory_order_relaxed);
					if (now == _pos_write.load (std::memory_order_acquire)) {
						return false;
					}
				#else
					const uint_fast32_t now = _pos_read;
					if (now == _pos_write) {
						return false;
					}
				#endif // SHAGA_THREADING

				uint_fast32_t next = now + 1;
				RING (next);

				_data[now].~T();

				#ifdef SHAGA_THREADING
					_pos_read.store (next, std::memory_order_release);
				#else
					_pos_read = next;
				#endif // SHAGA_THREADING
				return true;
			}

			T* front (void)
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t now = _pos_read.load (std::memory_order_relaxed);
					if (now == _pos_write.load (std::memory_order_acquire)) {
						return nullptr;
					}
				#else
					const uint_fast32_t now = _pos_read;
					if (now == _pos_write) {
						return nullptr;
					}
				#endif // SHAGA_THREADING

				return &_data[now];
			}

			void clear (void)
			{
				while (pop_front () == true);
			}

			bool empty (void) const
			{
				#ifdef SHAGA_THREADING
					return _pos_read.load (std::memory_order_consume) == _pos_write.load (std::memory_order_consume);
				#else
					return _pos_read == _pos_write;
				#endif // SHAGA_THREADING
			}

			bool full (void) const
			{
				#ifdef SHAGA_THREADING
					uint_fast32_t next = _pos_write.load (std::memory_order_consume) + 1;
				#else
					uint_fast32_t next = _pos_write + 1;
				#endif // SHAGA_THREADING

				RING (next);

				#ifdef SHAGA_THREADING
					return (next == _pos_read.load (std::memory_order_consume));
				#else
					return (next == _pos_read);
				#endif // SHAGA_THREADING
			}

			/* Disable copy and assignment */
			SPSC (const SPSC &) = delete;
			SPSC& operator= (const SPSC &) = delete;
	};

	template<class T> class PaSPSC
	{
		private:
			const uint_fast32_t _size;

			#ifdef SHAGA_THREADING
				std::atomic<uint_fast32_t> _pos_read {0};
				std::atomic<uint_fast32_t> _pos_write {0};
			#else
				volatile uint_fast32_t _pos_read {0};
				volatile uint_fast32_t _pos_write {0};
			#endif // SHAGA_THREADING

			T* const _data;

		public:
			template<class ...Args>
			explicit PaSPSC (const uint_fast32_t sze, Args&&... args) :
				_size (sze),
				_data (static_cast<T*> (::malloc (sizeof (T) * sze)))
			{
				if (sze < 2) {
					cThrow ("Ring size must be at least 2"sv);
				}

				if (_data == nullptr) {
					cThrow ("Ring data allocation failed"sv);
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
				#ifdef SHAGA_THREADING
					return _pos_read.is_lock_free () && _pos_write.is_lock_free ();
				#else
					return true;
				#endif // SHAGA_THREADING
			}

			T& back (void)
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t now = _pos_write.load (std::memory_order_relaxed);
				#else
					const uint_fast32_t now = _pos_write;
				#endif // SHAGA_THREADING

				uint_fast32_t next = now + 1;
				RING (next);

				#ifdef SHAGA_THREADING
				if (next == _pos_read.load (std::memory_order_acquire)) {
				#else
				if (next == _pos_read) {
				#endif // SHAGA_THREADING
					cThrow ("Ring full"sv);
				}

				return _data[now];
			}

			T& push_back (void)
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t now = _pos_write.load (std::memory_order_relaxed);
				#else
					const uint_fast32_t now = _pos_write;
				#endif // SHAGA_THREADING

				uint_fast32_t next = now + 1;
				RING (next);

				#ifdef SHAGA_THREADING
				if (next == _pos_read.load (std::memory_order_acquire)) {
				#else
				if (next == _pos_read) {
				#endif // SHAGA_THREADING
					cThrow ("Ring full");
				}

				#ifdef SHAGA_THREADING
					_pos_write.store (next, std::memory_order_release);
				#else
					_pos_write = next;
				#endif // SHAGA_THREADING

				return _data[next];
			}

			T& front (void)
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t now = _pos_read.load (std::memory_order_relaxed);
					if (now == _pos_write.load (std::memory_order_acquire)) {
						cThrow ("Ring empty"sv);
					}
				#else
					const uint_fast32_t now = _pos_read;
					if (now == _pos_write) {
						cThrow ("Ring emptysv");
					}
				#endif // SHAGA_THREADING

				return _data[now];
			}

			void pop_front (void)
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t now = _pos_read.load (std::memory_order_relaxed);
					if (now == _pos_write.load (std::memory_order_acquire)) {
						cThrow ("Ring empty"sv);
					}
				#else
					const uint_fast32_t now = _pos_read;
					if (now == _pos_write) {
						cThrow ("Ring empty"sv);
					}
				#endif // SHAGA_THREADING

				uint_fast32_t next = now + 1;
				RING (next);

				#ifdef SHAGA_THREADING
					_pos_read.store (next, std::memory_order_release);
				#else
					_pos_read = next;
				#endif // SHAGA_THREADING
			}

			void clear (void)
			{
				while (pop_front () == true);
			}

			bool empty (void) const
			{
				#ifdef SHAGA_THREADING
					return _pos_read.load (std::memory_order_consume) == _pos_write.load (std::memory_order_consume);
				#else
					return _pos_read == _pos_write;
				#endif // SHAGA_THREADING
			}

			bool full (void) const
			{
				#ifdef SHAGA_THREADING
					uint_fast32_t next = _pos_write.load (std::memory_order_consume) + 1;
				#else
					uint_fast32_t next = _pos_write + 1;
				#endif // SHAGA_THREADING

				RING (next);

				#ifdef SHAGA_THREADING
					return (next == _pos_read.load (std::memory_order_consume));
				#else
					return (next == _pos_read);
				#endif // SHAGA_THREADING
			}

			/* Disable default copy constructors */
			PaSPSC (const PaSPSC &) = delete;
			PaSPSC& operator= (const PaSPSC &) = delete;
	};

#undef RING

	typedef SPSC<std::string> StringSPSC;
}

#endif // HEAD_shaga_SPSC

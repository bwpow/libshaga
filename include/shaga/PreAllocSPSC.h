/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_PreAllocSPSC
#define HEAD_shaga_PreAllocSPSC

#include "common.h"

namespace shaga
{
	template<class T = std::string>
	class PreAllocSPSC
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

			T* const _data {nullptr};

		public:
			template<class ...Args>
			explicit PreAllocSPSC (const uint_fast32_t sze, Args&&... args) :
				_size (sze),
				_data (static_cast<T*> (::malloc (sizeof (T) * sze)))
			{
				try {
					if (sze < 2) {
						cThrow ("Ring size must be at least 2"sv);
					}

					if (nullptr == _data) {
						cThrow ("Ring data allocation failed"sv);
					}
				}
				catch (...) {
					if (nullptr != _data) {
						::free(_data);
					}
					throw;
				}

				for (uint_fast32_t now = 0; now < _size; ++now) {
					try {
						new (&_data[now]) T(std::forward<Args>(args)...);
					}
					catch (...) {
						/* Clear all already created objects */
						while (now > 0) {
							--now;
							_data[now].~T();
						}
						::free(_data);
						throw;
					}
				}
			}

			~PreAllocSPSC ()
			{
				for (uint_fast32_t now = 0; now < _size; ++now) {
					_data[now].~T();
				}

				::free(_data);
			}

			bool is_lock_free (void) const
			{
				#ifdef SHAGA_THREADING
					return (_pos_read.is_lock_free ()) && (_pos_write.is_lock_free ());
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

				_SHAGA_SPSC_RING (next, now);

				#ifdef SHAGA_THREADING
					if (next == _pos_read.load (std::memory_order_acquire)) {
				#else
					if (HEDLEY_UNLIKELY (next == _pos_read)) {
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

				_SHAGA_SPSC_RING (next, now);

				#ifdef SHAGA_THREADING
					if (next == _pos_read.load (std::memory_order_acquire)) {
				#else
					if (HEDLEY_UNLIKELY (next == _pos_read)) {
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
				#else
					const uint_fast32_t now = _pos_read;
					if (HEDLEY_UNLIKELY (now == _pos_write)) {
				#endif // SHAGA_THREADING
					cThrow ("Ring empty"sv);
				}

				return _data[now];
			}

			void pop_front (void)
			{
				#ifdef SHAGA_THREADING
					const uint_fast32_t now = _pos_read.load (std::memory_order_relaxed);
					if (now == _pos_write.load (std::memory_order_acquire)) {
				#else
					const uint_fast32_t now = _pos_read;
					if (HEDLEY_UNLIKELY (now == _pos_write)) {
				#endif // SHAGA_THREADING
					cThrow ("Ring empty"sv);
				}

				_SHAGA_SPSC_RING (next, now);

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
					return _pos_read.load (std::memory_order_relaxed) == _pos_write.load (std::memory_order_acquire);
				#else
					return _pos_read == _pos_write;
				#endif // SHAGA_THREADING
			}

			bool full (void) const
			{
				#ifdef SHAGA_THREADING
					_SHAGA_SPSC_RING (next, _pos_write.load (std::memory_order_relaxed));
					return (next == _pos_read.load (std::memory_order_acquire));
				#else
					_SHAGA_SPSC_RING (next, _pos_write);
					return (next == _pos_read);
				#endif // SHAGA_THREADING
			}

			/* Disable default copy constructors */
			PreAllocSPSC (const PreAllocSPSC &) = delete;
			PreAllocSPSC& operator= (const PreAllocSPSC &) = delete;
	};
}

#endif // HEAD_shaga_PreAllocSPSC

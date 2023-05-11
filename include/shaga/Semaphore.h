/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2023, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_Semaphore
#define HEAD_shaga_Semaphore

#include "common.h"

#ifdef SHAGA_THREADING

namespace shaga {
	class Semaphore
	{
		private:
			int_fast32_t _count;
			std::mutex _mutex;
			std::condition_variable _cv;

		public:
			using native_handle_type = std::condition_variable::native_handle_type;

			explicit Semaphore (const uint16_t n = 0)
			{
				reset (n);
			}

			Semaphore (const Semaphore&) = delete;
			Semaphore& operator=(const Semaphore&) = delete;

			void reset (const uint16_t n = 0)
			{
				std::lock_guard<std::mutex> lock (_mutex);
				_count = static_cast<int_fast32_t> (n);
			}

			void notify (void)
			{
				std::lock_guard<std::mutex> lock (_mutex);
				++_count;
				_cv.notify_one();
			}

			void wait()
			{
				std::unique_lock<std::mutex> lock (_mutex);
				_cv.wait(lock, [this]{ return (_count > 0); });
				--_count;
			}


			template<class Rep, class Period>
			bool wait_for(const std::chrono::duration<Rep, Period>& d)
			{
				std::unique_lock<std::mutex> lock (_mutex);
				const bool finished = _cv.wait_for(lock, d, [this]{ return (_count > 0); });

				if (finished) {
					--_count;
				}

				return finished;
			}


			template<class Clock, class Duration>
			bool wait_until(const std::chrono::time_point<Clock, Duration>& t)
			{
				std::unique_lock<std::mutex> lock (_mutex);
				const bool finished = _cv.wait_until(lock, t, [this]{ return (_count > 0); });

				if (finished) {
					--_count;
				}

				return finished;
			}

			native_handle_type native_handle (void)
			{
				return _cv.native_handle();
			}
	};
}

#endif // SHAGA_THREADING
#endif // HEAD_shaga_Semaphore

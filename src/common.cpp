/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {

	#if BYTE_ORDER == LITTLE_ENDIAN
		const bool _shaga_compiled_little_endian {true};
		#pragma message "Endian: LITTLE"
	#elif BYTE_ORDER == BIG_ENDIAN
		const bool _shaga_compiled_little_endian {false};
		#pragma message "Endian: BIG"
	#else
		#error Unable to detect version of the library
	#endif

	#if defined SHAGA_MULTI_THREAD
		const bool _shaga_compiled_with_threading {true};
		#pragma message "Threading support: YES"
	#elif defined SHAGA_SINGLE_THREAD
		const bool _shaga_compiled_with_threading {false};
		#pragma message "Threading support: NO"
	#else
		#error Unable to detect version of the library
	#endif

	#if defined SHAGA_LITE
		const bool _shaga_compiled_full {false};
		#pragma message "Version: LITE"
	#elif defined SHAGA_FULL
		const bool _shaga_compiled_full {true};
		#pragma message "Version: FULL"
	#else
		#error Unable to detect version of the library
	#endif

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Most used templates  ////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template class SPSC<std::string>;

	template class Uart8EncodeSPSC<SPSCDataPreAlloc>;
	template class Uart8DecodeSPSC<SPSCDataPreAlloc>;

	template class Uart16EncodeSPSC<SPSCDataPreAlloc>;
	template class Uart16DecodeSPSC<SPSCDataPreAlloc>;

	template class PacketEncodeSPSC<SPSCDataDynAlloc>;
	template class PacketDecodeSPSC<SPSCDataDynAlloc>;

	template class SeqPacketEncodeSPSC<SPSCDataDynAlloc>;
	template class SeqPacketDecodeSPSC<SPSCDataDynAlloc>;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static definitions  /////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	#ifdef SHAGA_THREADING
		static std::mutex _callback_mutex;
		static std::recursive_mutex _exit_mutex;
	#endif // SHAGA_THREADING

	typedef std::list<std::function<void (void)>> CALLBACK_LIST;

	static CALLBACK_LIST _at_shutdown_callback_list;
	static CALLBACK_LIST _at_exit_callback_list;
	static FINAL_CALL _final_call {nullptr};

	#ifdef SHAGA_THREADING
		static std::atomic<bool> _is_shutdown {false};
		static std::atomic<bool> _is_in_exit {false};
	#else
		volatile bool _is_shutdown {false};
		bool _is_in_exit {false};
	#endif // SHAGA_THREADING

	void add_at_exit_callback (std::function<void (void)> func)
	{
		#ifdef SHAGA_THREADING
		std::lock_guard<std::mutex> lock (_callback_mutex);
		#endif // SHAGA_THREADING
		_at_exit_callback_list.push_back (func);
	}

	[[noreturn]] void _exit (const std::string_view text, const int rcode, const std::string_view prefix) noexcept
	{
		try {
			#ifdef SHAGA_THREADING
			std::unique_lock<std::recursive_mutex> exitlck (_exit_mutex);
			#endif // SHAGA_THREADING

			#ifdef SHAGA_THREADING
			if (_is_in_exit.exchange (true) == true)
			#else
			if (std::exchange (_is_in_exit, true) == true)
			#endif // SHAGA_THREADING
			{
				/* This function is already being executed, clearly from one of the callback functions. */
				P::_print ("Exit executed recursively from callback function."sv, "FATAL ERROR: "sv, true);
				P::set_enabled (false);
				::exit (EXIT_FAILURE);
			}

			#ifdef SHAGA_THREADING
			std::unique_lock<std::mutex> lck (_callback_mutex);
			#endif // SHAGA_THREADING

			CALLBACK_LIST lst;
			lst.swap (_at_exit_callback_list);

			#ifdef SHAGA_THREADING
			lck.unlock ();
			#endif // SHAGA_THREADING

			for (auto &func : lst) {
				if (func != nullptr) {
					func ();
					func = nullptr;
				}
			}
			lst.clear ();

			if (text.empty () == false || prefix.empty () == false) {
				P::_print (text, prefix, true);
			}

			P::print ("Application exit with errorcode {}"sv, rcode);

			if (nullptr != _final_call) {
				_final_call (text, rcode);
				_final_call = nullptr;
			}

			P::set_enabled (false);
			::exit (rcode);
		}
		catch (...) {
			P::_print ("Exception caught in exit."sv, "FATAL ERROR: "sv, true);
			P::set_enabled (false);
			::exit (EXIT_FAILURE);
		}
	}

	[[noreturn]] void exit (const int rcode) noexcept
	{
		_exit (nullptr, rcode);
	}

	[[noreturn]] void exit_failure (void) noexcept
	{
		_exit (nullptr, EXIT_FAILURE);
	}

	[[noreturn]] void exit (void) noexcept
	{
		_exit (nullptr, EXIT_SUCCESS);
	}

	void set_final_call (FINAL_CALL func)
	{
		_final_call = func;
	}

	void add_at_shutdown_callback (std::function<void (void)> func)
	{
		#ifdef SHAGA_THREADING
		std::lock_guard<std::mutex> lock(_callback_mutex);
		#endif // SHAGA_THREADING
		_at_shutdown_callback_list.push_back (func);
	}

	void _try_to_shutdown (const char *file, const char *funct, const int line)
	{
		#ifdef SHAGA_THREADING
		if (_is_shutdown.exchange (true, std::memory_order_seq_cst) == false)
		#else
		if (std::exchange (_is_shutdown, true) == false)
		#endif // SHAGA_THREADING
		{
			P::print ("Shutdown requested from {}: {} line {}"sv, file, funct, line);

			#ifdef SHAGA_THREADING
			std::unique_lock<std::mutex> lck (_callback_mutex);
			#endif // SHAGA_THREADING

			CALLBACK_LIST lst;
			lst.swap (_at_shutdown_callback_list);

			#ifdef SHAGA_THREADING
			lck.unlock ();
			#endif // SHAGA_THREADING

			for (auto &func : lst) {
				if (func != nullptr) {
					func ();
					func = nullptr;
				}
			}
		}
	}

	SHAGA_NODISCARD bool is_shutting_down (void)
	{
		#ifdef SHAGA_THREADING
		return _is_shutdown.load (std::memory_order_relaxed);
		#else
		return _is_shutdown;
		#endif // SHAGA_THREADING
	}

	int64_t timeval_diff_msec (const struct timeval &starttime, const struct timeval &finishtime)
	{
		return ((static_cast<int64_t> (finishtime.tv_sec) - static_cast<int64_t> (starttime.tv_sec)) * 1'000) +
			(static_cast<int64_t> (finishtime.tv_usec) - static_cast<int64_t> (starttime.tv_usec)) / 1'000;
	}

	int64_t timespec_diff_msec (const struct timespec &starttime, const struct timespec &finishtime)
	{
		return ((static_cast<int64_t> (finishtime.tv_sec) - static_cast<int64_t> (starttime.tv_sec)) * 1'000) +
			(static_cast<int64_t> (finishtime.tv_nsec / 1'000'000) - static_cast<int64_t> (starttime.tv_nsec / 1'000'000));
	}

	uint64_t get_monotime_sec (void)
	{
		struct timespec monotime;
		#ifdef CLOCK_MONOTONIC_RAW
		::clock_gettime(CLOCK_MONOTONIC_RAW, &monotime);
		#else
		::clock_gettime(CLOCK_MONOTONIC, &monotime);
		#endif // CLOCK_MONOTONIC_RAW
		return static_cast<uint64_t> (monotime.tv_sec);
	}

	uint64_t get_monotime_msec (void)
	{
		struct timespec monotime;
		#ifdef CLOCK_MONOTONIC_RAW
		::clock_gettime(CLOCK_MONOTONIC_RAW, &monotime);
		#else
		::clock_gettime(CLOCK_MONOTONIC, &monotime);
		#endif // CLOCK_MONOTONIC_RAW
		return (static_cast<uint64_t> (monotime.tv_sec) * 1'000) + (static_cast<uint64_t> (monotime.tv_nsec) / 1'000'000);
	}

	uint64_t get_monotime_usec (void)
	{
		struct timespec monotime;
		#ifdef CLOCK_MONOTONIC_RAW
		::clock_gettime(CLOCK_MONOTONIC_RAW, &monotime);
		#else
		::clock_gettime(CLOCK_MONOTONIC, &monotime);
		#endif // CLOCK_MONOTONIC_RAW
		return (static_cast<uint64_t> (monotime.tv_sec) * 1'000'000) + (static_cast<uint64_t> (monotime.tv_nsec) / 1'000);
	}

	uint64_t get_realtime_sec (void)
	{
		struct timespec monotime;
		::clock_gettime (CLOCK_REALTIME, &monotime);
		return static_cast<uint64_t> (monotime.tv_sec);
	}

	uint64_t get_realtime_msec (void)
	{
		struct timespec monotime;
		::clock_gettime (CLOCK_REALTIME, &monotime);
		return (static_cast<uint64_t> (monotime.tv_sec) * 1'000) + (static_cast<uint64_t> (monotime.tv_nsec) / 1'000'000);
	}

	uint64_t get_realtime_usec (void)
	{
		struct timespec monotime;
		::clock_gettime (CLOCK_REALTIME, &monotime);
		return (static_cast<uint64_t> (monotime.tv_sec) * 1'000'000) + (static_cast<uint64_t> (monotime.tv_nsec) / 1'000);
	}
}

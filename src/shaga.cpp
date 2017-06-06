/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#include <atomic>
#include <mutex>
#include <cstdarg>

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Most used templates  ////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template class SPSC<std::string>;

	template class UartEncodeSPSC<SPSCDataPreAlloc>;
	template class UartDecodeSPSC<SPSCDataPreAlloc>;

	template class PacketEncodeSPSC<SPSCDataDynAlloc>;
	template class PacketDecodeSPSC<SPSCDataDynAlloc>;

	template class SeqPacketEncodeSPSC<SPSCDataDynAlloc>;
	template class SeqPacketDecodeSPSC<SPSCDataDynAlloc>;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static definitions  /////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static std::mutex _callback_mutex;
	static std::recursive_mutex _exit_mutex;

	typedef std::list<std::function<void (void)>> CALLBACK_LIST;

	static CALLBACK_LIST _at_shutdown_callback_list;
	static CALLBACK_LIST _at_exit_callback_list;
	static FINAL_CALL _final_call = nullptr;

	static std::atomic<bool> _is_shutdown (false);
	static std::atomic<bool> _is_in_exit (false);

	CommonException::CommonException (const bool log, const char *str_file, const char *str_function, int str_line, const char *fmt, ...) throw ()
	{
		va_list ap;

		::snprintf (_text, sizeof (_text), "%s(%d): %s: ", str_file, str_line, str_function);
		_text[sizeof (_text) - 1] = '\0';

		_info_pos = strlen (_text);

		::va_start (ap, fmt);
		::vsnprintf (_text + _info_pos, sizeof (_text) - _info_pos, fmt, ap);
		::va_end (ap);
		_text[sizeof (_text) - 1] = '\0';

		if (true == log) {
			P::printf ("Exception thrown: %s", _text);
		}
		else {
			P::debug_printf ("Exception thrown: %s", _text);
		}
	}

	const char * CommonException::debugwhat () const throw ()
	{
		return _text;
	}

	const char * CommonException::what () const throw ()
	{
		return _text + _info_pos;
	}

	void add_at_exit_callback (std::function<void (void)> func)
	{
		std::lock_guard<std::mutex> lock(_callback_mutex);
		_at_exit_callback_list.push_back (func);
	}

	[[noreturn]] static void _exit (const char *text, const int rcode)
	{
		std::unique_lock<std::recursive_mutex> exitlck (_exit_mutex);

		if (_is_in_exit.exchange (true) == true) {
			/* This function is already being executed, clearly from one of the callback functions. */
			P::printf ("FATAL ERROR: Exit executed recursively from callback function.");
			fprintf (stderr, "FATAL ERROR: Exit executed recursively from callback function.\n");
			::exit (EXIT_FAILURE);
		}

		std::unique_lock<std::mutex> lck(_callback_mutex);
		CALLBACK_LIST lst;
		lst.swap (_at_exit_callback_list);
		lck.unlock ();

		for (const auto &func : lst) {
			if (func != nullptr) {
				func ();
			}
		}
		lst.clear ();

		if (text != nullptr && ::strlen (text) > 0) {
			P::printf ("Exit message: %s", text);
			fprintf (stderr, "%s\n", text);
		}
		P::printf ("Application exit with errorcode %d", rcode);

		if (nullptr != _final_call) {
			_final_call (text, rcode);
		}

		::exit (rcode);
	}

	[[noreturn]] void exit (const int rcode, const char *fmt, ...)
	{
		char buf[1024];
		va_list ap;
		::va_start (ap, fmt);
		::vsnprintf (buf, sizeof (buf), fmt, ap);
		::va_end (ap);
		_exit (buf, rcode);
	}

	[[noreturn]] void exit (const char *fmt, ...)
	{
		char buf[1024];
		va_list ap;
		::va_start (ap, fmt);
		::vsnprintf (buf, sizeof (buf), fmt, ap);
		::va_end (ap);
		_exit (buf, EXIT_FAILURE);
	}

	[[noreturn]] void exit (const int rcode)
	{
		_exit (nullptr, rcode);
	}

	[[noreturn]] void exit_failure (void)
	{
		_exit (nullptr, EXIT_FAILURE);
	}

	[[noreturn]] void exit (void)
	{
		_exit (nullptr, EXIT_SUCCESS);
	}

	void set_final_call (FINAL_CALL func)
	{
		_final_call = func;
	}

	void add_at_shutdown_callback (std::function<void (void)> func)
	{
		std::lock_guard<std::mutex> lock(_callback_mutex);
		_at_shutdown_callback_list.push_back (func);
	}

	void _try_to_shutdown (const char *file, const char *funct, const int line)
	{
		if (_is_shutdown.exchange (true) == false) {
			P::printf ("Shutdown requested from %s: %s line %d", file, funct, line);

			std::unique_lock<std::mutex> lck(_callback_mutex);
			CALLBACK_LIST lst;
			lst.swap (_at_shutdown_callback_list);
			lck.unlock ();

			for (const auto &func : lst) {
				if (func != nullptr) {
					func ();
				}
			}
		}
	}

	bool is_shutting_down (void)
	{
		return _is_shutdown.load ();
	}

	int64_t timeval_diff_msec (const struct timeval &starttime, const struct timeval &finishtime)
	{
		int64_t msec;
		msec = (static_cast<int64_t> (finishtime.tv_sec) - static_cast<int64_t> (starttime.tv_sec)) * 1000;
		msec += (static_cast<int64_t> (finishtime.tv_usec) - static_cast<int64_t> (starttime.tv_usec)) / 1000;
		return msec;
	}

	int64_t timespec_diff_msec (const struct timespec &starttime, const struct timespec &finishtime)
	{
		int64_t msec;
		msec = (static_cast<int64_t> (finishtime.tv_sec) - static_cast<int64_t> (starttime.tv_sec)) * 1000;
		msec += (static_cast<int64_t> (finishtime.tv_nsec / 1000000) - static_cast<int64_t> (starttime.tv_nsec / 1000000));
		return msec;
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
		return (static_cast<uint64_t> (monotime.tv_sec) * 1000) + (static_cast<uint64_t> (monotime.tv_nsec) / 1000000);
	}

	uint64_t get_monotime_usec (void)
	{
		struct timespec monotime;
		#ifdef CLOCK_MONOTONIC_RAW
		::clock_gettime(CLOCK_MONOTONIC_RAW, &monotime);
		#else
		::clock_gettime(CLOCK_MONOTONIC, &monotime);
		#endif // CLOCK_MONOTONIC_RAW
		return (static_cast<uint64_t> (monotime.tv_sec) * 1000000) + (static_cast<uint64_t> (monotime.tv_nsec) / 1000);
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
		return (static_cast<uint64_t> (monotime.tv_sec) * 1000) + (static_cast<uint64_t> (monotime.tv_nsec) / 1000000);
	}

	uint64_t get_realtime_usec (void)
	{
		struct timespec monotime;
		::clock_gettime (CLOCK_REALTIME, &monotime);
		return (static_cast<uint64_t> (monotime.tv_sec) * 1000000) + (static_cast<uint64_t> (monotime.tv_nsec) / 1000);
	}

}

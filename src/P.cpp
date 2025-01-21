/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifndef OS_WIN
	#include <syslog.h>
#endif // OS_WIN

namespace shaga::P {
	static P_CACHE_TYPE _p_cache;
	static std::array<char, 32> _p_cache_time;
	static std::array<char, 256> _p_cache_fname;

	static const off64_t _bytes_per_mb {1'024 * 1'024};

	static bool _printf_ms {false};
	static bool _use_gmtime {false};

	static bool _check_size {false};
	static volatile bool _soft_limit_reached {false};
	static off64_t _limit_soft {0};
	static off64_t _limit_hard {0};

	#ifdef SHAGA_THREADING
		static std::mutex _print_mutex;
		static std::mutex _p_cache_mutex;
	#endif // SHAGA_THREADING

	static volatile bool _disabled_permanently {false};
	static volatile bool _disabled_temporarily {false};
	static volatile bool _enabled {false};
	static volatile bool _debug_enabled {false};

	static COMMON_VECTOR _dir_log;
	static size_t _dir_log_pos {0};

	static std::string _name_log;
	static std::string _app_name;

	static enum class _DirLogEnum {
		FILE,
		SYSLOG,
		STDOUT,
		STDERR
	} _dir_log_enum = _DirLogEnum::FILE;

	static ShFile _printf_file;
	static COMMON_DEQUE _printf_entries;

	static uint64_t _p_cache_rt {UINT64_MAX};
	static time_t _p_cache_theTime {-1};
	static struct tm _p_cache_local_tm;
	static size_t _p_cache_time_sze {0};
	static size_t _p_cache_fname_sze {0};

	static void _p_clear_cache (void)
	{
		_printf_file.close ();
		_p_cache_rt = UINT64_MAX;
		_p_cache_theTime = -1;
		_p_cache_time_sze = 0;
		_p_cache_fname_sze = 0;
	}

	static void _p_cache_scan_directories (void)
	{
		if (_dir_log_enum != _DirLogEnum::FILE) {
			cThrow ("Logging is not set to output to files"sv);
		}

		_dir_log_pos = SIZE_MAX;
		for (size_t pos = 0; pos < _dir_log.size (); ++pos) {
			if (FS::is_dir (_dir_log[pos])) {
				_dir_log_pos = pos;
				return;
			}
		}

		cThrow ("All directories in list are unavailable"sv);
	}

	template <typename T>
	static void _set_dir_log (const T &lst)
	{
		if (lst.empty () == true) {
			return;
		}

		#ifdef SHAGA_THREADING
			std::lock_guard<std::mutex> lock (_print_mutex);
		#endif // SHAGA_THREADING

		_dir_log.clear ();
		std::copy (lst.cbegin (), lst.cend (), std::back_inserter (_dir_log));
		_dir_log_pos = 0;

		_p_clear_cache ();
	}

}

namespace shaga
{
	void P::set_dir_log (const std::string_view var)
	{
		#ifdef SHAGA_THREADING
			std::lock_guard<std::mutex> lock (_print_mutex);
		#endif // SHAGA_THREADING

		_dir_log.clear ();
		_dir_log.emplace_back (var);
		_dir_log_pos = 0;

		_p_clear_cache ();
	}

	void P::set_dir_log (const COMMON_DEQUE &lst)
	{
		_set_dir_log (lst);
	}

	void P::set_dir_log (const COMMON_LIST &lst)
	{
		_set_dir_log (lst);
	}

	void P::set_dir_log (const COMMON_VECTOR &lst)
	{
		_set_dir_log (lst);
	}

	void P::set_name_log (const std::string_view var)
	{
		#ifdef SHAGA_THREADING
			std::lock_guard<std::mutex> lock (_print_mutex);
		#endif // SHAGA_THREADING

		_name_log.assign (var);
	}

	void P::set_app_name (const std::string_view var)
	{
		#ifdef SHAGA_THREADING
			std::lock_guard<std::mutex> lock (_print_mutex);
		#endif // SHAGA_THREADING

		_app_name.assign (var);
	}

	/////////////////////////////////////////////////////////////

	void P::check_size (const bool enabled) noexcept
	{
		#ifdef SHAGA_THREADING
			std::lock_guard<std::mutex> lock (_print_mutex);
		#endif // SHAGA_THREADING

		_check_size = enabled;
		if (enabled == false) {
			_soft_limit_reached = false;
		}
	}

	void P::set_max_size_mb (const int soft, const int hard) noexcept
	{
		#ifdef SHAGA_THREADING
			std::lock_guard<std::mutex> lock (_print_mutex);
		#endif // SHAGA_THREADING

		_limit_soft = static_cast<off64_t> (soft) * _bytes_per_mb;
		_limit_hard = static_cast<off64_t> (hard) * _bytes_per_mb;
		_soft_limit_reached = false;
	}

	bool P::soft_limit_reached (void) noexcept
	{
		return _soft_limit_reached;
	}

	/////////////////////////////////////////////////////////////

	void P::rescan_available_directories (void)
	{
		#ifdef SHAGA_THREADING
			std::lock_guard<std::mutex> lock (_print_mutex);
		#endif // SHAGA_THREADING

		_p_cache_scan_directories ();
		_p_clear_cache ();
	}

	void P::set_enabled (const bool enabled)
	{
		#ifdef SHAGA_THREADING
			std::lock_guard<std::mutex> lock (_print_mutex);
		#endif // SHAGA_THREADING

		if (enabled) {
			if (_dir_log.empty () || _dir_log.front().empty () ||  _name_log.empty () || _app_name.empty ()) {
				cThrow ("Can't enable P::print, variables are not set."sv);
			}

			if (STR::icompare (_dir_log.front(), "syslog"sv) == true) {
				_dir_log_enum = _DirLogEnum::SYSLOG;
			}
			else if (STR::icompare (_dir_log.front(), "-"sv) == true || STR::icompare (_dir_log.front(), "stdout"sv) == true) {
				_dir_log_enum = _DirLogEnum::STDOUT;
			}
			else if (STR::icompare (_dir_log.front(), "stderr"sv) == true) {
				_dir_log_enum = _DirLogEnum::STDERR;
			}
			else {
				_dir_log_enum = _DirLogEnum::FILE;
				_p_cache_scan_directories ();
			}
		}
		_enabled = enabled;
		_p_clear_cache ();
	}

	void P::show_ms (const bool enabled) noexcept
	{
		#ifdef SHAGA_THREADING
			std::lock_guard<std::mutex> lock (_print_mutex);
		#endif // SHAGA_THREADING

		_printf_ms = enabled;
		_p_clear_cache ();
	}

	void P::use_gmt (const bool enabled) noexcept
	{
		#ifdef SHAGA_THREADING
			std::lock_guard<std::mutex> lock (_print_mutex);
		#endif // SHAGA_THREADING

		_use_gmtime = enabled;
		_p_clear_cache ();
	}

	bool P::is_enabled (void) noexcept
	{
		return (true == _enabled && false == _disabled_temporarily && false == _disabled_permanently);
	}

	P::P_CACHE_TYPE& P::_p_cache_lock (void) noexcept
	{
		#ifdef SHAGA_THREADING
			_p_cache_mutex.lock ();
		#endif // SHAGA_THREADING

		return _p_cache;
	}

	void P::_p_cache_release (void) noexcept
	{
		#ifdef SHAGA_THREADING
			_p_cache_mutex.unlock ();
		#endif // SHAGA_THREADING
	}

	void P::_print (const std::string_view message, const std::string_view prefix, const bool also_to_stderr) noexcept
	{
		if (false == is_enabled ()) {
			if (true == also_to_stderr) {
				/* Printing is not yet enabled, but output to stderr is requested */
				::fwrite (prefix.data (), 1, prefix.size (), stderr);
				::fwrite (message.data (), 1, message.size (), stderr);
				::fputc ('\n', stderr);
				::fflush (stderr);
			}
			return;
		}

		#ifdef SHAGA_THREADING
			// Since we can write logs to non-POSIX filesystems, let's do locking here instead on FS level.
			std::lock_guard<std::mutex> lock (_print_mutex);
		#endif // SHAGA_THREADING

		const uint64_t rt = (true == _printf_ms) ? get_realtime_msec () : 0;
		const time_t theTime = (rt > 0) ? (rt / 1'000) : get_realtime_sec ();

		if (rt != _p_cache_rt || theTime != _p_cache_theTime) {
			_p_cache_rt = rt;
			_p_cache_theTime = theTime;

			::memset (&_p_cache_local_tm, 0, sizeof (_p_cache_local_tm));
			if (true == _use_gmtime) {
				::gmtime_r (&theTime, &_p_cache_local_tm);
			}
			else {
				::localtime_r (&theTime, &_p_cache_local_tm);
			}

#ifndef OS_WIN
			if (true == _printf_ms) {
				_p_cache_time_sze = fmt::format_to_n (_p_cache_time.begin (), _p_cache_time.size (), "{:04}-{:02}-{:02} {:02}:{:02}:{:02d}.{:03} {} : "sv,
					_p_cache_local_tm.tm_year + 1900,
					_p_cache_local_tm.tm_mon + 1,
					_p_cache_local_tm.tm_mday,
					_p_cache_local_tm.tm_hour,
					_p_cache_local_tm.tm_min,
					_p_cache_local_tm.tm_sec,
					rt % 1'000,
					_p_cache_local_tm.tm_zone).size;
			}
			else {
				_p_cache_time_sze = fmt::format_to_n (_p_cache_time.begin (), _p_cache_time.size (), "{:04}-{:02}-{:02} {:02}:{:02}:{:02d} {} : "sv,
					_p_cache_local_tm.tm_year + 1900,
					_p_cache_local_tm.tm_mon + 1,
					_p_cache_local_tm.tm_mday,
					_p_cache_local_tm.tm_hour,
					_p_cache_local_tm.tm_min,
					_p_cache_local_tm.tm_sec,
					_p_cache_local_tm.tm_zone).size;
			}
#else
			if (true == _printf_ms) {
				_p_cache_time_sze = fmt::format_to_n (_p_cache_time.begin (), _p_cache_time.size (), "{:04}-{:02}-{:02} {:02}:{:02}:{:02d}.{:03} : "sv,
					_p_cache_local_tm.tm_year + 1900,
					_p_cache_local_tm.tm_mon + 1,
					_p_cache_local_tm.tm_mday,
					_p_cache_local_tm.tm_hour,
					_p_cache_local_tm.tm_min,
					_p_cache_local_tm.tm_sec,
					rt % 1'000).size;
			}
			else {
				_p_cache_time_sze = fmt::format_to_n (_p_cache_time.begin (), _p_cache_time.size (), "{:04}-{:02}-{:02} {:02}:{:02}:{:02d} : "sv,
					_p_cache_local_tm.tm_year + 1900,
					_p_cache_local_tm.tm_mon + 1,
					_p_cache_local_tm.tm_mday,
					_p_cache_local_tm.tm_hour,
					_p_cache_local_tm.tm_min,
					_p_cache_local_tm.tm_sec).size;
			}
#endif // OS_WIN

			if (_DirLogEnum::FILE == _dir_log_enum) {
				_p_cache_fname_sze = fmt::format_to_n (_p_cache_fname.begin (), _p_cache_fname.size (), "{}/{}_{:04}-{:02}-{:02}.log"sv,
					_dir_log.at (_dir_log_pos),
					_name_log,
					_p_cache_local_tm.tm_year + 1900,
					_p_cache_local_tm.tm_mon + 1,
					_p_cache_local_tm.tm_mday).size;
			}
		}

		const std::string_view str_time (_p_cache_time.data (), std::min (_p_cache_time_sze, _p_cache_time.size ()));

#ifndef OS_WIN
		if (_DirLogEnum::SYSLOG == _dir_log_enum) {
			::openlog (_name_log.c_str (), LOG_PID | LOG_CONS, LOG_USER);
			::syslog (LOG_NOTICE, message.data (), message.size ());
			::closelog ();
		}
#endif // OS_WIN

		if (_DirLogEnum::STDOUT == _dir_log_enum && false == also_to_stderr) {
			::fwrite (str_time.data (), 1, str_time.size (), stdout);
			::fwrite (prefix.data (), 1, prefix.size (), stdout);
			::fwrite (message.data (), 1, message.size (), stdout);
			::fputc ('\n', stdout);
			::fflush (stdout);
		}

		if (_DirLogEnum::STDERR == _dir_log_enum || true == also_to_stderr) {
			::fwrite (str_time.data (), 1, str_time.size (), stderr);
			::fwrite (prefix.data (), 1, prefix.size (), stderr);
			::fwrite (message.data (), 1, message.size (), stderr);
			::fputc ('\n', stderr);
			::fflush (stderr);
		}

		if (_DirLogEnum::FILE == _dir_log_enum) {
			const std::string_view str_fname (_p_cache_fname.data (), std::min (_p_cache_fname_sze, _p_cache_fname.size ()));

			[[maybe_unused]] bool dumped_message {false};

			_disabled_temporarily = true;
			try {
				if (_printf_file.get_file_name () != str_fname) {
					_printf_file.close ();
				}

				if (_printf_file.is_opened () == false) {
					_printf_file.set_file_name (str_fname);
					_printf_file.set_mode (ShFile::mWRITE | ShFile::mAPPEND);
					_printf_file.set_mask (ShFile::mask644);
					_printf_file.open ();
				}

				if (true == _check_size) {
					const off64_t st_size = _printf_file.get_file_size ();
					if (_limit_soft != 0 && st_size >= _limit_soft) {
						if (false == _soft_limit_reached) {
							_printf_entries.push_back ("Log file is getting too large. Soft limit reached."s);
							_soft_limit_reached = true;
						}
					}

					if (_limit_hard != 0 && st_size >= _limit_hard) {
						_printf_entries.push_back ("Log file is too large. Shutting down!"s);
						_disabled_permanently = true;
					}
				}

				/* Dump message */
				_printf_file.write (str_time);
				_printf_file.write (prefix);
				_printf_file.write (message);
				_printf_file.write ('\n');

				dumped_message = true;

				/* Dump rest of entries */
				while (_printf_entries.empty () == false) {
					_printf_file.write (str_time);
					_printf_file.write (_printf_entries.front ());
					_printf_file.write ('\n');

					_printf_entries.pop_front ();
				}
			}
			catch (...) {
#ifndef OS_WIN
				::openlog (_name_log.c_str (), LOG_PID | LOG_CONS, LOG_USER);

				if (false == dumped_message) {
					::syslog (LOG_NOTICE, message.data (), message.size ());
				}

				for (const std::string &entry : _printf_entries) {
					::syslog (LOG_ERR, entry.data (), entry.size ());
				}
				::closelog ();
#endif // OS_WIN
				_printf_entries.clear ();
			}
			_disabled_temporarily = false;
		}
	}

	/////////////////////////////////////////////////////////////

	void P::debug_set_enabled (const bool enabled) noexcept
	{
		_debug_enabled = enabled;
	}

	bool P::debug_is_enabled (void) noexcept
	{
		return (true == _debug_enabled && true == is_enabled ());
	}

	/////////////////////////////////////////////////////////////

	#ifdef SHAGA_THREADING
	static thread_local bool _commonexception_muted_state {false};
	#else
	static bool _commonexception_muted_state {false};
	#endif // SHAGA_THREADING

	bool commonexception_is_muted (void) noexcept
	{
		return _commonexception_muted_state;
	}

	void commonexception_set_muted (const bool state) noexcept
	{
		_commonexception_muted_state = state;
	}
}

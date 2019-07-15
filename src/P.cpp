/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#include <cstdarg>
#include <climits>
#include <sys/stat.h>

#ifndef OS_WIN
	#include <syslog.h>
#endif // OS_WIN

namespace shaga::P {
	static volatile bool _disabled_permanently {false};

	static const size_t _buffer_size {4'096};
	static const off64_t _bytes_per_mb {1'024 * 1'024};

	static volatile bool _enabled {false};
	static bool _printf_ms {false};

	static bool _check_size {false};
	static bool _soft_limit_reached {false};
	static off64_t _limit_soft {0};
	static off64_t _limit_hard {0};

	static bool _debug_enabled {false};

	#ifdef SHAGA_THREADING
	static std::mutex _printf_mutex;
	static std::mutex _debug_printf_mutex;
	#endif // SHAGA_THREADING

	static std::string _dir_log;
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
}

namespace shaga {
	void P::set_dir_log (const std::string_view var)
	{
		_dir_log.assign (var);
	}

	void P::set_name_log (const std::string_view var)
	{
		_name_log.assign (var);
	}

	void P::set_app_name (const std::string_view var)
	{
		_app_name.assign (var);
	}

	/////////////////////////////////////////////////////////////

	void P::check_size (const bool enabled) noexcept
	{
		_check_size = enabled;
		if (enabled == false) {
			_soft_limit_reached = false;
		}
	}

	void P::set_max_size_mb (const int soft, const int hard) noexcept
	{
		_limit_soft = static_cast<off64_t> (soft) * _bytes_per_mb;
		_limit_hard = static_cast<off64_t> (hard) * _bytes_per_mb;
		_soft_limit_reached = false;
	}

	bool P::soft_limit_reached (void) noexcept
	{
		return _soft_limit_reached;
	}

	/////////////////////////////////////////////////////////////

	void P::set_enabled (const bool enabled)
	{
		if (enabled) {
			if (_dir_log.empty () || _name_log.empty () || _app_name.empty ()) {
				cThrow ("Can't enable P::print, variables are not set."sv);
			}

			if (STR::icompare (_dir_log, "syslog"sv) == true) {
				_dir_log_enum = _DirLogEnum::SYSLOG;
			}
			else if (STR::icompare (_dir_log, "-"sv) == true || STR::icompare (_dir_log, "stdout"sv) == true) {
				_dir_log_enum = _DirLogEnum::STDOUT;
			}
			else if (STR::icompare (_dir_log, "stderr"sv) == true) {
				_dir_log_enum = _DirLogEnum::STDERR;
			}
			else {
				_dir_log_enum = _DirLogEnum::FILE;
			}
		}
		_enabled = enabled;
		_printf_file.close ();
	}

	void P::show_ms (const bool enabled) noexcept
	{
		_printf_ms = enabled;
	}

	bool P::is_enabled (void) noexcept
	{
		return (true == _enabled && false == _disabled_permanently);
	}

	void P::_print (const std::string &message, const char *prefix) noexcept
	{
		_print (message.c_str (), prefix);
	}

	void P::_print (const char *message, const char *prefix) noexcept
	{
		if (false == _enabled || true == _disabled_permanently) {
			return;
		}

		struct tm local_tm;
		std::string _printf_time;

		{
			const uint64_t rt = (true == _printf_ms) ? get_realtime_msec () : 0;
			const time_t theTime = (rt > 0) ? (rt / 1'000) : get_realtime_sec ();

			::memset (&local_tm, 0, sizeof (local_tm));
			::localtime_r (&theTime, &local_tm);

#ifndef OS_WIN
			if (true == _printf_ms) {
				_printf_time = fmt::format ("{:04}-{:02}-{:02} {:02}:{:02}:{:02d}.{:03} {} : "sv,
					local_tm.tm_year + 1900,
					local_tm.tm_mon + 1,
					local_tm.tm_mday,
					local_tm.tm_hour,
					local_tm.tm_min,
					local_tm.tm_sec,
					rt % 1'000,
					local_tm.tm_zone);
			}
			else {
				_printf_time = fmt::format ("{:04}-{:02}-{:02} {:02}:{:02}:{:02d} {} : "sv,
					local_tm.tm_year + 1900,
					local_tm.tm_mon + 1,
					local_tm.tm_mday,
					local_tm.tm_hour,
					local_tm.tm_min,
					local_tm.tm_sec,
					local_tm.tm_zone);
			}
#else
			if (true == _printf_ms) {
				_printf_time = fmt::format ("{:04}-{:02}-{:02} {:02}:{:02}:{:02d}.{:03} : "sv,
					local_tm.tm_year + 1900,
					local_tm.tm_mon + 1,
					local_tm.tm_mday,
					local_tm.tm_hour,
					local_tm.tm_min,
					local_tm.tm_sec,
					rt % 1'000);
			}
			else {
				_printf_time = fmt::format ("{:04}-{:02}-{:02} {:02}:{:02}:{:02d} : "sv,
					local_tm.tm_year + 1900,
					local_tm.tm_mon + 1,
					local_tm.tm_mday,
					local_tm.tm_hour,
					local_tm.tm_min,
					local_tm.tm_sec);
			}
#endif // OS_WIN
		}

		#ifdef SHAGA_THREADING
		// Since we can write logs to non-POSIX filesystems, let's do locking here instead on FS level.
		std::lock_guard<std::mutex> lock (_printf_mutex);
		#endif // SHAGA_THREADING

#ifndef OS_WIN
		if (_DirLogEnum::SYSLOG == _dir_log_enum) {
			::openlog (_name_log.c_str (), LOG_PID | LOG_CONS, LOG_USER);
			::syslog (LOG_NOTICE, message);
			::closelog ();
		}
		else
#endif // OS_WIN
		if (_DirLogEnum::STDOUT == _dir_log_enum) {
			::fputs (_printf_time.c_str (), stdout);
			if (nullptr != prefix) {
				::fputs (prefix, stdout);
			}
			::fputs (message, stdout);
			::fputc ('\n', stdout);
			::fflush (stdout);
		}
		else if (_DirLogEnum::STDERR == _dir_log_enum) {
			::fputs (_printf_time.c_str (), stderr);
			if (nullptr != prefix) {
				::fputs (prefix, stderr);
			}
			::fputs (message, stderr);
			::fputc ('\n', stderr);
			::fflush (stderr);
		}
		else {
			const std::string _printf_fname = fmt::format ("{}/{}_{:04}-{:02}-{:02}.log",
				_dir_log,
				_name_log,
				local_tm.tm_year + 1900,
				local_tm.tm_mon + 1,
				local_tm.tm_mday);

			_printf_entries.push_back (std::string ());

			try {
				if (_printf_file.get_file_name ().compare (_printf_fname) != 0) {
					_printf_file.close ();
				}

				if (_printf_file.is_opened () == false) {
					_printf_file.set_file_name (_printf_fname);
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

				while (_printf_entries.empty () == false) {
					if (_printf_entries.front ().empty () == true) {
						_printf_file.write (_printf_time);
						if (nullptr != prefix) {
							_printf_file.write (prefix);
						}
						_printf_file.write (message);
						_printf_file.write ('\n');
					}
					else {
						_printf_file.write (_printf_time);
						_printf_file.write (_printf_entries.front ());
						_printf_file.write ('\n');
					}
					_printf_entries.pop_front ();
				}

			}
			catch (...) {
#ifndef OS_WIN
				::openlog (_name_log.c_str (), LOG_PID | LOG_CONS, LOG_USER);
				for (const std::string &entry : _printf_entries) {
					if (entry.empty () == true) {
						::syslog (LOG_NOTICE, message);
					}
					else {
						::syslog (LOG_ERR, entry.c_str ());
					}
				}
				_printf_entries.clear ();
				::closelog ();
#endif // OS_WIN
			}
		}
	}

	/////////////////////////////////////////////////////////////
	void P::debug_set_enabled (const bool enabled) noexcept
	{
		_debug_enabled = enabled;
	}

	bool P::debug_is_enabled (void) noexcept
	{
		return (true == _debug_enabled && false == _disabled_permanently);
	}
}

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#include <cstdarg>
#include <climits>
#include <sys/stat.h>

#ifndef OS_WIN
	#include <syslog.h>
#endif // OS_WIN

namespace shaga {

	namespace P {
		static volatile bool _disabled_permanently {false};

		static const int _buffer_size {4096};
		static const off64_t _bytes_per_mb {1024 * 1024};

		static volatile bool _enabled {false};
		static char _printf_buf[_buffer_size];
		static char _printf_time[_buffer_size];
		static char _printf_fname[PATH_MAX];
		static bool _printf_ms {false};

		static bool _check_size {false};
		static bool _soft_limit_reached {false};
		static off64_t _limit_soft {0};
		static off64_t _limit_hard {0};

		static bool _debug_enabled {false};
		static char _debug_printf_buf[_buffer_size];

		#ifdef SHAGA_THREADING
		static std::mutex _printf_mutex;
		static std::mutex _debug_printf_mutex;
		#endif // SHAGA_THREADING

		static std::string _dir_log;
		static std::string _name_log;
		static std::string _app_name;

		static ShFile _printf_file;
	}

	/////////////////////////////////////////////////////////////

	void P::set_dir_log (const std::string &var)
	{
		_dir_log.assign (var);
	}

	void P::set_name_log (const std::string &var)
	{
		_name_log.assign (var);
	}

	void P::set_app_name (const std::string &var)
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
				cThrow ("Can't enable P::printf, variables are not set.");
			}
		}
		_enabled = enabled;

		_printf_file.close ();
	}

	void P::show_ms (const bool enabled) noexcept
	{
		_printf_ms = enabled;
	}

	void P::printf (const char *fmt, ...) noexcept
	{
		if (false == _enabled || true == _disabled_permanently) {
			return;
		}

		#ifdef SHAGA_THREADING
		// Since we can write logs to non-POSIX filesystems, let's do locking here instead on FS level.
		std::lock_guard<std::mutex> lock(_printf_mutex);
		#endif // SHAGA_THREADING

		::va_list ap;
		::va_start (ap, fmt);
		::vsnprintf (_printf_buf, sizeof (_printf_buf) - 1, fmt, ap);
		::va_end (ap);
		_printf_buf[sizeof (_printf_buf) - 1] = '\0';

		const uint64_t rt = (true == _printf_ms) ? get_realtime_msec () : 0;
		const time_t theTime = (rt > 0) ? (rt / 1000) : get_realtime_sec ();

		struct tm t;
		::memset (&t, 0, sizeof (t));
		::localtime_r (&theTime, &t);

#ifndef OS_WIN
		if (true == _printf_ms) {
			::snprintf (_printf_time, sizeof (_printf_time) - 1, "%04d-%02d-%02d %02d:%02d:%02d.%03" PRIu64 " %s : ", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, rt % 1000, t.tm_zone);
		}
		else {
			::snprintf (_printf_time, sizeof (_printf_time) - 1, "%04d-%02d-%02d %02d:%02d:%02d %s : ", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, t.tm_zone);
		}
#else
		if (true == _printf_ms) {
			::snprintf (_printf_time, sizeof (_printf_time) - 1, "%04d-%02d-%02d %02d:%02d:%02d.%03" PRIu64 " : ", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, rt % 1000);
		}
		else {
			::snprintf (_printf_time, sizeof (_printf_time) - 1, "%04d-%02d-%02d %02d:%02d:%02d : ", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
		}
#endif // OS_WIN

		_printf_time[sizeof (_printf_time) - 1] = '\0';

#ifndef OS_WIN
		if (_dir_log == "syslog") {
			::openlog (_name_log.c_str (), LOG_PID | LOG_CONS, LOG_USER);
			::syslog (LOG_NOTICE, _printf_buf);
			::closelog ();
		}
		else
#endif // OS_WIN
		if (_dir_log == "-") {
			::fprintf (stdout, "%s%s\n", _printf_time, _printf_buf);
			::fflush (stdout);
		}
		else {
			::snprintf (_printf_fname, sizeof (_printf_fname), "%s/%s_%04d-%02d-%02d.log", _dir_log.c_str (), _name_log.c_str (), t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

			COMMON_LIST entries;
			entries.push_back (std::string ());

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
							entries.push_back ("Log file is getting too large. Soft limit reached.");
							_soft_limit_reached = true;
						}
					}

					if (_limit_hard != 0 && st_size >= _limit_hard) {
						entries.push_back ("Log file is too large. Shutting down!");
						_disabled_permanently = true;
					}
				}

				for (COMMON_LIST::iterator iter = entries.begin (); iter != entries.end ();) {
					if (iter->empty () == true) {
						_printf_file.write (_printf_time);
						_printf_file.write (_printf_buf);
						_printf_file.write ('\n');
					}
					else {
						_printf_file.write (_printf_time);
						_printf_file.write (*iter);
						_printf_file.write ('\n');
					}
					iter = entries.erase (iter);
				}

			}
			catch (...) {
#ifndef OS_WIN
				::openlog (_name_log.c_str (), LOG_PID | LOG_CONS, LOG_USER);
				for (const std::string &entry : entries) {
					if (entry.empty () == true) {
						::syslog (LOG_NOTICE, _printf_buf);
					}
					else {
						::syslog (LOG_ERR, entry.c_str ());
					}
				}
				::closelog ();
#endif // OS_WIN
			}

		}

	}

	/////////////////////////////////////////////////////////////

	void P::debug_set_enabled (const bool enabled)
	{
		_debug_enabled = enabled;
	}

	void P::debug_printf (const char *fmt, ...) noexcept
	{
		if (false == _debug_enabled || true == _disabled_permanently) {
			return;
		}

		#ifdef SHAGA_THREADING
		std::lock_guard<std::mutex> lock(_debug_printf_mutex);
		#endif // SHAGA_THREADING

		va_list ap;

		::va_start (ap, fmt);
		::vsnprintf (_debug_printf_buf, sizeof (_debug_printf_buf) - 1, fmt, ap);
		::va_end (ap);

		_debug_printf_buf[sizeof (_debug_printf_buf) - 1] = '\0';

		printf ("[DEBUG] %s", _debug_printf_buf);
	}

}

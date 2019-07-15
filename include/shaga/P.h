/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_P
#define HEAD_shaga_P

#include "common.h"

namespace shaga::P {
	void set_dir_log (const std::string_view var);
	void set_name_log (const std::string_view var);
	void set_app_name (const std::string_view var);

	void check_size (const bool enabled) noexcept;
	void set_max_size_mb (const int soft, const int hard) noexcept;
	bool soft_limit_reached (void) noexcept;

	void set_enabled (const bool enabled);
	void show_ms (const bool enabled) noexcept;
	bool is_enabled (void) noexcept;

	void _print (const std::string &message, const char *prefix = nullptr) noexcept;
	void _print (const char *message, const char *prefix = nullptr) noexcept;

	template <typename... Args>
	void print (const char *format, const Args & ... args) noexcept
	{
		if (true == is_enabled ()) {
			if (sizeof...(Args) == 0) {
				_print (format);
			}
			else {
				try {
					_print (fmt::format (format, args...));
				}
				catch (...) {
					_print (format, "(!FORMAT ERROR!) ");
				}
			}
		}
	}

	void debug_set_enabled (const bool enabled) noexcept;
	bool debug_is_enabled (void) noexcept;

	template <typename... Args>
	void debug_print (const char *format, const Args & ... args) noexcept
	{
		if (true == debug_is_enabled ()) {
			if (sizeof...(Args) == 0) {
				_print (format);
			}
			else {
				try {
					_print (fmt::format (format, args...), "[DEBUG] ");
				}
				catch (...) {
					_print (format, "[DEBUG] (!FORMAT ERROR!) ");
				}
			}
		}
	}
}

#endif // HEAD_shaga_P

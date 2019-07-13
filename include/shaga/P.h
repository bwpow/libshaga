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

	void printf (const char *fmt, ...) noexcept;

	void debug_set_enabled (const bool enabled);
	void debug_printf (const char *fmt, ...) noexcept;
}

#endif // HEAD_shaga_P

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_P
#define HEAD_shaga_P

#include "common.h"

namespace shaga {
	namespace P {
		void set_dir_log (const std::string &var);
		void set_name_log (const std::string &var);
		void set_app_name (const std::string &var);

		void check_size (const bool enabled);
		void set_max_size_mb (const int soft, const int hard);
		bool soft_limit_reached (void);

		void set_enabled (const bool enabled);
		void show_ms (const bool enabled);

		void printf (const char *fmt, ...) throw ();

		void debug_set_enabled (const bool enabled);
		void debug_printf (const char *fmt, ...) throw ();
	}
}

#endif // HEAD_shaga_P

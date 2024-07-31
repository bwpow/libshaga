/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2024, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_UNIX
#define HEAD_shaga_UNIX

#include "common.h"

#ifndef OS_WIN

namespace shaga {
	void signal_handler (int sgn);
	void signal_register_handlers (const bool ignore_pipe = false, const bool ignore_hup = false);
}

namespace shaga::UNIX {
	std::string get_hostname (const bool to_lowercase = false, const char specchar = '\0');
	pid_t daemonize (const std::string_view pidfile);

	bool renice (const int prio);
	bool renice (std::shared_ptr<shaga::INI> ini);
	bool renice (const shaga::INI *const ini);
}

#endif // OS_WIN

#endif // HEAD_shaga_UNIX

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2022, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_WIN
#define HEAD_shaga_WIN

#include "common.h"

#ifdef OS_WIN

namespace shaga {
	void signal_handler (int sgn);
	void signal_register_handlers (const bool ignore_pipe = false, const bool ignore_hup = false);
}

#endif // OS_WIN

#endif // HEAD_shaga_WIN

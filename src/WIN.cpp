/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifdef OS_WIN

#include <signal.h>

namespace shaga {

	void signal_handler (int sgn)
	{
		if (SIGINT == sgn) {
			P::print ("Caught signal: CTRL+C signal"sv);
		}
		else if (SIGTERM == sgn) {
			P::print ("Caught signal: Termination request"sv);
		}
		else {
			P::print ("Caught signal: {}"sv, sgn);
		}
		TRY_TO_SHUTDOWN ();
	}

	void signal_register_handlers ([[maybe_unused]] const bool ignore_pipe, [[maybe_unused]] const bool ignore_hup)
	{
		::signal (SIGINT, signal_handler);
		::signal (SIGTERM, signal_handler);
	}

}
#endif // OS_WIN

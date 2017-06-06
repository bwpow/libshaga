/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifdef OS_WIN

#include <signal.h>

namespace shaga {

	void signal_handler (int sgn)
	{
		if (SIGINT == sgn) {
			P::printf ("Caught signal: CTRL+C signal");
		}
		else if (SIGTERM == sgn) {
			P::printf ("Caught signal: Termination request");
		}
		else {
			P::printf ("Caught signal: %d", sgn);
		}
		TRY_TO_SHUTDOWN ();
	}

	void signal_register_handlers (const bool ignore_pipe, const bool ignore_hup)
	{
		::signal (SIGINT, signal_handler);
		::signal (SIGTERM, signal_handler);

		(void) ignore_pipe;
		(void) ignore_hup;
	}

}
#endif // OS_WIN

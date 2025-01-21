/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

extern "C" {
	const char *__asan_default_options (void)
	{
		return "debug=1:atexit=1";
	}
}

int main (int argc, char **argv) try
{
	::testing::InitGoogleTest (&argc, argv);
	shaga::shaga_check ();
	return RUN_ALL_TESTS ();
}
catch (const std::exception &e) {
	shaga::exit ("FATAL ERROR: {}", e.what ());
}
catch (...) {
	shaga::exit ("FATAL ERROR: Unknown failure");
}

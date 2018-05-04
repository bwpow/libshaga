/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <shaga.h>
#include <gtest/gtest.h>

int main(int argc, char **argv) try
{
	::testing::InitGoogleTest(&argc, argv);
	shaga::shaga_check_threading ();
	shaga::BIN::endian_detect ();
	return RUN_ALL_TESTS();
}
catch (const std::exception &e) {
	shaga::exit ("FATAL ERROR: %s", e.what ());
}
catch (...) {
	shaga::exit ("FATAL ERROR: Unknown failure");
}

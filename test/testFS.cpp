/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

TEST (FS, is_dir)
{
	EXPECT_TRUE (FS::is_dir ("./test"sv));
	EXPECT_FALSE (FS::is_dir ("./test/testFS.cpp"sv));
	EXPECT_FALSE (FS::is_dir ("./test/test"sv));
}

TEST (FS, is_file)
{
	EXPECT_FALSE (FS::is_file ("./test"sv));
	EXPECT_TRUE (FS::is_file ("./test/testFS.cpp"sv));
	EXPECT_FALSE (FS::is_file ("./test/test"sv));
}

TEST (FS, glob)
{
	COMMON_VECTOR lst;
	EXPECT_NO_THROW (FS::glob ("./test/*.cpp"sv, lst));
	ASSERT_FALSE (lst.empty ());
}

TEST (FS, read_file)
{
	COMMON_LIST l;

	EXPECT_NO_THROW (FS::read_file ("./test/testFS.cpp"sv, l));
	ASSERT_FALSE (l.empty ());

	EXPECT_THROW (FS::read_file ("./test/test"sv, l), CommonException);
}

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <shaga.h>
#include <gtest/gtest.h>

using namespace shaga;

TEST (FS, is_dir)
{
	EXPECT_TRUE (FS::is_dir ("./test"));
	EXPECT_FALSE (FS::is_dir ("./test/testFS.cpp"));
	EXPECT_FALSE (FS::is_dir ("./test/test"));
}

TEST (FS, is_file)
{
	EXPECT_FALSE (FS::is_file ("./test"));
	EXPECT_TRUE (FS::is_file ("./test/testFS.cpp"));
	EXPECT_FALSE (FS::is_file ("./test/test"));
}

TEST (FS, glob)
{
	COMMON_VECTOR lst;
	EXPECT_NO_THROW (FS::glob ("./test/*.cpp", lst));
	ASSERT_FALSE (lst.empty ());
}

TEST (FS, read_file)
{
	COMMON_LIST l;

	EXPECT_NO_THROW (FS::read_file ("./test/testFS.cpp", l));

	ASSERT_FALSE (l.empty ());

	EXPECT_THROW (FS::read_file (std::string_view ("./test/test"), l), CommonException);
}

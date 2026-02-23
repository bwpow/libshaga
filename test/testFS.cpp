/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>
#include <filesystem>

using namespace shaga;

namespace {
	std::string make_temp_path (void)
	{
		static uint64_t seq = 0;
		++seq;
		return fmt::format ("./test/.tmp_fs_{}_{}"sv, static_cast<uint64_t> (std::time (nullptr)), seq);
	}

	class TempPathGuard {
		private:
			std::string _path;

		public:
			explicit TempPathGuard (std::string path)
				: _path (path)
			{ }

			~TempPathGuard ()
			{
				std::error_code ec;
				std::filesystem::remove (_path, ec);
			}

			const std::string &path (void) const
			{
				return _path;
			}
	};
}

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

TEST (FS, read_file_last_line_without_newline)
{
	TempPathGuard tmp (make_temp_path ());

	{
		ShFile out (tmp.path (), ShFile::mW);
		out.write ("first\nsecond"sv);
	}

	COMMON_VECTOR lines;
	EXPECT_NO_THROW (FS::read_file (tmp.path (), lines));
	ASSERT_EQ (lines.size (), 2U);
	EXPECT_EQ (lines[0], "first"sv);
	EXPECT_EQ (lines[1], "second"sv);
}

TEST (FS, file_size_optional)
{
	const auto existing = FS::file_size_optional ("./test/testFS.cpp"sv);
	ASSERT_TRUE (existing.has_value ());
	EXPECT_GT (*existing, 0);

	const auto missing = FS::file_size_optional ("./test/does_not_exist_123456"sv);
	EXPECT_FALSE (missing.has_value ());
}

TEST (FS, mkdir_existing_directory)
{
	EXPECT_TRUE (FS::mkdir ("./test"sv));
}

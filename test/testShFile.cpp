/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

namespace {
	std::string make_temp_path (void)
	{
		static uint64_t seq = 0;
		++seq;
		return fmt::format ("./test/.tmp_shfile_{}_{}.bin"sv, static_cast<uint64_t> (std::time (nullptr)), seq);
	}

	class TempFileGuard {
		private:
			std::string _path;

		public:
			explicit TempFileGuard (std::string path)
				: _path (path)
			{ }

			~TempFileGuard ()
			{
				try {
					if (FS::is_file (_path)) {
						::unlink (_path.c_str ());
					}
				}
				catch (...) { }
			}

			const std::string &path (void) const
			{
				return _path;
			}
	};
}

TEST (ShFile, WriteReadRoundtrip)
{
	TempFileGuard tmp (make_temp_path ());

	{
		ShFile out (tmp.path (), ShFile::mW);
		out.write ("hello world"sv);
	}

	{
		ShFile in (tmp.path (), ShFile::mREAD);
		const std::string data = in.read_whole_file ();
		EXPECT_EQ (data, "hello world"sv);
	}
}

TEST (ShFile, LockApiBasicSemantics)
{
	TempFileGuard tmp (make_temp_path ());

	ShFile f (tmp.path (), ShFile::mRW);
	EXPECT_NO_THROW (f.lock ());
	EXPECT_TRUE (f.try_lock ());
	EXPECT_NO_THROW (f.unlock ());
	EXPECT_NO_THROW (f.unlock ());
}

TEST (ShFile, LockApiRequiresOpen)
{
	ShFile f;
	EXPECT_THROW (f.lock (), CommonException);
	EXPECT_THROW (f.try_lock (), CommonException);
	EXPECT_NO_THROW (f.unlock ());
}

TEST (ShFile, SetFileMtime)
{
	TempFileGuard tmp (make_temp_path ());

	ShFile f (tmp.path (), ShFile::mRW);
	f.write ("x"sv);
	const time_t target = std::time (nullptr) - 3600;

	EXPECT_NO_THROW (f.set_file_mtime (target));
	const time_t got = f.get_file_mtime ();
	EXPECT_GE (got, target - 2);
	EXPECT_LE (got, target + 2);
}

TEST (ShFile, SetFileTimes)
{
	TempFileGuard tmp (make_temp_path ());

	ShFile f (tmp.path (), ShFile::mRW);
	f.write ("x"sv);
	const time_t atime = std::time (nullptr) - 7200;
	const time_t mtime = std::time (nullptr) - 1800;

	EXPECT_NO_THROW (f.set_file_times (atime, mtime));
	const time_t got_mtime = f.get_file_mtime ();
	EXPECT_GE (got_mtime, mtime - 2);
	EXPECT_LE (got_mtime, mtime + 2);
}

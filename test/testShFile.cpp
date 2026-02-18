/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>
#ifndef OS_WIN
	#include <sys/stat.h>
#endif // OS_WIN

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

TEST (ShFile, RenameOnCloseAppliesMask)
{
	TempFileGuard tmp (make_temp_path ());
	TempFileGuard final (tmp.path () + ".final");

	{
		ShFile f;
		f.set_mask (ShFile::mask600);
		f.set_file_name (tmp.path (), ShFile::mWRITE | ShFile::mTRUNC);
		f.set_rename_on_close_file_name (final.path ());
		f.open ();
		f.write ("abc"sv);
	}

	ShFile chk (final.path (), ShFile::mREAD);
	const struct stat st = chk.get_stat ();
	EXPECT_EQ (st.st_mode & 0777, static_cast<mode_t> (0600));
}

#ifndef OS_WIN
TEST (ShFile, NewFileRespectsMaskEvenWithUmask)
{
	TempFileGuard tmp (make_temp_path ());

	const mode_t old_umask = ::umask (0022);
	try {
		{
			ShFile f;
			f.set_mask (ShFile::mask666);
			f.set_file_name (tmp.path (), ShFile::mWRITE | ShFile::mTRUNC);
			f.open ();
			f.write ("x"sv);
		}

		ShFile chk (tmp.path (), ShFile::mREAD);
		const struct stat st = chk.get_stat ();
		EXPECT_EQ (st.st_mode & 0777, static_cast<mode_t> (0666));
	}
	catch (...) {
		::umask (old_umask);
		throw;
	}
	::umask (old_umask);
}
#endif // OS_WIN

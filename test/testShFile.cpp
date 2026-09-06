/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>
#ifndef OS_WIN
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <dirent.h>
#endif // OS_WIN

using namespace shaga;

static_assert (noexcept (std::declval<const ShFile &> ().has_file_name ()));
static_assert (noexcept (std::declval<const ShFile &> ().get_file_descriptor ()));

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

#ifndef OS_WIN
	size_t count_open_descriptors (void)
	{
		DIR *const dir = ::opendir ("/proc/self/fd");
		if (nullptr == dir) {
			throw std::runtime_error ("Unable to inspect open descriptors");
		}
		size_t count {0};
		while (nullptr != ::readdir (dir)) {
			++count;
		}
		::closedir (dir);
		return count;
	}
#endif
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

#ifndef OS_WIN
TEST (ShFile, CloseAlwaysReleasesDescriptorWhenCallbackThrows)
{
	TempFileGuard tmp (make_temp_path ());
	ShFile file (tmp.path (), ShFile::mW);
	const int fd = file.get_file_descriptor ();
	int close_callbacks {0};
	file.set_callback ([&close_callbacks](ShFile &, const ShFile::CallbackAction action) -> bool {
		if (ShFile::CallbackAction::CLOSE == action) {
			++close_callbacks;
			throw std::runtime_error ("injected close callback failure");
		}
		return true;
	});

	file.close ();
	EXPECT_EQ (1, close_callbacks);
	errno = 0;
	EXPECT_EQ (-1, ::fcntl (fd, F_GETFD));
	EXPECT_EQ (EBADF, errno);
	file.close ();
	EXPECT_EQ (1, close_callbacks);
}

TEST (ShFile, DestructorReleasesDescriptorWhenCallbackThrows)
{
	TempFileGuard tmp (make_temp_path ());
	int fd {-1};
	{
		auto file = std::make_unique<ShFile> (tmp.path (), ShFile::mW);
		fd = file->get_file_descriptor ();
		file->set_callback ([](ShFile &, const ShFile::CallbackAction action) -> bool {
			if (ShFile::CallbackAction::CLOSE == action) {
				throw std::runtime_error ("injected destructor callback failure");
			}
			return true;
		});
	}
	errno = 0;
	EXPECT_EQ (-1, ::fcntl (fd, F_GETFD));
	EXPECT_EQ (EBADF, errno);
}

TEST (ShFile, ReadJsonRewindsAndPreservesLogicalPosition)
{
	TempFileGuard tmp (make_temp_path ());
	const std::string json_text {R"({"answer":42})"};
	{
		ShFile out (tmp.path (), ShFile::mW);
		out.write (json_text);
	}

	ShFile file (tmp.path (), ShFile::mREAD);
	EXPECT_EQ ("{\""sv, file.read (2));
	const nlohmann::json parsed = file.read_json (true);
	EXPECT_EQ (42, parsed.at ("answer"));
	EXPECT_EQ (static_cast<off64_t> (json_text.size ()), file.tell ());

	file.seek (2);
	const off64_t before_error = file.tell ();
	EXPECT_THROW ((void) file.read_json (false), CommonException);
	EXPECT_EQ (before_error, file.tell ());
}

TEST (ShFile, ReadJsonFdopenAndParseFailuresDoNotLeakDescriptors)
{
	TempFileGuard write_only_path (make_temp_path ());
	ShFile write_only (write_only_path.path (), ShFile::mW);
	const size_t before_fdopen = count_open_descriptors ();
	EXPECT_THROW ((void) write_only.read_json (false), CommonException);
	EXPECT_EQ (before_fdopen, count_open_descriptors ());

	TempFileGuard invalid_path (make_temp_path ());
	{
		ShFile out (invalid_path.path (), ShFile::mW);
		out.write ("not-json"sv);
	}
	ShFile invalid (invalid_path.path (), ShFile::mREAD);
	const size_t before_parse = count_open_descriptors ();
	for (size_t i = 0; i < 32; ++i) {
		EXPECT_THROW ((void) invalid.read_json (true), CommonException);
		EXPECT_EQ (0, invalid.tell ());
	}
	EXPECT_EQ (before_parse, count_open_descriptors ());
}
#endif // OS_WIN

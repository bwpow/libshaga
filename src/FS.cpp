/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifndef OS_WIN
	#include <glob.h>
#endif // OS_WIN

#include <utime.h>

namespace shaga {

	std::string FS::realpath ([[maybe_unused]] const std::string_view path, [[maybe_unused]] const bool strip_fname)
	{
#ifndef OS_WIN
		char *pth = ::realpath (s_c_str (path), nullptr);
		if (nullptr == pth) {
			return std::string ();
		}
		std::string out (pth);
		::free (pth);

		if (true == strip_fname) {
			const size_t pos = out.find_last_of ('/');
			if (pos != std::string::npos) {
				out.erase (pos + 1);
			}
		}

		return out;
#else
		cThrow ("This function is not supported in this OS"sv);
#endif // OS_WIN
	}

	struct stat FS::file_stat (const std::string_view fname)
	{
		struct stat st;
		if (::stat (s_c_str (fname), &st) != 0) {
			cThrow ("Unable to get file stat of '{}': {}"sv, fname, strerror (errno));
		}
		return st;
	}

	off64_t FS::file_size (const std::string_view fname)
	{
		struct stat st;
		if (::stat (s_c_str (fname), &st) != 0) {
			cThrow ("Unable to get file size of '{}'"sv, fname);
		}
		return st.st_size;
	}

	std::optional<off64_t> FS::file_size_optional (const char *const fname)
	{
		struct stat st;
		if (::stat (s_c_str (fname), &st) != 0) {
			return std::nullopt;
		}
		return st.st_size;
	}

	time_t FS::file_mtime (const std::string_view fname)
	{
		struct stat st;
		if (::stat (s_c_str (fname), &st) != 0) {
			cThrow ("Unable to get file mtime of '{}'"sv, fname);
		}
		return st.st_mtime;
	}

	std::optional<time_t> FS::file_mtime_optional (const std::string_view fname)
	{
		struct stat st;
		if (::stat (s_c_str (fname), &st) != 0) {
			return std::nullopt;
		}
		return st.st_mtime;
	}

	bool FS::is_dir (const std::string_view dname)
	{
		struct stat st;
		if (::stat (s_c_str (dname), &st) != 0) {
			return false;
		}
		if (!S_ISDIR (st.st_mode)) {
			return false;
		}
		return true;
	}

	bool FS::is_file (const std::string_view fname)
	{
		struct stat st;
		if (::stat (s_c_str (fname), &st) != 0) {
			return false;
		}
		if (!S_ISREG (st.st_mode)) {
			return false;
		}
		return true;
	}

	bool FS::mkdir (const std::string_view dname)
	{
		if (is_dir (dname)) {
			return true;
		}
#ifdef OS_WIN
		if (::mkdir (s_c_str (dname)) != 0) {
			return false;
		}
#else
		::umask (0);
		if (::mkdir (s_c_str (dname), 0755) != 0) {
			return false;
		}
#endif // OS_WIN
		return is_dir (dname);
	}

	int FS::unlink (const std::string_view fname, const bool should_throw)
	{
		const int ret = ::unlink (s_c_str (fname));
		if (ret != 0 && true == should_throw) {
			cThrow ("Unable to unlink '{}': {}"sv, fname, strerror (errno));
		}
		return ret;
	}

	int FS::rename (const std::string_view old_fname, const std::string_view new_fname, const bool should_throw)
	{
		const int ret = ::rename (s_c_str (old_fname), s_c_str (new_fname));
		if (ret != 0 && true == should_throw) {
			cThrow ("Unable to rename '{}' to '{}': {}"sv, old_fname, new_fname, strerror (errno));
		}
		return ret;
	}

	int FS::stat (const std::string_view fname, struct stat *buf, const bool should_throw)
	{
		const int ret = ::stat (s_c_str (fname), buf);
		if (ret != 0 && true == should_throw) {
			cThrow ("Unable to stat '{}': {}"sv, fname, strerror (errno));
		}
		return ret;
	}

	void FS::touch (const std::string_view fname, const time_t timestamp)
	{
		int ret;
		if (!is_file (fname)) {
			ShFile file (fname, ShFile::mBRANDNEW);
		}
		if (0 == timestamp || std::numeric_limits<time_t>::max() == timestamp) {
			ret = ::utime (s_c_str (fname), nullptr);
		}
		else {
			struct utimbuf ts;
			::memset (&ts, 0, sizeof (ts));
			ts.actime = timestamp;
			ts.modtime = timestamp;
			ret = ::utime (s_c_str (fname), &ts);
		}
		if (0 != ret) {
			cThrow ("Unable to touch '{}': {}"sv, fname, strerror (errno));
		}
	}

	void FS::glob ([[maybe_unused]] const std::string_view pattern, [[maybe_unused]] GLOB_CALLBACK callback)
	{
#ifndef OS_WIN
		glob_t glob_result;
		try {
			glob_result.gl_offs = 0;
			const int ret = ::glob (s_c_str (pattern), GLOB_BRACE | GLOB_DOOFFS, NULL, &glob_result);
			if (ret != 0) {
				if (ret == GLOB_NOMATCH) {
					::globfree (&glob_result);
					return;
				}
				else if (ret == GLOB_NOSPACE) {
					cThrow ("glob error for pattern '{}': Out of memory"sv, pattern);
				}
				else if (ret == GLOB_ABORTED ) {
					cThrow ("glob error for pattern '{}': Read error"sv, pattern);
				}
				cThrow ("glob error for pattern '{}': Unknown error"sv, pattern);
			}
			for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
				callback (glob_result.gl_pathv[i]);
			}
			::globfree (&glob_result);
		}
		catch (...) {
			::globfree (&glob_result);
			throw;
		}
#else
		cThrow ("This function is not supported in this OS"sv);
#endif // OS_WIN
	}

	void FS::read_file (const std::string_view fname, READ_FILE_CALLBACK callback)
	{
		std::ifstream infile;

		infile.open (std::string (fname), std::ifstream::in);
		if (infile.is_open () == false) {
			cThrow ("Unable to open file '{}'"sv, fname);
		}

		std::string line;
		while (true) {
			std::getline (infile, line);
			if (infile.good () == false) {
				break;
			}
			callback (line);
		}

		if (infile.fail () == true && infile.eof () == false) {
			cThrow ("Error reading from file '{}'"sv, fname);
		}
	}
}

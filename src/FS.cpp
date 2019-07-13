/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifndef OS_WIN
	#include <glob.h>
#endif // OS_WIN

namespace shaga {

	std::string FS::realpath ([[maybe_unused]] const std::string_view path, [[maybe_unused]] const bool strip_fname)
	{
#ifndef OS_WIN
		char *pth = ::realpath (~path, nullptr);
		if (nullptr == pth) {
			return std::string ();
		}
		std::string out (pth);
		::free (pth);

		if (true == strip_fname) {
			const size_t pos = out.find_last_of ("/");
			if (pos != std::string::npos) {
				out.erase (pos + 1);
			}
		}

		return out;
#else
		cThrow ("This function is not supported in this OS");
#endif // OS_WIN
	}

	struct stat FS::file_stat (const std::string_view fname)
	{
		struct stat st;
		if (::stat (~fname, &st) != 0) {
			cThrow ("Unable to get file stat of '%s'", ~fname);
		}

		return st;
	}

	off64_t FS::file_size (const std::string_view fname)
	{
		struct stat st;
		if (::stat (~fname, &st) != 0) {
			cThrow ("Unable to get file size of '%s'", ~fname);
		}

		return st.st_size;
	}

	time_t FS::file_mtime (const std::string_view fname)
	{
		struct stat st;
		if (::stat (~fname, &st) != 0) {
			cThrow ("Unable to get file mtime of '%s'", ~fname);
		}

		return st.st_mtime;
	}

	bool FS::is_dir (const std::string_view dname)
	{
		struct stat st;
		if (::stat (~dname, &st) != 0) {
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
		if (::stat (~fname, &st) != 0) {
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
		if (::mkdir (~dname) != 0) {
			return false;
		}
#else
		::umask (0);
		if (::mkdir (~dname, 0755) != 0) {
			return false;
		}
#endif // OS_WIN
		if (is_dir (dname)) {
			return true;
		}
		return false;
	}

	void FS::glob ([[maybe_unused]] const std::string_view pattern, [[maybe_unused]] std::function<void (const std::string_view)> callback)
	{
#ifndef OS_WIN
		glob_t glob_result;
		try {
			glob_result.gl_offs = 0;
			const int ret = ::glob (~pattern, GLOB_BRACE | GLOB_DOOFFS, NULL, &glob_result);
			if (ret != 0) {
				if (ret == GLOB_NOMATCH) {
					::globfree (&glob_result);
					return;
				}
				else if (ret == GLOB_NOSPACE) {
					cThrow ("glob error for pattern '%s': Out of memory", ~pattern);
				}
				else if (ret == GLOB_ABORTED ) {
					cThrow ("glob error for pattern '%s': Read error", ~pattern);
				}
				cThrow ("glob error for pattern '%s': Unknown error", ~pattern);
			}
			for(size_t i = 0 ; i < glob_result.gl_pathc; i++) {
				callback (glob_result.gl_pathv[i]);
			}
			::globfree (&glob_result);
		}
		catch (...) {
			::globfree (&glob_result);
			throw;
		}
#else
		cThrow ("This function is not supported in this OS");
#endif // OS_WIN
	}

	void FS::read_file (const std::string_view fname, std::function<void (const std::string_view)> callback)
	{
		std::ifstream infile;

		infile.open (std::string (fname), std::ifstream::in);
		if (infile.is_open () == false) {
			cThrow ("Unable to open file '%s'", ~fname);
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
			cThrow ("Error reading from file '%s'", ~fname);
		}
	}
}

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifndef OS_WIN
	#include <glob.h>
#endif // OS_WIN

namespace shaga {

	std::string FS::realpath (const std::string &path, const bool strip_fname)
	{
#ifndef OS_WIN
		char *pth = ::realpath (path.c_str (), nullptr);
		if (nullptr == pth) {
			return std::string ();
		}
		std::string out (pth);
		::free (pth);

		if (strip_fname == true) {
			const size_t pos = out.find_last_of ("/");
			if (pos != std::string::npos) {
				out.erase (pos + 1);
			}
		}

		return out;
#else
		(void) path;
		(void) strip_fname;
		cThrow ("This function is not supported in this OS");
#endif // OS_WIN
	}

	struct stat FS::file_stat (const std::string &fname)
	{
		struct stat st;
		if (::stat (fname.c_str (), &st) != 0) {
			cThrow ("Unable to get file stat of '%s'", fname.c_str ());
		}

		return st;
	}

	off64_t FS::file_size (const std::string &fname)
	{
		struct stat st;
		if (::stat (fname.c_str (), &st) != 0) {
			cThrow ("Unable to get file size of '%s'", fname.c_str ());
		}

		return st.st_size;
	}

	time_t FS::file_mtime (const std::string &fname)
	{
		struct stat st;
		if (::stat (fname.c_str (), &st) != 0) {
			cThrow ("Unable to get file mtime of '%s'", fname.c_str ());
		}

		return st.st_mtime;
	}

	bool FS::is_dir (const std::string &dname)
	{
		struct stat st;
		if (::stat (dname.c_str (), &st) != 0) {
			return false;
		}
		if (!S_ISDIR (st.st_mode)) {
			return false;
		}
		return true;
	}

	bool FS::is_file (const std::string &fname)
	{
		struct stat st;
		if (::stat (fname.c_str (), &st) != 0) {
			return false;
		}
		if (!S_ISREG (st.st_mode)) {
			return false;
		}
		return true;
	}

	bool FS::mkdir (const std::string &dname)
	{
		if (is_dir (dname)) {
			return true;
		}
#ifdef OS_WIN
		if (::mkdir (dname.c_str ()) != 0) {
			return false;
		}
#else
		::umask (0);
		if (::mkdir (dname.c_str (), 0755) != 0) {
			return false;
		}
#endif // OS_WIN
		if (is_dir (dname)) {
			return true;
		}
		return false;
	}

	void FS::glob (const std::string &pattern, std::function<void (const std::string &)> f)
	{
#ifndef OS_WIN
		glob_t glob_result;
		try {
			glob_result.gl_offs = 0;
			const int ret = ::glob (pattern.c_str (), GLOB_BRACE | GLOB_DOOFFS, NULL, &glob_result);
			if (ret != 0) {
				if (ret == GLOB_NOMATCH) {
					::globfree (&glob_result);
					return;
				}
				else if (ret == GLOB_NOSPACE) {
					cThrow ("glob error for pattern '%s': Out of memory", pattern.c_str ());
				}
				else if (ret == GLOB_ABORTED ) {
					cThrow ("glob error for pattern '%s': Read error", pattern.c_str ());
				}
				cThrow ("glob error for pattern '%s': Unknown error", pattern.c_str ());
			}
			for(size_t i = 0 ; i < glob_result.gl_pathc; i++) {
				f (glob_result.gl_pathv[i]);
			}
			::globfree (&glob_result);
		}
		catch (...) {
			::globfree (&glob_result);
			throw;
		}
#else
		(void) pattern;
		(void) f;
		cThrow ("This function is not supported in this OS");
#endif // OS_WIN
	}

	COMMON_LIST FS::glob_to_list (const std::string &pattern)
	{
		COMMON_LIST out;
		glob (pattern, out);
		return out;
	}

	COMMON_VECTOR FS::glob_to_vector (const std::string &pattern)
	{
		COMMON_VECTOR out;
		glob (pattern, out);
		return out;
	}

	COMMON_DEQUE FS::glob_to_deque (const std::string &pattern)
	{
		COMMON_DEQUE out;
		glob (pattern, out);
		return out;
	}

	void FS::read_file (const std::string &fname, std::function<void (const std::string &)> f)
	{
		std::ifstream infile;

		infile.open (fname, std::ifstream::in);
		if (infile.is_open () == false) {
			cThrow ("Unable to open file '%s'", fname.c_str ());
		}

		std::string line;
		while (true) {
			std::getline (infile, line);
			if (infile.good () == false) {
				break;
			}
			f (line);
		}

		if (infile.fail () == true && infile.eof () == false) {
			cThrow ("Error reading from file '%s'", fname.c_str ());
		}
	}

	COMMON_LIST FS::read_file_to_list (const std::string &fname)
	{
		COMMON_LIST out;
		read_file (fname, out);
		return out;
	}

	COMMON_VECTOR FS::read_file_to_vector (const std::string &fname)
	{
		COMMON_VECTOR out;
		read_file (fname, out);
		return out;
	}

	COMMON_DEQUE FS::read_file_to_deque (const std::string &fname)
	{
		COMMON_DEQUE out;
		read_file (fname, out);
		return out;
	}

}

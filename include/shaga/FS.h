/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_FS
#define HEAD_shaga_FS

#include "common.h"

namespace shaga::FS {
	typedef std::function<void (const std::string_view)> GLOB_CALLBACK;
	typedef std::function<void (const std::string_view)> READ_FILE_CALLBACK;

	std::string realpath (const std::string_view path, const bool strip_fname = false);

	struct stat file_stat (const std::string_view fname);

	off64_t file_size (const std::string_view fname);

	time_t file_mtime (const std::string_view fname);

	bool is_dir (const std::string_view dname);

	bool is_file (const std::string_view fname);

	bool mkdir (const std::string_view dname);

	void unlink (const std::string_view fname);

	void glob (const std::string_view pattern, GLOB_CALLBACK callback);

	template <typename T>
	void glob (const std::string_view pattern, T &out)
	{
		glob (pattern, [&out](const std::string_view entry) { out.emplace_back (entry); });
	}

	template <typename T>
	auto glob (const std::string_view pattern) -> T
	{
		T out;
		glob (pattern, [&out](const std::string_view entry) { out.emplace_back (entry); });
		return out;
	}

	void read_file (const std::string_view fname, READ_FILE_CALLBACK callback);

	template <typename T>
	void read_file (const std::string_view fname, T &out)
	{
		read_file (fname, [&out](const std::string_view line) { out.emplace_back (line); });
	}

	template <typename T>
	auto read_file (const std::string_view fname) -> T
	{
		T out;
		read_file (fname, [&out](const std::string_view line) { out.emplace_back (line); });
		return out;
	}
}

#endif // HEAD_shaga_FS

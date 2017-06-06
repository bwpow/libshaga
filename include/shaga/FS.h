/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_FS
#define HEAD_shaga_FS

#include "common.h"

namespace shaga {
	namespace FS {

		std::string realpath (const std::string &path, const bool strip_fname = false);

		off64_t file_size (const std::string &fname);

		bool is_dir (const std::string &dname);
		bool is_file (const std::string &fname);
		bool mkdir (const std::string &dname);

		void glob (const std::string &pattern, std::function<void (const std::string &)> f);

		template <typename T>
		void glob (const std::string &pattern, T &out)
		{
			glob (pattern, [&out](const std::string &entry) { out.push_back (entry); });
		}

		COMMON_LIST glob_to_list (const std::string &pattern);
		COMMON_VECTOR glob_to_vector (const std::string &pattern);
		COMMON_DEQUE glob_to_deque (const std::string &pattern);

		void read_file (const std::string &fname, std::function<void (const std::string &)> f);

		template <typename T>
		void read_file (const std::string &fname, T &out)
		{
			read_file (fname, [&out](const std::string &line) { out.push_back (line); });
		}

		COMMON_LIST read_file_to_list (const std::string &fname);
		COMMON_VECTOR read_file_to_vector (const std::string &fname);
		COMMON_DEQUE read_file_to_deque (const std::string &fname);

	}
}

#endif // HEAD_shaga_FS

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_LINUX
#define HEAD_shaga_LINUX

#include "common.h"

#ifdef OS_LINUX

namespace shaga::LINUX {
	void add_to_epoll (int sock, const uint32_t ev, int epoll_fd, const bool should_modify_if_exists = false);
	void modify_epoll (int sock, const uint32_t ev, int epoll_fd);
	void remove_from_epoll (int sock, int epoll_fd, const bool ignore_failure = false);

	SHAGA_NODISCARD bool login_to_uid (const std::string_view text, uid_t &uid);
	SHAGA_NODISCARD bool group_to_gid (const std::string_view text, gid_t &gid);
}

#endif // OS_LINUX

#endif // HEAD_shaga_LINUX

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2024, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_LINUX
#define HEAD_shaga_LINUX

#include "common.h"

#ifdef OS_LINUX

namespace shaga::LINUX {
	/* Type:
		0 - Allow any type of namespace to be joined.
		CLONE_NEWCGROUP (since Linux 4.6) - fd must refer to a cgroup namespace.
		CLONE_NEWIPC (since Linux 3.0) - fd must refer to an IPC namespace.
		CLONE_NEWNET (since Linux 3.0) - fd must refer to a network namespace.
		CLONE_NEWNS (since Linux 3.8) - fd must refer to a mount namespace.
		CLONE_NEWPID (since Linux 3.8) - fd must refer to a descendant PID namespace.
		CLONE_NEWUSER (since Linux 3.8) - fd must refer to a user namespace.
		CLONE_NEWUTS (since Linux 3.0) - fd must refer to a UTS namespace.

		If pid == 0, /proc/self/ will be used
		name can start with 'pid:'
	*/
	UNIQUE_SOCKET get_namespace_fd_by_pid (const int pid, const int type);
	UNIQUE_SOCKET get_namespace_fd_by_name (const std::string_view name, const int type);
	UNIQUE_SOCKET get_self_namespace_fd (const int type);
	void set_namespace (const int fd, const int type);
	void set_namespace (const std::string_view name, const int type);

	void add_to_epoll (int sock, const uint32_t ev, int epoll_fd, const bool should_modify_if_exists = false);
	void modify_epoll (int sock, const uint32_t ev, int epoll_fd);
	void remove_from_epoll (int sock, int epoll_fd, const bool ignore_failure = false);

	HEDLEY_WARN_UNUSED_RESULT bool login_to_uid (const std::string_view text, uid_t &uid);
	HEDLEY_WARN_UNUSED_RESULT bool group_to_gid (const std::string_view text, gid_t &gid);
}

#endif // OS_LINUX

#endif // HEAD_shaga_LINUX

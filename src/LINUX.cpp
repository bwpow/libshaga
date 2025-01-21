/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifdef OS_LINUX

#include <sys/epoll.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>

namespace shaga {

	UNIQUE_SOCKET LINUX::get_namespace_fd_by_pid (const int pid, const int type)
	{
		std::string fname ("/proc/"sv);
		if (0 == pid) {
			fname.append ("self"sv);
		}
		else {
			fname.append (STR::from_int (pid));
		}
		fname.append ("/ns/"sv);

		switch (type) {
			#ifdef CLONE_NEWCGROUP
			case CLONE_NEWCGROUP:
				fname.append ("cgroup"sv);
				break;
			#endif // CLONE_NEWCGROUP

			#ifdef CLONE_NEWIPC
			case CLONE_NEWIPC:
				fname.append ("ipc"sv);
				break;
			#endif // CLONE_NEWIPC

			#ifdef CLONE_NEWNET
			case CLONE_NEWNET:
				fname.append ("net"sv);
				break;
			#endif // CLONE_NEWNET

			#ifdef CLONE_NEWNS
			case CLONE_NEWNS:
				fname.append ("mnt"sv);
				break;
			#endif // CLONE_NEWNS

			#ifdef CLONE_NEWPID
			case CLONE_NEWPID:
				fname.append ("pid"sv);
				break;
			#endif // CLONE_NEWPID

			#ifdef CLONE_NEWUSER
			case CLONE_NEWUSER:
				fname.append ("user"sv);
				break;
			#endif // CLONE_NEWUSER

			#ifdef CLONE_NEWUTS
			case CLONE_NEWUTS:
				fname.append ("uts"sv);
				break;
			#endif // CLONE_NEWUTS

			default:
				cThrow ("Unknown or unsupported type '{}'"sv, type);
		}

		const int fd = ::open (fname.c_str (), O_RDONLY);
		if (fd < 0) {
			cThrow ("Error opening file '{}': {}"sv, fname, strerror (errno));
		}

		return std::make_unique<ShSocket> (fd);
	}

	UNIQUE_SOCKET LINUX::get_namespace_fd_by_name (const std::string_view name, const int type)
	{
		if (true == STR::has_iprefix (name, "pid:")) {
			const int pid = STR::to_int<int> (name.substr (4));
			return get_namespace_fd_by_pid (pid, type);
		}
		else if (true == STR::icompare (name, "self")) {
			return get_namespace_fd_by_pid (0, type);
		}

		std::string fname ("/run/"sv);

		switch (type) {
			#ifdef CLONE_NEWNET
			case CLONE_NEWNET:
				fname.append ("netns"sv);
				break;
			#endif // CLONE_NEWNET

			default:
				cThrow ("Unknown or unsupported type '{}'"sv, type);
		}

		fname.append ("/"sv);
		fname.append (name);

		const int fd = ::open (fname.c_str (), O_RDONLY);
		if (fd < 0) {
			cThrow ("Error opening file '{}': {}"sv, fname, strerror (errno));
		}

		return std::make_unique<ShSocket> (fd);
	}

	UNIQUE_SOCKET LINUX::get_self_namespace_fd (const int type)
	{
		return get_namespace_fd_by_pid (0, type);
	}

	void LINUX::set_namespace (const int fd, const int type)
	{
		const int ret = ::setns (fd, type);
		if (ret != 0) {
			cThrow ("Error setting namespace: {}"sv, strerror (errno));
		}
	}

	void LINUX::set_namespace (const std::string_view name, const int type)
	{
		auto fd = get_namespace_fd_by_name (name, type);
		set_namespace (fd->get (), type);
	}

	void LINUX::add_to_epoll (int sock, const uint32_t ev, int epoll_fd, const bool should_modify_if_exists)
	{
		if (sock < 0) {
			return;
		}

		struct epoll_event e;
		::memset (&e, 0, sizeof (e));
		e.events = ev;
		e.data.fd = sock;

		if (::epoll_ctl (epoll_fd, EPOLL_CTL_ADD, sock, &e) == -1) {
			if (EEXIST == errno && true == should_modify_if_exists) {
				if (epoll_ctl (epoll_fd, EPOLL_CTL_MOD, sock, &e) == -1) {
					cThrow ("Unable to modify file descriptor in epoll: {}"sv, strerror (errno));
				}
			}
			else {
				cThrow ("Unable to add new file descriptor to epoll: {}"sv, strerror (errno));
			}
		}
	}

	void LINUX::modify_epoll (int sock, const uint32_t ev, int epoll_fd)
	{
		if (sock < 0) {
			return;
		}

		struct epoll_event e;
		::memset (&e, 0, sizeof (e));
		e.events = ev;
		e.data.fd = sock;

		if (::epoll_ctl (epoll_fd, EPOLL_CTL_MOD, sock, &e) == -1) {
			cThrow ("Unable to modify file descriptor in epoll: {}"sv, strerror (errno));
		}
	}

	void LINUX::remove_from_epoll (int sock, int epoll_fd, const bool ignore_failure)
	{
		if (sock < 0) {
			return;
		}

		if (::epoll_ctl (epoll_fd, EPOLL_CTL_DEL, sock, nullptr) == -1) {
			if (false == ignore_failure) {
				cThrow ("Unable to remove file descriptor from epoll: {}"sv, strerror (errno));
			}
		}
	}

	HEDLEY_WARN_UNUSED_RESULT bool LINUX::login_to_uid (const std::string_view text, uid_t &uid)
	{
		bool ok = false;

		long bufsize = ::sysconf (_SC_GETPW_R_SIZE_MAX);
		if (bufsize == -1) {
			bufsize = 16384;
		}

		char *buf = reinterpret_cast<char *> (::malloc (bufsize));
		if (nullptr == buf) {
			cThrow ("Alloc failed"sv);
		}

		try {
			struct passwd pwd;
			struct passwd *result;

			int ret = getpwnam_r (s_c_str (text), &pwd, buf, bufsize, &result);
			if (ret == 0) {
				if (result != nullptr) {
					uid = static_cast<int> (pwd.pw_uid);
					ok = true;
				}
			}
			else {
				cThrow ("getpwnam_r error: {}"sv, strerror (ret));
			}
		}
		catch (...) {
			::free (buf);
			throw;
		}
		::free (buf);

		return ok;
	}

	HEDLEY_WARN_UNUSED_RESULT bool LINUX::group_to_gid (const std::string_view text, gid_t &gid)
	{
		bool ok = false;

		long bufsize = ::sysconf (_SC_GETGR_R_SIZE_MAX);
		if (bufsize == -1) {
			bufsize = 16'384;
		}

		char *buf = reinterpret_cast<char *> (::malloc (bufsize));
		if (nullptr == buf) {
			cThrow ("Alloc failed"sv);
		}

		try {
			struct group grp;
			struct group *result;

			int ret = getgrnam_r (s_c_str (text), &grp, buf, bufsize, &result);
			if (ret == 0) {
				if (result != nullptr) {
					gid = static_cast<int> (grp.gr_gid);
					ok = true;
				}
			}
			else {
				cThrow ("getgrnam_r error: {}"sv, strerror (ret));
			}
		}
		catch (...) {
			::free (buf);
			throw;
		}
		::free (buf);

		return ok;
	}
}
#endif // OS_LINUX

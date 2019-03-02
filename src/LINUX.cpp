/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifdef OS_LINUX

#include <sys/epoll.h>
#include <pwd.h>
#include <grp.h>

namespace shaga {

	void LINUX::add_to_epoll (int sock, const uint32_t ev, int epoll_fd, const bool should_modify_if_exists)
	{
		if (sock < 0) {
			return;
		}

		struct epoll_event e;
		::memset (&e, 0, sizeof (e));
		e.events = ev;
		e.data.fd = sock;

		if (epoll_ctl (epoll_fd, EPOLL_CTL_ADD, sock, &e) == -1) {
			if (EEXIST == errno && true == should_modify_if_exists) {
				if (epoll_ctl (epoll_fd, EPOLL_CTL_MOD, sock, &e) == -1) {
					cThrow ("Unable to modify file descriptor in epoll: %s", strerror (errno));
				}
			}
			else {
				cThrow ("Unable to add new file descriptor to epoll: %s", strerror (errno));
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

		if (epoll_ctl (epoll_fd, EPOLL_CTL_MOD, sock, &e) == -1) {
			cThrow ("Unable to modify file descriptor in epoll: %s", strerror (errno));
		}
	}

	void LINUX::remove_from_epoll (int sock, int epoll_fd, const bool ignore_failure)
	{
		if (sock < 0) {
			return;
		}

		if (epoll_ctl (epoll_fd, EPOLL_CTL_DEL, sock, nullptr) == -1) {
			if (false == ignore_failure) {
				cThrow ("Unable to remove file descriptor from epoll: %s", strerror (errno));
			}
		}
	}

	SHAGA_NODISCARD bool LINUX::login_to_uid (const std::string &text, uid_t &uid)
	{
		bool ok = false;

		long bufsize = ::sysconf (_SC_GETPW_R_SIZE_MAX);
		if (bufsize == -1) {
			bufsize = 16384;
		}

		char *buf = reinterpret_cast<char *> (::malloc (bufsize));
		if (nullptr == buf) {
			cThrow ("Alloc failed");
		}

		try {
			struct passwd pwd;
			struct passwd *result;

			int ret = getpwnam_r (text.c_str (), &pwd, buf, bufsize, &result);
			if (ret == 0) {
				if (result != nullptr) {
					uid = static_cast<int> (pwd.pw_uid);
					ok = true;
				}
			}
			else {
				cThrow ("getpwnam_r error: %s", strerror (ret));
			}
		}
		catch (...) {
			::free (buf);
			throw;
		}
		::free (buf);

		return ok;
	}

	SHAGA_NODISCARD bool LINUX::group_to_gid (const std::string &text, gid_t &gid)
	{
		bool ok = false;

		long bufsize = ::sysconf (_SC_GETGR_R_SIZE_MAX);
		if (bufsize == -1) {
			bufsize = 16384;
		}

		char *buf = reinterpret_cast<char *> (::malloc (bufsize));
		if (nullptr == buf) {
			cThrow ("Alloc failed");
		}

		try {
			struct group grp;
			struct group *result;

			int ret = getgrnam_r (text.c_str (), &grp, buf, bufsize, &result);
			if (ret == 0) {
				if (result != nullptr) {
					gid = static_cast<int> (grp.gr_gid);
					ok = true;
				}
			}
			else {
				cThrow ("getgrnam_r error: %s", strerror (ret));
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

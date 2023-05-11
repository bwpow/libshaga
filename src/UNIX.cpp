/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2023, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifndef OS_WIN

#include <climits>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace shaga {

	void signal_handler (int sgn)
	{
		P::print ("Caught signal: {}", strsignal (sgn));
		TRY_TO_SHUTDOWN ();
	}

	void signal_register_handlers (const bool ignore_pipe, const bool ignore_hup)
	{
		::signal (SIGINT, signal_handler);
		::signal (SIGTERM, signal_handler);
		::signal (SIGQUIT, signal_handler);
		if (true == ignore_pipe) {
			::signal (SIGPIPE, SIG_IGN);
		}
		if (true == ignore_hup) {
			::signal (SIGHUP, SIG_IGN);
		}
	}

	std::string UNIX::get_hostname (const bool to_lowercase, const char specchar)
	{
		char hostname[HOST_NAME_MAX];
		::memset (hostname, 0, sizeof (hostname));

		if (::gethostname (hostname, sizeof (hostname)) != 0) {
			cThrow ("Could not read hostname: {}", strerror (errno));
		}
		hostname[sizeof (hostname) - 1] = '\0';

		const size_t l = ::strlen (hostname);
		for (size_t i = 0; i < l; ++i) {
			if (to_lowercase == true && hostname[i] >= 'A' && hostname[i] <= 'Z') {
				hostname[i] -= 'A';
				hostname[i] += 'a';
				continue;
			}

			if (specchar != '\0') {
				if (hostname[i] >= 'a' && hostname[i] <= 'z') { }
				else if (hostname[i] >= '0' && hostname[i] <= '9') { }
				else if (hostname[i] == '-') { }
				else if (hostname[i] == '.') { }
				else {
					hostname[i] = specchar;
				}
			}
		}

		return std::string (hostname);
	}

	pid_t UNIX::daemonize (const std::string_view pidfile)
	{
		pid_t pid;
		FILE *pidf = nullptr;

		if (pidfile.empty () == false) {
			pidf = ::fopen (s_c_str (pidfile), "w");
			if (nullptr == pidf) {
				cThrow ("Could not open PID file '{}' for writing", pidfile);
			}
		}

		/* Fork off the parent process */
		pid = ::fork ();

		/* An error occurred */
		if (pid < 0) {
			cThrow (strerror (errno));
		}

		/* Success: Let the parent terminate */
		if (pid > 0) {
			::exit (EXIT_SUCCESS);
		}

		/* On success: The child process becomes session leader */
		if (::setsid () < 0) {
			cThrow (strerror (errno));
		}

		//signal (SIGCHLD, SIG_IGN);
		::signal (SIGHUP, SIG_IGN);

		/* Fork off for the second time*/
		pid = ::fork ();

		/* An error occurred */
		if (pid < 0) {
			cThrow (strerror (errno));
		}

		/* Success: Let the parent terminate */
		if (pid > 0) {
			::exit (EXIT_SUCCESS);
		}

		pid = ::getpid ();

		if (pidf != nullptr) {
			::fprintf (pidf, "%d\n", static_cast<int> (pid));
			::fclose (pidf);
		}

		/* Close all open file descriptors */
		for (long x = ::sysconf (_SC_OPEN_MAX); x > 0; x--) {
			::close (x);
		}

		/* Set new file permissions */
		::umask (0);

		return pid;
	}

	void UNIX::renice (const int prio)
	{
		pid_t id = ::getpgrp ();

		errno = 0;
		if (::setpriority (PRIO_PGRP, static_cast<int>(id), prio) != 0){
			cThrow ("Could not lower priority: {}", strerror (errno));
		}
	}

	void UNIX::renice (std::shared_ptr<shaga::INI> ini)
	{
		if (nullptr == ini) {
			return;
		}

		const int prio = ini->get_uint8 (""sv, "renice"sv, UINT8_MAX);
		if (prio != UINT8_MAX) {
			P::print ("Changing priority to {}", prio);
			renice (prio);
		}
	}

	void UNIX::renice (const shaga::INI *ini)
	{
		if (nullptr == ini) {
			return;
		}

		const int prio = ini->get_uint8 (""sv, "renice"sv, UINT8_MAX);
		if (prio != UINT8_MAX) {
			P::print ("Changing priority to {}", prio);
			renice (prio);
		}
	}

}
#endif // OS_WIN

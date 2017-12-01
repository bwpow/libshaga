/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool operator== (const SHARED_SOCKET &a, const int b)
	{
		return a->get () == b;
	}

	bool operator== (const int b, const SHARED_SOCKET &a)
	{
		return a->get () == b;
	}

	bool operator< (const SHARED_SOCKET &a, const int b)
	{
		return a->get () < b;
	}

	bool operator< (const int b, const SHARED_SOCKET &a)
	{
		return b < a->get ();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ShSocket::ShSocket (const int sock) :
		_fd (sock)
	{
		if (_fd < 0) {
			if (errno > 0) {
				cThrow ("Unable to create socket: %s", strerror (errno));
			}
			else {
				cThrow ("Unable to use empty socket");
			}
		}

		P::debug_printf ("Opening socket %d", _fd);
	}

	ShSocket::ShSocket (const int domain, const int type, const int protocol) :
		ShSocket (::socket (domain, type, protocol))
	{ }

	ShSocket::~ShSocket ()
	{
		if (_fd >= 0) {
			P::debug_printf ("Closing socket %d", _fd);
			::close (_fd);
		}
	}

	int ShSocket::get (void) const noexcept
	{
		return _fd;
	}

	bool ShSocket::operator== (const ShSocket &other) const
	{
		return _fd == other._fd;
	}

	bool ShSocket::operator!= (const ShSocket &other) const
	{
		return _fd != other._fd;
	}

	bool ShSocket::operator< (const ShSocket &other) const
	{
		return _fd < other._fd;
	}

	bool ShSocket::operator== (const int other) const
	{
		return _fd == other;
	}

	bool ShSocket::operator!= (const int other) const
	{
		return _fd != other;
	}

	bool ShSocket::operator< (const int other) const
	{
		return _fd < other;
	}

}

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#pragma once

#include "common.h"

namespace shaga {
	class ShSocket : public std::enable_shared_from_this<ShSocket>
	{
		private:
			const int _fd {-1};

		public:
			ShSocket (const int sock);
			ShSocket (const int domain, const int type, const int protocol);
			~ShSocket ();

			int get (void) const noexcept;

			bool operator== (const ShSocket &other) const;
			bool operator!= (const ShSocket &other) const;
			bool operator< (const ShSocket &other) const;

			bool operator== (const int other) const;
			bool operator!= (const int other) const;
			bool operator< (const int other) const;

			ShSocket (ShSocket const&) = delete;
			ShSocket& operator= (ShSocket const&) = delete;
	};

	typedef std::shared_ptr<ShSocket> SHARED_SOCKET;
	typedef std::unique_ptr<ShSocket> UNIQUE_SOCKET;

	/* Takes ownership of sock even if allocation or ShSocket construction fails. */
	SHARED_SOCKET make_shared_socket (int sock);

	bool operator== (const SHARED_SOCKET &a, const int b);
	bool operator== (const int b, const SHARED_SOCKET &a);

	bool operator< (const SHARED_SOCKET &a, const int b);
	bool operator< (const int b, const SHARED_SOCKET &a);

	bool operator== (const UNIQUE_SOCKET &a, const int b);
	bool operator== (const int b, const UNIQUE_SOCKET &a);

	bool operator< (const UNIQUE_SOCKET &a, const int b);
	bool operator< (const int b, const UNIQUE_SOCKET &a);
}

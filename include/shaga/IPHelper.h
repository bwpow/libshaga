/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2023, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_IPHelper
#define HEAD_shaga_IPHelper

#include "common.h"

#ifdef OS_WIN
	#include <ws2def.h>
	#include <ws2tcpip.h>
#else
	#include <arpa/inet.h>
	#include <sys/socket.h>
#endif // OS_WIN

namespace shaga {
	class IPHelper
	{
		public:
			enum class Type {
				Autodetect,
				IPv4,
				IPv6
			};

			static bool is_same (const int af, const struct sockaddr_storage &addr_in1, const struct sockaddr_storage &addr_in2);
			static int compare (const int af, const struct sockaddr_storage &addr_in1, const struct sockaddr_storage &addr_in2);
			static struct sockaddr_storage NOT (const int af, const struct sockaddr_storage &addr_in);
			static struct sockaddr_storage AND (const int af, const struct sockaddr_storage &addr_in1, const struct sockaddr_storage &addr_in2);
			static struct sockaddr_storage OR (const int af, const struct sockaddr_storage &addr_in1, const struct sockaddr_storage &addr_in2);
			static std::string get_ip_string (const int af, const struct sockaddr_storage &addr);

		private:
			struct sockaddr_storage _addr;
			int _af;
			int _mask;
			int _max_mask;

			std::string get_byte_mask (const int mask, const size_t offset) const;

			void test (void) const;
			bool init_ipv4 (const std::string &buf, const uint16_t port);
			bool init_ipv6 (const std::string &buf, const uint16_t port);
		public:
			IPHelper (const Type type, const std::string &buf);
			IPHelper (const struct sockaddr_storage &addr_in);
			IPHelper (const Type type, const void *buf, const size_t sze);

			int get_af (void) const;
			std::string get_af_string (void) const;

			struct sockaddr_storage get_sockaddr_storage (void) const;

			void set_port (const uint16_t port);
			uint16_t get_port (void) const;

			void set_mask (const int mask);
			int get_mask (void) const;
			struct sockaddr_storage get_mask_struct (void) const;
			std::string get_mask_string (void) const;

			struct sockaddr_storage get_ip_struct (void) const;
			std::string get_ip_string (const bool append_mask = true) const;

			struct sockaddr_storage get_subnet_struct (void) const;
			std::string get_subnet_string (const bool append_mask = true) const;

			struct sockaddr_storage get_broadcast_struct (void) const;
			std::string get_broadcast_string (const bool append_mask = true) const;

			bool is_host (void) const;
			bool is_subnet (void) const;
			bool is_broadcast (void) const;
			bool is_in_my_subnet (const IPHelper &ip) const;

			void convert_to_subnet (void);
			void convert_to_broadcast (void);

			friend bool operator|| (const IPHelper &a, const IPHelper &b);
			friend bool operator== (const IPHelper &a, const IPHelper &b);
			friend bool operator< (const IPHelper &a, const IPHelper &b);
	};
}

#endif // HEAD_shaga_IPHelper

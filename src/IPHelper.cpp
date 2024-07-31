/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2024, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool IPHelper::is_same (const int af, const struct sockaddr_storage &addr_in1, const struct sockaddr_storage &addr_in2)
	{
		return compare (af, addr_in1, addr_in2) == 0;
	}

	int IPHelper::compare (const int af, const struct sockaddr_storage &addr_in1, const struct sockaddr_storage &addr_in2)
	{
		if (af == AF_INET) {
			struct sockaddr_in* addr4_in1 = (struct sockaddr_in*)&addr_in1;
			struct sockaddr_in* addr4_in2 = (struct sockaddr_in*)&addr_in2;

			for (size_t i = 0; i < sizeof (addr4_in1->sin_addr); ++i) {
				if (reinterpret_cast<uint8_t *>(&addr4_in1->sin_addr)[i] < reinterpret_cast<uint8_t *>(&addr4_in2->sin_addr)[i]) {
					return -1;
				}
				if (reinterpret_cast<uint8_t *>(&addr4_in1->sin_addr)[i] > reinterpret_cast<uint8_t *>(&addr4_in2->sin_addr)[i]) {
					return 1;
				}
			}

			return 0;
		}

		if (af == AF_INET6) {
			struct sockaddr_in6* addr6_in1 = (struct sockaddr_in6*)&addr_in1;
			struct sockaddr_in6* addr6_in2 = (struct sockaddr_in6*)&addr_in2;

			for (size_t i = 0; i < sizeof (addr6_in1->sin6_addr); ++i) {
				if (reinterpret_cast<uint8_t *>(&addr6_in1->sin6_addr)[i] < reinterpret_cast<uint8_t *>(&addr6_in2->sin6_addr)[i]) {
					return -1;
				}
				if (reinterpret_cast<uint8_t *>(&addr6_in1->sin6_addr)[i] > reinterpret_cast<uint8_t *>(&addr6_in2->sin6_addr)[i]) {
					return 1;
				}
			}

			return 0;
		}

		cThrow ("Unknown protocol"sv);
	}

	struct sockaddr_storage IPHelper::NOT (const int af, const struct sockaddr_storage &addr_in)
	{
		struct sockaddr_storage addr_out;

		if (af == AF_INET) {
			struct sockaddr_in* addr4_in = (struct sockaddr_in*)&addr_in;
			struct sockaddr_in* addr4_out = (struct sockaddr_in*)&addr_out;

			for (size_t i = 0; i < sizeof (addr4_in->sin_addr); ++i) {
				reinterpret_cast<uint8_t *>(&addr4_out->sin_addr)[i] = reinterpret_cast<uint8_t *>(&addr4_in->sin_addr)[i] ^ 0xff;
			}
		}
		else if (af == AF_INET6) {
			struct sockaddr_in6* addr6_in = (struct sockaddr_in6*)&addr_in;
			struct sockaddr_in6* addr6_out = (struct sockaddr_in6*)&addr_out;

			for (size_t i = 0; i < sizeof (addr6_in->sin6_addr); ++i) {
				reinterpret_cast<uint8_t *>(&addr6_out->sin6_addr)[i] = reinterpret_cast<uint8_t *>(&addr6_in->sin6_addr)[i] ^ 0xff;
			}
		}
		else {
			cThrow ("Unknown protocol"sv);
		}

		return addr_out;
	}

	struct sockaddr_storage IPHelper::AND (const int af, const struct sockaddr_storage &addr_in1, const struct sockaddr_storage &addr_in2)
	{
		struct sockaddr_storage addr_out;

		if (af == AF_INET) {
			struct sockaddr_in* addr4_in1 = (struct sockaddr_in*)&addr_in1;
			struct sockaddr_in* addr4_in2 = (struct sockaddr_in*)&addr_in2;
			struct sockaddr_in* addr4_out = (struct sockaddr_in*)&addr_out;

			for (size_t i = 0; i < sizeof (addr4_in1->sin_addr); ++i) {
				reinterpret_cast<uint8_t *>(&addr4_out->sin_addr)[i] = reinterpret_cast<uint8_t *>(&addr4_in1->sin_addr)[i] & reinterpret_cast<uint8_t *>(&addr4_in2->sin_addr)[i];
			}
		}
		else if (af == AF_INET6) {
			struct sockaddr_in6* addr6_in1 = (struct sockaddr_in6*)&addr_in1;
			struct sockaddr_in6* addr6_in2 = (struct sockaddr_in6*)&addr_in2;
			struct sockaddr_in6* addr6_out = (struct sockaddr_in6*)&addr_out;

			for (size_t i = 0; i < sizeof (addr6_in1->sin6_addr); ++i) {
				reinterpret_cast<uint8_t *>(&addr6_out->sin6_addr)[i] = reinterpret_cast<uint8_t *>(&addr6_in1->sin6_addr)[i] & reinterpret_cast<uint8_t *>(&addr6_in2->sin6_addr)[i];
			}
		}
		else {
			cThrow ("Unknown protocol"sv);
		}

		return addr_out;
	}

	struct sockaddr_storage IPHelper::OR (const int af, const struct sockaddr_storage &addr_in1, const struct sockaddr_storage &addr_in2)
	{
		struct sockaddr_storage addr_out;

		if (af == AF_INET) {
			struct sockaddr_in* addr4_in1 = (struct sockaddr_in*)&addr_in1;
			struct sockaddr_in* addr4_in2 = (struct sockaddr_in*)&addr_in2;
			struct sockaddr_in* addr4_out = (struct sockaddr_in*)&addr_out;

			for (size_t i = 0; i < sizeof (addr4_in1->sin_addr); ++i) {
				reinterpret_cast<uint8_t *>(&addr4_out->sin_addr)[i] = reinterpret_cast<uint8_t *>(&addr4_in1->sin_addr)[i] | reinterpret_cast<uint8_t *>(&addr4_in2->sin_addr)[i];
			}
		}
		else if (af == AF_INET6) {
			struct sockaddr_in6* addr6_in1 = (struct sockaddr_in6*)&addr_in1;
			struct sockaddr_in6* addr6_in2 = (struct sockaddr_in6*)&addr_in2;
			struct sockaddr_in6* addr6_out = (struct sockaddr_in6*)&addr_out;

			for (size_t i = 0; i < sizeof (addr6_in1->sin6_addr); ++i) {
				reinterpret_cast<uint8_t *>(&addr6_out->sin6_addr)[i] = reinterpret_cast<uint8_t *>(&addr6_in1->sin6_addr)[i] | reinterpret_cast<uint8_t *>(&addr6_in2->sin6_addr)[i];
			}
		}
		else {
			cThrow ("Unknown protocol"sv);
		}

		return addr_out;
	}

	std::string IPHelper::get_ip_string (const int af, const struct sockaddr_storage &addr)
	{
		if (af == AF_INET) {
			const struct sockaddr_in* addr4 = (struct sockaddr_in*)&addr;
			char str_ip4[INET_ADDRSTRLEN + 1];
			if (::inet_ntop (AF_INET, (void *) &(addr4->sin_addr), str_ip4, sizeof (str_ip4)) == NULL) {
				cThrow ("Unable to get IPv4 address: {}"sv, strerror (errno));
			}
			return std::string (str_ip4);
		}
		else if (af == AF_INET6) {
			const struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&addr;
			char str_ip6[INET6_ADDRSTRLEN + 1];
			if (::inet_ntop (AF_INET6, (void *) &(addr6->sin6_addr), str_ip6, sizeof (str_ip6)) == NULL) {
				cThrow ("Unable to get IPv6 address: {}"sv, strerror (errno));
			}
			return std::string (str_ip6);
		}
		else {
			cThrow ("Unknown protocol family or unrecognized address"sv);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string IPHelper::get_byte_mask (const int mask, const size_t offset) const
	{
		std::string str;
		if (offset > 0) {
			if (_af == AF_INET) {
				str.append (".");
			}
			else if (_af == AF_INET6 && (offset % 2) == 0) {
				str.append (":");
			}
		}

		int m = mask - (offset * 8);
		uint8_t out = 0;

		for (size_t i = 0; i < 8; ++i) {
			out <<= 1;
			if (m > 0) {
				m--;
				out |= 1;
			}
		}

		if (_af == AF_INET) {
			str.append (STR::from_int (out, 10));
		}
		else if (_af == AF_INET6) {
			str.append (STR::from_int (out, 16));
		}

		return str;
	}

	void IPHelper::test (void) const
	{
		if (_mask < 0 || _mask > _max_mask) {
			cThrow ("Mask out of range, expected value between 0 and {}", _max_mask);
		}

		if (_af == AF_INET) {
			const struct sockaddr_in* addr4 = (struct sockaddr_in*)&_addr;
			if (addr4->sin_family != _af) {
				cThrow ("Incorrect protocol family set. Expected IPv4.");
			}
			P::debug_print ("IPHelper: IPv4 address '{}' port {} mask {}"sv, get_ip_string (_af, _addr), ntohs (addr4->sin_port), _mask);
		}
		else if (_af == AF_INET6) {
			const struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&_addr;
			if (addr6->sin6_family != _af) {
				cThrow ("Incorrect protocol family set. Expected IPv6.");
			}
			P::debug_print ("IPHelper: IPv6 address '{}' port {} mask {}"sv, get_ip_string (_af, _addr), ntohs (addr6->sin6_port), _mask);
		}
		else {
			cThrow ("Unknown protocol family or unrecognized address"sv);
		}
	}

	bool IPHelper::init_ipv4 (const std::string &buf, const uint16_t port)
	{
		if (_af != AF_INET && _af != -1) {
			return false;
		}

		struct sockaddr_in* addr4 = (struct sockaddr_in*)&_addr;
		static_assert (sizeof (addr4->sin_addr) == 4, "Size of sin_addr needs to be 4 bytes");

		if (::inet_pton (AF_INET, buf.c_str (), &(addr4->sin_addr)) == 1) {
			_af = AF_INET;
			addr4->sin_family = AF_INET;
			addr4->sin_port = htons(port);
			_max_mask = 32;
			return true;
		}

		return false;
	}

	bool IPHelper::init_ipv6 (const std::string &buf, const uint16_t port)
	{
		if (_af != AF_INET6 && _af != -1) {
			return false;
		}

		struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&_addr;
		static_assert (sizeof (addr6->sin6_addr) == 16, "Size of sin6_addr needs to be 16 bytes");

		if (::inet_pton (AF_INET6, buf.c_str (), &(addr6->sin6_addr)) == 1) {
			_af = AF_INET6;
			addr6->sin6_family = AF_INET6;
			addr6->sin6_port = htons(port);
			_max_mask = 128;
			return true;
		}

		return false;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	IPHelper::IPHelper (const Type type, const std::string &buf)
	{
		memset (&_addr, 0, sizeof (_addr));
		_af = -1;
		_mask = -1;
		_max_mask = -1;

		uint16_t port = 0;
		std::string addr = buf;

		const size_t pos = addr.find_first_of (":;,");
		if (pos != std::string::npos) {
			const std::string tstr = addr.substr (0, pos);
			if (tstr == "ipv4"s || tstr == "ip4"s || tstr == "ip"s || tstr == "IPv4"s) {
				_af = AF_INET;
				addr.erase (0, pos + 1);
			}
			else if (tstr == "ipv6"s || tstr == "ip6"s || tstr == "IPv6"s) {
				_af = AF_INET6;
				addr.erase (0, pos + 1);
			}
		}

		const size_t mpos = addr.find_last_of ("/");
		if (mpos != std::string::npos) {
			_mask = STR::to_uint8 (addr.substr (mpos + 1));
			addr.erase (mpos);
		}

		const size_t a_begin = addr.find_first_of ("[");
		const size_t a_end = addr.find_last_of ("]");
		if (a_begin == 0 && a_end != std::string::npos) {
			const std::string str_port = addr.substr (a_end + 1);
			addr.erase (a_end);
			addr.erase (0, 1);

			if (str_port.empty () == false) {
				if (str_port.at (0) == ':') {
					port = STR::to_uint16 (str_port.substr (1));
				}
				else {
					cThrow ("Malformed port specification '{}'"sv, str_port);
				}
			}
		}

		bool ok = false;

		switch (type) {
			case Type::Autodetect:
				ok |= init_ipv4 (addr, port);
				ok |= init_ipv6 (addr, port);
				break;

			case Type::IPv4:
				ok |= init_ipv4 (addr, port);
				break;

			case Type::IPv6:
				ok |= init_ipv6 (addr, port);
				break;
		}

		if (ok == false) {
			cThrow ("String '{}' not recognized as valid IP address"sv, buf);
		}

		if (_mask < 0) {
			_mask = _max_mask;
		}

		test ();
	}

	IPHelper::IPHelper (const struct sockaddr_storage &addr_in)
	{
		switch (addr_in.ss_family) {
			case AF_INET:
				_af = AF_INET;
				_mask = _max_mask = 32;
				break;

			case AF_INET6:
				_af = AF_INET6;
				_mask = _max_mask = 128;
				break;

			default:
				cThrow ("Unknown protocol family"sv);
		}

		::memcpy (&_addr, &addr_in, sizeof (struct sockaddr_storage));
		test ();
	}

	IPHelper::IPHelper (const Type type, const void *buf, const size_t sze)
	{
		::memset (&_addr, 0, sizeof (_addr));
		struct sockaddr_in* addr4 = (struct sockaddr_in*)&_addr;
		struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&_addr;

		switch (type) {
			case Type::Autodetect:
				cThrow ("Not possible to use autodetect with binary format"sv);
				break;

			case Type::IPv4:
				_af = AF_INET;
				_mask = _max_mask = 32;
				addr4->sin_family = AF_INET;
				if (sze < sizeof (addr4->sin_addr)) {
					cThrow ("Buffer too small. Expected at least {} bytes"sv, sizeof (addr4->sin_addr));
				}
				memcpy (&(addr4->sin_addr), buf, sizeof (addr4->sin_addr));
				break;

			case Type::IPv6:
				_af = AF_INET6;
				_mask = _max_mask = 128;
				addr6->sin6_family = AF_INET6;
				if (sze < sizeof (addr6->sin6_addr)) {
					cThrow ("Buffer too small. Expected at least {} bytes"sv, sizeof (addr6->sin6_addr));
				}
				memcpy (&(addr6->sin6_addr), buf, sizeof (addr6->sin6_addr));
				break;
		}

		test ();
	}

	int IPHelper::get_af (void) const
	{
		return _af;
	}

	std::string IPHelper::get_af_string (void) const
	{
		switch (_af) {
			case AF_INET:
				return "IPv4"s;
			case AF_INET6:
				return "IPv6"s;

			default:
				return "UNK"s;
		}
	}

	struct sockaddr_storage IPHelper::get_sockaddr_storage (void) const
	{
		return _addr;
	}

	void IPHelper::set_port (const uint16_t port)
	{
		struct sockaddr_in* addr4 = (struct sockaddr_in*)&_addr;
		struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&_addr;

		switch (_af) {
			case AF_INET:
				addr4->sin_port = htons (port);
				break;

			case AF_INET6:
				addr6->sin6_port = htons (port);
				break;

			default:
				cThrow ("Unknown protocol family"sv);
		}
	}

	uint16_t IPHelper::get_port (void) const
	{
		struct sockaddr_in* addr4 = (struct sockaddr_in*)&_addr;
		struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&_addr;

		switch (_af) {
			case AF_INET:
				return ntohs (addr4->sin_port);
			case AF_INET6:
				return ntohs (addr6->sin6_port);
			default:
				cThrow ("Unknown protocol family"sv);
		}
	}

	void IPHelper::set_mask (const int mask)
	{
		if (mask < 0 || mask > _max_mask) {
			cThrow ("Value out of range"sv);
		}
		else {
			cThrow ("Unknown protocol family"sv);
		}

		_mask = mask;
	}

	int IPHelper::get_mask (void) const
	{
		return _mask;
	}

	struct sockaddr_storage IPHelper::get_mask_struct (void) const
	{
		struct sockaddr_storage addr;
		memset (&addr, 0, sizeof (addr));

		struct sockaddr_in* addr4 = (struct sockaddr_in*)&addr;
		struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&addr;
		std::string str;

		switch (_af) {
			case AF_INET:
				for (size_t i = 0; i < sizeof (addr4->sin_addr); ++i) {
					str.append (get_byte_mask (_mask, i));
				}
				if (::inet_pton (AF_INET, str.c_str (), &(addr4->sin_addr)) != 1) {
					cThrow ("Malformed network mask"sv);
				}
				addr4->sin_family = AF_INET;
				break;

			case AF_INET6:
				for (size_t i = 0; i < sizeof (addr6->sin6_addr); ++i) {
					str.append (get_byte_mask (_mask, i));
				}

				if (::inet_pton (AF_INET6, str.c_str (), &(addr6->sin6_addr)) != 1) {
					cThrow ("Malformed network mask"sv);
				}
				addr6->sin6_family = AF_INET6;
				break;

			default:
				cThrow ("Unknown protocol family or unrecognized address"sv);
		}

		return addr;
	}

	std::string IPHelper::get_mask_string (void) const
	{
		return get_ip_string (_af, get_mask_struct ());
	}

	struct sockaddr_storage IPHelper::get_ip_struct (void) const
	{
		return _addr;
	}

	std::string IPHelper::get_ip_string (const bool append_mask) const
	{
		std::string out = get_ip_string (_af, _addr);

		if (append_mask == true) {
			out.append ("/"s + STR::from_int (_mask));
		}

		return out;
	}

	struct sockaddr_storage IPHelper::get_subnet_struct (void) const
	{
		return AND (_af, _addr, get_mask_struct ());
	}

	std::string IPHelper::get_subnet_string (const bool append_mask) const
	{
		std::string out = get_ip_string (_af, get_subnet_struct ());

		if (append_mask == true) {
			out.append ("/"s + STR::from_int (_mask));
		}

		return out;
	}

	struct sockaddr_storage IPHelper::get_broadcast_struct (void) const
	{
		return OR (_af, get_subnet_struct (), NOT (_af, get_mask_struct ()));
	}

	std::string IPHelper::get_broadcast_string (const bool append_mask) const
	{
		std::string out = get_ip_string (_af, get_broadcast_struct ());

		if (append_mask == true) {
			out.append ("/"s + STR::from_int (_mask));
		}

		return out;
	}

	bool IPHelper::is_host (void) const
	{
		if (_mask == _max_mask) {
			return true;
		}
		return is_subnet () == false && is_broadcast () == false;
	}

	bool IPHelper::is_subnet (void) const
	{
		if (_mask == _max_mask) {
			return false;
		}
		return is_same (_af, _addr, get_subnet_struct ());
	}

	bool IPHelper::is_broadcast (void) const
	{
		if (_mask == _max_mask) {
			return false;
		}
		return is_same (_af, _addr, get_broadcast_struct ());
	}

	bool IPHelper::is_in_my_subnet (const IPHelper &ip) const
	{
		if (ip._af != _af) return false;
		if (ip._mask < _mask) return false;

		const struct sockaddr_storage binmask = get_mask_struct ();

		return is_same (_af, AND (_af, _addr, binmask), AND (_af, ip._addr, binmask));
	}

	void IPHelper::convert_to_subnet (void)
	{
		_addr = get_subnet_struct ();
	}

	void IPHelper::convert_to_broadcast (void)
	{
		_addr = get_broadcast_struct ();
	}

	bool operator|| (const IPHelper &a, const IPHelper &b)
	{
		return a.is_in_my_subnet (b) || b.is_in_my_subnet (a);
	}

	bool operator== (const IPHelper &a, const IPHelper &b)
	{
		if (a._af != b._af) return false;
		if (a._mask != b._mask) return false;

		return IPHelper::is_same (a._af, a._addr, b._addr);
	}

	bool operator< (const IPHelper &a, const IPHelper &b)
	{
		if (a._af < b._af) return true;
		if (a._af > b._af) return false;

		if (a._mask < b._mask) return true;
		if (a._mask > b._mask) return false;

		return IPHelper::compare (a._af, a._addr, b._addr) < 0;
	}

}

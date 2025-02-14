/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

TEST (IPHelperTest, IPv4Basic)
{
	// Create IPv4 object with mask, check IP, subnet and broadcast strings.
	IPHelper ip (IPHelper::Type::IPv4, "ipv4:192.168.1.10/24");
	EXPECT_EQ (ip.get_ip_string (false), "192.168.1.10");
	EXPECT_EQ (ip.get_subnet_string (false), "192.168.1.0");
	EXPECT_EQ (ip.get_broadcast_string (false), "192.168.1.255");
	EXPECT_TRUE (ip.is_host ());
	EXPECT_FALSE (ip.is_subnet ());
	EXPECT_FALSE (ip.is_broadcast ());
}

TEST (IPHelperTest, IPv4Host)
{
	// Without mask provided, should default to /32 - acting as host.
	IPHelper ip (IPHelper::Type::IPv4, "ipv4:127.0.0.1");
	EXPECT_EQ (ip.get_ip_string (false), "127.0.0.1");
	EXPECT_TRUE (ip.is_host ());
	EXPECT_FALSE (ip.is_subnet ());
	EXPECT_FALSE (ip.is_broadcast ());
}

TEST (IPHelperTest, IPv6Basic)
{
	// Create IPv6 object with explicit mask /128.
	IPHelper ip (IPHelper::Type::IPv6, "ipv6:[::1]/128");
	EXPECT_EQ (ip.get_ip_string (false), "::1");
	EXPECT_TRUE (ip.is_host ());

	// Set port and check.
	ip.set_port (8080);
	EXPECT_EQ (ip.get_port (), 8080);
}

TEST (IPHelperTest, SubnetEqualityAndOperators)
{
	// Two IPv4 addresses in the same subnet.
	IPHelper ip1 (IPHelper::Type::IPv4, "ipv4:192.168.1.10/24");
	IPHelper ip2 (IPHelper::Type::IPv4, "ipv4:192.168.1.20/24");
	EXPECT_TRUE (ip1.is_in_my_subnet (ip2));
	EXPECT_TRUE (ip2.is_in_my_subnet (ip1));
	EXPECT_TRUE (ip1 || ip2);

	// Comparison operators.
	EXPECT_FALSE (ip1 == ip2);	// different host bits
	EXPECT_TRUE (ip1 < ip2 || ip2 < ip1);
}

TEST (IPHelperTest, ConversionFunctions)
{
	// Test convert_to_subnet and convert_to_broadcast.
	IPHelper ip (IPHelper::Type::IPv4, "ipv4:192.168.1.130/24");
	ip.convert_to_subnet ();
	EXPECT_EQ (ip.get_ip_string (false), "192.168.1.0");

	// recreate and test broadcast conversion.
	IPHelper ip2 (IPHelper::Type::IPv4, "ipv4:192.168.1.130/24");
	ip2.convert_to_broadcast ();
	EXPECT_EQ (ip2.get_ip_string (false), "192.168.1.255");
}

TEST (IPHelperTest, InvalidInput)
{
	// Expect an exception when given invalid IP string.
	EXPECT_THROW ({ IPHelper ip (IPHelper::Type::IPv4, "invalid:input/24"); }, std::exception);

	// Test set_mask always throws.
	IPHelper ip (IPHelper::Type::IPv4, "ipv4:192.168.1.10/24");
	EXPECT_THROW ({ ip.set_mask (16); }, std::exception);
}

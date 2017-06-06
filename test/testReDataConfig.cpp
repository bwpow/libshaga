/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.txt):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <shaga.h>
#include <gtest/gtest.h>

using namespace shaga;

TEST (ReDataConfig, constructors)
{
	ReDataConfig first;
	first.set_digest (ReDataConfig::DIGEST::SHA1).set_crypto (ReDataConfig::CRYPTO::AES256).set_coding (ReDataConfig::CODING::BASE64_ALT);

	ReDataConfig second (first);

	EXPECT_TRUE (first.get_digest() == second.get_digest ());
	EXPECT_TRUE (first.get_crypto() == second.get_crypto ());
	EXPECT_TRUE (first.get_coding() == second.get_coding ());

	ReDataConfig third (std::move (first));

	EXPECT_TRUE (third.get_digest() == second.get_digest ());
	EXPECT_TRUE (third.get_crypto() == second.get_crypto ());
	EXPECT_TRUE (third.get_coding() == second.get_coding ());
}

TEST (ReDataConfig, assign_operators)
{
	ReDataConfig first;
	first.set_digest (ReDataConfig::DIGEST::SHA1).set_crypto (ReDataConfig::CRYPTO::AES256).set_coding (ReDataConfig::CODING::BASE64_ALT);

	ReDataConfig second = first;

	EXPECT_TRUE (first.get_digest() == second.get_digest ());
	EXPECT_TRUE (first.get_crypto() == second.get_crypto ());
	EXPECT_TRUE (first.get_coding() == second.get_coding ());

	ReDataConfig third = std::move (first);

	EXPECT_TRUE (third.get_digest() == second.get_digest ());
	EXPECT_TRUE (third.get_crypto() == second.get_crypto ());
	EXPECT_TRUE (third.get_coding() == second.get_coding ());
}

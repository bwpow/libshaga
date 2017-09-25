/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.txt):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <shaga.h>
#include <gtest/gtest.h>

using namespace shaga;

TEST (DiSig, genkey)
{
	std::string name;
	const std::string hsh_ok = CRC::sha256 (std::string ("test string"));
	const std::string hsh_other = CRC::sha256 (std::string ("different test string"));
	const std::string hsh_bad = "abcd";

	DiSig disig ("seed string");

	disig.generate_new_key ("secp521r1");
	disig.add_decryption_key (disig.get_encryption_pubkey ());
	std::string test1 = disig.sign (MBEDTLS_MD_SHA256, hsh_ok);
	EXPECT_THROW (disig.sign (MBEDTLS_MD_SHA256, hsh_bad), CommonException);

	disig.generate_new_key ("secp384r1");
	disig.add_decryption_key (disig.get_encryption_pubkey ());
	std::string test2 = disig.sign (MBEDTLS_MD_SHA256, hsh_ok);
	EXPECT_THROW (disig.sign (MBEDTLS_MD_SHA256, hsh_bad), CommonException);

	disig.generate_new_key ("secp256r1");
	disig.add_decryption_key (disig.get_encryption_pubkey ());
	std::string test3 = disig.sign (MBEDTLS_MD_SHA256, hsh_ok);
	EXPECT_THROW (disig.sign (MBEDTLS_MD_SHA256, hsh_bad), CommonException);

	EXPECT_THROW (disig.verify (MBEDTLS_MD_SHA256, hsh_bad, test1, name), CommonException);
	EXPECT_THROW (disig.verify (MBEDTLS_MD_SHA256, hsh_bad, test2, name), CommonException);
	EXPECT_THROW (disig.verify (MBEDTLS_MD_SHA256, hsh_bad, test3, name), CommonException);

	EXPECT_TRUE (disig.verify (MBEDTLS_MD_SHA256, hsh_ok, test1, name));
	EXPECT_TRUE (disig.verify (MBEDTLS_MD_SHA256, hsh_ok, test2, name));
	EXPECT_TRUE (disig.verify (MBEDTLS_MD_SHA256, hsh_ok, test3, name));

	EXPECT_FALSE (disig.verify (MBEDTLS_MD_SHA256, hsh_other, test1, name));
	EXPECT_FALSE (disig.verify (MBEDTLS_MD_SHA256, hsh_other, test2, name));
	EXPECT_FALSE (disig.verify (MBEDTLS_MD_SHA256, hsh_other, test3, name));

	EXPECT_FALSE (disig.verify (MBEDTLS_MD_SHA256, hsh_ok, hsh_bad, name));
	EXPECT_FALSE (disig.verify (MBEDTLS_MD_SHA256, hsh_other, hsh_bad, name));
	EXPECT_FALSE (disig.verify (MBEDTLS_MD_SHA256, hsh_ok, hsh_other, name));
}


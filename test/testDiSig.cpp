/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

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

	DiSig disig;
	COMMON_VECTOR vec;

	const mbedtls_ecp_curve_info *curve_info;
	for (curve_info = mbedtls_ecp_curve_list(); curve_info->grp_id != MBEDTLS_ECP_DP_NONE; curve_info++) {
		disig.generate_new_key (curve_info->name);
		disig.add_decryption_key (disig.get_encryption_pubkey ());

		std::string test = disig.sign (MBEDTLS_MD_SHA256, hsh_ok);
		EXPECT_THROW (disig.sign (MBEDTLS_MD_SHA256, hsh_bad), CommonException);

		vec.push_back (test);
	}

	for (const std::string &test: vec) {
		EXPECT_THROW (disig.verify (MBEDTLS_MD_SHA256, hsh_bad, test, name), CommonException);
		EXPECT_TRUE (disig.verify (MBEDTLS_MD_SHA256, hsh_ok, test, name));
		EXPECT_FALSE (disig.verify (MBEDTLS_MD_SHA256, hsh_other, test, name));
	}

	EXPECT_FALSE (disig.verify (MBEDTLS_MD_SHA256, hsh_ok, hsh_bad, name));
	EXPECT_FALSE (disig.verify (MBEDTLS_MD_SHA256, hsh_other, hsh_bad, name));
	EXPECT_FALSE (disig.verify (MBEDTLS_MD_SHA256, hsh_ok, hsh_other, name));
}

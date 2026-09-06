/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

#ifdef SHAGA_FULL
TEST (ReDataConfig, incremental_hmac_matches_mbedtls_and_supports_aliasing)
{
	const std::string plain (256'000, 'p');
	for (const auto digest : {ReDataConfig::DIGEST::HMAC_SHA256, ReDataConfig::DIGEST::HMAC_SHA512}) {
		const mbedtls_md_type_t md_type = (ReDataConfig::DIGEST::HMAC_SHA256 == digest) ? MBEDTLS_MD_SHA256 : MBEDTLS_MD_SHA512;
		const mbedtls_md_info_t *const info = ::mbedtls_md_info_from_type (md_type);
		ASSERT_NE (nullptr, info);

		for (const std::string &key : {std::string (20, '\x0b'), std::string (200, 'k')}) {
			std::string expected (::mbedtls_md_get_size (info), '\0');
			ASSERT_EQ (0, ::mbedtls_md_hmac (info,
				reinterpret_cast<const unsigned char *> (key.data ()), key.size (),
				reinterpret_cast<const unsigned char *> (plain.data ()), plain.size (),
				reinterpret_cast<unsigned char *> (expected.data ())));

			ReDataConfig config;
			config.set_digest (digest);
			std::string actual;
			config.calc_digest (plain, actual, key);
			EXPECT_EQ (expected, actual);

			std::string alias = plain;
			config.calc_digest (alias, alias, key);
			EXPECT_EQ (expected, alias);
		}
	}
}
#endif // SHAGA_FULL

TEST (ReDataConfig, constructors)
{
	ReDataConfig first;
	first.set_digest (ReDataConfig::DIGEST::SHA256).set_crypto (ReDataConfig::CRYPTO::AES_256_CBC);

	ReDataConfig second (first);

	EXPECT_TRUE (first.get_digest() == second.get_digest ());
	EXPECT_TRUE (first.get_crypto() == second.get_crypto ());

	ReDataConfig third (std::move (first));

	EXPECT_TRUE (third.get_digest() == second.get_digest ());
	EXPECT_TRUE (third.get_crypto() == second.get_crypto ());
}

TEST (ReDataConfig, assign_operators)
{
	ReDataConfig first;
	first.set_digest (ReDataConfig::DIGEST::SHA256).set_crypto (ReDataConfig::CRYPTO::AES_256_CBC);

	ReDataConfig second = first;

	EXPECT_TRUE (first.get_digest() == second.get_digest ());
	EXPECT_TRUE (first.get_crypto() == second.get_crypto ());

	ReDataConfig third = std::move (first);

	EXPECT_TRUE (third.get_digest() == second.get_digest ());
	EXPECT_TRUE (third.get_crypto() == second.get_crypto ());
}

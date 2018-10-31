/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <shaga.h>
#include <gtest/gtest.h>

using namespace shaga;

static std::string _redata_make_string (const size_t sze, const uint32_t seed)
{
	uint32_t nSeed = 5323 + seed;
	std::string str;
	str.reserve (sze);

	for (size_t i = 0; i < sze; ++i) {
		nSeed = (8253729 * nSeed + 2396403);
		str.push_back (nSeed & UINT8_MAX);
	}

	return str;
}

static void _redata_test (const shaga::ReDataConfig &config, const bool use_header, const bool use_key_ident)
{
	COMMON_VECTOR keys_hmac, keys_crypto;
	ReData::KEY_IDENT_VECTOR keys_ident;

	const size_t hmac_key_size = config.get_digest_hmac_block_size ();
	const size_t crypto_key_size = config.get_crypto_key_size ();

	for (size_t i = 0; i < 16; i++) {
		keys_hmac.push_back (_redata_make_string (hmac_key_size, i));
		keys_crypto.push_back (_redata_make_string (crypto_key_size, i + 1));
		keys_ident.push_back (i);
	}

	ReData rd1, rd2;
	rd1.set_hmac_keys (keys_hmac);
	rd1.set_crypto_keys (keys_crypto);
	rd1.set_key_idents (keys_ident);
	rd1.use_config_header (use_header);
	rd1.use_key_ident (use_key_ident);
	rd1.set_config (config);

	std::reverse (keys_hmac.begin (), keys_hmac.end ());
	std::reverse (keys_crypto.begin (), keys_crypto.end ());
	std::reverse (keys_ident.begin (), keys_ident.end ());

	rd2.set_hmac_keys (keys_hmac);
	rd2.set_crypto_keys (keys_crypto);
	rd2.set_key_idents (keys_ident);
	rd2.use_config_header (use_header);
	rd2.use_key_ident (use_key_ident);
	if (false == use_header) {
		/* Set config only if header is not present */
		rd2.set_config (config);
	}

	const int test_str_sizes[] = {0, 1, 2, 3, 5, 16, 21, 25, 28, 31, 32, 60, 64, 65, 100, 1000, 10000, 0x100, 0x10000, -1};

	for (size_t i = 0; test_str_sizes[i] >= 0; ++i) {
		if ((i % 7) == 0) {
			rd1.set_key_id (i / 5);
		}
		if ((i % 3) == 0) {
			rd2.set_key_id (0);
		}

		const std::string plain = _redata_make_string (test_str_sizes[i], i);

		std::string msg;
		ASSERT_NO_THROW (rd1.encode (plain, msg));

		std::string plain2;
		ASSERT_NO_THROW (rd2.decode (msg, plain2));

		EXPECT_TRUE (plain.compare (plain2) == 0);

		/* Expect first or the last key minus rd1.key_id has to be used, depending on whether encryption and HMAC is used */
		/* rd1 and rd2 got keys in reversed orders from each other */
		EXPECT_TRUE (rd2.get_key_id () == 0 || rd2.get_key_id () == (keys_hmac.size () - rd1.get_key_id () - 1));
	}
}

TEST (ReData, enc_dec)
{
	ReDataConfig config;

	/* Try all combinations of crypto and digest algorithms */
	for (const auto &digest : config.DIGEST_MAP) {
		for (const auto &crypto : config.CRYPTO_MAP) {
			config.reset ();
			config.set_digest (digest.first).set_crypto (crypto.first);
			for (int i = 0; i < 4; ++i) {
				_redata_test (config, (i & 0b01) == 0, (i & 0b10) == 0);
			}

			config.reset ();
			config.set_digest (digest.second).set_crypto (crypto.second);
			for (int i = 0; i < 4; ++i) {
				_redata_test (config, (i & 0b01) == 0, (i & 0b10) == 0);
			}
		}
	}
}

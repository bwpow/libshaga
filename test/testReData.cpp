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

static void _redata_test (const shaga::ReDataConfig &config, const bool use_header, const bool use_key_ident, const bool use_mix)
{
	COMMON_VECTOR keys_hmac, keys_crypto;
	ReData::KEY_IDENT_VECTOR keys_ident;

	/* Key mixing doesn't support more than 64 bytes for key */
	const size_t hmac_key_size = config.get_digest_hmac_block_size ();
	const size_t crypto_key_size = config.get_crypto_key_size ();

	if (0 == crypto_key_size && 0 == hmac_key_size && true == use_mix) {
		/* This test is pointless, because there is no key to mix */
		return;
	}

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
		std::string plain2;

		if (true == use_mix) {
			const std::string mix = _redata_make_string (16, i);
			rd1.set_mix_key (mix);
			rd2.set_mix_key (mix);

			ASSERT_NO_THROW (rd1.encode (plain, msg, true));
			ASSERT_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::ONLY_NORMAL), CommonException);

			ASSERT_NO_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::BOTH_MIXED_FIRST));
			EXPECT_TRUE (plain.compare (plain2) == 0);
			EXPECT_TRUE (rd2.get_last_decode_was_mixed ());

			ASSERT_NO_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::BOTH_NORMAL_FIRST));
			EXPECT_TRUE (plain.compare (plain2) == 0);
			EXPECT_TRUE (rd2.get_last_decode_was_mixed ());

			ASSERT_NO_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::ONLY_MIXED));
			EXPECT_TRUE (plain.compare (plain2) == 0);
			EXPECT_TRUE (rd2.get_last_decode_was_mixed ());
		}
		else {
			rd1.clear_mix_key ();
			rd2.clear_mix_key ();

			ASSERT_THROW (rd1.encode (plain, msg, true), CommonException);

			ASSERT_NO_THROW (rd1.encode (plain, msg));
			ASSERT_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::ONLY_MIXED), CommonException);

			ASSERT_NO_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::BOTH_MIXED_FIRST));
			EXPECT_TRUE (plain.compare (plain2) == 0);
			EXPECT_FALSE (rd2.get_last_decode_was_mixed ());

			ASSERT_NO_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::BOTH_NORMAL_FIRST));
			EXPECT_TRUE (plain.compare (plain2) == 0);
			EXPECT_FALSE (rd2.get_last_decode_was_mixed ());

			ASSERT_NO_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::ONLY_NORMAL));
			EXPECT_TRUE (plain.compare (plain2) == 0);
			EXPECT_FALSE (rd2.get_last_decode_was_mixed ());
		}

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
			for (int i = 0; i <= 0b111; ++i) {
				_redata_test (config, (i & 0b001) == 0, (i & 0b010) == 0, (i & 0b100) == 0);
			}

			config.reset ();
			config.set_digest (digest.second).set_crypto (crypto.second);
			for (int i = 0; i <= 0b111; ++i) {
				_redata_test (config, (i & 0b001) == 0, (i & 0b010) == 0, (i & 0b100) == 0);
			}
		}
	}
}

#include "../src/internal_shake256.h"

TEST (ReData, shake256)
{
	/* SHAKE256, message of length 0 */
	const std::string ref_empty = BIN::from_hex ("AB0BAE316339894304E35877B0C28A9B1FD166C796B9CC258A064A8F57E27F2A");
	/* SHAKE256, 1600-bit test pattern */
	const std::string ref_pattern = BIN::from_hex ("6A1A9D7846436E4DCA5728B6F760EEF0CA92BF0BE5615E96959D767197A0BEEB");

	_shake256_ctx_t ctx;
	std::string out;

	/* 20 bytes of 0xA3 */
	const std::string buf (20, 0xA3);

	/* 200 bytes of 0xA3 */
	const std::string buf2 (200, 0xA3);

	for (size_t i = 0; i <= 0b111; ++i) {
		_shake256_init (&ctx);

		if (i & 0b01) {
			if (i & 0b100) {
				for (size_t j = 0; j < 10; ++j) {
					_shake256_update (&ctx, buf);
				}
			}
			else {
				_shake256_update (&ctx, buf2);
			}
		}

		_shake256_xof (&ctx);

		/* Generate output, erase first 480 bytes and leave only last 32 bytes */
		if (i & 0b10) {
			_shake256_out (&ctx, out, 512);
			out.erase (0, 480);
		}
		else {
			for (size_t j = 0; j < 512; j += 32) {
				_shake256_out (&ctx, out, 32);
			}
		}

		if (i & 0b01) {
			EXPECT_TRUE (out.compare (ref_pattern) == 0);
		}
		else {
			EXPECT_TRUE (out.compare (ref_empty) == 0);
		}
	}
}

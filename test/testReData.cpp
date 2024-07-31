/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2024, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

static const std::map <std::string, ReDataConfig::DIGEST> DIGEST_MAP {
	{"none"s, ReDataConfig::DIGEST::NONE},

	{"crc8"s, ReDataConfig::DIGEST::CRC8},
	{"crc-8"s, ReDataConfig::DIGEST::CRC8},

	{"crc32"s, ReDataConfig::DIGEST::CRC32},
	{"crc-32"s, ReDataConfig::DIGEST::CRC32},
	{"crc32c"s, ReDataConfig::DIGEST::CRC32},
	{"crc-32c"s, ReDataConfig::DIGEST::CRC32},

	{"crc64"s, ReDataConfig::DIGEST::CRC64},
	{"crc-64"s, ReDataConfig::DIGEST::CRC64},

	{"sha256"s, ReDataConfig::DIGEST::SHA256},
	{"sha-256"s, ReDataConfig::DIGEST::SHA256},

	{"sha512"s, ReDataConfig::DIGEST::SHA512},
	{"sha-512"s, ReDataConfig::DIGEST::SHA512},

	{"hmac-sha256"s, ReDataConfig::DIGEST::HMAC_SHA256},
	{"hmac-sha-256"s, ReDataConfig::DIGEST::HMAC_SHA256},

	{"hmac-sha512"s, ReDataConfig::DIGEST::HMAC_SHA512},
	{"hmac-sha-512"s, ReDataConfig::DIGEST::HMAC_SHA512},

	{"halfsiphash24-32"s, ReDataConfig::DIGEST::HALFSIPHASH24_32},
	{"halfsiphash-2-4-32"s, ReDataConfig::DIGEST::HALFSIPHASH24_32},

	{"halfsiphash24-64"s, ReDataConfig::DIGEST::HALFSIPHASH24_64},
	{"halfsiphash-2-4-64"s, ReDataConfig::DIGEST::HALFSIPHASH24_64},

	{"halfsiphash48-32"s, ReDataConfig::DIGEST::HALFSIPHASH48_32},
	{"halfsiphash-4-8-32"s, ReDataConfig::DIGEST::HALFSIPHASH48_32},

	{"halfsiphash48-64"s, ReDataConfig::DIGEST::HALFSIPHASH48_64},
	{"halfsiphash-4-8-64"s, ReDataConfig::DIGEST::HALFSIPHASH48_64},

	{"siphash24-64"s, ReDataConfig::DIGEST::SIPHASH24_64},
	{"siphash-2-4-64"s, ReDataConfig::DIGEST::SIPHASH24_64},

	{"siphash24-128"s, ReDataConfig::DIGEST::SIPHASH24_128},
	{"siphash-2-4-128"s, ReDataConfig::DIGEST::SIPHASH24_128},

	{"siphash48-64"s, ReDataConfig::DIGEST::SIPHASH48_64},
	{"siphash-4-8-64"s, ReDataConfig::DIGEST::SIPHASH48_64},

	{"siphash48-128"s, ReDataConfig::DIGEST::SIPHASH48_128},
	{"siphash-4-8-128"s, ReDataConfig::DIGEST::SIPHASH48_128},
};

static const std::map <std::string, ReDataConfig::CRYPTO> CRYPTO_MAP {
	{"none"s, ReDataConfig::CRYPTO::NONE},

	{"aes"s, ReDataConfig::CRYPTO::AES_128_CBC},
	{"aes128"s, ReDataConfig::CRYPTO::AES_128_CBC},
	{"aes-128"s, ReDataConfig::CRYPTO::AES_128_CBC},
	{"aes-128-cbc"s, ReDataConfig::CRYPTO::AES_128_CBC},

	{"aes256"s, ReDataConfig::CRYPTO::AES_256_CBC},
	{"aes-256"s, ReDataConfig::CRYPTO::AES_256_CBC},
	{"aes-256-cbc"s, ReDataConfig::CRYPTO::AES_256_CBC},

	{"chacha20"s, ReDataConfig::CRYPTO::CHACHA20},
	{"chacha"s, ReDataConfig::CRYPTO::CHACHA20},

	{"chacha20-poly1305"s, ReDataConfig::CRYPTO::CHACHA20_POLY1305},
	{"chacha-poly"s, ReDataConfig::CRYPTO::CHACHA20_POLY1305},
	{"chachapoly"s, ReDataConfig::CRYPTO::CHACHA20_POLY1305},
};

static std::string _redata_make_string (const size_t sze, const uint32_t seed)
{
	uint32_t nSeed = seed;
	std::string str;
	str.reserve (sze);

	for (size_t i = 0; i < sze; ++i) {
		nSeed = (8253729 * nSeed + 2396403);
		str.push_back (nSeed & UINT8_MAX);
	}

	return str;
}

static void _redata_test (shaga::ReDataConfig &config, const bool use_header, const bool use_key_ident, const bool use_mix, const bool use_iv_counter)
{
	COMMON_VECTOR keys_hmac;
	COMMON_VECTOR keys_crypto;
	ReData::KEY_IDENT_VECTOR keys_ident;

	/* Key mixing doesn't support more than 64 bytes for key */
	const size_t hmac_key_size = config.get_digest_hmac_key_size ();
	const size_t crypto_key_size = config.get_crypto_key_size ();
	const size_t crypto_block_size = config.get_crypto_block_size ();
	const size_t crypto_iv_size = config.get_crypto_iv_size ();

	if (0 == crypto_key_size && 0 == hmac_key_size && true == use_mix) {
		/* This test is pointless, because there is no key to mix */
		return;
	}

	if (true == use_iv_counter && 0 == crypto_iv_size) {
		/* It is not possible to use counter without IV */
		return;
	}

	ASSERT_TRUE ((crypto_key_size == 0) || (crypto_block_size != 0 && crypto_iv_size != 0));

	for (size_t i = 0; i < 16; i++) {
		keys_hmac.push_back (_redata_make_string (hmac_key_size, 0x11 << i));
		keys_crypto.push_back (_redata_make_string (crypto_key_size, 0x12 << i));
		keys_ident.push_back (i);
	}

	if (crypto_iv_size > 0) {
		/* IV is normally generated randomly, but during testing, we want to use reproducible execution. */
		if (use_iv_counter) {
			const std::string iv = _redata_make_string (crypto_iv_size - sizeof (uint32_t), 0xACE2);
			config.set_user_iv (iv);
			config.set_iv_counter (0xB00F1337);
		}
		else {
			const std::string iv = _redata_make_string (crypto_iv_size, 0xACE2);
			config.set_user_iv (iv);
			config.unset_iv_counter ();
		}
	}
	else {
		config.unset_iv_counter ();
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

	const int test_str_sizes[] = {0, 1, 2, 3, 5, 9, 16, 21, 25, 28, 29, 30, 31, 32, 60, 61, 62, 63, 64, 65, 100, 1'000, 10'000, 0x100, 0x10'000, -1};

	for (size_t i = 0; test_str_sizes[i] >= 0; ++i) {
		if ((i % 7) == 0) {
			rd1.set_key_id (i / 5);
		}
		if ((i % 3) == 0) {
			rd2.set_key_id (0);
		}

		const std::string desc = fmt::format ("Digest = {}, Crypto = {}, use_header = {}, use_key_ident = {}, use_mix = {}, str_size = {}"sv,
			config.get_digest_text (),
			config.get_crypto_text (),
			use_header,
			use_key_ident,
			use_mix,
			test_str_sizes[i]);

		const std::string plain = _redata_make_string (test_str_sizes[i], i);
		std::string msg;
		std::string plain2;

		msg.reserve (plain.size () * 2);
		plain2.reserve (plain.size () * 2);

		if (0 == config.get_digest_result_size () && false == config.has_mac ()) {
			ASSERT_THROW (rd1.encode (plain, msg), CommonException) << desc;
			continue;
		}

		if (true == use_mix) {
			const std::string mix = _redata_make_string (16, i ^ 0b1010101);
			rd1.set_mix_key (mix);
			rd2.set_mix_key (mix);

			ASSERT_NO_THROW (rd1.encode (plain, msg, true)) << desc;
			ASSERT_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::ONLY_NORMAL), CommonException) << desc;

			ASSERT_NO_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::BOTH_MIXED_FIRST)) << desc;
			EXPECT_TRUE (plain.compare (plain2) == 0) << desc;
			EXPECT_TRUE (rd2.get_last_decode_was_mixed ()) << desc;

			ASSERT_NO_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::BOTH_NORMAL_FIRST)) << desc;
			EXPECT_TRUE (plain.compare (plain2) == 0) << desc;
			EXPECT_TRUE (rd2.get_last_decode_was_mixed ()) << desc;

			ASSERT_NO_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::ONLY_MIXED)) << desc;
			EXPECT_TRUE (plain.compare (plain2) == 0) << desc;
			EXPECT_TRUE (rd2.get_last_decode_was_mixed ()) << desc;
		}
		else {
			rd1.clear_mix_key ();
			rd2.clear_mix_key ();

			ASSERT_THROW (rd1.encode (plain, msg, true), CommonException) << desc;

			ASSERT_NO_THROW (rd1.encode (plain, msg)) << desc;
			ASSERT_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::ONLY_MIXED), CommonException) << desc;

			ASSERT_NO_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::BOTH_MIXED_FIRST)) << desc;
			EXPECT_TRUE (plain.compare (plain2) == 0) << desc;
			EXPECT_FALSE (rd2.get_last_decode_was_mixed ()) << desc;

			ASSERT_NO_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::BOTH_NORMAL_FIRST)) << desc;
			EXPECT_TRUE (plain.compare (plain2) == 0) << desc;
			EXPECT_FALSE (rd2.get_last_decode_was_mixed ()) << desc;

			ASSERT_NO_THROW (rd2.decode (msg, plain2, nullptr, ReData::MixKeysUse::ONLY_NORMAL)) << desc;
			EXPECT_TRUE (plain.compare (plain2) == 0) << desc;
			EXPECT_FALSE (rd2.get_last_decode_was_mixed ()) << desc;
		}

		/* Expect first or the last key minus rd1.key_id has to be used, depending on whether encryption and HMAC is used */
		/* rd1 and rd2 got keys in reversed orders from each other */
		EXPECT_TRUE (rd2.get_key_id () == 0 || rd2.get_key_id () == (keys_hmac.size () - rd1.get_key_id () - 1)) << desc;
	}
}

TEST (ReData, enc_dec)
{
	ReDataConfig config;

	/* Try all combinations of crypto and digest algorithms */
	for (const auto &digest : DIGEST_MAP) {
		for (const auto &crypto : CRYPTO_MAP) {
			config.reset ();
			config.set_digest (digest.first).set_crypto (crypto.first);
			for (int i = 0; i <= 0b1111; i += 2) {
				_redata_test (config, (i & 0b1) == 0, (i & 0b10) == 0, (i & 0b100) == 0, (i & 0b1000) == 0);
			}

			config.reset ();
			config.set_digest (digest.second).set_crypto (crypto.second);
			for (int i = 1; i <= 0b1111; i += 2) {
				_redata_test (config, (i & 0b1) == 0, (i & 0b10) == 0, (i & 0b100) == 0, (i & 0b1000) == 0);
			}
		}
	}
}

#include "shaga/internal_shake256.h"

TEST (ReData, shake256)
{
	/* SHAKE256, message of length 0 */
	const std::string ref_empty = BIN::from_hex ("AB0BAE316339894304E35877B0C28A9B1FD166C796B9CC258A064A8F57E27F2A"sv);
	/* SHAKE256, 1600-bit test pattern */
	const std::string ref_pattern = BIN::from_hex ("6A1A9D7846436E4DCA5728B6F760EEF0CA92BF0BE5615E96959D767197A0BEEB"sv);

	_shake256_ctx_t ctx;
	std::string out;

	/* 20 bytes of 0xA3 */
	const std::string buf (20, static_cast<char> (0xA3));

	/* 200 bytes of 0xA3 */
	const std::string buf2 (200, static_cast<char> (0xA3));

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

TEST (ReData, random_iv)
{
	ReData rd;
	ReDataConfig config;
	config.set_digest (ReDataConfig::DIGEST::HMAC_SHA512).set_crypto (ReDataConfig::CRYPTO::AES_128_CBC);

	const size_t hmac_key_size = config.get_digest_hmac_key_size ();
	const size_t crypto_key_size = config.get_crypto_key_size ();

	COMMON_VECTOR keys_hmac, keys_crypto;
	keys_hmac.push_back (_redata_make_string (hmac_key_size, 0));
	keys_crypto.push_back (_redata_make_string (crypto_key_size, 1));

	rd.set_hmac_keys (keys_hmac);
	rd.set_crypto_keys (keys_crypto);

	auto check = [&](const ReDataConfig &conf) -> bool {
		if (conf.get_digest () != config.get_digest ()) {
			return false;
		}

		if (conf.get_crypto () != config.get_crypto ()) {
			return false;
		}

		return true;
	};

	const std::string plain = _redata_make_string (64, 2);

	/* This set will contain all CRC64 of encrypted messages. Even when we encrypt the same plain text, IV is random each time */
	std::set<uint64_t> msg_crc_set;

	for (int i = 0; i < 2048; ++i) {
		std::string msg;
		std::string plain2;

		rd.set_config (config);
		ASSERT_NO_THROW (rd.encode (plain, msg));

		ASSERT_THROW (rd.decode (msg.substr (1), plain2, check), std::exception);
		ASSERT_THROW (rd.decode (msg.substr (0, msg.size () - 1), plain2, check), std::exception);
		ASSERT_NO_THROW (rd.decode (msg, plain2, check));

		EXPECT_TRUE (plain.compare (plain2) == 0);

		const uint64_t crc = CRC::crc64 (msg);
		auto res = msg_crc_set.insert (crc);
		EXPECT_TRUE (res.second);
	}
}

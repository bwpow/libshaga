/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2024, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

/*
   SipHash reference C implementation

   Copyright (c) 2016 Jean-Philippe Aumasson <jeanphilippe.aumasson@gmail.com>

   To the extent possible under law, the author(s) have dedicated all copyright
   and related and neighboring rights to this software to the public domain
   worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along
   with this software. If not, see
   <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#define ROTL(x, b) (uint32_t)(((x) << (b)) | ((x) >> (32 - (b))))

#define U32TO8_LE(p, v)                                                        \
	(p)[0] = (uint8_t)((v));                                                   \
	(p)[1] = (uint8_t)((v) >> 8);                                              \
	(p)[2] = (uint8_t)((v) >> 16);                                             \
	(p)[3] = (uint8_t)((v) >> 24);

#define U8TO32_LE(p)                                                           \
	(((uint32_t)((p)[0])) | ((uint32_t)((p)[1]) << 8) |                        \
	 ((uint32_t)((p)[2]) << 16) | ((uint32_t)((p)[3]) << 24))

#define SIPROUND                                                               \
	do {                                                                       \
		v0 += v1;                                                              \
		v1 = ROTL(v1, 5);                                                      \
		v1 ^= v0;                                                              \
		v0 = ROTL(v0, 16);                                                     \
		v2 += v3;                                                              \
		v3 = ROTL(v3, 8);                                                      \
		v3 ^= v2;                                                              \
		v0 += v3;                                                              \
		v3 = ROTL(v3, 7);                                                      \
		v3 ^= v0;                                                              \
		v2 += v1;                                                              \
		v1 = ROTL(v1, 13);                                                     \
		v1 ^= v2;                                                              \
		v2 = ROTL(v2, 16);                                                     \
	} while (0)

static void _halfsiphash_ref_impl (const uint8_t *in, const size_t inlen, const uint8_t *k, uint8_t *out, const size_t outlen, const int cROUNDS, const int dROUNDS)
{
	uint32_t v0 = 0;
	uint32_t v1 = 0;
	uint32_t v2 = UINT32_C(0x6c796765);
	uint32_t v3 = UINT32_C(0x74656462);
	uint32_t k0 = U8TO32_LE(k);
	uint32_t k1 = U8TO32_LE(k + 4);
	uint32_t m;
	int i;
	const uint8_t *end = in + inlen - (inlen % sizeof(uint32_t));
	const int left = inlen & 3;
	uint32_t b = ((uint32_t)inlen) << 24;
	v3 ^= k1;
	v2 ^= k0;
	v1 ^= k1;
	v0 ^= k0;

	if (outlen == 8)
		v1 ^= 0xee;

	for (; in != end; in += 4) {
		m = U8TO32_LE(in);
		v3 ^= m;

		for (i = 0; i < cROUNDS; ++i)
			SIPROUND;

		v0 ^= m;
	}

	switch (left) {
	case 3:
		b |= ((uint32_t)in[2]) << 16;
		HEDLEY_FALL_THROUGH;
	case 2:
		b |= ((uint32_t)in[1]) << 8;
		HEDLEY_FALL_THROUGH;
	case 1:
		b |= ((uint32_t)in[0]);
		break;
	case 0:
		break;
	}

	v3 ^= b;

	for (i = 0; i < cROUNDS; ++i)
		SIPROUND;

	v0 ^= b;

	if (outlen == 8)
		v2 ^= 0xee;
	else
		v2 ^= 0xff;

	for (i = 0; i < dROUNDS; ++i)
		SIPROUND;

	b = v1 ^ v3;
	U32TO8_LE(out, b);

	if (outlen == 4)
		return;

	v1 ^= 0xdd;

	for (i = 0; i < dROUNDS; ++i)
		SIPROUND;

	b = v1 ^ v3;
	U32TO8_LE(out + 4, b);
}

using namespace shaga;

/* When running the same test more times, generated string will differ. This is intended. */
static std::string _halfsiphash_make_string (const size_t sze)
{
	static uint32_t nSeed = 5323;
	std::string str;

	for (size_t i = 0; i < sze; ++i) {
		nSeed = (8253729 * nSeed + 2396403);
		str.push_back (nSeed & UINT8_MAX);
	}

	return str;
}

static void _halfsiphash_ref (const std::string &plain, const std::string &key, std::string &out, const size_t outlen, const int cROUNDS, const int dROUNDS)
{
	ASSERT_TRUE ((outlen == 4) || (outlen == 8));
	ASSERT_TRUE (key.size () == 8);

	uint8_t temp [8];
	_halfsiphash_ref_impl (reinterpret_cast<const uint8_t *> (plain.data ()), plain.size (), reinterpret_cast<const uint8_t *> (key.data ()), temp, outlen, cROUNDS, dROUNDS);

	out.resize (0);
	out.append (reinterpret_cast <const char *> (temp), outlen);
}

static void _halfsiphash_digest_test (const size_t outlen, const int cROUNDS, const int dROUNDS)
{
	const int test_str_sizes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 21, 25, 28, 31, 32, 60, 64, 65, 100, 1000, 1024, 10000, UINT16_MAX + 1, -1};

	for (int j = 0; j < 6; ++j) {
		for (size_t i = 0; test_str_sizes[i] >= 0; ++i) {
			const std::string key_str = _halfsiphash_make_string (8);
			const auto key = Digest::halfsiphash_extract_key (key_str);
			const std::string plain = _halfsiphash_make_string (test_str_sizes[i]);

			/* Create message using Digest functions */
			std::string msg1;
			if (4 == outlen) {
				if (2 == cROUNDS && 4 == dROUNDS) {
					msg1 = BIN::from_uint32 (Digest::halfsiphash24_32t (plain, key));
				}
				else if (4 == cROUNDS && 8 == dROUNDS) {
					msg1 = BIN::from_uint32 (Digest::halfsiphash48_32t (plain, key));
				}
			}
			else if (8 == outlen) {
				if (2 == cROUNDS && 4 == dROUNDS) {
					if (j & 1) {
						auto ret = Digest::halfsiphash24_64t (plain, key);
						msg1 = BIN::from_uint32 (ret.first) + BIN::from_uint32 (ret.second);
					}
					else {
						msg1 = Digest::halfsiphash24_64s (plain, key);
					}
				}
				else if (4 == cROUNDS && 8 == dROUNDS) {
					if (j & 1) {
						auto ret = Digest::halfsiphash48_64t (plain, key);
						msg1 = BIN::from_uint32 (ret.first) + BIN::from_uint32 (ret.second);
					}
					else {
						msg1 = Digest::halfsiphash48_64s (plain, key);
					}
				}
			}
			ASSERT_FALSE (msg1.empty ());

			/* Create message using reference implementation */
			std::string msg2;
			_halfsiphash_ref (plain, key_str, msg2, outlen, cROUNDS, dROUNDS);

			EXPECT_TRUE (msg1.compare (msg2) == 0);
		}
	}
}

static void _halfsiphash_digest_keylen_test (void)
{
	for (int i = 0; i < 100; ++i) {
		if (i != 8) {
			EXPECT_THROW ([[maybe_unused]] auto res = Digest::halfsiphash_extract_key (_halfsiphash_make_string (i)), std::exception);
		}
	}
}

static void _halfsiphash_test (const ReDataConfig &config, const size_t outlen, const int cROUNDS, const int dROUNDS)
{
	ReData redata (config);
	redata.use_config_header (false);

	const int test_str_sizes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 21, 25, 28, 31, 32, 60, 64, 65, 100, 1000, 1024, 10000, UINT16_MAX + 1, -1};

	for (int j = 0; j < 5; ++j) {
		for (size_t i = 0; test_str_sizes[i] >= 0; ++i) {
			const std::string key = _halfsiphash_make_string (8);
			const std::string plain = _halfsiphash_make_string (test_str_sizes[i]);

			redata.set_hmac_key (key);

			/* Create message using ReData */
			std::string msg1;
			ASSERT_NO_THROW (redata.encode (plain, msg1));

			/* Create message using reference implementation */
			std::string msg2;
			_halfsiphash_ref (plain, key, msg2, outlen, cROUNDS, dROUNDS);
			msg2.append (plain);

			EXPECT_TRUE (msg1.compare (msg2) == 0);
		}
	}
}

TEST (ReData, HalfSipHash)
{
	ReDataConfig config;

	//config.reset ();
	//config.set_digest (ReDataConfig::DIGEST::HALFSIPHASH24_32).set_crypto (ReDataConfig::CRYPTO::NONE);
	//_halfsiphash_test (config, 4, 2, 4);

	config.reset ();
	config.set_digest (ReDataConfig::DIGEST::HALFSIPHASH24_64).set_crypto (ReDataConfig::CRYPTO::NONE);
	_halfsiphash_test (config, 8, 2, 4);

	config.reset ();
	config.set_digest (ReDataConfig::DIGEST::HALFSIPHASH48_32).set_crypto (ReDataConfig::CRYPTO::NONE);
	_halfsiphash_test (config, 4, 4, 8);

	config.reset ();
	config.set_digest (ReDataConfig::DIGEST::HALFSIPHASH48_64).set_crypto (ReDataConfig::CRYPTO::NONE);
	_halfsiphash_test (config, 8, 4, 8);

}

TEST (Digest, HalfSipHash)
{
	_halfsiphash_digest_keylen_test ();

	_halfsiphash_digest_test (4, 2, 4);
	_halfsiphash_digest_test (4, 4, 8);

	_halfsiphash_digest_test (8, 2, 4);
	_halfsiphash_digest_test (8, 4, 8);
}

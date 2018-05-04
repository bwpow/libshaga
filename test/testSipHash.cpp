/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <shaga.h>
#include <gtest/gtest.h>

/*
	SipHash reference C implementation

	Copyright (c) 2012-2016 Jean-Philippe Aumasson
	<jeanphilippe.aumasson@gmail.com>
	Copyright (c) 2012-2014 Daniel J. Bernstein <djb@cr.yp.to>

	To the extent possible under law, the author(s) have dedicated all copyright
	and related and neighboring rights to this software to the public domain
	worldwide. This software is distributed without any warranty.

	You should have received a copy of the CC0 Public Domain Dedication along
	with this software. If not, see
	<http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#define ROTL(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define U32TO8_LE(p, v)                                                        \
	(p)[0] = (uint8_t)((v));                                                   \
	(p)[1] = (uint8_t)((v) >> 8);                                              \
	(p)[2] = (uint8_t)((v) >> 16);                                             \
	(p)[3] = (uint8_t)((v) >> 24);

#define U64TO8_LE(p, v)                                                        \
	U32TO8_LE((p), (uint32_t)((v)));                                           \
	U32TO8_LE((p) + 4, (uint32_t)((v) >> 32));

#define U8TO64_LE(p)                                                           \
	(((uint64_t)((p)[0])) | ((uint64_t)((p)[1]) << 8) |                        \
	 ((uint64_t)((p)[2]) << 16) | ((uint64_t)((p)[3]) << 24) |                 \
	 ((uint64_t)((p)[4]) << 32) | ((uint64_t)((p)[5]) << 40) |                 \
	 ((uint64_t)((p)[6]) << 48) | ((uint64_t)((p)[7]) << 56))

#define SIPROUND                                                               \
	do {                                                                       \
		v0 += v1;                                                              \
		v1 = ROTL(v1, 13);                                                     \
		v1 ^= v0;                                                              \
		v0 = ROTL(v0, 32);                                                     \
		v2 += v3;                                                              \
		v3 = ROTL(v3, 16);                                                     \
		v3 ^= v2;                                                              \
		v0 += v3;                                                              \
		v3 = ROTL(v3, 21);                                                     \
		v3 ^= v0;                                                              \
		v2 += v1;                                                              \
		v1 = ROTL(v1, 17);                                                     \
		v1 ^= v2;                                                              \
		v2 = ROTL(v2, 32);                                                     \
	} while (0)

static void _siphash_ref_impl (const uint8_t *in, const size_t inlen, const uint8_t *k, uint8_t *out, const size_t outlen, const int cROUNDS, const int dROUNDS)
{
	uint64_t v0 = 0x736f6d6570736575ULL;
	uint64_t v1 = 0x646f72616e646f6dULL;
	uint64_t v2 = 0x6c7967656e657261ULL;
	uint64_t v3 = 0x7465646279746573ULL;
	uint64_t k0 = U8TO64_LE(k);
	uint64_t k1 = U8TO64_LE(k + 8);
	uint64_t m;
	int i;
	const uint8_t *end = in + inlen - (inlen % sizeof(uint64_t));
	const int left = inlen & 7;
	uint64_t b = ((uint64_t)inlen) << 56;
	v3 ^= k1;
	v2 ^= k0;
	v1 ^= k1;
	v0 ^= k0;

	if (outlen == 16)
		v1 ^= 0xee;

	for (; in != end; in += 8) {
		m = U8TO64_LE(in);
		v3 ^= m;

		for (i = 0; i < cROUNDS; ++i)
			SIPROUND;

		v0 ^= m;
	}

	switch (left) {
	case 7:
		b |= ((uint64_t)in[6]) << 48;
		SHAGA_FALLTHROUGH;
	case 6:
		b |= ((uint64_t)in[5]) << 40;
		SHAGA_FALLTHROUGH;
	case 5:
		b |= ((uint64_t)in[4]) << 32;
		SHAGA_FALLTHROUGH;
	case 4:
		b |= ((uint64_t)in[3]) << 24;
		SHAGA_FALLTHROUGH;
	case 3:
		b |= ((uint64_t)in[2]) << 16;
		SHAGA_FALLTHROUGH;
	case 2:
		b |= ((uint64_t)in[1]) << 8;
		SHAGA_FALLTHROUGH;
	case 1:
		b |= ((uint64_t)in[0]);
		break;
	case 0:
		break;
	}

	v3 ^= b;

	for (i = 0; i < cROUNDS; ++i)
		SIPROUND;

	v0 ^= b;

	if (outlen == 16)
		v2 ^= 0xee;
	else
		v2 ^= 0xff;

	for (i = 0; i < dROUNDS; ++i)
		SIPROUND;

	b = v0 ^ v1 ^ v2 ^ v3;
	U64TO8_LE(out, b);

	if (outlen == 8)
		return;

	v1 ^= 0xdd;

	for (i = 0; i < dROUNDS; ++i)
		SIPROUND;

	b = v0 ^ v1 ^ v2 ^ v3;
	U64TO8_LE(out + 8, b);
}

using namespace shaga;

static std::string _siphash_make_string (const size_t sze)
{
	static uint32_t nSeed = 5323;
	std::string str;

	for (size_t i = 0; i < sze; ++i) {
		nSeed = (8253729 * nSeed + 2396403);
		str.push_back (nSeed & UINT8_MAX);
	}

	return str;
}

static void _siphash_ref (const std::string &plain, const std::string &key, std::string &out, const size_t outlen, const int cROUNDS, const int dROUNDS)
{
	ASSERT_TRUE ((outlen == 8) || (outlen == 16));
	ASSERT_TRUE (key.size () == 16);

	uint8_t temp [16];
	_siphash_ref_impl (reinterpret_cast<const uint8_t *> (plain.data ()), plain.size (), reinterpret_cast<const uint8_t *> (key.data ()), temp, outlen, cROUNDS, dROUNDS);

	out.resize (0);
	out.append (reinterpret_cast <const char *> (temp), outlen);
}

static void _siphash_test (const ReDataConfig &config, const size_t outlen, const int cROUNDS, const int dROUNDS)
{
	ReData redata (config);
	redata.use_config_header (false);

	const int test_str_sizes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 21, 25, 28, 31, 32, 60, 64, 65, 100, 1000, 1024, 10000, UINT16_MAX + 1, -1};

	for (int j = 0; j < 5; ++j) {
		for (size_t i = 0; test_str_sizes[i] >= 0; ++i) {
			const std::string key = _siphash_make_string (16);
			const std::string plain = _siphash_make_string (test_str_sizes[i]);

			redata.set_hmac_key (key);

			/* Create message using ReData */
			std::string msg1;
			ASSERT_NO_THROW (redata.encode (plain, msg1));

			/* Create message using reference implementation */
			std::string msg2;
			_siphash_ref (plain, key, msg2, outlen, cROUNDS, dROUNDS);
			msg2.append (plain);

			EXPECT_TRUE (msg1.compare (msg2) == 0);
		}
	}
}

static void _siphash_crc_test (const size_t outlen, const int cROUNDS, const int dROUNDS)
{
	const int test_str_sizes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 21, 25, 28, 31, 32, 60, 64, 65, 100, 1000, 1024, 10000, UINT16_MAX + 1, -1};

	for (int j = 0; j < 5; ++j) {
		for (size_t i = 0; test_str_sizes[i] >= 0; ++i) {
			const std::string key_str = _siphash_make_string (16);
			const auto key = CRC::siphash_extract_key (key_str);
			const std::string plain = _siphash_make_string (test_str_sizes[i]);

			/* Create message using CRC functions */
			std::string msg1;
			if (8 == outlen) {
				if (2 == cROUNDS && 4 == dROUNDS) {
					msg1 = BIN::from_uint64 (CRC::siphash24 (plain, key));
				}
				else if (4 == cROUNDS && 8 == dROUNDS) {
					msg1 = BIN::from_uint64 (CRC::siphash48 (plain, key));
				}
			}
			else if (16 == outlen) {
				if (2 == cROUNDS && 4 == dROUNDS) {
					msg1 = CRC::siphash24_128 (plain, key);
				}
				else if (4 == cROUNDS && 8 == dROUNDS) {
					msg1 = CRC::siphash48_128 (plain, key);
				}
			}
			ASSERT_FALSE (msg1.empty ());

			/* Create message using reference implementation */
			std::string msg2;
			_siphash_ref (plain, key_str, msg2, outlen, cROUNDS, dROUNDS);

			EXPECT_TRUE (msg1.compare (msg2) == 0);
		}
	}
}

static void _siphash_crc_keylen_test (void)
{
	for (int i = 0; i < 100; ++i) {
		if (i != 16) {
			EXPECT_THROW (CRC::siphash_extract_key (_siphash_make_string (i)), std::exception);
		}
	}

}

TEST (ReData, SipHash)
{
	ReDataConfig config;

	config.reset ();
	config.set_digest (ReDataConfig::DIGEST::SIPHASH24_64).set_crypto (ReDataConfig::CRYPTO::NONE);
	_siphash_test (config, 8, 2, 4);

	config.reset ();
	config.set_digest (ReDataConfig::DIGEST::SIPHASH24_128).set_crypto (ReDataConfig::CRYPTO::NONE);
	_siphash_test (config, 16, 2, 4);

	config.reset ();
	config.set_digest (ReDataConfig::DIGEST::SIPHASH48_64).set_crypto (ReDataConfig::CRYPTO::NONE);
	_siphash_test (config, 8, 4, 8);

	config.reset ();
	config.set_digest (ReDataConfig::DIGEST::SIPHASH48_128).set_crypto (ReDataConfig::CRYPTO::NONE);
	_siphash_test (config, 16, 4, 8);

}

TEST (CRC, SipHash)
{
	_siphash_crc_keylen_test ();

	_siphash_crc_test (8, 2, 4);
	_siphash_crc_test (8, 4, 8);

	_siphash_crc_test (16, 2, 4);
	_siphash_crc_test (16, 4, 8);
}

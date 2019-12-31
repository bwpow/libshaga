/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/

/* SHAKE256 (SHA-3) implementation
 * Downloaded from https://github.com/mjosaarinen/tiny_sha3 on 2018-12-02.
 * Slightly modified to be used only for SHAKE256.
 */

static_assert (sizeof (uint64_t) == 8, "Expected uint64_t to be 8 bytes");

#ifndef ROTL64
	#define ROTL64(x, b) (((x) << (b)) | ((x) >> (64 - (b))))
#endif // ROTL64

// sha3.c
// 19-Nov-11  Markku-Juhani O. Saarinen <mjos@iki.fi>

// Revised 07-Aug-15 to match with official release of FIPS PUB 202 "SHA3"
// Revised 03-Sep-15 for portability + OpenSSL - style API

// update the state with given number of rounds

#ifndef KECCAKF_ROUNDS
	#define KECCAKF_ROUNDS 24
#endif // KECCAKF_ROUNDS

// state context
typedef struct {
	union {                                 // state:
		uint8_t b[200];                     // 8-bit bytes
		uint64_t q[25];                     // 64-bit words
	} st;
	int pt, rsiz, mdlen;                    // these don't overflow
} _shake256_ctx_t;

static inline void _sha3_keccakf (uint64_t st[25])
{
	// constants
	static const uint64_t keccakf_rndc[24] = {
		UINT64_C(0x0000000000000001),
		UINT64_C(0x0000000000008082),
		UINT64_C(0x800000000000808a),
		UINT64_C(0x8000000080008000),
		UINT64_C(0x000000000000808b),
		UINT64_C(0x0000000080000001),
		UINT64_C(0x8000000080008081),
		UINT64_C(0x8000000000008009),
		UINT64_C(0x000000000000008a),
		UINT64_C(0x0000000000000088),
		UINT64_C(0x0000000080008009),
		UINT64_C(0x000000008000000a),
		UINT64_C(0x000000008000808b),
		UINT64_C(0x800000000000008b),
		UINT64_C(0x8000000000008089),
		UINT64_C(0x8000000000008003),
		UINT64_C(0x8000000000008002),
		UINT64_C(0x8000000000000080),
		UINT64_C(0x000000000000800a),
		UINT64_C(0x800000008000000a),
		UINT64_C(0x8000000080008081),
		UINT64_C(0x8000000000008080),
		UINT64_C(0x0000000080000001),
		UINT64_C(0x8000000080008008)
	};
	static const int keccakf_rotc[24] = {
		1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14,
		27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
	};
	static const int keccakf_piln[24] = {
		10, 7,  11, 17, 18, 3, 5,  16, 8,  21, 24, 4,
		15, 23, 19, 13, 12, 2, 20, 14, 22, 9,  6,  1
	};

	// variables
	int i, j, r;
	uint64_t t, bc[5];

	ENDIAN_IS_BIG
	for (i = 0; i < 25; ++i) {
		ENDIAN_BSWAP64 (st[i]);
	}
	ENDIAN_END

	// actual iteration
	for (r = 0; r < KECCAKF_ROUNDS; r++) {

		// Theta
		for (i = 0; i < 5; i++) {
			bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];
		}

		for (i = 0; i < 5; i++) {
			t = bc[(i + 4) % 5] ^ ROTL64 (bc[(i + 1) % 5], 1);
			for (j = 0; j < 25; j += 5) {
				st[j + i] ^= t;
			}
		}

		// Rho Pi
		t = st[1];
		for (i = 0; i < 24; i++) {
			j = keccakf_piln[i];
			bc[0] = st[j];
			st[j] = ROTL64 (t, keccakf_rotc[i]);
			t = bc[0];
		}

		//  Chi
		for (j = 0; j < 25; j += 5) {
			for (i = 0; i < 5; i++) {
				bc[i] = st[j + i];
			}
			for (i = 0; i < 5; i++) {
				st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
			}
		}

		//  Iota
		st[0] ^= keccakf_rndc[r];
	}

	ENDIAN_IS_BIG
	for (i = 0; i < 25; ++i) {
		ENDIAN_BSWAP64 (st[i]);
	}
	ENDIAN_END
}

static void _shake256_init (_shake256_ctx_t *c)
{
	for (size_t i = 0; i < 25; ++i) {
		c->st.q[i] = 0;
	}
	c->mdlen = 32;
	c->rsiz = 200 - 2 * c->mdlen;
	c->pt = 0;
}

static void _shake256_update (_shake256_ctx_t *c, const std::string &data)
{
	int j = c->pt;
	for (size_t i = 0; i < data.size (); i++) {
		c->st.b[j++] ^= static_cast<uint8_t> (data[i]);
		if (j >= c->rsiz) {
			_sha3_keccakf(c->st.q);
			j = 0;
		}
	}
	c->pt = j;
}

static void _shake256_xof (_shake256_ctx_t *c)
{
	c->st.b[c->pt] ^= 0x1F;
	c->st.b[c->rsiz - 1] ^= 0x80;
	_sha3_keccakf(c->st.q);
	c->pt = 0;
}

static void _shake256_out (_shake256_ctx_t *c, std::string &out, const size_t len)
{
	size_t i;
	int j;

	out.resize (len);

	j = c->pt;
	for (i = 0; i < len; ++i) {
		if (j >= c->rsiz) {
			_sha3_keccakf(c->st.q);
			j = 0;
		}
		out[i] = static_cast<char> (c->st.b[j++]);
	}
	c->pt = j;
}

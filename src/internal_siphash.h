/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/

/* SipHash implementation based on Reference implementation of SipHash.
 * Website: https://131002.net/siphash/
 * Downloaded from https://github.com/veorq/SipHash on 2017-11-01.
 *
 * Copyright (c) 2012-2016 Jean-Philippe Aumasson <jeanphilippe.aumasson@gmail.com>
 * Copyright (c) 2012-2014 Daniel J. Bernstein <djb@cr.yp.to>
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 */

static_assert (sizeof (uint64_t) == 8, "Expected uint64_t to be 8 bytes");

static inline void _siphash_to_uint64 (const uint8_t *data, uint64_t &v)
{
	::memcpy (&v, data, 8);

	ENDIAN_IS_BIG
	ENDIAN_BSWAP64 (v);
	ENDIAN_END
}

static inline uint64_t _siphash_to_uint64 (const uint8_t *data)
{
	uint64_t v;

	::memcpy (&v, data, 8);

	ENDIAN_IS_BIG
	ENDIAN_BSWAP64 (v);
	ENDIAN_END

	return v;
}

#ifndef ROTL64
	#define ROTL64(x, b) (((x) << (b)) | ((x) >> (64 - (b))))
#endif // ROTL64

#define SIPHASH_ROUND \
	vv0 += vv1;												\
	vv2 += vv3;												\
	vv1 = ROTL64 (vv1, 13ULL);								\
	vv3 = ROTL64 (vv3, 16ULL);								\
	vv1 ^= vv0;												\
	vv3 ^= vv2;												\
	vv0 = ROTL64 (vv0, 32ULL);								\
	vv2 += vv1;												\
	vv0 += vv3;												\
	vv1 = ROTL64 (vv1, 17ULL);								\
	vv3 = ROTL64 (vv3, 21ULL);								\
	vv1 ^= vv2;												\
	vv3 ^= vv0;												\
	vv2 = ROTL64 (vv2, 32ULL);

#define SIPHASH_LAST \
	val = static_cast<uint64_t> (SIPHASH_DATA_SIZE & 0xff) << 56;	\
	switch (SIPHASH_DATA_SIZE & 7) {								\
		case 7:														\
			val |= static_cast<uint64_t> (in[6]) << 48;				\
			SHAGA_FALLTHROUGH;                                      \
		case 6:														\
			val |= static_cast<uint64_t> (in[5]) << 40;				\
			SHAGA_FALLTHROUGH;                                      \
		case 5:														\
			val |= static_cast<uint64_t> (in[4]) << 32;				\
			SHAGA_FALLTHROUGH;                                      \
		case 4:														\
			val |= static_cast<uint64_t> (in[3]) << 24;				\
			SHAGA_FALLTHROUGH;                                      \
		case 3:														\
			val |= static_cast<uint64_t> (in[2]) << 16;				\
			SHAGA_FALLTHROUGH;                                      \
		case 2:														\
			val |= static_cast<uint64_t> (in[1]) << 8;				\
			SHAGA_FALLTHROUGH;                                      \
		case 1:														\
			val |= static_cast<uint64_t> (in[0]);					\
			SHAGA_FALLTHROUGH;                                      \
		case 0:														\
			break;													\
	}

#define SIPHASH_BEGIN \
	uint64_t val;																\
	const uint8_t *in = SIPHASH_DATA;											\
	const uint8_t *end = in + (SIPHASH_DATA_SIZE - (SIPHASH_DATA_SIZE & 7));	\
	SIPHASH_BEGIN_KEYS

static inline uint64_t _calc_siphash24 SIPHASH_PARAMS
{
	SIPHASH_BEGIN

	for (; in != end; in += 8) {
		_siphash_to_uint64 (in, val);
		vv3 ^= val;
		SIPHASH_ROUND
		SIPHASH_ROUND
		vv0 ^= val;
	}

	SIPHASH_LAST

	vv3 ^= val;
	SIPHASH_ROUND
	SIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xffULL;

	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND

	return (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline std::string _calc_siphash24_64 SIPHASH_PARAMS
{
	SIPHASH_BEGIN

	for (; in != end; in += 8) {
		_siphash_to_uint64 (in, val);
		vv3 ^= val;
		SIPHASH_ROUND
		SIPHASH_ROUND
		vv0 ^= val;
	}

	SIPHASH_LAST

	vv3 ^= val;
	SIPHASH_ROUND
	SIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xffULL;

	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND

	return BIN::from_uint64 (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline std::string _calc_siphash24_128 SIPHASH_PARAMS
{
	SIPHASH_BEGIN

	vv1 ^= 0xeeULL;

	for (; in != end; in += 8) {
		_siphash_to_uint64 (in, val);
		vv3 ^= val;
		SIPHASH_ROUND
		SIPHASH_ROUND
		vv0 ^= val;
	}


	SIPHASH_LAST
	vv3 ^= val;
	SIPHASH_ROUND
	SIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xeeULL;

	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND

	val = vv0 ^ vv1 ^ vv2 ^ vv3;

	vv1 ^= 0xddULL;

	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND

	return BIN::from_uint64 (val) + BIN::from_uint64 (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline CRC::SipHash128_t _calc_siphash24_128t SIPHASH_PARAMS
{
	SIPHASH_BEGIN

	vv1 ^= 0xeeULL;

	for (; in != end; in += 8) {
		_siphash_to_uint64 (in, val);
		vv3 ^= val;
		SIPHASH_ROUND
		SIPHASH_ROUND
		vv0 ^= val;
	}


	SIPHASH_LAST
	vv3 ^= val;
	SIPHASH_ROUND
	SIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xeeULL;

	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND

	val = vv0 ^ vv1 ^ vv2 ^ vv3;

	vv1 ^= 0xddULL;

	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND

	return std::make_pair (val, vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline uint64_t _calc_siphash48 SIPHASH_PARAMS
{
	SIPHASH_BEGIN

	for (; in != end; in += 8) {
		_siphash_to_uint64 (in, val);
		vv3 ^= val;
		SIPHASH_ROUND
		SIPHASH_ROUND
		SIPHASH_ROUND
		SIPHASH_ROUND
		vv0 ^= val;
	}

	SIPHASH_LAST
	vv3 ^= val;
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xffULL;

	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND

	return (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline std::string _calc_siphash48_64 SIPHASH_PARAMS
{
	SIPHASH_BEGIN

	for (; in != end; in += 8) {
		_siphash_to_uint64 (in, val);
		vv3 ^= val;
		SIPHASH_ROUND
		SIPHASH_ROUND
		SIPHASH_ROUND
		SIPHASH_ROUND
		vv0 ^= val;
	}

	SIPHASH_LAST
	vv3 ^= val;
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xffULL;

	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND

	return BIN::from_uint64 (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline std::string _calc_siphash48_128 SIPHASH_PARAMS
{
	SIPHASH_BEGIN

	vv1 ^= 0xeeULL;

	for (; in != end; in += 8) {
		_siphash_to_uint64 (in, val);
		vv3 ^= val;
		SIPHASH_ROUND
		SIPHASH_ROUND
		SIPHASH_ROUND
		SIPHASH_ROUND
		vv0 ^= val;
	}

	SIPHASH_LAST
	vv3 ^= val;
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xeeULL;

	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND

	val = vv0 ^ vv1 ^ vv2 ^ vv3;

	vv1 ^= 0xddULL;

	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND

	return BIN::from_uint64 (val) + BIN::from_uint64 (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline CRC::SipHash128_t _calc_siphash48_128t SIPHASH_PARAMS
{
	SIPHASH_BEGIN

	vv1 ^= 0xeeULL;

	for (; in != end; in += 8) {
		_siphash_to_uint64 (in, val);
		vv3 ^= val;
		SIPHASH_ROUND
		SIPHASH_ROUND
		SIPHASH_ROUND
		SIPHASH_ROUND
		vv0 ^= val;
	}

	SIPHASH_LAST
	vv3 ^= val;
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xeeULL;

	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND

	val = vv0 ^ vv1 ^ vv2 ^ vv3;

	vv1 ^= 0xddULL;

	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND
	SIPHASH_ROUND

	return std::make_pair (val, vv0 ^ vv1 ^ vv2 ^ vv3);
}

#undef SIPHASH_ROUND
#undef SIPHASH_LAST
#undef SIPHASH_BEGIN

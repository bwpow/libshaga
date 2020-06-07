/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

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

#ifndef SIPHASH_ROTL64
	#define SIPHASH_ROTL64(x, b) (((x) << (b)) | ((x) >> (64 - (b))))
#endif // SIPHASH_ROTL64

#define SIPHASH_ROUND \
	vv0 += vv1;							\
	vv2 += vv3;							\
	vv1 = SIPHASH_ROTL64 (vv1, 13ULL);	\
	vv3 = SIPHASH_ROTL64 (vv3, 16ULL);	\
	vv1 ^= vv0;							\
	vv3 ^= vv2;							\
	vv0 = SIPHASH_ROTL64 (vv0, 32ULL);	\
	vv2 += vv1;							\
	vv0 += vv3;							\
	vv1 = SIPHASH_ROTL64 (vv1, 17ULL);	\
	vv3 = SIPHASH_ROTL64 (vv3, 21ULL);	\
	vv1 ^= vv2;							\
	vv3 ^= vv0;							\
	vv2 = SIPHASH_ROTL64 (vv2, 32ULL);

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
	uint64_t val;																	\
	const uint8_t *in = SIPHASH_DATA;												\
	const uint8_t *const end = in + (SIPHASH_DATA_SIZE - (SIPHASH_DATA_SIZE & 7));	\
	SIPHASH_BEGIN_KEYS

///////////////////////////////////////////////////////////////////////////////
// SipHash-2-4 output 64bit  //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define SIPHASH24_64_BODY \
	SIPHASH_BEGIN					\
	for (; in != end; in += 8) {	\
		BIN::_to_uint64 (val, in);	\
		vv3 ^= val;					\
		SIPHASH_ROUND				\
		SIPHASH_ROUND				\
		vv0 ^= val;					\
	}								\
	SIPHASH_LAST					\
	vv3 ^= val;						\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	vv0 ^= val;						\
	vv2 ^= 0xffULL;					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND

static inline uint64_t _calc_siphash24_64t (SIPHASH_PARAMS)
{
	SIPHASH24_64_BODY
	return (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline void _calc_siphash24_64tv (SIPHASH_PARAMS, uint64_t &out)
{
	SIPHASH24_64_BODY
	out = (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline std::string _calc_siphash24_64s (SIPHASH_PARAMS)
{
	SIPHASH24_64_BODY
	return BIN::from_uint64 (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline void _calc_siphash24_64sv (SIPHASH_PARAMS, std::string &out)
{
	SIPHASH24_64_BODY
	out.resize (8);
	BIN::_from_uint64 (vv0 ^ vv1 ^ vv2 ^ vv3, out.data ());
}

#undef SIPHASH24_64_BODY

///////////////////////////////////////////////////////////////////////////////
// SipHash-4-8 output 64bit  //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define SIPHASH48_64_BODY \
	SIPHASH_BEGIN					\
	for (; in != end; in += 8) {	\
		BIN::_to_uint64 (val, in);	\
		vv3 ^= val;					\
		SIPHASH_ROUND				\
		SIPHASH_ROUND				\
		SIPHASH_ROUND				\
		SIPHASH_ROUND				\
		vv0 ^= val;					\
	}								\
	SIPHASH_LAST					\
	vv3 ^= val;						\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	vv0 ^= val;						\
	vv2 ^= 0xffULL;					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND

static inline uint64_t _calc_siphash48_64t (SIPHASH_PARAMS)
{
	SIPHASH48_64_BODY
	return (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline void _calc_siphash48_64tv (SIPHASH_PARAMS, uint64_t &out)
{
	SIPHASH48_64_BODY
	out = (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline std::string _calc_siphash48_64s (SIPHASH_PARAMS)
{
	SIPHASH48_64_BODY
	return BIN::from_uint64 (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline void _calc_siphash48_64sv (SIPHASH_PARAMS, std::string &out)
{
	SIPHASH48_64_BODY
	out.resize (8);
	BIN::_from_uint64 (vv0 ^ vv1 ^ vv2 ^ vv3, out.data ());
}

#undef SIPHASH48_64_BODY

///////////////////////////////////////////////////////////////////////////////
// SipHash-2-4 output 128bit  /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define SIPHASH24_128_BODY \
	SIPHASH_BEGIN					\
	vv1 ^= 0xeeULL;					\
	for (; in != end; in += 8) {	\
		BIN::_to_uint64 (val, in);	\
		vv3 ^= val;					\
		SIPHASH_ROUND				\
		SIPHASH_ROUND				\
		vv0 ^= val;					\
	}								\
	SIPHASH_LAST					\
	vv3 ^= val;						\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	vv0 ^= val;						\
	vv2 ^= 0xeeULL;					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	val = vv0 ^ vv1 ^ vv2 ^ vv3;	\
	vv1 ^= 0xddULL;					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND

static inline Digest::SipHash128_t _calc_siphash24_128t (SIPHASH_PARAMS)
{
	SIPHASH24_128_BODY
	return std::make_pair (val, vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline void _calc_siphash24_128tv (SIPHASH_PARAMS, Digest::SipHash128_t &out)
{
	SIPHASH24_128_BODY
	out.first = val;
	out.second = (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline std::string _calc_siphash24_128s (SIPHASH_PARAMS)
{
	SIPHASH24_128_BODY
	return BIN::from_uint64 (val) + BIN::from_uint64 (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline void _calc_siphash24_128sv (SIPHASH_PARAMS, std::string &out)
{
	SIPHASH24_128_BODY
	out.resize (16);
	BIN::_from_uint64 (val, out.data ());
	BIN::_from_uint64 (vv0 ^ vv1 ^ vv2 ^ vv3, out.data () + 8);
}

#undef SIPHASH24_128_BODY

///////////////////////////////////////////////////////////////////////////////
// SipHash-4-8 output 128bit  /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define SIPHASH48_128_BODY \
	SIPHASH_BEGIN					\
	vv1 ^= 0xeeULL;					\
	for (; in != end; in += 8) {	\
		BIN::_to_uint64 (val, in);	\
		vv3 ^= val;					\
		SIPHASH_ROUND				\
		SIPHASH_ROUND				\
		SIPHASH_ROUND				\
		SIPHASH_ROUND				\
		vv0 ^= val;					\
	}								\
	SIPHASH_LAST					\
	vv3 ^= val;						\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	vv0 ^= val;						\
	vv2 ^= 0xeeULL;					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	val = vv0 ^ vv1 ^ vv2 ^ vv3;	\
	vv1 ^= 0xddULL;					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND					\
	SIPHASH_ROUND

static inline Digest::SipHash128_t _calc_siphash48_128t (SIPHASH_PARAMS)
{
	SIPHASH48_128_BODY
	return std::make_pair (val, vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline void _calc_siphash48_128tv (SIPHASH_PARAMS, Digest::SipHash128_t &out)
{
	SIPHASH48_128_BODY
	out.first = val;
	out.second = (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline std::string _calc_siphash48_128s (SIPHASH_PARAMS)
{
	SIPHASH48_128_BODY
	return BIN::from_uint64 (val) + BIN::from_uint64 (vv0 ^ vv1 ^ vv2 ^ vv3);
}

static inline void _calc_siphash48_128sv (SIPHASH_PARAMS, std::string &out)
{
	SIPHASH48_128_BODY
	out.resize (16);
	BIN::_from_uint64 (val, out.data ());
	BIN::_from_uint64 (vv0 ^ vv1 ^ vv2 ^ vv3, out.data () + 8);
}

#undef SIPHASH48_128_BODY

#undef SIPHASH_ROUND
#undef SIPHASH_LAST
#undef SIPHASH_BEGIN

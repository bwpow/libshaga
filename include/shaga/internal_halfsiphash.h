/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2021, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/

/* HalfSipHash implementation based on Reference implementation of SipHash.
 * Website: https://131002.net/siphash/
 * Downloaded from https://github.com/veorq/SipHash on 2020-02-20.
 *
 * Copyright (c) 2016 Jean-Philippe Aumasson <jeanphilippe.aumasson@gmail.com>
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 */

#ifndef HALFSIPHASH_ROTL32
	#define HALFSIPHASH_ROTL32(x, b) (uint32_t)(((x) << (b)) | ((x) >> (32 - (b))))
#endif // HALFSIPHASH_ROTL32

#define HALFSIPHASH_ROUND \
	vv0 += vv1;							\
	vv1 = HALFSIPHASH_ROTL32 (vv1, 5);	\
	vv1 ^= vv0;							\
	vv0 = HALFSIPHASH_ROTL32 (vv0, 16);	\
	vv2 += vv3;							\
	vv3 = HALFSIPHASH_ROTL32 (vv3, 8);	\
	vv3 ^= vv2;							\
	vv0 += vv3;							\
	vv3 = HALFSIPHASH_ROTL32 (vv3, 7);	\
	vv3 ^= vv0;							\
	vv2 += vv1;							\
	vv1 = HALFSIPHASH_ROTL32 (vv1, 13);	\
	vv1 ^= vv2;							\
	vv2 = HALFSIPHASH_ROTL32 (vv2, 16);

#define HALFSIPHASH_LAST \
	val = static_cast<uint32_t> (HALFSIPHASH_DATA_SIZE & 0xff) << 24;	\
	switch (HALFSIPHASH_DATA_SIZE & 3) {								\
		case 3:															\
			val |= static_cast<uint32_t> (in[2]) << 16;					\
			HEDLEY_FALL_THROUGH;                                     	\
		case 2:															\
			val |= static_cast<uint32_t> (in[1]) << 8;					\
			HEDLEY_FALL_THROUGH;                                    	\
		case 1:															\
			val |= static_cast<uint32_t> (in[0]);						\
			HEDLEY_FALL_THROUGH;										\
		case 0:															\
			break;														\
	}

#define HALFSIPHASH_BEGIN \
	uint32_t val;																			\
	const uint8_t *in = HALFSIPHASH_DATA;													\
	const uint8_t *const end = in + (HALFSIPHASH_DATA_SIZE - (HALFSIPHASH_DATA_SIZE & 3));	\
	HALFSIPHASH_BEGIN_KEYS

///////////////////////////////////////////////////////////////////////////////
// HalfSipHash-2-4 output 32bit  //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define HALFSIPHASH24_32_BODY \
	HALFSIPHASH_BEGIN				\
	for (; in != end; in += 4) {	\
		BIN::_to_uint32 (val, in);	\
		vv3 ^= val;					\
		HALFSIPHASH_ROUND			\
		HALFSIPHASH_ROUND			\
		vv0 ^= val;					\
	}								\
	HALFSIPHASH_LAST				\
	vv3 ^= val;						\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	vv0 ^= val;						\
	vv2 ^= 0xff;					\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND

static inline uint32_t _calc_halfsiphash24_32t (HALFSIPHASH_PARAMS)
{
	HALFSIPHASH24_32_BODY
	return (vv1 ^ vv3);
}

static inline void _calc_halfsiphash24_32tv (HALFSIPHASH_PARAMS, uint32_t &out)
{
	HALFSIPHASH24_32_BODY
	out = (vv1 ^ vv3);
}

static inline std::string _calc_halfsiphash24_32s (HALFSIPHASH_PARAMS)
{
	HALFSIPHASH24_32_BODY
	return BIN::from_uint32 (vv1 ^ vv3);
}

static inline void _calc_halfsiphash24_32sv (HALFSIPHASH_PARAMS, std::string &out)
{
	HALFSIPHASH24_32_BODY
	out.resize (4);
	BIN::_from_uint32 (vv1 ^ vv3, out.data ());
}

#undef HALFSIPHASH24_32_BODY

///////////////////////////////////////////////////////////////////////////////
// HalfSipHash-4-8 output 32bit  //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define HALFSIPHASH48_32_BODY \
	HALFSIPHASH_BEGIN				\
	for (; in != end; in += 4) {	\
		BIN::_to_uint32 (val, in);	\
		vv3 ^= val;					\
		HALFSIPHASH_ROUND			\
		HALFSIPHASH_ROUND			\
		HALFSIPHASH_ROUND			\
		HALFSIPHASH_ROUND			\
		vv0 ^= val;					\
	}								\
	HALFSIPHASH_LAST				\
	vv3 ^= val;						\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	vv0 ^= val;						\
	vv2 ^= 0xff;					\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND

static inline uint32_t _calc_halfsiphash48_32t (HALFSIPHASH_PARAMS)
{
	HALFSIPHASH48_32_BODY
	return (vv1 ^ vv3);
}

static inline void _calc_halfsiphash48_32tv (HALFSIPHASH_PARAMS, uint32_t &out)
{
	HALFSIPHASH48_32_BODY
	out = (vv1 ^ vv3);
}

static inline std::string _calc_halfsiphash48_32s (HALFSIPHASH_PARAMS)
{
	HALFSIPHASH48_32_BODY
	return BIN::from_uint32 (vv1 ^ vv3);
}

static inline void _calc_halfsiphash48_32sv (HALFSIPHASH_PARAMS, std::string &out)
{
	HALFSIPHASH48_32_BODY
	out.resize (4);
	BIN::_from_uint32 (vv1 ^ vv3, out.data ());
}

#undef HALFSIPHASH48_32_BODY

///////////////////////////////////////////////////////////////////////////////
// HalfSipHash-2-4 output 64bit  //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define HALFSIPHASH24_64_BODY \
	HALFSIPHASH_BEGIN				\
	vv1 ^= 0xee;					\
	for (; in != end; in += 4) {	\
		BIN::_to_uint32 (val, in);	\
		vv3 ^= val;					\
		HALFSIPHASH_ROUND			\
		HALFSIPHASH_ROUND			\
		vv0 ^= val;					\
	}								\
	HALFSIPHASH_LAST				\
	vv3 ^= val;						\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	vv0 ^= val;						\
	vv2 ^= 0xee;					\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	val = vv1 ^ vv3;				\
	vv1 ^= 0xdd;					\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND

static inline Digest::SipHash64_t _calc_halfsiphash24_64t (HALFSIPHASH_PARAMS)
{
	HALFSIPHASH24_64_BODY
	return std::make_pair (val, vv1 ^ vv3);
}

static inline void _calc_halfsiphash24_64tv (HALFSIPHASH_PARAMS, Digest::SipHash64_t &out)
{
	HALFSIPHASH24_64_BODY
	out.first = val;
	out.second = (vv1 ^ vv3);
}

static inline std::string _calc_halfsiphash24_64s (HALFSIPHASH_PARAMS)
{
	HALFSIPHASH24_64_BODY
	return BIN::from_uint32 (val) + BIN::from_uint32 (vv1 ^ vv3);
}

static inline void _calc_halfsiphash24_64sv (HALFSIPHASH_PARAMS, std::string &out)
{
	HALFSIPHASH24_64_BODY
	out.resize (8);
	BIN::_from_uint32 (val, out.data ());
	BIN::_from_uint32 (vv1 ^ vv3, out.data () + 4);
}

#undef HALFSIPHASH24_64_BODY

///////////////////////////////////////////////////////////////////////////////
// HalfSipHash-4-8 output 64bit  //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define HALFSIPHASH48_64_BODY \
	HALFSIPHASH_BEGIN				\
	vv1 ^= 0xee;					\
	for (; in != end; in += 4) {	\
		BIN::_to_uint32 (val, in);	\
		vv3 ^= val;					\
		HALFSIPHASH_ROUND			\
		HALFSIPHASH_ROUND			\
		HALFSIPHASH_ROUND			\
		HALFSIPHASH_ROUND			\
		vv0 ^= val;					\
	}								\
	HALFSIPHASH_LAST				\
	vv3 ^= val;						\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	vv0 ^= val;						\
	vv2 ^= 0xee;					\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	val = vv1 ^ vv3;				\
	vv1 ^= 0xdd;					\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND				\
	HALFSIPHASH_ROUND

static inline Digest::SipHash64_t _calc_halfsiphash48_64t (HALFSIPHASH_PARAMS)
{
	HALFSIPHASH48_64_BODY
	return std::make_pair (val, vv1 ^ vv3);
}

static inline void _calc_halfsiphash48_64tv (HALFSIPHASH_PARAMS, Digest::SipHash64_t &out)
{
	HALFSIPHASH48_64_BODY
	out.first = val;
	out.second = (vv1 ^ vv3);
}

static inline std::string _calc_halfsiphash48_64s (HALFSIPHASH_PARAMS)
{
	HALFSIPHASH48_64_BODY
	return BIN::from_uint32 (val) + BIN::from_uint32 (vv1 ^ vv3);
}

static inline void _calc_halfsiphash48_64sv (HALFSIPHASH_PARAMS, std::string &out)
{
	HALFSIPHASH48_64_BODY
	out.resize (8);
	BIN::_from_uint32 (val, out.data ());
	BIN::_from_uint32 (vv1 ^ vv3, out.data () + 4);
}

#undef HALFSIPHASH48_64_BODY

#undef HALFSIPHASH_ROUND
#undef HALFSIPHASH_LAST
#undef HALFSIPHASH_BEGIN

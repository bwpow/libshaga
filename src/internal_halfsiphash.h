/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

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

static_assert (sizeof (uint32_t) == 4, "Expected uint32_t to be 4 bytes");

static inline void _siphash_to_uint32 (const uint8_t *const data, uint32_t &v)
{
	::memcpy (&v, data, 4);

	ENDIAN_IS_BIG
	ENDIAN_BSWAP32 (v);
	ENDIAN_END
}

static inline uint32_t _siphash_to_uint32 (const uint8_t *const data)
{
	uint32_t v;

	::memcpy (&v, data, 4);

	ENDIAN_IS_BIG
	ENDIAN_BSWAP32 (v);
	ENDIAN_END

	return v;
}

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
			val |= static_cast<uint64_t> (in[2]) << 16;					\
			SHAGA_FALLTHROUGH;                                     		\
		case 2:															\
			val |= static_cast<uint64_t> (in[1]) << 8;					\
			SHAGA_FALLTHROUGH;                                    		\
		case 1:															\
			val |= static_cast<uint64_t> (in[0]);						\
			SHAGA_FALLTHROUGH;                                      	\
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

static inline uint32_t _calc_halfsiphash24_32t HALFSIPHASH_PARAMS
{
	HALFSIPHASH_BEGIN

	for (; in != end; in += 4) {
		_siphash_to_uint32 (in, val);
		vv3 ^= val;
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		vv0 ^= val;
	}

	HALFSIPHASH_LAST

	vv3 ^= val;
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xff;

	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND

	return (vv1 ^ vv3);
}

static inline std::string _calc_halfsiphash24_32s HALFSIPHASH_PARAMS
{
	HALFSIPHASH_BEGIN

	for (; in != end; in += 4) {
		_siphash_to_uint32 (in, val);
		vv3 ^= val;
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		vv0 ^= val;
	}

	HALFSIPHASH_LAST

	vv3 ^= val;
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xff;

	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND

	return BIN::from_uint32 (vv1 ^ vv3);
}

///////////////////////////////////////////////////////////////////////////////
// HalfSipHash-2-4 output 64bit  //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static inline Digest::SipHash64_t _calc_halfsiphash24_64t HALFSIPHASH_PARAMS
{
	HALFSIPHASH_BEGIN

	vv1 ^= 0xee;

	for (; in != end; in += 4) {
		_siphash_to_uint32 (in, val);
		vv3 ^= val;
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		vv0 ^= val;
	}


	HALFSIPHASH_LAST

	vv3 ^= val;
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xee;

	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND

	val = vv1 ^ vv3;

	vv1 ^= 0xdd;

	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND

	return std::make_pair (val, vv1 ^ vv3);
}

static inline std::string _calc_halfsiphash24_64s HALFSIPHASH_PARAMS
{
	HALFSIPHASH_BEGIN

	vv1 ^= 0xee;

	for (; in != end; in += 4) {
		_siphash_to_uint32 (in, val);
		vv3 ^= val;
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		vv0 ^= val;
	}


	HALFSIPHASH_LAST

	vv3 ^= val;
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xee;

	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND

	val = vv1 ^ vv3;

	vv1 ^= 0xdd;

	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND

	return BIN::from_uint32 (val) + BIN::from_uint32 (vv1 ^ vv3);
}

///////////////////////////////////////////////////////////////////////////////
// HalfSipHash-4-8 output 32bit  //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static inline uint32_t _calc_halfsiphash48_32t HALFSIPHASH_PARAMS
{
	HALFSIPHASH_BEGIN

	for (; in != end; in += 4) {
		_siphash_to_uint32 (in, val);
		vv3 ^= val;
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		vv0 ^= val;
	}

	HALFSIPHASH_LAST

	vv3 ^= val;
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xff;

	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND

	return (vv1 ^ vv3);
}

static inline std::string _calc_halfsiphash48_32s HALFSIPHASH_PARAMS
{
	HALFSIPHASH_BEGIN

	for (; in != end; in += 4) {
		_siphash_to_uint32 (in, val);
		vv3 ^= val;
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		vv0 ^= val;
	}

	HALFSIPHASH_LAST

	vv3 ^= val;
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xff;

	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND

	return BIN::from_uint32 (vv1 ^ vv3);
}

///////////////////////////////////////////////////////////////////////////////
// HalfSipHash-4-8 output 64bit  //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static inline Digest::SipHash64_t _calc_halfsiphash48_64t HALFSIPHASH_PARAMS
{
	HALFSIPHASH_BEGIN

	vv1 ^= 0xee;

	for (; in != end; in += 4) {
		_siphash_to_uint32 (in, val);
		vv3 ^= val;
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		vv0 ^= val;
	}


	HALFSIPHASH_LAST

	vv3 ^= val;
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xee;

	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND

	val = vv1 ^ vv3;

	vv1 ^= 0xdd;

	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND

	return std::make_pair (val, vv1 ^ vv3);
}

static inline std::string _calc_halfsiphash48_64s HALFSIPHASH_PARAMS
{
	HALFSIPHASH_BEGIN

	vv1 ^= 0xee;

	for (; in != end; in += 4) {
		_siphash_to_uint32 (in, val);
		vv3 ^= val;
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		HALFSIPHASH_ROUND
		vv0 ^= val;
	}


	HALFSIPHASH_LAST

	vv3 ^= val;
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	vv0 ^= val;

	vv2 ^= 0xee;

	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND

	val = vv1 ^ vv3;

	vv1 ^= 0xdd;

	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND
	HALFSIPHASH_ROUND

	return BIN::from_uint32 (val) + BIN::from_uint32 (vv1 ^ vv3);
}

#undef HALFSIPHASH_ROUND
#undef HALFSIPHASH_LAST
#undef HALFSIPHASH_BEGIN

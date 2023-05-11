/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2023, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  SHA-256  ////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	HEDLEY_WARN_UNUSED_RESULT std::string Digest::sha256 ([[maybe_unused]] const void *const buf, [[maybe_unused]] const size_t len)
	{
#ifdef SHAGA_FULL
		const size_t sze {32};
		std::string output (sze, '\0');
		const int ret = ::mbedtls_sha256_ret (
			reinterpret_cast<const uint8_t *> (buf),
			len,
			reinterpret_cast<uint8_t *> (output.data ()),
			0);

		if (HEDLEY_UNLIKELY (0 != ret)) {
			cThrow ("SHA-256 failed"sv);
		}

		return output;
#else
		cThrow ("SHA-256 is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  SHA-512  ////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	HEDLEY_WARN_UNUSED_RESULT std::string Digest::sha512 ([[maybe_unused]] const void *const buf, [[maybe_unused]] const size_t len)
	{
#ifdef SHAGA_FULL
		const size_t sze {64};
		std::string output (sze, '\0');
		const int ret = ::mbedtls_sha512_ret (
			reinterpret_cast<const uint8_t *> (buf),
			len,
			reinterpret_cast<uint8_t *> (output.data ()),
			0);

		if (HEDLEY_UNLIKELY (0 != ret)) {
			cThrow ("SHA-512 failed"sv);
		}

		return output;
#else
		cThrow ("SHA-512 is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  SipHash  ////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	namespace Digest {
		#define SIPHASH_DATA _data
		#define SIPHASH_DATA_SIZE _sze
		#define SIPHASH_PARAMS const uint8_t *const _data, const size_t _sze, const Digest::SipHash128_t &siphash_key

		#define SIPHASH_BEGIN_KEYS \
			uint64_t vv0 = 0x736f6d6570736575ULL ^ siphash_key.first;	\
			uint64_t vv1 = 0x646f72616e646f6dULL ^ siphash_key.second;	\
			uint64_t vv2 = 0x6c7967656e657261ULL ^ siphash_key.first;	\
			uint64_t vv3 = 0x7465646279746573ULL ^ siphash_key.second;

		#include "shaga/internal_siphash.h"

		#undef SIPHASH_BEGIN_KEYS
		#undef SIPHASH_PARAMS
		#undef SIPHASH_DATA_SIZE
		#undef SIPHASH_DATA
	}

	HEDLEY_WARN_UNUSED_RESULT Digest::SipHash128_t Digest::siphash_extract_key (const std::string_view key)
	{
		if (key.size () != 16) {
			cThrow ("SipHash key must be exactly 16 bytes (128 bit) long"sv);
		}

		Digest::SipHash128_t out;

		out.first = BIN::_to_uint64 (key.data ());
		out.second = BIN::_to_uint64 (key.data () + 8);

		return out;
	}

	HEDLEY_WARN_UNUSED_RESULT uint64_t Digest::siphash24_64t (const void *const buf, const size_t len, const Digest::SipHash128_t &key)
	{
		return _calc_siphash24_64t (reinterpret_cast<const uint8_t *> (buf), len, key);
	}

	HEDLEY_WARN_UNUSED_RESULT uint64_t Digest::siphash48_64t (const void *const buf, const size_t len, const Digest::SipHash128_t &key)
	{
		return _calc_siphash48_64t (reinterpret_cast<const uint8_t *> (buf), len, key);
	}

	HEDLEY_WARN_UNUSED_RESULT Digest::SipHash128_t Digest::siphash24_128t (const void *const buf, const size_t len, const Digest::SipHash128_t &key)
	{
		return _calc_siphash24_128t (reinterpret_cast<const uint8_t *> (buf), len, key);
	}

	HEDLEY_WARN_UNUSED_RESULT Digest::SipHash128_t Digest::siphash48_128t (const void *const buf, const size_t len, const Digest::SipHash128_t &key)
	{
		return _calc_siphash48_128t (reinterpret_cast<const uint8_t *> (buf), len, key);
	}

	HEDLEY_WARN_UNUSED_RESULT std::string Digest::siphash24_128s (const void *const buf, const size_t len, const Digest::SipHash128_t &key)
	{
		return _calc_siphash24_128s (reinterpret_cast<const uint8_t *> (buf), len, key);
	}

	HEDLEY_WARN_UNUSED_RESULT std::string Digest::siphash48_128s (const void *const buf, const size_t len, const Digest::SipHash128_t &key)
	{
		return _calc_siphash48_128s (reinterpret_cast<const uint8_t *> (buf), len, key);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  HalfSipHash  ////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	namespace Digest {
		#define HALFSIPHASH_DATA _data
		#define HALFSIPHASH_DATA_SIZE _sze
		#define HALFSIPHASH_PARAMS const uint8_t *const _data, const size_t _sze, const Digest::SipHash64_t &halfsiphash_key

		#define HALFSIPHASH_BEGIN_KEYS \
			uint32_t vv0 = halfsiphash_key.first;				\
			uint32_t vv1 = halfsiphash_key.second;				\
			uint32_t vv2 = 0x6c796765 ^ halfsiphash_key.first;	\
			uint32_t vv3 = 0x74656462 ^ halfsiphash_key.second;

		#include "shaga/internal_halfsiphash.h"

		#undef HALFSIPHASH_BEGIN_KEYS
		#undef HALFSIPHASH_PARAMS
		#undef HALFSIPHASH_DATA_SIZE
		#undef HALFSIPHASH_DATA
	}

	HEDLEY_WARN_UNUSED_RESULT Digest::SipHash64_t Digest::halfsiphash_extract_key (const std::string_view key)
	{
		if (key.size () != 8) {
			cThrow ("HalfSipHash key must be exactly 8 bytes (64 bit) long"sv);
		}

		Digest::SipHash64_t out;

		out.first = BIN::_to_uint32 (key.data ());
		out.second = BIN::_to_uint32 (key.data () + 4);

		return out;
	}

	HEDLEY_WARN_UNUSED_RESULT uint32_t Digest::halfsiphash24_32t (const void *const buf, const size_t len, const Digest::SipHash64_t &key)
	{
		return _calc_halfsiphash24_32t (reinterpret_cast<const uint8_t *> (buf), len, key);
	}

	HEDLEY_WARN_UNUSED_RESULT uint32_t Digest::halfsiphash48_32t (const void *const buf, const size_t len, const Digest::SipHash64_t &key)
	{
		return _calc_halfsiphash48_32t (reinterpret_cast<const uint8_t *> (buf), len, key);
	}

	HEDLEY_WARN_UNUSED_RESULT Digest::SipHash64_t Digest::halfsiphash24_64t (const void *const buf, const size_t len, const Digest::SipHash64_t &key)
	{
		return _calc_halfsiphash24_64t (reinterpret_cast<const uint8_t *> (buf), len, key);
	}

	HEDLEY_WARN_UNUSED_RESULT Digest::SipHash64_t Digest::halfsiphash48_64t (const void *const buf, const size_t len, const Digest::SipHash64_t &key)
	{
		return _calc_halfsiphash48_64t (reinterpret_cast<const uint8_t *> (buf), len, key);
	}

	HEDLEY_WARN_UNUSED_RESULT std::string Digest::halfsiphash24_64s (const void *const buf, const size_t len, const Digest::SipHash64_t &key)
	{
		return _calc_halfsiphash24_64s (reinterpret_cast<const uint8_t *> (buf), len, key);
	}

	HEDLEY_WARN_UNUSED_RESULT std::string Digest::halfsiphash48_64s (const void *const buf, const size_t len, const Digest::SipHash64_t &key)
	{
		return _calc_halfsiphash48_64s (reinterpret_cast<const uint8_t *> (buf), len, key);
	}
}

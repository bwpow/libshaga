/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_Digest
#define HEAD_shaga_Digest

#include "common.h"

namespace shaga::Digest {
	/*** SHA-256 ***/
	std::string sha256 (const char *const buf, const size_t len);
	std::string sha256 (const uint8_t *const buf, const size_t len);

	template<class T>
	std::string sha256 (const T &plain)
	{
		return sha256 (plain.data (), plain.size ());
	}

	/*** SHA-512 ***/
	std::string sha512 (const char *const buf, const size_t len);
	std::string sha512 (const uint8_t *const buf, const size_t len);

	template<class T>
	std::string sha512 (const T &plain)
	{
		return sha512 (plain.data (), plain.size ());
	}

	/*** SipHash ***/
	typedef std::pair<uint64_t, uint64_t> SipHash128_t;

	SipHash128_t siphash_extract_key (const std::string_view key);

	uint64_t siphash24_64t (const char *const buf, const size_t len, const SipHash128_t &key);
	uint64_t siphash24_64t (const uint8_t *const buf, const size_t len, const SipHash128_t &key);
	uint64_t siphash48_64t (const char *const buf, const size_t len, const SipHash128_t &key);
	uint64_t siphash48_64t (const uint8_t *const buf, const size_t len, const SipHash128_t &key);

	template<typename T>
	uint64_t siphash24_64t (const T &plain, const SipHash128_t &key)
	{
		return siphash24_64t (plain.data (), plain.size (), key);
	}

	template<typename T>
	uint64_t siphash48_64t (const T &plain, const SipHash128_t &key)
	{
		return siphash48_64t (plain.data (), plain.size (), key);
	}

	SipHash128_t siphash24_128t (const char *const buf, const size_t len, const SipHash128_t &key);
	SipHash128_t siphash24_128t (const uint8_t *const buf, const size_t len, const SipHash128_t &key);
	SipHash128_t siphash48_128t (const char *const buf, const size_t len, const SipHash128_t &key);
	SipHash128_t siphash48_128t (const uint8_t *const buf, const size_t len, const SipHash128_t &key);

	template<typename T>
	SipHash128_t siphash24_128t (const T &plain, const SipHash128_t &key)
	{
		return siphash24_128t (plain.data (), plain.size (), key);
	}

	template<typename T>
	SipHash128_t siphash48_128t (const T &plain, const SipHash128_t &key)
	{
		return siphash48_128t (plain.data (), plain.size (), key);
	}

	std::string siphash24_128s (const char *const buf, const size_t len, const SipHash128_t &key);
	std::string siphash24_128s (const uint8_t *const buf, const size_t len, const SipHash128_t &key);
	std::string siphash48_128s (const char *const buf, const size_t len, const SipHash128_t &key);
	std::string siphash48_128s (const uint8_t *const buf, const size_t len, const SipHash128_t &key);

	template<typename T>
	std::string siphash24_128s (const T &plain, const SipHash128_t &key)
	{
		return siphash24_128s (plain.data (), plain.size (), key);
	}

	template<typename T>
	std::string siphash48_128s (const T &plain, const SipHash128_t &key)
	{
		return siphash48_128s (plain.data (), plain.size (), key);
	}

	/*** HalfSipHash ***/
	typedef std::pair<uint32_t, uint32_t> SipHash64_t;

	SipHash64_t halfsiphash_extract_key (const std::string_view key);

	uint32_t halfsiphash24_32t (const char *const buf, const size_t len, const SipHash64_t &key);
	uint32_t halfsiphash24_32t (const uint8_t *const buf, const size_t len, const SipHash64_t &key);
	uint32_t halfsiphash48_32t (const char *const buf, const size_t len, const SipHash64_t &key);
	uint32_t halfsiphash48_32t (const uint8_t *const buf, const size_t len, const SipHash64_t &key);

	template<typename T>
	uint32_t halfsiphash24_32t (const T &plain, const SipHash64_t &key)
	{
		return halfsiphash24_32t (plain.data (), plain.size (), key);
	}

	template<typename T>
	uint32_t halfsiphash48_32t (const T &plain, const SipHash64_t &key)
	{
		return halfsiphash48_32t (plain.data (), plain.size (), key);
	}

	SipHash64_t halfsiphash24_64t (const char *const buf, const size_t len, const SipHash64_t &key);
	SipHash64_t halfsiphash24_64t (const uint8_t *const buf, const size_t len, const SipHash64_t &key);
	SipHash64_t halfsiphash48_64t (const char *const buf, const size_t len, const SipHash64_t &key);
	SipHash64_t halfsiphash48_64t (const uint8_t *const buf, const size_t len, const SipHash64_t &key);

	template<typename T>
	SipHash64_t halfsiphash24_64t (const T &plain, const SipHash64_t &key)
	{
		return halfsiphash24_64t (plain.data (), plain.size (), key);
	}

	template<typename T>
	SipHash64_t halfsiphash48_64t (const T &plain, const SipHash64_t &key)
	{
		return halfsiphash48_64t (plain.data (), plain.size (), key);
	}

	std::string halfsiphash24_64s (const char *const buf, const size_t len, const SipHash64_t &key);
	std::string halfsiphash24_64s (const uint8_t *const buf, const size_t len, const SipHash64_t &key);
	std::string halfsiphash48_64s (const char *const buf, const size_t len, const SipHash64_t &key);
	std::string halfsiphash48_64s (const uint8_t *const buf, const size_t len, const SipHash64_t &key);

	template<typename T>
	std::string halfsiphash24_64s (const T &plain, const SipHash64_t &key)
	{
		return halfsiphash24_64s (plain.data (), plain.size (), key);
	}

	template<typename T>
	std::string halfsiphash48_64s (const T &plain, const SipHash64_t &key)
	{
		return halfsiphash48_64s (plain.data (), plain.size (), key);
	}
}

#endif // HEAD_shaga_Digest

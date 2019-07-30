/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static inline size_t _get_digest_result_size (const ReDataConfig::DIGEST digest)
	{
		switch (digest) {
			case ReDataConfig::DIGEST::CRC8: return (1);
			case ReDataConfig::DIGEST::CRC32: return (4);
			case ReDataConfig::DIGEST::CRC64: return (8);

			case ReDataConfig::DIGEST::SHA1: return (20);
			case ReDataConfig::DIGEST::SHA256: return (32);
			case ReDataConfig::DIGEST::SHA512: return (64);

			case ReDataConfig::DIGEST::HMAC_RIPEMD160: return (20);
			case ReDataConfig::DIGEST::HMAC_SHA1: return (20);
			case ReDataConfig::DIGEST::HMAC_SHA256: return (32);
			case ReDataConfig::DIGEST::HMAC_SHA512: return (64);

			case ReDataConfig::DIGEST::SIPHASH24_64: return (8);
			case ReDataConfig::DIGEST::SIPHASH24_128: return (16);
			case ReDataConfig::DIGEST::SIPHASH48_64: return (8);
			case ReDataConfig::DIGEST::SIPHASH48_128: return (16);

			case ReDataConfig::DIGEST::_MAX: break;
		}
		cThrow ("Unsupported digest");
	}

	static inline size_t _get_digest_hmac_block_size (const ReDataConfig::DIGEST digest)
	{
		switch (digest) {
			case ReDataConfig::DIGEST::CRC8:
			case ReDataConfig::DIGEST::CRC32:
			case ReDataConfig::DIGEST::CRC64:
			case ReDataConfig::DIGEST::SHA1:
			case ReDataConfig::DIGEST::SHA256:
			case ReDataConfig::DIGEST::SHA512:
				return 0;

			case ReDataConfig::DIGEST::HMAC_RIPEMD160:
			case ReDataConfig::DIGEST::HMAC_SHA1:
			case ReDataConfig::DIGEST::HMAC_SHA256:
				return 64;

			case ReDataConfig::DIGEST::HMAC_SHA512:
				return 128;

			case ReDataConfig::DIGEST::SIPHASH24_64:
			case ReDataConfig::DIGEST::SIPHASH24_128:
			case ReDataConfig::DIGEST::SIPHASH48_64:
			case ReDataConfig::DIGEST::SIPHASH48_128:
				return 16;

			case ReDataConfig::DIGEST::_MAX: break;
		}
		cThrow ("Unsupported digest"sv);
	}


	static inline ReDataConfig::DIGEST_HMAC_TYPE _get_digest_hmac_type (const ReDataConfig::DIGEST digest)
	{
		switch (digest) {
			case ReDataConfig::DIGEST::CRC8:
			case ReDataConfig::DIGEST::CRC32:
			case ReDataConfig::DIGEST::CRC64:
			case ReDataConfig::DIGEST::SHA1:
			case ReDataConfig::DIGEST::SHA256:
			case ReDataConfig::DIGEST::SHA512:
				return ReDataConfig::DIGEST_HMAC_TYPE::NONE;

			case ReDataConfig::DIGEST::HMAC_RIPEMD160:
			case ReDataConfig::DIGEST::HMAC_SHA1:
			case ReDataConfig::DIGEST::HMAC_SHA256:
			case ReDataConfig::DIGEST::HMAC_SHA512:
				return ReDataConfig::DIGEST_HMAC_TYPE::TYPICAL;

			case ReDataConfig::DIGEST::SIPHASH24_64:
			case ReDataConfig::DIGEST::SIPHASH24_128:
			case ReDataConfig::DIGEST::SIPHASH48_64:
			case ReDataConfig::DIGEST::SIPHASH48_128:
				return ReDataConfig::DIGEST_HMAC_TYPE::SIPHASH;

			case ReDataConfig::DIGEST::_MAX: break;
		}
		cThrow ("Unsupported digest"sv);
	}

	static inline std::string _calc_crc8 (const std::string_view plain, ReDataConfig::DigestCache &cache)
	{
		(void) cache;
		return BIN::from_uint8 (CRC::crc8_dallas (plain.data (), plain.size ()));
	}

	static inline std::string _calc_crc32 (const std::string_view plain, ReDataConfig::DigestCache &cache)
	{
		(void) cache;
		return BIN::from_uint32 (CRC::crc32c (plain.data (), plain.size ()));
	}

	static inline std::string _calc_crc64 (const std::string_view plain, ReDataConfig::DigestCache &cache)
	{
		(void) cache;
		return BIN::from_uint64 (CRC::crc64 (plain.data (), plain.size ()));
	}

	static inline std::string _calc_sha1 (const std::string_view plain, ReDataConfig::DigestCache &cache)
	{
#ifdef SHAGA_FULL
		const size_t sze = _get_digest_result_size (ReDataConfig::DIGEST::SHA1);
		::mbedtls_sha1 (reinterpret_cast<const unsigned char *> (plain.data ()), plain.size (), cache.output);
		return std::string (reinterpret_cast<const char *> (cache.output), sze);
#else
		(void) plain;
		(void) cache;
		cThrow ("Digest is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

	static inline std::string _calc_sha256 (const std::string_view plain, ReDataConfig::DigestCache &cache)
	{
#ifdef SHAGA_FULL
		const size_t sze = _get_digest_result_size (ReDataConfig::DIGEST::SHA256);
		::mbedtls_sha256 (reinterpret_cast<const unsigned char *> (plain.data ()), plain.size (), cache.output, 0);
		return std::string (reinterpret_cast<const char *> (cache.output), sze);
#else
		(void) plain;
		(void) cache;
		cThrow ("Digest is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

	static inline std::string _calc_sha512 (const std::string_view plain, ReDataConfig::DigestCache &cache)
	{
#ifdef SHAGA_FULL
		const size_t sze = _get_digest_result_size (ReDataConfig::DIGEST::SHA512);
		::mbedtls_sha512 (reinterpret_cast<const unsigned char *> (plain.data ()), plain.size (), cache.output, 0);
		return std::string (reinterpret_cast<const char *> (cache.output), sze);
#else
		(void) plain;
		(void) cache;
		cThrow ("Digest is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

	static inline std::string _calc_ripemd160 (const std::string_view plain, ReDataConfig::DigestCache &cache)
	{
#ifdef SHAGA_FULL
		const size_t sze = _get_digest_result_size (ReDataConfig::DIGEST::HMAC_RIPEMD160);
		::mbedtls_ripemd160 (reinterpret_cast<const unsigned char *> (plain.data ()), plain.size (), cache.output);
		return std::string (reinterpret_cast<const char *> (cache.output), sze);
#else
		(void) plain;
		(void) cache;
		cThrow ("Digest is not supported in lite version"sv);
#endif // SHAGA_FULL
	}


#define SIPHASH_DATA reinterpret_cast<const uint8_t *>(plain.data ())
#define SIPHASH_DATA_SIZE plain.size ()
#define SIPHASH_PARAMS (const std::string_view plain, ReDataConfig::DigestCache &cache)

#define SIPHASH_BEGIN_KEYS \
	uint64_t vv0 = 0x736f6d6570736575ULL ^ cache.siphash_k0;				\
	uint64_t vv1 = 0x646f72616e646f6dULL ^ cache.siphash_k1;				\
	uint64_t vv2 = 0x6c7967656e657261ULL ^ cache.siphash_k0;				\
	uint64_t vv3 = 0x7465646279746573ULL ^ cache.siphash_k1;

#include "internal_siphash.h"

#undef SIPHASH_BEGIN_KEYS
#undef SIPHASH_PARAMS
#undef SIPHASH_DATA_SIZE
#undef SIPHASH_DATA

	typedef std::function<std::string(const std::string_view, ReDataConfig::DigestCache &)> HASH_FUNC;

	static inline HASH_FUNC _get_digest_calc_function (const ReDataConfig::DIGEST digest)
	{
		switch (digest) {
			case ReDataConfig::DIGEST::CRC8: return _calc_crc8;
			case ReDataConfig::DIGEST::CRC32: return _calc_crc32;
			case ReDataConfig::DIGEST::CRC64: return _calc_crc64;

			case ReDataConfig::DIGEST::SHA1: return _calc_sha1;
			case ReDataConfig::DIGEST::SHA256: return _calc_sha256;
			case ReDataConfig::DIGEST::SHA512: return _calc_sha512;

			case ReDataConfig::DIGEST::HMAC_RIPEMD160: return _calc_ripemd160;
			case ReDataConfig::DIGEST::HMAC_SHA1: return _calc_sha1;
			case ReDataConfig::DIGEST::HMAC_SHA256: return _calc_sha256;
			case ReDataConfig::DIGEST::HMAC_SHA512: return _calc_sha512;

			case ReDataConfig::DIGEST::SIPHASH24_64: return _calc_siphash24_64;
			case ReDataConfig::DIGEST::SIPHASH24_128: return _calc_siphash24_128;
			case ReDataConfig::DIGEST::SIPHASH48_64: return _calc_siphash48_64;
			case ReDataConfig::DIGEST::SIPHASH48_128: return _calc_siphash48_128;

			case ReDataConfig::DIGEST::_MAX: break;
		}
		cThrow ("Unsupported digest");
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ReDataConfig::DigestCache::DigestCache () :
		digest (ReDataConfig::DIGEST::CRC32)
	{ }

	ReDataConfig::DigestCache::DigestCache (const ReDataConfig::DigestCache &other) :
		digest (other.digest),
		key (other.key),
		ipad (other.ipad),
		opad (other.opad),
		siphash_k0 (other.siphash_k0),
		siphash_k1 (other.siphash_k1)
	{ }

	ReDataConfig::DigestCache::DigestCache (ReDataConfig::DigestCache &&other) :
		digest (std::move (other.digest)),
		key (std::move (other.key)),
		ipad (std::move (other.ipad)),
		opad (std::move (other.opad)),
		siphash_k0 (other.siphash_k0),
		siphash_k1 (other.siphash_k1)
	{ }

	ReDataConfig::DigestCache& ReDataConfig::DigestCache::operator= (const ReDataConfig::DigestCache &other)
	{
		digest = other.digest;
		key = other.key;
		ipad = other.ipad;
		opad = other.opad;
		siphash_k0 = other.siphash_k0;
		siphash_k1 = other.siphash_k1;

		return *this;
	}

	ReDataConfig::DigestCache& ReDataConfig::DigestCache::operator= (ReDataConfig::DigestCache &&other)
	{
		digest = std::move (other.digest);
		key = std::move (other.key);
		ipad = std::move (other.ipad);
		opad = std::move (other.opad);
		siphash_k0 = other.siphash_k0;
		siphash_k1 = other.siphash_k1;

		return *this;
	}

	std::string ReDataConfig::calc_digest (const std::string_view plain, const std::string_view key)
	{
		HASH_FUNC hfunc = _get_digest_calc_function (_used_digest);
		const DIGEST_HMAC_TYPE hmac_type = _get_digest_hmac_type (_used_digest);

		if (DIGEST_HMAC_TYPE::NONE == hmac_type) {
			return hfunc (plain, _cache_digest);
		}

		if (key.empty () == true) {
			cThrow ("Key for HMAC is not provided"sv);
		}

		const size_t blocksize = _get_digest_hmac_block_size (_used_digest);

		if (DIGEST_HMAC_TYPE::TYPICAL == hmac_type) {
#ifdef SHAGA_FULL
			if (_cache_digest.digest != _used_digest || _cache_digest.key != key) {
				/* Key is not cached, so prepare IPAD and OPAD for HMAC calculation for the new key */
				_cache_digest.digest = _used_digest;
				_cache_digest.key.assign (key);

				_cache_digest.opad.assign (blocksize, 0x5c);
				_cache_digest.ipad.assign (blocksize, 0x36);

				std::string wkey;
				if (key.size () > blocksize) {
					wkey = hfunc (key, _cache_digest);
				}
				else {
					wkey.assign (key);
				}
				wkey.resize (blocksize, 0x00);

				BIN::XOR (_cache_digest.ipad, wkey);
				BIN::XOR (_cache_digest.opad, wkey);
			}

			/* Calculate HFUNC (OPAD + HFUNC (IPAD + TEXT)) */
			return hfunc (_cache_digest.opad + hfunc (_cache_digest.ipad + std::string (plain), _cache_digest), _cache_digest);
#else
		cThrow ("Digest is not supported in lite version"sv);
#endif // SHAGA_FULL
		}
		else if (DIGEST_HMAC_TYPE::SIPHASH == hmac_type) {
			if (key.size () != blocksize) {
				cThrow ("Wrong digest key size. Expected {} bytes, got {} bytes."sv, blocksize, key.size ());
			}

			if (_cache_digest.digest != _used_digest || _cache_digest.key != key) {
				/* Key is not cached, so fill k0 and k1 */
				_cache_digest.digest = _used_digest;
				_cache_digest.key.assign (key);

				BIN::_to_uint64 (_cache_digest.siphash_k0, key.data ());
				BIN::_to_uint64 (_cache_digest.siphash_k1, key.data () + sizeof (uint64_t));
			}

			return hfunc (plain, _cache_digest);
		}

		cThrow ("Unsupported HMAC type"sv);
	}

	size_t ReDataConfig::get_digest_result_size (void) const
	{
		return _get_digest_result_size (_used_digest);
	}

	size_t ReDataConfig::get_digest_hmac_block_size (void) const
	{
		return _get_digest_hmac_block_size (_used_digest);
	}

	bool ReDataConfig::has_hmac (void) const
	{
		return _get_digest_hmac_type (_used_digest) != DIGEST_HMAC_TYPE::NONE;
	}

	bool ReDataConfig::has_digest_size_at_least_bits (const size_t limit) const
	{
		return (_get_digest_result_size (_used_digest) * 8) >= limit;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Global functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifdef SHAGA_FULL
	#include <mbedtls/ripemd160.h>
	#include <mbedtls/sha1.h>
	#include <mbedtls/sha256.h>
	#include <mbedtls/sha512.h>
#endif // SHAGA_FULL

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static inline size_t _get_digest_result_size (const ReDataConfig::DIGEST digest)
	{
		switch (digest) {
			case ReDataConfig::DIGEST::CRC32: return (4);
			case ReDataConfig::DIGEST::SHA1: return (20);
			case ReDataConfig::DIGEST::SHA256: return (32);
			case ReDataConfig::DIGEST::SHA512: return (64);

			case ReDataConfig::DIGEST::HMAC_RIPEMD160: return (20);
			case ReDataConfig::DIGEST::HMAC_SHA1: return (20);
			case ReDataConfig::DIGEST::HMAC_SHA256: return (32);
			case ReDataConfig::DIGEST::HMAC_SHA512: return (64);

			case ReDataConfig::DIGEST::_MAX: break;
		}
		cThrow ("Unsupported digest");
	}

	static inline size_t _get_digest_hmac_block_size (const ReDataConfig::DIGEST digest)
	{
		switch (digest) {
			case ReDataConfig::DIGEST::CRC32: return 0;
			case ReDataConfig::DIGEST::SHA1: return 0;
			case ReDataConfig::DIGEST::SHA256: return 0;
			case ReDataConfig::DIGEST::SHA512: return 0;

			case ReDataConfig::DIGEST::HMAC_RIPEMD160: return 64;
			case ReDataConfig::DIGEST::HMAC_SHA1: return 64;
			case ReDataConfig::DIGEST::HMAC_SHA256: return 64;
			case ReDataConfig::DIGEST::HMAC_SHA512: return 128;

			case ReDataConfig::DIGEST::_MAX: break;
		}
		cThrow ("Unsupported digest");
	}

	extern const uint_fast32_t _crc32_table[256];

	static inline std::string _calc_crc32 (const std::string &plain, ReDataConfig::DigestCache &cache)
	{
		(void) cache;
		uint_fast32_t val = UINT32_MAX;

		#define UPDC32(octet,crc) crc=(_crc32_table[((crc) ^ (octet)) & 0xff] ^ ((crc) >> 8))
		for (std::string::const_iterator iter = plain.begin (); iter != plain.end (); ++iter) {
			UPDC32 (static_cast<uint_fast32_t> (*iter), val);
		}
		#undef UPDC32

		return BIN::from_uint32 (val ^ UINT32_MAX);
	}

	static inline std::string _calc_sha1 (const std::string &plain, ReDataConfig::DigestCache &cache)
	{
#ifdef SHAGA_FULL
		const size_t sze = _get_digest_result_size (ReDataConfig::DIGEST::SHA1);
		mbedtls_sha1 (reinterpret_cast<const unsigned char *> (plain.data ()), plain.size (), cache.output);
		return std::string (reinterpret_cast<const char *> (cache.output), sze);
#else
		(void) plain;
		(void) cache;
		cThrow ("Digest is not supported in lite version");
#endif // SHAGA_FULL
	}

	static inline std::string _calc_sha256 (const std::string &plain, ReDataConfig::DigestCache &cache)
	{
#ifdef SHAGA_FULL
		const size_t sze = _get_digest_result_size (ReDataConfig::DIGEST::SHA256);
		mbedtls_sha256 (reinterpret_cast<const unsigned char *> (plain.data ()), plain.size (), cache.output, 0);
		return std::string (reinterpret_cast<const char *> (cache.output), sze);
#else
		(void) plain;
		(void) cache;
		cThrow ("Digest is not supported in lite version");
#endif // SHAGA_FULL
	}

	static inline std::string _calc_sha512 (const std::string &plain, ReDataConfig::DigestCache &cache)
	{
#ifdef SHAGA_FULL
		const size_t sze = _get_digest_result_size (ReDataConfig::DIGEST::SHA512);
		mbedtls_sha512 (reinterpret_cast<const unsigned char *> (plain.data ()), plain.size (), cache.output, 0);
		return std::string (reinterpret_cast<const char *> (cache.output), sze);
#else
		(void) plain;
		(void) cache;
		cThrow ("Digest is not supported in lite version");
#endif // SHAGA_FULL
	}

	static inline std::string _calc_ripemd160 (const std::string &plain, ReDataConfig::DigestCache &cache)
	{
#ifdef SHAGA_FULL
		const size_t sze = _get_digest_result_size (ReDataConfig::DIGEST::HMAC_RIPEMD160);
		mbedtls_ripemd160 (reinterpret_cast<const unsigned char *> (plain.data ()), plain.size (), cache.output);
		return std::string (reinterpret_cast<const char *> (cache.output), sze);
#else
		(void) plain;
		(void) cache;
		cThrow ("Digest is not supported in lite version");
#endif // SHAGA_FULL
	}

	typedef std::function<std::string(const std::string &, ReDataConfig::DigestCache &)> HASH_FUNC;

	static inline HASH_FUNC _get_digest_calc_function (const ReDataConfig::DIGEST digest)
	{
		switch (digest) {
			case ReDataConfig::DIGEST::CRC32: return _calc_crc32;
			case ReDataConfig::DIGEST::SHA1: return _calc_sha1;
			case ReDataConfig::DIGEST::SHA256: return _calc_sha256;
			case ReDataConfig::DIGEST::SHA512: return _calc_sha512;

			case ReDataConfig::DIGEST::HMAC_RIPEMD160: return _calc_ripemd160;
			case ReDataConfig::DIGEST::HMAC_SHA1: return _calc_sha1;
			case ReDataConfig::DIGEST::HMAC_SHA256: return _calc_sha256;
			case ReDataConfig::DIGEST::HMAC_SHA512: return _calc_sha512;

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

#ifdef SHAGA_FULL
	ReDataConfig::DigestCache::DigestCache () :
		digest (ReDataConfig::DIGEST::CRC32)
	{
	}

	ReDataConfig::DigestCache::DigestCache (const ReDataConfig::DigestCache &other) :
		digest (other.digest),
		key (other.key),
		ipad (other.ipad),
		opad (other.opad)
	{
	}

	ReDataConfig::DigestCache::DigestCache (ReDataConfig::DigestCache &&other) :
		digest (std::move (other.digest)),
		key (std::move (other.key)),
		ipad (std::move (other.ipad)),
		opad (std::move (other.opad))
	{
	}

	ReDataConfig::DigestCache& ReDataConfig::DigestCache::operator= (const ReDataConfig::DigestCache &other)
	{
		digest = other.digest;
		key = other.key;
		ipad = other.ipad;
		opad = other.opad;

		return *this;
	}

	ReDataConfig::DigestCache& ReDataConfig::DigestCache::operator= (ReDataConfig::DigestCache &&other)
	{
		digest = std::move (other.digest);
		key = std::move (other.key);
		ipad = std::move (other.ipad);
		opad = std::move (other.opad);

		return *this;
	}
#endif // SHAGA_FULL

	std::string ReDataConfig::calc_digest (const std::string &plain, const std::string &key)
	{
		const size_t blocksize = _get_digest_hmac_block_size (_used_digest);
		HASH_FUNC hfunc = _get_digest_calc_function (_used_digest);

		if (0 == blocksize) {
			return hfunc (plain, _cache_digest);
		}

#ifdef SHAGA_FULL
		if (key.empty () == true) {
			cThrow ("Key for HMAC is not provided");
		}

		if (_cache_digest.digest != _used_digest || _cache_digest.key != key) {
			_cache_digest.digest = _used_digest;
			_cache_digest.key.assign (key);

			_cache_digest.opad = std::string (blocksize, 0x5c);
			_cache_digest.ipad = std::string (blocksize, 0x36);

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

		return hfunc (_cache_digest.opad + hfunc (_cache_digest.ipad + plain, _cache_digest), _cache_digest);
#else
		(void) key;
		cThrow ("Digest is not supported in lite version");
#endif // SHAGA_FULL
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
		return _get_digest_hmac_block_size (_used_digest) > 0;
	}

	bool ReDataConfig::has_digest_size_at_least_bits (const size_t limit) const
	{
		return (_get_digest_result_size (_used_digest) * 8) >= limit;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Global functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

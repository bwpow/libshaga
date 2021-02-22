/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2021, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* Result size in bytes */
	SHAGA_NODISCARD static inline size_t _get_digest_result_size (const ReDataConfig::DIGEST digest)
	{
		switch (digest) {
			case ReDataConfig::DIGEST::NONE: return (0);

			case ReDataConfig::DIGEST::CRC8: return (1);
			case ReDataConfig::DIGEST::CRC32: return (4);
			case ReDataConfig::DIGEST::CRC64: return (8);

			case ReDataConfig::DIGEST::SHA256: return (32);
			case ReDataConfig::DIGEST::SHA512: return (64);

			case ReDataConfig::DIGEST::HMAC_SHA256: return (32);
			case ReDataConfig::DIGEST::HMAC_SHA512: return (64);

			case ReDataConfig::DIGEST::HALFSIPHASH24_32: return (4);
			case ReDataConfig::DIGEST::HALFSIPHASH24_64: return (8);
			case ReDataConfig::DIGEST::HALFSIPHASH48_32: return (4);
			case ReDataConfig::DIGEST::HALFSIPHASH48_64: return (8);

			case ReDataConfig::DIGEST::SIPHASH24_64: return (8);
			case ReDataConfig::DIGEST::SIPHASH24_128: return (16);
			case ReDataConfig::DIGEST::SIPHASH48_64: return (8);
			case ReDataConfig::DIGEST::SIPHASH48_128: return (16);

			case ReDataConfig::DIGEST::_MAX: break;
		}
		cThrow ("Unsupported digest"sv);
	}

	/* Key size in bytes */
	SHAGA_NODISCARD static inline size_t _get_digest_hmac_key_size (const ReDataConfig::DIGEST digest)
	{
		switch (digest) {
			case ReDataConfig::DIGEST::NONE:
			case ReDataConfig::DIGEST::CRC8:
			case ReDataConfig::DIGEST::CRC32:
			case ReDataConfig::DIGEST::CRC64:
			case ReDataConfig::DIGEST::SHA256:
			case ReDataConfig::DIGEST::SHA512:
				return 0;

			case ReDataConfig::DIGEST::HMAC_SHA256:
				return 64;

			case ReDataConfig::DIGEST::HMAC_SHA512:
				return 128;

			case ReDataConfig::DIGEST::HALFSIPHASH24_32:
			case ReDataConfig::DIGEST::HALFSIPHASH24_64:
			case ReDataConfig::DIGEST::HALFSIPHASH48_32:
			case ReDataConfig::DIGEST::HALFSIPHASH48_64:
				return 8;

			case ReDataConfig::DIGEST::SIPHASH24_64:
			case ReDataConfig::DIGEST::SIPHASH24_128:
			case ReDataConfig::DIGEST::SIPHASH48_64:
			case ReDataConfig::DIGEST::SIPHASH48_128:
				return 16;

			case ReDataConfig::DIGEST::_MAX: break;
		}
		cThrow ("Unsupported digest"sv);
	}


	SHAGA_NODISCARD static inline ReDataConfig::DIGEST_HMAC_TYPE _get_digest_hmac_type (const ReDataConfig::DIGEST digest)
	{
		switch (digest) {
			case ReDataConfig::DIGEST::NONE:
			case ReDataConfig::DIGEST::CRC8:
			case ReDataConfig::DIGEST::CRC32:
			case ReDataConfig::DIGEST::CRC64:
			case ReDataConfig::DIGEST::SHA256:
			case ReDataConfig::DIGEST::SHA512:
				return ReDataConfig::DIGEST_HMAC_TYPE::NONE;

			case ReDataConfig::DIGEST::HMAC_SHA256:
			case ReDataConfig::DIGEST::HMAC_SHA512:
				return ReDataConfig::DIGEST_HMAC_TYPE::TYPICAL;

			case ReDataConfig::DIGEST::HALFSIPHASH24_32:
			case ReDataConfig::DIGEST::HALFSIPHASH24_64:
			case ReDataConfig::DIGEST::HALFSIPHASH48_32:
			case ReDataConfig::DIGEST::HALFSIPHASH48_64:
				return ReDataConfig::DIGEST_HMAC_TYPE::HALFSIPHASH;

			case ReDataConfig::DIGEST::SIPHASH24_64:
			case ReDataConfig::DIGEST::SIPHASH24_128:
			case ReDataConfig::DIGEST::SIPHASH48_64:
			case ReDataConfig::DIGEST::SIPHASH48_128:
				return ReDataConfig::DIGEST_HMAC_TYPE::SIPHASH;

			case ReDataConfig::DIGEST::_MAX: break;
		}
		cThrow ("Unsupported digest"sv);
	}

	static inline void _calc_crc8 (const std::string_view plain, [[maybe_unused]] ReDataConfig::DigestCache &cache, std::string &out)
	{
		out.resize (sizeof (uint8_t));
		BIN::_from_uint8 (CRC::crc8_dallas (plain.data (), plain.size ()), out.data ());
	}

	static inline void _calc_crc32 (const std::string_view plain, [[maybe_unused]] ReDataConfig::DigestCache &cache, std::string &out)
	{
		out.resize (sizeof (uint32_t));
		BIN::_from_uint32 (CRC::crc32c (plain.data (), plain.size ()), out.data ());
	}

	static inline void _calc_crc64 (const std::string_view plain, [[maybe_unused]] ReDataConfig::DigestCache &cache, std::string &out)
	{
		out.resize (sizeof (uint64_t));
		BIN::_from_uint64 (CRC::crc64 (plain.data (), plain.size ()), out.data ());
	}

	static inline void _calc_sha256 ([[maybe_unused]] const std::string_view plain, [[maybe_unused]] ReDataConfig::DigestCache &cache, [[maybe_unused]] std::string &out)
	{
#ifdef SHAGA_FULL
		const size_t sze = _get_digest_result_size (ReDataConfig::DIGEST::SHA256);
		out.resize (sze);
		const int ret = ::mbedtls_sha256_ret (
			reinterpret_cast<const uint8_t *> (plain.data ()),
			plain.size (),
			reinterpret_cast<uint8_t *> (out.data ()),
			0);

		if (0 != ret) {
			cThrow ("SHA-256 failed"sv);
		}
#else
		cThrow ("Digest is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

	static inline void _calc_sha512 ([[maybe_unused]] const std::string_view plain, [[maybe_unused]] ReDataConfig::DigestCache &cache, [[maybe_unused]] std::string &out)
	{
#ifdef SHAGA_FULL
		const size_t sze = _get_digest_result_size (ReDataConfig::DIGEST::SHA512);
		out.resize (sze);
		const int ret = ::mbedtls_sha512_ret (
			reinterpret_cast<const uint8_t *> (plain.data ()),
			plain.size (),
			reinterpret_cast<uint8_t *> (out.data ()),
			0);

		if (0 != ret) {
			cThrow ("SHA-512 failed"sv);
		}

#else
		cThrow ("Digest is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

#define SIPHASH_DATA reinterpret_cast<const uint8_t *>(plain.data ())
#define SIPHASH_DATA_SIZE plain.size ()
#define SIPHASH_PARAMS const std::string_view plain, const ReDataConfig::DigestCache &cache

#define SIPHASH_BEGIN_KEYS \
	uint64_t vv0 = 0x736f6d6570736575ULL ^ cache.siphash_k0;				\
	uint64_t vv1 = 0x646f72616e646f6dULL ^ cache.siphash_k1;				\
	uint64_t vv2 = 0x6c7967656e657261ULL ^ cache.siphash_k0;				\
	uint64_t vv3 = 0x7465646279746573ULL ^ cache.siphash_k1;

#include "shaga/internal_siphash.h"

#undef SIPHASH_BEGIN_KEYS
#undef SIPHASH_PARAMS
#undef SIPHASH_DATA_SIZE
#undef SIPHASH_DATA


#define HALFSIPHASH_DATA reinterpret_cast<const uint8_t *>(plain.data ())
#define HALFSIPHASH_DATA_SIZE plain.size ()
#define HALFSIPHASH_PARAMS const std::string_view plain, const ReDataConfig::DigestCache &cache

#define HALFSIPHASH_BEGIN_KEYS \
	uint32_t vv0 = cache.halfsiphash_k0;				\
	uint32_t vv1 = cache.halfsiphash_k1;				\
	uint32_t vv2 = 0x6c796765 ^ cache.halfsiphash_k0;	\
	uint32_t vv3 = 0x74656462 ^ cache.halfsiphash_k1;

#include "shaga/internal_halfsiphash.h"

#undef HALFSIPHASH_BEGIN_KEYS
#undef HALFSIPHASH_PARAMS
#undef HALFSIPHASH_DATA_SIZE
#undef HALFSIPHASH_DATA

	typedef std::function<void(const std::string_view, ReDataConfig::DigestCache &, std::string &)> HASH_FUNC;

	SHAGA_NODISCARD static inline HASH_FUNC _get_digest_calc_function (const ReDataConfig::DIGEST digest)
	{
		switch (digest) {
			case ReDataConfig::DIGEST::NONE: break;

			case ReDataConfig::DIGEST::CRC8: return _calc_crc8;
			case ReDataConfig::DIGEST::CRC32: return _calc_crc32;
			case ReDataConfig::DIGEST::CRC64: return _calc_crc64;

			case ReDataConfig::DIGEST::SHA256: return _calc_sha256;
			case ReDataConfig::DIGEST::SHA512: return _calc_sha512;

			case ReDataConfig::DIGEST::HMAC_SHA256: return _calc_sha256;
			case ReDataConfig::DIGEST::HMAC_SHA512: return _calc_sha512;

			case ReDataConfig::DIGEST::HALFSIPHASH24_32: return _calc_halfsiphash24_32sv;
			case ReDataConfig::DIGEST::HALFSIPHASH24_64: return _calc_halfsiphash24_64sv;
			case ReDataConfig::DIGEST::HALFSIPHASH48_32: return _calc_halfsiphash48_32sv;
			case ReDataConfig::DIGEST::HALFSIPHASH48_64: return _calc_halfsiphash48_64sv;

			case ReDataConfig::DIGEST::SIPHASH24_64: return _calc_siphash24_64sv;
			case ReDataConfig::DIGEST::SIPHASH24_128: return _calc_siphash24_128sv;
			case ReDataConfig::DIGEST::SIPHASH48_64: return _calc_siphash48_64sv;
			case ReDataConfig::DIGEST::SIPHASH48_128: return _calc_siphash48_128sv;

			case ReDataConfig::DIGEST::_MAX: break;
		}

		cThrow ("Unsupported digest"sv);
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

	void ReDataConfig::DigestCache::reset (void)
	{
		digest = DIGEST::CRC32;
		key.clear ();
		ipad.clear ();
		opad.clear ();
		temp.clear ();
		siphash_k0 = 0;
		siphash_k1 = 0;
		halfsiphash_k0 = 0;
		halfsiphash_k1 = 0;
	}

	void ReDataConfig::calc_digest (const std::string_view plain, std::string &out, const std::string_view key) const
	{
		if (DIGEST::NONE == _used_digest) {
			out.resize (0);
			return;
		}

		const HASH_FUNC hfunc = _get_digest_calc_function (_used_digest);
		const DIGEST_HMAC_TYPE hmac_type = _get_digest_hmac_type (_used_digest);

		if (DIGEST_HMAC_TYPE::NONE == hmac_type) {
			hfunc (plain, _cache_digest, out);
			return;
		}

		if (key.empty () == true) {
			cThrow ("Key for HMAC is not provided"sv);
		}

		const size_t keysize = _get_digest_hmac_key_size (_used_digest);

		if (DIGEST_HMAC_TYPE::TYPICAL == hmac_type) {
#ifdef SHAGA_FULL
			if (_cache_digest.digest != _used_digest || _cache_digest.key != key) {
				/* Key is not cached, so prepare IPAD and OPAD for HMAC calculation for the new key */
				_cache_digest.digest = _used_digest;
				_cache_digest.key.assign (key);

				_cache_digest.opad.assign (keysize, 0x5c);
				_cache_digest.ipad.assign (keysize, 0x36);

				if (key.size () > keysize) {
					hfunc (key, _cache_digest, _cache_digest.temp);
				}
				else {
					_cache_digest.temp.assign (key);
				}
				_cache_digest.temp.resize (keysize, 0x00);

				BIN::XOR (_cache_digest.ipad, _cache_digest.temp);
				BIN::XOR (_cache_digest.opad, _cache_digest.temp);
			}

			/* Calculate HFUNC (OPAD + HFUNC (IPAD + TEXT)) */
			_cache_digest.temp.resize (0);
			_cache_digest.temp.append (_cache_digest.ipad);
			_cache_digest.temp.append (plain);
			hfunc (_cache_digest.temp, _cache_digest, out);

			_cache_digest.temp.resize (0);
			_cache_digest.temp.append (_cache_digest.opad);
			_cache_digest.temp.append (out);
			hfunc (_cache_digest.temp, _cache_digest, out);

			_cache_digest.temp.resize (0);
			return;
#else
		cThrow ("Digest is not supported in lite version"sv);
#endif // SHAGA_FULL
		}
		else if (DIGEST_HMAC_TYPE::SIPHASH == hmac_type) {
			if (key.size () != keysize) {
				cThrow ("Wrong digest key size. Expected {} bytes, got {} bytes."sv, keysize, key.size ());
			}

			if (_cache_digest.digest != _used_digest || _cache_digest.key != key) {
				/* Key is not cached, so fill k0 and k1 */
				_cache_digest.digest = _used_digest;
				_cache_digest.key.assign (key);

				BIN::_to_uint64 (_cache_digest.siphash_k0, key.data ());
				BIN::_to_uint64 (_cache_digest.siphash_k1, key.data () + sizeof (uint64_t));
			}

			hfunc (plain, _cache_digest, out);
			return;
		}
		else if (DIGEST_HMAC_TYPE::HALFSIPHASH == hmac_type) {
			if (key.size () != keysize) {
				cThrow ("Wrong digest key size. Expected {} bytes, got {} bytes."sv, keysize, key.size ());
			}

			if (_cache_digest.digest != _used_digest || _cache_digest.key != key) {
				/* Key is not cached, so fill k0 and k1 */
				_cache_digest.digest = _used_digest;
				_cache_digest.key.assign (key);

				BIN::_to_uint32 (_cache_digest.halfsiphash_k0, key.data ());
				BIN::_to_uint32 (_cache_digest.halfsiphash_k1, key.data () + sizeof (uint32_t));
			}

			hfunc (plain, _cache_digest, out);
			return;
		}
		else if (DIGEST_HMAC_TYPE::RAWKEY == hmac_type) {
			if (key.size () != keysize) {
				cThrow ("Wrong digest key size. Expected {} bytes, got {} bytes."sv, keysize, key.size ());
			}

			if (_cache_digest.digest != _used_digest || _cache_digest.key != key) {
				/* Key is not cached */
				_cache_digest.digest = _used_digest;
				_cache_digest.key.assign (key);
			}

			hfunc (plain, _cache_digest, out);
			return;
		}

		cThrow ("Unsupported HMAC type"sv);
	}

	SHAGA_NODISCARD size_t ReDataConfig::get_digest_result_size (void) const
	{
		return _get_digest_result_size (_used_digest);
	}

	SHAGA_NODISCARD size_t ReDataConfig::get_digest_hmac_key_size (void) const
	{
		return _get_digest_hmac_key_size (_used_digest);
	}

	SHAGA_NODISCARD bool ReDataConfig::has_hmac (void) const
	{
		return _get_digest_hmac_type (_used_digest) != DIGEST_HMAC_TYPE::NONE;
	}

	SHAGA_NODISCARD bool ReDataConfig::has_digest_result_size_at_least_bits (const size_t limit) const
	{
		return (_get_digest_result_size (_used_digest) * 8) >= limit;
	}

	SHAGA_NODISCARD bool ReDataConfig::has_digest_hmac_key_size_at_least_bits (const size_t limit) const
	{
		return (_get_digest_hmac_key_size (_used_digest) * 8) >= limit;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Global functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

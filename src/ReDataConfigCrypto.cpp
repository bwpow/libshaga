/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifdef SHAGA_FULL
	#include <mbedtls/aes.h>
#endif // SHAGA_FULL

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static inline size_t _get_crypto_block_size (const ReDataConfig::CRYPTO crypto)
	{
		switch (crypto) {
			case ReDataConfig::CRYPTO::NONE: return 0;
			case ReDataConfig::CRYPTO::AES128: return 16;
			case ReDataConfig::CRYPTO::AES256: return 16;

			case ReDataConfig::CRYPTO::_MAX: break;
		}
		cThrow ("Unsupported crypto");
	}

	static inline size_t _get_crypto_iv_size (const ReDataConfig::CRYPTO crypto)
	{
		switch (crypto) {
			case ReDataConfig::CRYPTO::NONE: return 0;
			case ReDataConfig::CRYPTO::AES128: return 16;
			case ReDataConfig::CRYPTO::AES256: return 16;

			case ReDataConfig::CRYPTO::_MAX: break;
		}
		cThrow ("Unsupported crypto");
	}

	static inline size_t _get_crypto_key_size (const ReDataConfig::CRYPTO crypto)
	{
		switch (crypto) {
			case ReDataConfig::CRYPTO::NONE: return 0;
			case ReDataConfig::CRYPTO::AES128: return 16;
			case ReDataConfig::CRYPTO::AES256: return 32;

			case ReDataConfig::CRYPTO::_MAX: break;
		}
		cThrow ("Unsupported crypto");
	}

	static inline std::string _calc_aes (const std::string &msg, const std::string &iv, const std::string &key, const bool enc, ReDataConfig::CryptoCache &cache)
	{
#ifdef SHAGA_FULL
		std::call_once(cache._aes_init_flag, [&cache](){ mbedtls_aes_init (&cache._aes_ctx); });

		if ((enc == true ? mbedtls_aes_setkey_enc : mbedtls_aes_setkey_dec)(&cache._aes_ctx, reinterpret_cast<const unsigned char *> (key.data ()), key.size () * 8) != 0) {
			cThrow ("Wrong crypto key size");
		}

		if (iv.size () != 16) {
			cThrow ("Wrong crypto IV size");
		}

		::memcpy (cache.temp_iv, iv.data (), 16);

		std::string out;
		out.resize (msg.size ());
		if (mbedtls_aes_crypt_cbc (&cache._aes_ctx, (enc == true) ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT, msg.size (), cache.temp_iv, reinterpret_cast<const unsigned char *> (msg.data ()), reinterpret_cast<unsigned char *> (&out[0])) != 0) {
			cThrow ("Wrong crypto message size");
		}

		return out;
#else
		(void) msg;
		(void) iv;
		(void) key;
		(void) enc;
		(void) cache;
		cThrow ("Cryptography is not supported in lite version");
#endif // SHAGA_FULL
	}

	typedef std::function<std::string(const std::string &, const std::string &, const std::string &, const bool, ReDataConfig::CryptoCache &)> CRYPTO_FUNC;

	static inline CRYPTO_FUNC _get_crypto_calc_function (const ReDataConfig::CRYPTO crypto)
	{
		switch (crypto) {
			case ReDataConfig::CRYPTO::NONE: break;

			case ReDataConfig::CRYPTO::AES128: return _calc_aes;
			case ReDataConfig::CRYPTO::AES256: return _calc_aes;

			case ReDataConfig::CRYPTO::_MAX: break;
		}
		cThrow ("Unsupported crypto");
	}

	static inline std::string _random_string (const size_t sze, ReDataConfig::CryptoCache &cache)
	{
#ifdef SHAGA_FULL
		std::string out (sze, 0);
		for (size_t i = 0; i < sze; ++i) {
			out[i] = cache._rand_dist (cache._rand_rng);
		}
		return out;
#else
		(void) sze;
		(void) cache;
		cThrow ("Cryptography is not supported in lite version");
#endif // SHAGA_FULL
	}

	static inline size_t round_up_pow2 (const size_t numToRound, const size_t multiple)
	{
		return (numToRound + multiple - 1) & ~(multiple - 1);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	size_t ReDataConfig::calc_crypto_enc (const std::string &plain, std::string &out, const std::string &key)
	{
		const size_t key_size = _get_crypto_key_size (_used_crypto);
		if (key_size == 0) {
			out.append (plain);
			return plain.size ();
		}
		if (key_size != key.size ()) {
			cThrow ("Wrong crypto key size. Expected %" PRIu32 " bits, got %" PRIu32 " bits.", static_cast<uint32_t> (key_size), static_cast<uint32_t> (key.size ()));
		}

		const size_t orig_size = out.size ();
		const size_t iv_size = _get_crypto_iv_size (_used_crypto);
		const size_t block_size = _get_crypto_block_size (_used_crypto);
		CRYPTO_FUNC func = _get_crypto_calc_function (_used_crypto);
		const std::string iv = _random_string (iv_size, _cache_crypto);

		BIN::from_size (plain.size (), out);
		out.append (iv);

		std::string msg = plain;
		msg.resize (round_up_pow2 (plain.size (), block_size));

		out.append (func (msg, iv, key, true, _cache_crypto));

		return out.size () - orig_size;
	}

	std::string ReDataConfig::calc_crypto_enc (const std::string &plain, const std::string &key)
	{
		std::string out;
		calc_crypto_enc (plain, out, key);
		return out;
	}

	size_t ReDataConfig::calc_crypto_dec (const std::string &msg, std::string &out, const std::string &key)
	{
		const size_t key_size = _get_crypto_key_size (_used_crypto);

		if (key_size == 0) {
			out.append (msg);
			return msg.size ();
		}

		if (key_size != key.size ()) {
			cThrow ("Wrong crypto key size. Expected %" PRIu32 " bits, got %" PRIu32 " bits.", static_cast<uint32_t> (key_size), static_cast<uint32_t> (key.size ()));
		}

		const size_t iv_size = _get_crypto_iv_size (_used_crypto);
		CRYPTO_FUNC func = _get_crypto_calc_function (_used_crypto);

		size_t offset = 0;

		const size_t plain_size = BIN::to_size (msg, offset);

		const std::string iv = msg.substr (offset, iv_size);
		if (iv.size () != iv_size) {
			cThrow ("Not enough data for IV");
		}
		offset += iv_size;

		const std::string plain = func (msg.substr (offset), iv, key, false, _cache_crypto);
		if (plain.size () < plain_size) {
			cThrow ("Decrypted less bytes than expected");
		}

		out.append (plain.substr (0, plain_size));

		return plain_size;
	}

	std::string ReDataConfig::calc_crypto_dec (const std::string &msg, const std::string &key)
	{
		std::string out;
		calc_crypto_dec (msg, out, key);
		return out;
	}

	size_t ReDataConfig::get_crypto_block_size (void) const
	{
		return _get_crypto_block_size (_used_crypto);
	}

	size_t ReDataConfig::get_crypto_key_size (void) const
	{
		return _get_crypto_key_size (_used_crypto);
	}

	bool ReDataConfig::has_crypto (void) const
	{
		return _get_crypto_key_size (_used_crypto) > 0;
	}

	bool ReDataConfig::has_crypto_size_at_least_bits (const size_t limit) const
	{
		return _get_crypto_key_size (_used_crypto) * 8 >= limit;
	}

}

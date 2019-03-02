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

	static inline size_t _get_crypto_block_size (const ReDataConfig::CRYPTO crypto)
	{
		switch (crypto) {
			case ReDataConfig::CRYPTO::NONE:
				return 0;

			case ReDataConfig::CRYPTO::AES_128_CBC:
			case ReDataConfig::CRYPTO::AES_256_CBC:
				return 16;

			case ReDataConfig::CRYPTO::_MAX:
				break;
		}
		cThrow ("Unsupported crypto");
	}

	static inline size_t _get_crypto_iv_size (const ReDataConfig::CRYPTO crypto)
	{
		switch (crypto) {
			case ReDataConfig::CRYPTO::NONE:
				return 0;

			case ReDataConfig::CRYPTO::AES_128_CBC:
			case ReDataConfig::CRYPTO::AES_256_CBC:
				return 16;

			case ReDataConfig::CRYPTO::_MAX:
				break;
		}
		cThrow ("Unsupported crypto");
	}

	static inline size_t _get_crypto_key_size (const ReDataConfig::CRYPTO crypto)
	{
		switch (crypto) {
			case ReDataConfig::CRYPTO::NONE:
				return 0;

			case ReDataConfig::CRYPTO::AES_128_CBC:
				return 16;

			case ReDataConfig::CRYPTO::AES_256_CBC:
				return 32;

			case ReDataConfig::CRYPTO::_MAX:
				break;
		}
		cThrow ("Unsupported crypto");
	}

	static inline size_t _calc_aes (std::string &out, const char *msg, const size_t msg_size, unsigned char *iv_data, const size_t iv_size, const std::string &key, const bool enc, ReDataConfig::CryptoCache &cache)
	{
#ifdef SHAGA_FULL
		if (iv_size != 16) {
			cThrow ("Wrong crypto IV size");
		}

		#ifdef SHAGA_THREADING
			std::call_once (cache._aes_init_flag, [&]()-> void {
				mbedtls_aes_init (&cache._aes_ctx);
			});
		#else
			if (std::exchange (cache._aes_init_flag, true) == false) {
				mbedtls_aes_init (&cache._aes_ctx);
			}
		#endif // SHAGA_THREADING

		if ((true == enc ? mbedtls_aes_setkey_enc : mbedtls_aes_setkey_dec)(&cache._aes_ctx, reinterpret_cast<const unsigned char *> (key.data ()), key.size () * 8) != 0) {
			cThrow ("Wrong crypto key size");
		}

		const size_t orig_out_size = out.size ();
		out.resize (orig_out_size + msg_size);
		if (mbedtls_aes_crypt_cbc (&cache._aes_ctx, (true == enc) ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT, msg_size, iv_data,
			reinterpret_cast<const unsigned char *> (msg), reinterpret_cast<unsigned char *> (out.data () + orig_out_size)) != 0)
		{
			cThrow ("Wrong crypto message size");
		}

		return msg_size;

#else
		(void) out;
		(void) msg;
		(void) msg_size;
		(void) iv_data;
		(void) iv_size;
		(void) key;
		(void) enc;
		(void) cache;
		cThrow ("Cryptography is not supported in lite version");
#endif // SHAGA_FULL
	}

	typedef std::function<size_t(std::string &, const char *, const size_t, unsigned char *, const size_t, const std::string &, const bool, ReDataConfig::CryptoCache &)> CRYPTO_FUNC;

	static inline CRYPTO_FUNC _get_crypto_calc_function (const ReDataConfig::CRYPTO crypto)
	{
		switch (crypto) {
			case ReDataConfig::CRYPTO::NONE:
				break;

			case ReDataConfig::CRYPTO::AES_128_CBC:
			case ReDataConfig::CRYPTO::AES_256_CBC:
				return _calc_aes;

			case ReDataConfig::CRYPTO::_MAX:
				break;
		}
		cThrow ("Unsupported crypto");
	}

	static inline void _random_string (unsigned char *str, const size_t sze, ReDataConfig::CryptoCache &cache)
	{
#ifdef SHAGA_FULL
		cache._rng.generate_n<uint8_t> (str, sze, 0x00, 0xff);
#else
		(void) str;
		(void) sze;
		(void) cache;
		cThrow ("Cryptography is not supported in lite version");
#endif // SHAGA_FULL
	}

	static inline size_t _round_up_pow2 (const size_t numToRound, const size_t multiple)
	{
		return (numToRound + multiple - 1) & ~(multiple - 1);
	}

	static inline void _pad_to_blocksize (std::string &str, const size_t block_size)
	{
#ifdef SHAGA_FULL
		/* Store original data size before any modification */
		const size_t orig_size = str.size ();

		/* Find nearest greater size divisible by block_size. Add 3 bytes at the end that will be used to store original size */
		const size_t new_size = _round_up_pow2 (orig_size + 3, block_size);

		/* Add needed padding and reserve enough memory for the 3 additional bytes */
		str.reserve (new_size);
		str.resize (new_size - 3);

		/* Pad with data */
		std::iota (str.begin () + orig_size, str.end (), 0x33);
		//cache._rng.generate<uint8_t> (str.begin () + orig_size, str.end (), 0x00, 0xff);

		/* Append last 3 bytes with 24-bit little endian encoded original data size */
		BIN::from_uint24 (orig_size, str);
#else
		(void) str;
		(void) block_size;
		cThrow ("Cryptography is not supported in lite version");
#endif // SHAGA_FULL
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReDataConfig::calc_crypto_enc (std::string &plain, std::string &out, const std::string &key)
	{
		/* This function is appending data to out, because header might already be present */
		const size_t key_size = _get_crypto_key_size (_used_crypto);

		if (0 == key_size) {
			cThrow ("No crypto algorithm selected");
		}
		else if (key_size != key.size ()) {
			cThrow ("Wrong crypto key size. Expected %" PRIu32 " bytes, got %" PRIu32 " bytes.", static_cast<uint32_t> (key_size), static_cast<uint32_t> (key.size ()));
		}
		else {
			const size_t iv_size = _get_crypto_iv_size (_used_crypto);
			if (iv_size > MBEDTLS_MAX_IV_LENGTH) {
				cThrow ("IV size larger than allowed maximum");
			}

			const size_t block_size = _get_crypto_block_size (_used_crypto);
			CRYPTO_FUNC func = _get_crypto_calc_function (_used_crypto);

			if (false == _user_iv_enabled) {
				/* Create random IV */
				_random_string (_temp_iv, iv_size, _cache_crypto);
			}
			else {
				/* Use user defined IV */
				if (_user_iv.size () != iv_size) {
					cThrow ("User defined IV size does not match encryption IV size");
				}
				::memcpy (_temp_iv, _user_iv.data (), _user_iv.size ());
			}

			/* Add IV to output */
			out.append (reinterpret_cast<const char *> (_temp_iv), iv_size);

			/* Add padding to plain data to match block size */
			_pad_to_blocksize (plain, block_size);

			func (out, plain.data (), plain.size (), _temp_iv, iv_size, key, true, _cache_crypto);
		}
	}

	void ReDataConfig::calc_crypto_dec (const std::string &msg, size_t offset, std::string &out, const std::string &key)
	{
		/* This function is replacing data in out */
		const size_t key_size = _get_crypto_key_size (_used_crypto);

		if (0 == key_size) {
			cThrow ("No crypto algorithm selected");
		}
		else if (key_size != key.size ()) {
			cThrow ("Wrong crypto key size. Expected %" PRIu32 " bytes, got %" PRIu32 " bytes.", static_cast<uint32_t> (key_size), static_cast<uint32_t> (key.size ()));
		}
		else {
			/* Get IV size */
			const size_t iv_size = _get_crypto_iv_size (_used_crypto);
			if (iv_size > MBEDTLS_MAX_IV_LENGTH) {
				cThrow ("IV size larger than allowed maximum");
			}

			const size_t block_size = _get_crypto_block_size (_used_crypto);
			if (block_size < 3) {
				cThrow ("Block size smaller than allowed minimum");
			}

			/* Get crypto function */
			CRYPTO_FUNC func = _get_crypto_calc_function (_used_crypto);

			/* IV + encrypted data are following, but are there enought bytes? */
			if ((offset + iv_size) > msg.size ()) {
				cThrow ("Not enough data for IV");
			}

			/* Copy IV from message to temporary memory, that needs to be modifiable */
			::memcpy (_temp_iv, msg.data () + offset, iv_size);

			/* Move offset to the beginning of the encrypted data */
			offset += iv_size;

			/* Check if the rest of the data has correct size */
			size_t decrypted_size = msg.size () - offset;
			if (0 == decrypted_size || (decrypted_size % block_size) != 0) {
				/* Decrypted size cannot be zero and it must be multiplication of block_size */
				cThrow ("Decryption not possible, data not multiply of block size");
			}

			/* Clear out */
			out.resize (0);

			decrypted_size = func (out, msg.data () + offset, decrypted_size, _temp_iv, iv_size, key, false, _cache_crypto);
			if (0 == decrypted_size || (decrypted_size % block_size) != 0) {
				/* Decrypted size cannot be zero and it must be multiplication of block_size */
				cThrow ("Decryption failed");
			}

			/* Last 3 bytes contain plain size */
			decrypted_size -= 3;
			const size_t plain_size = BIN::to_uint24 (out, decrypted_size);

			/* Plain size cannot be larger than decrypted size minus last 3 bytes */
			if (plain_size > (decrypted_size - 3)) {
				cThrow ("Decryption failed, size mismatch");
			}

			/* Strip padding from the end of decrypted data */
			out.resize (plain_size);
		}
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

	void ReDataConfig::set_user_iv (const std::string &iv)
	{
		_user_iv_enabled = true;
		_user_iv.assign (iv);
	}

	void ReDataConfig::unset_user_iv (void)
	{
		_user_iv_enabled = false;
		_user_iv.clear ();
	}
}

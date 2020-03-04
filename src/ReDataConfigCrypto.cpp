/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SHAGA_NODISCARD static inline size_t _get_crypto_block_size (const ReDataConfig::CRYPTO crypto)
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
		cThrow ("Unsupported crypto"sv);
	}

	SHAGA_NODISCARD static inline size_t _get_crypto_iv_size (const ReDataConfig::CRYPTO crypto)
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
		cThrow ("Unsupported crypto"sv);
	}

	SHAGA_NODISCARD static inline size_t _get_crypto_key_size (const ReDataConfig::CRYPTO crypto)
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
		cThrow ("Unsupported crypto"sv);
	}

	SHAGA_NODISCARD static inline size_t _calc_aes (
		[[maybe_unused]] std::string &out,
		[[maybe_unused]] const char *msg,
		[[maybe_unused]] const size_t msg_size,
		[[maybe_unused]] unsigned char *iv_data,
		[[maybe_unused]] const size_t iv_size,
		[[maybe_unused]] const std::string_view key,
		[[maybe_unused]] const bool enc,
		[[maybe_unused]] ReDataConfig::CryptoCache &cache)
	{
#ifdef SHAGA_FULL
		if (iv_size != 16) {
			cThrow ("Wrong crypto IV size"sv);
		}

		if ((true == enc ? mbedtls_aes_setkey_enc : mbedtls_aes_setkey_dec)(&cache._aes_ctx, reinterpret_cast<const unsigned char *> (key.data ()), key.size () * 8) != 0) {
			cThrow ("Wrong crypto key size"sv);
		}

		const size_t orig_out_size = out.size ();
		out.resize (orig_out_size + msg_size);
		if (mbedtls_aes_crypt_cbc (&cache._aes_ctx,
			(true == enc) ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT,
			msg_size,
			iv_data,
			reinterpret_cast<const unsigned char *> (msg),
			reinterpret_cast<unsigned char *> (out.data () + orig_out_size)) != 0)
		{
			cThrow ("Wrong crypto message size"sv);
		}

		return msg_size;
#else
		cThrow ("Cryptography is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

	typedef std::function<size_t(std::string &, const char *, const size_t, unsigned char *, const size_t, const std::string_view, const bool, ReDataConfig::CryptoCache &)> CRYPTO_FUNC;

	SHAGA_NODISCARD static inline CRYPTO_FUNC _get_crypto_calc_function (const ReDataConfig::CRYPTO crypto)
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
		cThrow ("Unsupported crypto"sv);
	}

	static inline void _random_string ([[maybe_unused]] unsigned char *str, [[maybe_unused]] const size_t sze, [[maybe_unused]] ReDataConfig::CryptoCache &cache)
	{
#ifdef SHAGA_FULL
		cache._rng.generate_n<uint8_t> (str, sze, 0x00, 0xff);
#else
		cThrow ("Cryptography is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

	SHAGA_NODISCARD static inline size_t _round_up_pow2 (const size_t numToRound, const size_t multiple)
	{
		return (numToRound + multiple - 1) & ~(multiple - 1);
	}

	static inline void _pad_to_blocksize ([[maybe_unused]] std::string &str, [[maybe_unused]] const size_t block_size)
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

		/* Append last 3 bytes with 24-bit little endian encoded original data size */
		BIN::from_uint24 (orig_size, str);
#else
		cThrow ("Cryptography is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReDataConfig::calc_crypto_enc (std::string &plain, std::string &out, const std::string_view key)
	{
		/* This function is appending data to out, because header might already be present */
		const size_t key_size = _get_crypto_key_size (_used_crypto);

		if (0 == key_size) {
			cThrow ("No crypto algorithm selected"sv);
		}
		else if (key_size != key.size ()) {
			cThrow ("Wrong crypto key size. Expected {} bytes, got {} bytes."sv, static_cast<uint32_t> (key_size), static_cast<uint32_t> (key.size ()));
		}
		else {
			const size_t iv_size = _get_crypto_iv_size (_used_crypto);
			if (iv_size > MBEDTLS_MAX_IV_LENGTH) {
				cThrow ("IV size larger than allowed maximum"sv);
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
					cThrow ("User defined IV size does not match encryption IV size"sv);
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

	void ReDataConfig::calc_crypto_dec (const std::string_view msg, size_t offset, std::string &out, const std::string_view key)
	{
		/* This function is replacing data in out */
		const size_t key_size = _get_crypto_key_size (_used_crypto);

		if (0 == key_size) {
			cThrow ("No crypto algorithm selected"sv);
		}
		else if (key_size != key.size ()) {
			cThrow ("Wrong crypto key size. Expected {} bytes, got {} bytes."sv, static_cast<uint32_t> (key_size), static_cast<uint32_t> (key.size ()));
		}
		else {
			/* Get IV size */
			const size_t iv_size = _get_crypto_iv_size (_used_crypto);
			if (iv_size > MBEDTLS_MAX_IV_LENGTH) {
				cThrow ("IV size larger than allowed maximum"sv);
			}

			const size_t block_size = _get_crypto_block_size (_used_crypto);
			if (block_size < 3) {
				cThrow ("Block size smaller than allowed minimum"sv);
			}

			/* Get crypto function */
			CRYPTO_FUNC func = _get_crypto_calc_function (_used_crypto);

			/* IV + encrypted data are following, but are there enought bytes? */
			if ((offset + iv_size) > msg.size ()) {
				cThrow ("Not enough data for IV"sv);
			}

			/* Copy IV from message to temporary memory, that needs to be modifiable */
			::memcpy (_temp_iv, msg.data () + offset, iv_size);

			/* Move offset to the beginning of the encrypted data */
			offset += iv_size;

			/* Check if the rest of the data has correct size */
			size_t decrypted_size = msg.size () - offset;
			if (0 == decrypted_size || (decrypted_size % block_size) != 0) {
				/* Decrypted size cannot be zero and it must be multiplication of block_size */
				cThrow ("Decryption not possible, data not multiply of block size"sv);
			}

			/* Clear out */
			out.resize (0);

			decrypted_size = func (out, msg.data () + offset, decrypted_size, _temp_iv, iv_size, key, false, _cache_crypto);
			if (0 == decrypted_size || (decrypted_size % block_size) != 0) {
				/* Decrypted size cannot be zero and it must be multiplication of block_size */
				cThrow ("Decryption failed"sv);
			}

			/* Last 3 bytes contain plain size */
			decrypted_size -= 3;
			const size_t plain_size = BIN::to_uint24 (out, decrypted_size);

			/* Plain size cannot be larger than decrypted size minus last 3 bytes */
			if (plain_size > (decrypted_size - 3)) {
				cThrow ("Decryption failed, size mismatch"sv);
			}

			/* Strip padding from the end of decrypted data */
			out.resize (plain_size);
		}
	}

	SHAGA_NODISCARD size_t ReDataConfig::get_crypto_block_size (void) const
	{
		return _get_crypto_block_size (_used_crypto);
	}

	SHAGA_NODISCARD size_t ReDataConfig::get_crypto_key_size (void) const
	{
		return _get_crypto_key_size (_used_crypto);
	}

	SHAGA_NODISCARD bool ReDataConfig::has_crypto (void) const
	{
		return _get_crypto_key_size (_used_crypto) > 0;
	}

	SHAGA_NODISCARD bool ReDataConfig::has_crypto_key_size_at_least_bits (const size_t limit) const
	{
		return (_get_crypto_key_size (_used_crypto) * 8) >= limit;
	}

	void ReDataConfig::set_user_iv (const std::string_view iv)
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

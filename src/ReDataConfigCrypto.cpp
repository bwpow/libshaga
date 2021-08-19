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

	[[maybe_unused]] static const constexpr std::string_view _ad {"ShaGaAD"sv};

	HEDLEY_WARN_UNUSED_RESULT static inline size_t _get_crypto_block_size (const ReDataConfig::CRYPTO crypto)
	{
		switch (crypto) {
			case ReDataConfig::CRYPTO::NONE:
				return 0;

			case ReDataConfig::CRYPTO::AES_128_CBC:
			case ReDataConfig::CRYPTO::AES_256_CBC:
				return 16;

			case ReDataConfig::CRYPTO::CHACHA20:
			case ReDataConfig::CRYPTO::CHACHA20_POLY1305:
				return 1;

			case ReDataConfig::CRYPTO::_MAX:
				break;
		}
		cThrow ("Unsupported crypto"sv);
	}

	HEDLEY_WARN_UNUSED_RESULT static inline size_t _get_crypto_iv_size (const ReDataConfig::CRYPTO crypto)
	{
		switch (crypto) {
			case ReDataConfig::CRYPTO::NONE:
				return 0;

			case ReDataConfig::CRYPTO::AES_128_CBC:
			case ReDataConfig::CRYPTO::AES_256_CBC:
				return 16;

			case ReDataConfig::CRYPTO::CHACHA20:
			case ReDataConfig::CRYPTO::CHACHA20_POLY1305:
				return 12;

			case ReDataConfig::CRYPTO::_MAX:
				break;
		}
		cThrow ("Unsupported crypto"sv);
	}

	HEDLEY_WARN_UNUSED_RESULT static inline size_t _get_crypto_key_size (const ReDataConfig::CRYPTO crypto)
	{
		switch (crypto) {
			case ReDataConfig::CRYPTO::NONE:
				return 0;

			case ReDataConfig::CRYPTO::AES_128_CBC:
				return 16;

			case ReDataConfig::CRYPTO::AES_256_CBC:
				return 32;

			case ReDataConfig::CRYPTO::CHACHA20:
			case ReDataConfig::CRYPTO::CHACHA20_POLY1305:
				return 32;

			case ReDataConfig::CRYPTO::_MAX:
				break;
		}
		cThrow ("Unsupported crypto"sv);
	}

	HEDLEY_WARN_UNUSED_RESULT static inline size_t _get_crypto_mac_size (const ReDataConfig::CRYPTO crypto)
	{
		switch (crypto) {
			case ReDataConfig::CRYPTO::NONE:
			case ReDataConfig::CRYPTO::AES_128_CBC:
			case ReDataConfig::CRYPTO::AES_256_CBC:
			case ReDataConfig::CRYPTO::CHACHA20:
				return 0;

			case ReDataConfig::CRYPTO::CHACHA20_POLY1305:
				return 16;

			case ReDataConfig::CRYPTO::_MAX:
				break;
		}
		cThrow ("Unsupported crypto"sv);
	}

	static inline void _calc_aes (
		[[maybe_unused]] uint8_t *const out_data,
		[[maybe_unused]] const uint8_t *const msg_data,
		[[maybe_unused]] const size_t msg_size,
		[[maybe_unused]] const uint8_t *const iv_data,
		[[maybe_unused]] const size_t iv_size,
		[[maybe_unused]] const uint8_t *const key_data,
		[[maybe_unused]] const size_t key_size,
		[[maybe_unused]] const bool enc,
		[[maybe_unused]] ReDataConfig::CryptoCache &cache)
	{
#ifdef SHAGA_FULL
		if (iv_size != 16) {
			cThrow ("Wrong crypto IV size"sv);
		}

		if ((true == enc ? ::mbedtls_aes_setkey_enc : ::mbedtls_aes_setkey_dec)(&cache._aes_ctx, key_data, key_size * 8) != 0) {
			cThrow ("Wrong crypto key size"sv);
		}

		if (::mbedtls_aes_crypt_cbc (&cache._aes_ctx,
			(true == enc) ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT,
			msg_size,
			const_cast<uint8_t *> (iv_data),
			msg_data,
			out_data) != 0)
		{
			cThrow ("Wrong crypto message size"sv);
		}
#else
		cThrow ("Cryptography is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

	static inline void _calc_chacha20 (
		[[maybe_unused]] uint8_t *const out_data,
		[[maybe_unused]] const uint8_t *const msg_data,
		[[maybe_unused]] const size_t msg_size,
		[[maybe_unused]] const uint8_t *const iv_data,
		[[maybe_unused]] const size_t iv_size,
		[[maybe_unused]] const uint8_t *const key_data,
		[[maybe_unused]] const size_t key_size,
		[[maybe_unused]] const bool enc,
		[[maybe_unused]] ReDataConfig::CryptoCache &cache)
	{
#ifdef SHAGA_FULL
		if (iv_size != 12) {
			cThrow ("Wrong crypto IV size"sv);
		}

		if (key_size != 32) {
			cThrow ("Wrong crypto key size"sv);
		}

		if (::mbedtls_chacha20_setkey (&cache._chacha20_ctx, key_data) != 0) {
			cThrow ("Crypto key was rejected"sv);
		}

		if (::mbedtls_chacha20_starts (&cache._chacha20_ctx, iv_data, 0) != 0) {
			cThrow ("IV was rejected"sv);
		}

		if (::mbedtls_chacha20_update (&cache._chacha20_ctx, msg_size, msg_data, out_data) != 0)
		{
			cThrow ("Wrong crypto message size"sv);
		}
#else
		cThrow ("Cryptography is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

	static inline void _calc_chachapoly (
		[[maybe_unused]] uint8_t *const out_data,
		[[maybe_unused]] const uint8_t *const msg_data,
		[[maybe_unused]] const size_t msg_size,
		[[maybe_unused]] const uint8_t *const iv_data,
		[[maybe_unused]] const size_t iv_size,
		[[maybe_unused]] const uint8_t *const key_data,
		[[maybe_unused]] const size_t key_size,
		[[maybe_unused]] const bool enc,
		[[maybe_unused]] ReDataConfig::CryptoCache &cache)
	{
#ifdef SHAGA_FULL
		if (iv_size != 12) {
			cThrow ("Wrong crypto IV size"sv);
		}

		if (key_size != 32) {
			cThrow ("Wrong crypto key size"sv);
		}

		if (::mbedtls_chachapoly_setkey (&cache._chachapoly_ctx, key_data) != 0) {
			cThrow ("Crypto key was rejected"sv);
		}

		if (true == enc) {
			if (::mbedtls_chachapoly_encrypt_and_tag (&cache._chachapoly_ctx,
				msg_size,
				iv_data,
				reinterpret_cast<const uint8_t *> (_ad.data ()),
				_ad.size (),
				msg_data,
				out_data,
				out_data + msg_size) != 0)
			{
				cThrow ("Unable to encrypt and tag message"sv);
			}
		}
		else {
			if (::mbedtls_chachapoly_auth_decrypt (&cache._chachapoly_ctx,
				msg_size,
				iv_data,
				reinterpret_cast<const uint8_t *> (_ad.data ()),
				_ad.size (),
				msg_data + msg_size,
				msg_data,
				out_data) != 0)
			{
				cThrow ("Unable to decrypt and authenticate message"sv);
			}
		}
#else
		cThrow ("Cryptography is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

	typedef std::function<void(
		uint8_t *const out_data,
		const uint8_t *const msg_data,
		const size_t msg_size,
		const uint8_t *const iv_data,
		const size_t iv_size,
		const uint8_t *const key_data,
		const size_t key_size,
		const bool enc,
		ReDataConfig::CryptoCache &cache)> CRYPTO_FUNC;

	HEDLEY_WARN_UNUSED_RESULT static inline CRYPTO_FUNC _get_crypto_calc_function (const ReDataConfig::CRYPTO crypto)
	{
		switch (crypto) {
			case ReDataConfig::CRYPTO::NONE:
				break;

			case ReDataConfig::CRYPTO::AES_128_CBC:
			case ReDataConfig::CRYPTO::AES_256_CBC:
				return _calc_aes;

			case ReDataConfig::CRYPTO::CHACHA20:
				return _calc_chacha20;

			case ReDataConfig::CRYPTO::CHACHA20_POLY1305:
				return _calc_chachapoly;

			case ReDataConfig::CRYPTO::_MAX:
				break;
		}
		cThrow ("Unsupported crypto"sv);
	}

	static inline void _random_string ([[maybe_unused]] unsigned char *str, [[maybe_unused]] const size_t sze, [[maybe_unused]] ReDataConfig::CryptoCache &cache)
	{
#ifdef SHAGA_FULL
		cache._rng.generate_n<uint8_t> (str, sze);
#else
		cThrow ("Cryptography is not supported in lite version"sv);
#endif // SHAGA_FULL
	}

	HEDLEY_WARN_UNUSED_RESULT static inline size_t _round_up_pow2 (const size_t numToRound, const size_t multiple)
	{
		return (numToRound + multiple - 1) & ~(multiple - 1);
	}

	static inline void _pad_to_blocksize (std::string &str, const size_t block_size)
	{
		if (1 == block_size) {
			/* If the block size is one byte, there is no need for padding */
			return;
		}

		/* Store original data size before any modification */
		const size_t orig_size = str.size ();

		/* Find nearest greater size divisible by block_size. Add 1 byte at the end that will be used to store padding size */
		const size_t new_size = _round_up_pow2 (orig_size + 1, block_size);

		const size_t padding_size = (new_size - orig_size - 1) % block_size;

		/* Reserve enough memory */
		str.reserve (new_size);

		/* Pad with data */
		std::generate_n (std::back_inserter(str), padding_size, [var = 0x33](void) mutable -> uint8_t { return ++var; });

		/* Append last 1 byte with padding size */
		BIN::from_uint8 (padding_size, str);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SHAGA_FULL
	ReDataConfig::CryptoCache::CryptoCache ()
	{
		::mbedtls_aes_init (&_aes_ctx);
		::mbedtls_chacha20_init (&_chacha20_ctx);
		::mbedtls_chachapoly_init (&_chachapoly_ctx);
	}

	ReDataConfig::CryptoCache::~CryptoCache ()
	{
		::mbedtls_chachapoly_free (&_chachapoly_ctx);
		::mbedtls_chacha20_free (&_chacha20_ctx);
		::mbedtls_aes_free (&_aes_ctx);
	}
#endif // SHAGA_FULL

	void ReDataConfig::calc_crypto_enc (std::string &plain, std::string &out_append, const std::string_view key) const
	{
		/* This function is appending data to out, because header might already be present */
		/* Data may be appended to plain and out_append and not cleaned up on fail. This has to be handled from caller! */
		const size_t key_size = _get_crypto_key_size (_used_crypto);

		if (0 == key_size) {
			cThrow ("No crypto algorithm selected"sv);
		}
		else if (key_size != key.size ()) {
			cThrow ("Wrong crypto key size. Expected {} bytes, got {} bytes."sv, key_size, key.size ());
		}
		else {
			const size_t iv_size = _get_crypto_iv_size (_used_crypto);
			if (iv_size > MBEDTLS_MAX_IV_LENGTH) {
				cThrow ("IV size larger than allowed maximum"sv);
			}

			if (true == _counter_iv_enabled && iv_size > 0 && iv_size < sizeof (uint32_t)) {
				cThrow ("IV size smaller than counter size"sv);
			}

			const size_t block_size = _get_crypto_block_size (_used_crypto);
			const CRYPTO_FUNC func = _get_crypto_calc_function (_used_crypto);
			const size_t mac_data = _get_crypto_mac_size (_used_crypto);

			const size_t orig_out_size = out_append.size ();

			/* Add padding to plain data to match block size */
			_pad_to_blocksize (plain, block_size);

			/* Resize output to include IV and plain+padding + MAC data */
			out_append.resize (orig_out_size + iv_size + plain.size () + mac_data);

			if (iv_size > 0) {
				/* Random bytes of the whole IV */
				const size_t iv_size_random = (true == _counter_iv_enabled) ? (iv_size - sizeof (uint32_t)) : (iv_size);

				if (false == _user_iv_enabled) {
					/* Create random IV */
					_random_string (_temp_iv, iv_size_random, _cache_crypto);
				}
				else {
					/* Use user defined IV */
					if (_user_iv.size () != iv_size_random) {
						cThrow ("User defined IV size does not match encryption IV size"sv);
					}
					::memcpy (_temp_iv, _user_iv.data (), _user_iv.size ());
				}

				if (true == _counter_iv_enabled) {
					const uint32_t cnt = (_counter_iv++);
					BIN::_from_uint32 (cnt, _temp_iv + iv_size_random);
				}

				/* Add IV to output */
				::memcpy (out_append.data () + orig_out_size, _temp_iv, iv_size);
			}

			func (
				reinterpret_cast<uint8_t *> (out_append.data ()) + (orig_out_size + iv_size),	// out_data
				reinterpret_cast<const uint8_t *> (plain.data ()),								// msg_data
				plain.size (),																	// msg_size
				_temp_iv,																		// iv_data
				iv_size,																		// iv_size
				reinterpret_cast<const uint8_t *> (key.data ()),								// key_data
				key_size,																		// key_size
				true,																			// enc
				_cache_crypto);
		}
	}

	void ReDataConfig::calc_crypto_dec (std::string_view msg, std::string &out, const std::string_view key) const
	{
		/* This function is replacing data in out */
		const size_t key_size = _get_crypto_key_size (_used_crypto);

		if (0 == key_size) {
			cThrow ("No crypto algorithm selected"sv);
		}
		else if (key_size != key.size ()) {
			cThrow ("Wrong crypto key size. Expected {} bytes, got {} bytes."sv, key_size, key.size ());
		}
		else {
			/* Get IV size */
			const size_t iv_size = _get_crypto_iv_size (_used_crypto);
			if (iv_size > MBEDTLS_MAX_IV_LENGTH) {
				cThrow ("IV size larger than allowed maximum"sv);
			}

			const size_t block_size = _get_crypto_block_size (_used_crypto);
			if (block_size < 1) {
				cThrow ("Block size smaller than allowed minimum"sv);
			}

			const size_t mac_data = _get_crypto_mac_size (_used_crypto);

			/* Get crypto function */
			CRYPTO_FUNC func = _get_crypto_calc_function (_used_crypto);

			/* IV + encrypted data are following, but are there enought bytes? */
			if (iv_size > msg.size ()) {
				cThrow ("Not enough data for IV"sv);
			}

			if (iv_size > 0) {
				/* Copy IV from message to temporary memory, that needs to be modifiable */
				::memcpy (_temp_iv, msg.data (), iv_size);

				/* Move offset to the beginning of the encrypted data */
				msg.remove_prefix (iv_size);
			}

			/* Check if the rest of the data has correct size */
			size_t decrypted_size = msg.size ();

			/* MAC is always appended at the very end */
			if (decrypted_size < mac_data) {
				cThrow ("Not enough data for MAC"sv);
			}
			decrypted_size -= mac_data;

			if ((decrypted_size % block_size) != 0) {
				/* Decrypted size cannot be zero and it must be multiplication of block_size */
				cThrow ("Decryption not possible, data not multiply of block size"sv);
			}
			if (block_size > 1 && 0 == decrypted_size) {
				/* Block size larger than 1 requires padding, thus message cannot be empty */
				cThrow ("Decryption not possible, not enough data"sv);
			}

			/* Clear out */
			out.resize (decrypted_size);

			func (
				reinterpret_cast<uint8_t *> (out.data ()),			// out_data
				reinterpret_cast<const uint8_t *> (msg.data ()),	// msg_data
				decrypted_size,										// msg_size
				_temp_iv,											// iv_data
				iv_size,											// iv_size
				reinterpret_cast<const uint8_t *> (key.data ()),	// key_data
				key_size,											// key_size
				false,												// enc
				_cache_crypto);

			if (block_size > 1) {
				/* Last byte contain padding size, if block is larger than 1 byte */
				--decrypted_size;
				const uint_fast8_t padding_size = BIN::_to_uint8 (out.data () + decrypted_size);

				/* Padding size must be smaller than block size */
				if (padding_size >= block_size) {
					cThrow ("Decryption failed, padding size mismatch"sv);
				}

				/* Strip padding from the end of decrypted data */
				decrypted_size -= padding_size;
				out.resize (decrypted_size);
			}
			/* If block size is one byte, decrypted_size is exactly plain_size, because no padding was done */
		}
	}

	HEDLEY_WARN_UNUSED_RESULT size_t ReDataConfig::get_crypto_block_size (void) const
	{
		return _get_crypto_block_size (_used_crypto);
	}

	HEDLEY_WARN_UNUSED_RESULT size_t ReDataConfig::get_crypto_key_size (void) const
	{
		return _get_crypto_key_size (_used_crypto);
	}

	HEDLEY_WARN_UNUSED_RESULT size_t ReDataConfig::get_crypto_iv_size (void) const
	{
		return _get_crypto_iv_size (_used_crypto);
	}

	HEDLEY_WARN_UNUSED_RESULT size_t ReDataConfig::get_crypto_mac_size (void) const
	{
		return _get_crypto_mac_size (_used_crypto);
	}

	HEDLEY_WARN_UNUSED_RESULT bool ReDataConfig::has_crypto (void) const
	{
		return _get_crypto_key_size (_used_crypto) > 0;
	}

	HEDLEY_WARN_UNUSED_RESULT bool ReDataConfig::has_mac (void) const
	{
		return _get_crypto_mac_size (_used_crypto) > 0;
	}

	HEDLEY_WARN_UNUSED_RESULT bool ReDataConfig::has_crypto_key_size_at_least_bits (const size_t limit) const
	{
		return (_get_crypto_key_size (_used_crypto) * 8) >= limit;
	}

	HEDLEY_WARN_UNUSED_RESULT bool ReDataConfig::has_crypto_mac_size_at_least_bits (const size_t limit) const
	{
		return (_get_crypto_mac_size (_used_crypto) * 8) >= limit;
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

	void ReDataConfig::set_iv_counter (const uint32_t counter)
	{
		_counter_iv = counter;
		_counter_iv_enabled = true;
	}

	void ReDataConfig::unset_iv_counter (void)
	{
		_counter_iv_enabled = false;
	}
}

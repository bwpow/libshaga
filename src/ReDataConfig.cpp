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

	template <typename T>
	static T opts_get (const uint8_t opts, const uint8_t shift, const uint8_t bits)
	{
		return static_cast<T>((opts >> shift) & ((1 << bits) - 1));
	}

	template <typename T>
	static void opts_set (uint8_t &opts, const int shift, const T val)
	{
		opts |= static_cast<uint8_t>(val) << shift;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ReDataConfig::ReDataConfig ()
	{
		reset ();
	}

	ReDataConfig::ReDataConfig (const ReDataConfig &conf)
	{
		_used_crypto = conf._used_crypto;
		_used_digest = conf._used_digest;

		_cache_digest = conf._cache_digest;

		_user_iv = conf._user_iv;
		_user_iv_enabled = conf._user_iv_enabled;
	}

	ReDataConfig::ReDataConfig (ReDataConfig &&conf)
	{
		_used_crypto = std::move (conf._used_crypto);
		_used_digest = std::move (conf._used_digest);

		_cache_digest = std::move (conf._cache_digest);

		_user_iv = std::move (conf._user_iv);
		_user_iv_enabled = std::move (conf._user_iv_enabled);
	}

	ReDataConfig& ReDataConfig::operator= (const ReDataConfig &conf)
	{
		_used_crypto = conf._used_crypto;
		_used_digest = conf._used_digest;

		_cache_digest = conf._cache_digest;

		_user_iv = conf._user_iv;
		_user_iv_enabled = conf._user_iv_enabled;

		return *this;
	}

	ReDataConfig& ReDataConfig::operator= (ReDataConfig &&conf)
	{
		_used_crypto = std::move (conf._used_crypto);
		_used_digest = std::move (conf._used_digest);

		_cache_digest = std::move (conf._cache_digest);

		_user_iv = std::move (conf._user_iv);
		_user_iv_enabled = std::move (conf._user_iv_enabled);

		return *this;
	}

	void ReDataConfig::reset (void)
	{
		_used_crypto = CRYPTO::NONE;
		_used_digest = DIGEST::CRC32;
#ifdef SHAGA_FULL
		_cache_digest.digest = _used_digest;
#endif // SHAGA_FULL

		_user_iv.clear ();
		_user_iv_enabled = false;
	}

	void ReDataConfig::decode (const std::string_view msg, size_t &offset)
	{
		size_t mv = offset;
		try {
			const uint8_t opts = msg.at (mv); ++mv;

			if ((opts & 0x80) != 0x80) {
				cThrow ("Malformed data");
			}

			set_digest (opts_get<DIGEST> (opts, 0, 4));
			set_crypto (opts_get<CRYPTO> (opts, 5, 2));
		}
		catch (const std::exception &e) {
			cThrow ("Unable to decode message: {}", e.what());
		}
		catch (...) {
			throw;
		}
		offset = mv;
	}

	void ReDataConfig::decode (const std::string_view msg)
	{
		size_t offset = 0;
		decode (msg, offset);
	}

	void ReDataConfig::encode (std::string &msg) const
	{
		uint8_t opts = 0x80;

		opts_set (opts, 0, _used_digest);
		opts_set (opts, 5, _used_crypto);

		msg.push_back (opts);
	}

	std::string ReDataConfig::encode (void) const
	{
		std::string out;
		encode (out);
		return out;
	}

	ReDataConfig& ReDataConfig::set_digest (const ReDataConfig::DIGEST v)
	{
		if (v < DIGEST::_MAX) {
			_used_digest = v;
		}
		else {
			cThrow ("Unknown digest value");
		}

		return *this;
	}

	ReDataConfig& ReDataConfig::set_digest (const std::string_view str)
	{
		const auto res = DIGEST_MAP.find (std::string (str));
		if (res == DIGEST_MAP.end ()) {
			std::string vals;
			for (const auto &v : DIGEST_MAP) {
				if (vals.empty () == false) {
					vals.append (" ");
				}
				vals.append (v.first);
			}
			cThrow ("Unknown digest. Possible values: {}", vals);
		}

		_used_digest = res->second;

		return *this;
	}

	ReDataConfig& ReDataConfig::set_crypto (const ReDataConfig::CRYPTO v)
	{
		if (v < CRYPTO::_MAX) {
			_used_crypto = v;
		}
		else {
			cThrow ("Unknown crypto value");
		}

		return *this;
	}

	ReDataConfig& ReDataConfig::set_crypto (const std::string_view str)
	{
		const auto res = CRYPTO_MAP.find (std::string (str));
		if (res == CRYPTO_MAP.end ()) {
			std::string vals;
			for (const auto &v : CRYPTO_MAP) {
				if (vals.empty () == false) {
					vals.append (" ");
				}
				vals.append (v.first);
			}
			cThrow ("Unknown crypto. Possible values: {}", vals);
		}

		_used_crypto = res->second;

		return *this;
	}

	ReDataConfig::DIGEST ReDataConfig::get_digest (void) const
	{
		return _used_digest;
	}

	ReDataConfig::CRYPTO ReDataConfig::get_crypto (void) const
	{
		return _used_crypto;
	}

	SHAGA_STRV std::string_view ReDataConfig::get_digest_text (void) const
	{
		switch (_used_digest) {
			case DIGEST::CRC8: return "CRC-8"sv;
			case DIGEST::CRC32: return "CRC-32"sv;
			case DIGEST::CRC64: return "CRC-64"sv;

			case DIGEST::SHA1: return "SHA-1"sv;
			case DIGEST::SHA256: return "SHA-256"sv;
			case DIGEST::SHA512: return "SHA-512"sv;

			case DIGEST::HMAC_RIPEMD160: return "HMAC-RIPEMD-160"sv;
			case DIGEST::HMAC_SHA1: return "HMAC-SHA-1"sv;
			case DIGEST::HMAC_SHA256: return "HMAC-SHA-256"sv;
			case DIGEST::HMAC_SHA512: return "HMAC-SHA-512"sv;

			case DIGEST::SIPHASH24_64: return "SipHash-2-4-64"sv;
			case DIGEST::SIPHASH24_128: return "SipHash-2-4-128"sv;
			case DIGEST::SIPHASH48_64: return "SipHash-4-8-64"sv;
			case DIGEST::SIPHASH48_128: return "SipHash-4-8-128"sv;

			case DIGEST::_MAX: break;
		}
		cThrow ("Unknown digest value"sv);
	}

	SHAGA_STRV std::string_view ReDataConfig::get_crypto_text (void) const
	{
		switch (_used_crypto) {
			case CRYPTO::NONE: return "Plaintext"sv;
			case CRYPTO::AES_128_CBC: return "AES-128-CBC"sv;
			case CRYPTO::AES_256_CBC: return "AES-256-CBC"sv;

			case CRYPTO::_MAX: break;
		}
		cThrow ("Unknown crypto value"sv);
	}


	std::string ReDataConfig::describe (void) const
	{
		std::string out;
		out.append (get_digest_text ());
		out.append (", "sv);
		out.append (get_crypto_text ());
		return out;
	}
}

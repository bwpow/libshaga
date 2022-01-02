/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2022, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	#include "shaga/internal_shake256.h"

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReData::decode_message (std::string_view msg, const size_t offset, std::string &plain, const size_t key_id, const bool key_mixed) const
	{
		msg.remove_prefix (offset);

		if (_conf.get_crypto () != ReDataConfig::CRYPTO::NONE) {
			/* No need to clear _work_msg, because calc_crypto_dec will clear it for us */
			_conf.calc_crypto_dec (msg, _work_msg, (true == key_mixed) ? _crypto_keys_mixed.at (key_id) : _crypto_keys.at (key_id));
			msg = _work_msg;
		}

		if (_conf.get_digest () != ReDataConfig::DIGEST::NONE) {
			const size_t digest_size = _conf.get_digest_result_size ();
			if (msg.size () < digest_size) {
				cThrow ("Message is too short, no digest present"sv);
			}

			/* Calculate digest from plain data */
			_conf.calc_digest (msg.substr (digest_size), _work_msg2, (true == key_mixed) ? _hmac_keys_mixed.at (key_id) : _hmac_keys.at (key_id));

			/* Compare digest from message with calculated digest */
			if (msg.compare (0, digest_size, _work_msg2) != 0) {
				cThrow ("Digest check failed"sv);
			}
			_work_msg2.resize (0);

			/* Skip digest and copy the rest of data to plain output */
			msg.remove_prefix (digest_size);
		}

		plain.assign (msg);

		if (_conf.get_crypto () != ReDataConfig::CRYPTO::NONE) {
			_work_msg.resize (0);
		}
	}

	void ReData::encode_message (const std::string_view plain, std::string &msg, const size_t key_id, const bool key_mixed) const
	{
		/* This function is appending data to msg. */
		/* Data may be appended to msg and not cleaned up on fail. This has to be handled from caller! */
		_conf.calc_digest (plain, _work_msg, (true == key_mixed) ? _hmac_keys_mixed.at (key_id) : _hmac_keys.at (key_id));
		_work_msg.append (plain);

		if (_conf.get_crypto () != ReDataConfig::CRYPTO::NONE) {
			/* Encrypt everything in _work_msg and append to msg */
			_conf.calc_crypto_enc (_work_msg, msg, (true == key_mixed) ? _crypto_keys_mixed.at (key_id) : _crypto_keys.at (key_id));
		}
		else {
			msg.append (_work_msg);
		}
		_work_msg.resize (0);
	}

	void ReData::mix_keys (const COMMON_VECTOR &keys, COMMON_VECTOR &out) const
	{
		out.clear ();

		if (false == _mix_key_enabled) {
			/* No mix key, nothing to do */
			return;
		}

		out.reserve (keys.size ());
		_shake256_ctx_t ctx;

		for (const std::string &str : keys) {
			_shake256_init (&ctx);
			_shake256_update (&ctx, str);
			_shake256_update (&ctx, _mix_key);
			_shake256_xof (&ctx);

			std::string temp;
			_shake256_out (&ctx, temp, str.size ());
			out.push_back (std::move (temp));
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ReData::ReData ()
	{
		reset (true);
	}

	ReData::ReData (const ReDataConfig &conf) : ReData ()
	{
		_conf = conf;
	}

	ReData::ReData (ReDataConfig &&conf) : ReData ()
	{
		_conf = std::move (conf);
	}

	void ReData::reset (const bool also_reset_keys)
	{
		_conf.reset ();
		_use_config_header = true;
		_use_key_ident = false;
		_last_decode_was_mixed = false;

		if (true == also_reset_keys) {
			_key_id = 0;
			_last_key_ident = UINT_FAST16_MAX;
			_key_ident_map.clear ();

			_hmac_keys.clear ();
			_hmac_keys.push_back (""s);

			_crypto_keys.clear ();
			_crypto_keys.push_back (""s);

			_key_idents.clear ();
			_key_ident_map.clear ();

			_mix_key_enabled = false;
			_mix_key.clear ();
			_hmac_keys_mixed.clear ();
			_crypto_keys_mixed.clear ();
		}
	}

	void ReData::decode (const std::string_view msg, size_t &offset, std::string &out, std::function<bool(const ReDataConfig &)> check_callback, const MixKeysUse use)
	{
		const size_t orig_offset = offset;

		try {
			if (true == _use_config_header) {
				_conf.decode (msg, offset);
			}

			if (0 == _conf.get_digest_result_size () && false == _conf.has_mac ()) {
				cThrow ("Unable to decode message with no digest and no MAC"sv);
			}

			if (nullptr != check_callback) {
				if (check_callback (_conf) == false) {
					cThrow ("Checker callback function refused message parameters"sv);
				}
			}

			const uint_fast8_t ident = (true == _use_key_ident) ? BIN::to_uint8 (msg, offset) : 0;

			auto _decode = [&](const bool use_mixed) -> void {
				if (true == _use_key_ident) {
					/* Check, if we already have cached key_id */
					if (ident != _last_key_ident) {
						/* We don't, so we need to find it in map */
						auto res = _key_ident_map.find (ident);
						if (res == _key_ident_map.end ()) {
							cThrow ("Key ident not found"sv);
						}

						decode_message (msg, offset, out, res->second, use_mixed);
						/* If it didn't throw exception, decode is successfull, so cache key_id and ident */
						_last_decode_was_mixed = use_mixed;
						_key_id = res->second;
						_last_key_ident = ident;
					}
					else {
						/* We have cached key_id, so just use it */
						decode_message (msg, offset, out, _key_id, use_mixed);
						/* If it didn't throw exception, decode is successfull */
						_last_decode_was_mixed = use_mixed;
					}
				}
				else {
					/* We don't know the key_id, so check all stored keys, starting with the last successfull key_id */
					const size_t len = std::max (_hmac_keys.size (), _crypto_keys.size ());
					for (size_t i = 0; i < len; ++i) {
						try {
							cMute;
							decode_message (msg, offset, out, (i + _key_id) % len, use_mixed);
							/* If it didn't throw exception, decode is successfull */
							_last_decode_was_mixed = use_mixed;
							_key_id = (i + _key_id) % len;
							break;
						}
						catch (...) {
							if ((i + 1) >= len) {
								/* This is the last possible key, so rethrow away */
								throw;
							}
						}
					}
				}
			};

			if (false == _mix_key_enabled) {
				switch (use) {
					case MixKeysUse::ONLY_NORMAL:
					case MixKeysUse::BOTH_NORMAL_FIRST:
					case MixKeysUse::BOTH_MIXED_FIRST:
						_decode (false);
						break;

					case MixKeysUse::ONLY_MIXED:
						cThrow ("Mix key is not set"sv);
				}
			}
			else {
				switch (use) {
					case MixKeysUse::ONLY_NORMAL:
						_decode (false);
						break;

					case MixKeysUse::ONLY_MIXED:
						_decode (true);
						break;

					case MixKeysUse::BOTH_NORMAL_FIRST:
						try {
							cMute;
							_decode (false);
						}
						catch (...) {
							_decode (true);
						}
						break;

					case MixKeysUse::BOTH_MIXED_FIRST:
						try {
							cMute;
							_decode (true);
						}
						catch (...) {
							_decode (false);
						}
						break;
				}
			}

			/* Message was decoded, move offset to the end of msg */
			offset = msg.size ();
		}
		catch (const std::exception &e) {
			/* Fail, truncate out and return offset to original value */
			out.resize (0);
			offset = orig_offset;
			cThrow ("Unable to decode message: {}"sv, e.what());
		}
		catch (...) {
			/* Fail, truncate out and return offset to original value */
			out.resize (0);
			offset = orig_offset;
			throw;
		}
	}

	void ReData::decode (const std::string_view msg, std::string &out, std::function<bool(const ReDataConfig &)> check_callback, const MixKeysUse use)
	{
		size_t offset = 0;
		decode (msg, offset, out, check_callback, use);
	}

	void ReData::encode (const std::string_view plain, std::string &out, const bool use_mixed)
	{
		if (0 == _conf.get_digest_result_size () && false == _conf.has_mac ()) {
			cThrow ("Unable to encode message with no digest and no MAC"sv);
		}

		const size_t original_size = out.size ();
		try {
			if (true == use_mixed && false == _mix_key_enabled) {
				cThrow ("Mix key is not set"sv);
			}
			if (true == _use_config_header) {
				_conf.encode (out);
			}
			if (true == _use_key_ident) {
				BIN::from_uint8 (_key_idents.at (_key_id), out);
			}
			encode_message (plain, out, _key_id, use_mixed);
		}
		catch (const std::exception &e) {
			out.resize (original_size);
			cThrow ("Unable to encode message: {}"sv, e.what());
		}
		catch (...) {
			out.resize (original_size);
			throw;
		}
	}

	std::string ReData::encode (const std::string_view plain, const bool use_mixed)
	{
		std::string out;
		encode (plain, out, use_mixed);
		return out;
	}

	ReDataConfig ReData::get_config (void) const
	{
		return _conf;
	}

	void ReData::set_config (const ReDataConfig &conf)
	{
		_conf = conf;
	}

	void ReData::set_config (ReDataConfig &&conf)
	{
		_conf = std::move (conf);
	}

	ReDataConfig& ReData::set_config (void)
	{
		return _conf;
	}

	void ReData::set_hmac_key (const std::string_view key)
	{
		_hmac_keys.clear ();
		_hmac_keys.emplace_back (key);
		_key_id = 0;
		_last_key_ident = UINT_FAST16_MAX;
		mix_keys (_hmac_keys, _hmac_keys_mixed);
	}

	void ReData::set_crypto_key (const std::string_view key)
	{
		_crypto_keys.clear ();
		_crypto_keys.emplace_back (key);
		_key_id = 0;
		_last_key_ident = UINT_FAST16_MAX;
		mix_keys (_crypto_keys, _crypto_keys_mixed);
	}

	void ReData::set_hmac_keys (const COMMON_VECTOR &keys, const size_t key_id)
	{
		_hmac_keys = keys;
		if (_hmac_keys.empty () == true) {
			_hmac_keys.emplace_back ();
		}

		if (SIZE_MAX != key_id) {
			_key_id = key_id;
			_last_key_ident = UINT_FAST16_MAX;
		}

		mix_keys (_hmac_keys, _hmac_keys_mixed);
	}

	void ReData::set_crypto_keys (const COMMON_VECTOR &keys, const size_t key_id)
	{
		_crypto_keys = keys;
		if (_crypto_keys.empty () == true) {
			_crypto_keys.push_back (""s);
		}

		if (SIZE_MAX != key_id) {
			_key_id = key_id;
			_last_key_ident = UINT_FAST16_MAX;
		}

		mix_keys (_crypto_keys, _crypto_keys_mixed);
	}

	void ReData::set_key_idents (const KEY_IDENT_VECTOR &idents, const size_t key_id)
	{
		_key_idents = idents;
		_last_key_ident = UINT_FAST16_MAX;
		_key_ident_map.clear ();

		size_t i = 0;
		for (const uint_fast8_t id : idents) {
			const auto [iter, success] = _key_ident_map.insert (std::make_pair (id, i));
			(void) iter;

			if (false == success) {
				cThrow ("Duplicated key ident"sv);
			}
			++i;
		}

		if (SIZE_MAX != key_id) {
			_key_id = key_id;
			_last_key_ident = UINT_FAST16_MAX;
		}
	}

	void ReData::set_mix_key (const std::string_view mix)
	{
		_mix_key_enabled = true;
		_mix_key.assign (mix);
		mix_keys (_hmac_keys, _hmac_keys_mixed);
		mix_keys (_crypto_keys, _crypto_keys_mixed);
	}

	void ReData::clear_mix_key (void)
	{
		_mix_key_enabled = false;
		_mix_key.resize (0);
		_hmac_keys_mixed.clear ();
		_crypto_keys_mixed.clear ();
	}

	bool ReData::get_last_decode_was_mixed (void) const
	{
		return _last_decode_was_mixed;
	}

	COMMON_VECTOR ReData::get_mixed_hmac_keys (void) const
	{
		return _hmac_keys_mixed;
	}

	COMMON_VECTOR ReData::get_mixed_crypto_keys (void) const
	{
		return _crypto_keys_mixed;
	}

	size_t ReData::get_key_id (void) const
	{
		return _key_id;
	}

	void ReData::set_key_id (const size_t id)
	{
		if (id >= std::max (_hmac_keys.size (), _crypto_keys.size ())) {
			_key_id = std::max (_hmac_keys.size (), _crypto_keys.size ()) - 1;
		}
		else {
			_key_id = id;
		}
		_last_key_ident = UINT_FAST16_MAX;
	}

	bool ReData::config_header_enabled (void) const
	{
		return _use_config_header;
	}

	void ReData::use_config_header (const bool enabled)
	{
		_use_config_header = enabled;
	}

	bool ReData::key_ident_enabled (void) const
	{
		return _use_key_ident;
	}

	void ReData::use_key_ident (const bool enabled)
	{
		_use_key_ident = enabled;
	}
}

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReData::decode_message (const std::string &msg, const size_t offset, std::string &plain, const size_t key_id)
	{
		if (_conf.get_crypto () != ReDataConfig::CRYPTO::NONE) {
			/* No need to clear _work_msg, because calc_crypto_dec will clear it for us */
			_conf.calc_crypto_dec (msg, offset, _work_msg, _crypto_keys.at (key_id));
		}
		else {
			/* No encryption, just copy the data */
			_work_msg.assign (msg.substr (offset));
		}

		const size_t digest_size = _conf.get_digest_result_size ();
		if (_work_msg.size () < digest_size) {
			cThrow ("Message is too short, no digest present");
		}

		/* Skip digest and copy the rest of data to plain output */
		plain.assign (_work_msg.substr (digest_size));

		/* Calculate digest from plain data */
		const std::string tdigest = _conf.calc_digest (plain, _hmac_keys.at (key_id));

		/* Compare digest from message with calculated digest */
		if (_work_msg.compare (0, digest_size, tdigest) != 0) {
			cThrow ("Digest check failed");
		}
	}

	void ReData::encode_message (const std::string &plain, std::string &msg, const size_t key_id)
	{
		/* This function is appending data to msg */
		if (_conf.get_crypto () != ReDataConfig::CRYPTO::NONE) {
			/* Compute digest and append plain data after digest to _work_msg */
			_work_msg.assign (_conf.calc_digest (plain, _hmac_keys.at (key_id)) + plain);
			/* Encrypt everything in _work_msg and append to msg */
			_conf.calc_crypto_enc (_work_msg, msg, _crypto_keys.at (key_id));
		}
		else {
			/* Compute digest and append plain data after digest and append everything to msg */
			msg.append (_conf.calc_digest (plain, _hmac_keys.at (key_id)) + plain);
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

		if (true == also_reset_keys) {
			_key_id = 0;
			_last_key_ident = UINT_FAST16_MAX;
			_key_ident_map.clear ();

			_hmac_keys.clear ();
			_hmac_keys.push_back ("");

			_crypto_keys.clear ();
			_crypto_keys.push_back ("");

			_key_idents.clear ();
			_key_ident_map.clear ();
		}
	}

	void ReData::decode (const std::string &msg, size_t &offset, std::string &out, std::function<bool(const ReDataConfig &)> check_callback)
	{
		try {
			if (true == _use_config_header) {
				_conf.decode (msg, offset);
			}

			if (nullptr != check_callback) {
				if (check_callback (_conf) == false) {
					cThrow ("Checker callback function refused message parameters");
				}
			}

			if (true == _use_key_ident) {
				const uint_fast8_t ident = BIN::to_uint8 (msg, offset);

				/* Check, if we already have cached key_id */
				if (ident != _last_key_ident) {
					/* We don't, so we need to find it in map */
					auto res = _key_ident_map.find (ident);
					if (res == _key_ident_map.end ()) {
						cThrow ("Key ident not found");
					}

					decode_message (msg, offset, out, res->second);

					/* If it didn't throw exception, decode is successfull, so cache key_id and ident */
					_key_id = res->second;
					_last_key_ident = ident;
				}
				else {
					/* We have cached key_id, so just use it */
					decode_message (msg, offset, out, _key_id);
				}
			}
			else {
				/* We don't know the key_id, so check all stored keys, starting with the last successfull key_id */
				const size_t len = std::max (_hmac_keys.size (), _crypto_keys.size ());
				for (size_t i = 0; i < len; ++i) {
					try {
						decode_message (msg, offset, out, (i + _key_id) % len);
						/* If it didn't throw exception, decode is successfull */
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
		}
		catch (const std::exception &e) {
			cThrow ("Unable to decode message: %s", e.what());
		}
	}

	void ReData::decode (const std::string &msg, std::string &out, std::function<bool(const ReDataConfig &)> check_callback)
	{
		size_t offset = 0;
		decode (msg, offset, out, check_callback);
	}

	void ReData::encode (const std::string &plain, std::string &out)
	{
		try {
			if (true == _use_config_header) {
				_conf.encode (out);
			}
			if (true == _use_key_ident) {
				BIN::from_uint8 (_key_idents.at (_key_id), out);
			}
			encode_message (plain, out, _key_id);
		}
		catch (const std::exception &e) {
			cThrow ("Unable to encode message: %s", e.what());
		}
	}

	std::string ReData::encode (const std::string &plain)
	{
		std::string out;
		encode (plain, out);
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

	void ReData::set_hmac_key (const std::string &key)
	{
		_hmac_keys.clear ();
		_hmac_keys.push_back (key);
		_key_id = 0;
		_last_key_ident = UINT_FAST16_MAX;
	}

	void ReData::set_crypto_key (const std::string &key)
	{
		_crypto_keys.clear ();
		_crypto_keys.push_back (key);
		_key_id = 0;
		_last_key_ident = UINT_FAST16_MAX;
	}

	void ReData::set_hmac_keys (const COMMON_VECTOR &keys, const size_t key_id)
	{
		_hmac_keys = keys;
		if (_hmac_keys.empty () == true) {
			_hmac_keys.push_back ("");
		}

		if (SIZE_MAX != key_id) {
			_key_id = key_id;
			_last_key_ident = UINT_FAST16_MAX;
		}
	}

	void ReData::set_crypto_keys (const COMMON_VECTOR &keys, const size_t key_id)
	{
		_crypto_keys = keys;
		if (_crypto_keys.empty () == true) {
			_crypto_keys.push_back ("");
		}

		if (SIZE_MAX != key_id) {
			_key_id = key_id;
			_last_key_ident = UINT_FAST16_MAX;
		}
	}

	void ReData::set_key_idents (const KEY_IDENT_VECTOR &idents, const size_t key_id)
	{
		_key_idents = idents;
		_last_key_ident = UINT_FAST16_MAX;
		_key_ident_map.clear ();

		size_t i = 0;
		for (const uint_fast8_t id : idents) {
			auto res = _key_ident_map.insert (std::make_pair (id, i));
			if (false == res.second) {
				cThrow ("Duplicated key ident");
			}
			++i;
		}

		if (SIZE_MAX != key_id) {
			_key_id = key_id;
			_last_key_ident = UINT_FAST16_MAX;
		}
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

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

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

	/** \brief Decodes the messagge by applying set coding, crypto and digest. Final decoded message is stored in private class variable _plain.
	 *
	 * \param msg Encoded message without header to be decoded
	 * \param offset Offset from the start of the message
	 */
	void ReData::decode_message (const std::string &msg, size_t &offset, const size_t key_id)
	{
		try {
			std::string wmsg;

			switch (_conf.get_coding ()) {
				case ReDataConfig::CODING::BINARY:
					wmsg = msg.substr (offset);
					break;

				case ReDataConfig::CODING::BASE64:
					BIN::from_base64 (msg.substr (offset), wmsg, false);
					break;

				case ReDataConfig::CODING::BASE64_ALT:
					BIN::from_base64 (msg.substr (offset), wmsg, true);
					break;

				case ReDataConfig::CODING::HEX:
					BIN::from_hex (msg.substr (offset), wmsg);
					break;

				default:
					cThrow ("Unsupported coding");
			}

			if (_conf.get_crypto () != ReDataConfig::CRYPTO::NONE) {
				wmsg = _conf.calc_crypto_dec (wmsg, _crypto_keys.at (key_id));
			}

			const size_t digest_size = _conf.get_digest_result_size ();
			if (wmsg.size () < digest_size) {
				cThrow ("Message is too short");
			}

			_plain.assign (wmsg.substr (digest_size));

			const std::string digest = wmsg.substr (0, digest_size);
			const std::string tdigest = _conf.calc_digest (_plain, _hmac_keys.at (key_id));
			if (tdigest.compare (digest) != 0) {
				cThrow ("Digest check failed");
			}
		}
		catch (const std::exception &e) {
			cThrow ("Unable to decode message: %s", e.what());
		}
		catch (...) {
			throw;
		}

		offset = msg.size ();
	}

	/** \brief Encodes the messgae by applying set digest, crypto and coding. Plain message to be encoded is taken from private class variable _plain.
	 *
	 * \param msg Appended with encoded message.
	 */
	void ReData::encode_message (std::string &msg, const size_t key_id)
	{
		try {
			std::string wmsg;

			wmsg = _conf.calc_digest (_plain, _hmac_keys.at (key_id)) + _plain;

			if (_conf.get_crypto () != ReDataConfig::CRYPTO::NONE) {
				wmsg = _conf.calc_crypto_enc (wmsg, _crypto_keys.at (key_id));
			}

			switch (_conf.get_coding ()) {
				case ReDataConfig::CODING::BINARY:
					msg.append (wmsg);
					break;

				case ReDataConfig::CODING::BASE64:
					BIN::to_base64 (wmsg, msg, false);
					break;

				case ReDataConfig::CODING::BASE64_ALT:
					BIN::to_base64 (wmsg, msg, true);
					break;

				case ReDataConfig::CODING::HEX:
					BIN::to_hex (wmsg, msg);
					break;

				default:
					cThrow ("Unsupported coding");
			}
		}
		catch (const std::exception &e) {
			cThrow ("Unable to encode message: %s", e.what());
		}
		catch (...) {
			throw;
		}
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ReData::ReData ()
	{
		reset (true);
	}

	ReData::ReData (const ReDataConfig &conf)
	{
		reset (true);
		_conf = conf;
	}

	ReData::ReData (ReDataConfig &&conf)
	{
		reset (true);
		_conf = std::move (conf);
	}

	void ReData::reset (const bool reset_keys)
	{
		_conf.reset ();
		_plain.resize (0);
		_use_config_header = true;

		if (reset_keys) {
			_key_id = 0;

			_hmac_keys.clear ();
			_hmac_keys.push_back ("");

			_crypto_keys.clear ();
			_crypto_keys.push_back ("");
		}
	}

	void ReData::decode (const std::string &msg, size_t &offset, std::function<bool(const ReDataConfig &)> check_callback)
	{
		if (_use_config_header == true) {
			_conf.decode (msg, offset);
		}
		if (check_callback != nullptr) {
			if (check_callback (_conf) == false) {
				cThrow ("Checker callback function refused message parameters");
			}
		}
		const size_t len = std::max (_hmac_keys.size (), _crypto_keys.size ());
		for (size_t i = 0; i < len; ++i) {
			try {
				decode_message (msg, offset, (i + _key_id) % len);
				/* If it didn't throw exception, decode is successfull */
				_key_id = (i + _key_id) % len;
				break;
			}
			catch (...) {
				if ((i + 1) >= len) {
					throw;
				}
			}
		}
	}

	void ReData::decode (const std::string &msg, std::function<bool(const ReDataConfig &)> check_callback)
	{
		size_t offset = 0;
		decode (msg, offset, check_callback);
	}

	void ReData::encode (std::string &output)
	{
		if (_use_config_header == true) {
			_conf.encode (output);
		}
		encode_message (output, _key_id);
	}

	std::string ReData::encode (void)
	{
		std::string out;
		encode (out);
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
	}

	void ReData::set_crypto_key (const std::string &key)
	{
		_crypto_keys.clear ();
		_crypto_keys.push_back (key);
	}

	void ReData::set_hmac_keys (const COMMON_VECTOR &keys)
	{
		_hmac_keys = keys;
		if (_hmac_keys.empty () == true) {
			_hmac_keys.push_back ("");
		}
	}

	void ReData::set_crypto_key (const COMMON_VECTOR &keys)
	{
		_crypto_keys = keys;
		if (_crypto_keys.empty () == true) {
			_crypto_keys.push_back ("");
		}
	}

	size_t ReData::get_key_id (void) const
	{
		return _key_id;
	}

	void ReData::set_key_id (const size_t id)
	{
		if (id >= std::max (_hmac_keys.size (), _crypto_keys.size ())) {
			cThrow ("Key id out of range");
		}
		_key_id = id;
	}

	std::string& ReData::plaintext (void)
	{
		return _plain;
	}

	bool ReData::config_header_enabled (void) const
	{
		return _use_config_header;
	}

	void ReData::use_config_header (const bool enabled)
	{
		_use_config_header = enabled;
	}

}

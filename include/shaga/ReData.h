/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_ReData
#define HEAD_shaga_ReData

#include "common.h"

#ifdef SHAGA_FULL
	#include <mbedtls/aes.h>
#endif // SHAGA_FULL

namespace shaga {

	class ReDataConfig {
		public:
			enum class DIGEST {
				CRC32,
				SHA1,
				SHA256,
				SHA512,
				HMAC_RIPEMD160,
				HMAC_SHA1,
				HMAC_SHA256,
				HMAC_SHA512,
				_MAX
			};

			const std::map <std::string, DIGEST> DIGEST_MAP = {
				{"crc32", DIGEST::CRC32},
				{"sha1", DIGEST::SHA1},
				{"sha256", DIGEST::SHA256},
				{"sha512", DIGEST::SHA512},
				{"hmac-ripemd160", DIGEST::HMAC_RIPEMD160},
				{"hmac-sha1", DIGEST::HMAC_SHA1},
				{"hmac-sha256", DIGEST::HMAC_SHA256},
				{"hmac-sha512", DIGEST::HMAC_SHA512}
			};

			enum class CODING {
				BINARY,
				BASE64,
				BASE64_ALT,
				HEX,
				_MAX
			};

			const std::map <std::string, CODING> CODING_MAP = {
				{"binary", CODING::BINARY},
				{"base64", CODING::BASE64},
				{"base64alt", CODING::BASE64_ALT},
				{"hex", CODING::HEX}
			};

			enum class CRYPTO {
				NONE,
				AES128,
				AES256,
				_MAX
			};

			const std::map <std::string, CRYPTO> CRYPTO_MAP = {
				{"none", CRYPTO::NONE},
				{"aes128", CRYPTO::AES128},
				{"aes256", CRYPTO::AES256}
			};

#ifdef SHAGA_FULL
			struct CryptoCache {
				std::random_device _rand_rd;
				std::default_random_engine _rand_rng;
				std::uniform_int_distribution<> _rand_dist;
				mbedtls_aes_context _aes_ctx;
				std::once_flag _aes_init_flag;
				unsigned char temp_iv[16];

				CryptoCache () : _rand_rng(_rand_rd ()), _rand_dist (0x00, 0xFF) {}
				CryptoCache (const CryptoCache &) = delete;
				CryptoCache (CryptoCache &&) = delete;
			};

			struct DigestCache {
				unsigned char output[64];
				DIGEST digest;
				std::string key;
				std::string ipad;
				std::string opad;

				DigestCache ();
				DigestCache (const DigestCache &other);
				DigestCache (DigestCache &&other);
				DigestCache& operator= (const DigestCache &other);
				DigestCache& operator= (DigestCache &&other);
			};
#else
			struct CryptoCache { };
			struct DigestCache { };
#endif // SHAGA_FULL

		private:
			DIGEST _used_digest;
			CRYPTO _used_crypto;
			CODING _used_coding;

			CryptoCache _cache_crypto;
			DigestCache _cache_digest;

		public:
			ReDataConfig ();

			ReDataConfig (const ReDataConfig &conf);
			ReDataConfig (ReDataConfig &&conf);
			ReDataConfig& operator= (const ReDataConfig &conf);
			ReDataConfig& operator= (ReDataConfig &&conf);

			void reset (void);

			void decode (const std::string &msg, size_t &offset);
			void decode (const std::string &msg);

			void encode (std::string &msg) const;
			std::string encode (void) const;

			ReDataConfig& set_digest (const ReDataConfig::DIGEST v);
			ReDataConfig& set_digest (const std::string &str);

			ReDataConfig& set_crypto (const ReDataConfig::CRYPTO v);
			ReDataConfig& set_crypto (const std::string &str);

			ReDataConfig& set_coding (const ReDataConfig::CODING v);
			ReDataConfig& set_coding (const std::string &str);

			ReDataConfig::DIGEST get_digest (void) const;
			ReDataConfig::CRYPTO get_crypto (void) const;
			ReDataConfig::CODING get_coding (void) const;

			std::string get_digest_text (void) const;
			std::string get_crypto_text (void) const;
			std::string get_coding_text (void) const;

			std::string calc_digest (const std::string &plain, const std::string &key);
			size_t get_digest_result_size (void) const;
			size_t get_digest_hmac_block_size (void) const;
			bool has_hmac (void) const;
			bool has_digest_size_at_least_bits (const size_t limit) const;

			size_t calc_crypto_enc (const std::string &plain, std::string &out, const std::string &key);
			std::string calc_crypto_enc (const std::string &plain, const std::string &key);
			size_t calc_crypto_dec (const std::string &msg, std::string &out, const std::string &key);
			std::string calc_crypto_dec (const std::string &msg, const std::string &key);
			size_t get_crypto_block_size (void) const;
			size_t get_crypto_key_size (void) const;
			bool has_crypto (void) const;
			bool has_crypto_size_at_least_bits (const size_t limit) const;
	};

	class ReData {
		private:
			ReDataConfig _conf;
			std::string _plain;
			COMMON_VECTOR _hmac_keys;
			COMMON_VECTOR _crypto_keys;
			bool _use_config_header;
			size_t _key_id;

			void decode_message (const std::string &msg, size_t &offset, const size_t key_id);
			void encode_message (std::string &msg, const size_t key_id);

		public:
			ReData ();
			ReData (const ReDataConfig &conf);
			ReData (ReDataConfig &&conf);

			void reset (const bool reset_keys);

			void decode (const std::string &msg, size_t &offset, std::function<bool(const ReDataConfig &)> check_callback = nullptr);
			void decode (const std::string &msg, std::function<bool(const ReDataConfig &)> check_callback = nullptr);

			void encode (std::string &output);
			std::string encode (void);

			ReDataConfig get_config (void) const;
			void set_config (const ReDataConfig &conf);
			void set_config (ReDataConfig &&conf);
			ReDataConfig& set_config (void);

			void set_hmac_key (const std::string &key);
			void set_crypto_key (const std::string &key);

			void set_hmac_keys (const COMMON_VECTOR &keys);
			void set_crypto_key (const COMMON_VECTOR &keys);

			size_t get_key_id (void) const;
			void set_key_id (const size_t id);

			std::string& plaintext (void);

			bool config_header_enabled (void) const;
			void use_config_header (const bool enabled);
	};
}

#endif // HEAD_shaga_ReData

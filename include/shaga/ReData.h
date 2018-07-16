/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_ReData
#define HEAD_shaga_ReData

#include "common.h"

#ifdef SHAGA_FULL
	#include <mbedtls/aes.h>
	#include <mbedtls/cipher.h>
#endif // SHAGA_FULL

#ifndef MBEDTLS_MAX_IV_LENGTH
	#define MBEDTLS_MAX_IV_LENGTH 0
#endif // MBEDTLS_MAX_IV_LENGTH

namespace shaga {

	class ReDataConfig {
		public:
			enum class DIGEST_HMAC_TYPE {
				NONE,
				TYPICAL,
				SIPHASH,
				_MAX
			};

			enum class DIGEST {
				CRC8,
				CRC32,
				CRC64,

				SHA1,
				SHA256,
				SHA512,

				HMAC_RIPEMD160,
				HMAC_SHA1,
				HMAC_SHA256,
				HMAC_SHA512,

				SIPHASH24_64,
				SIPHASH24_128,
				SIPHASH48_64,
				SIPHASH48_128,

				_MAX
			};

			const std::map <std::string, DIGEST> DIGEST_MAP = {
				{"crc8", DIGEST::CRC8},
				{"crc-8", DIGEST::CRC8},

				{"crc32", DIGEST::CRC32},
				{"crc-32", DIGEST::CRC32},

				{"crc64", DIGEST::CRC64},
				{"crc-64", DIGEST::CRC64},

				{"sha1", DIGEST::SHA1},
				{"sha-1", DIGEST::SHA1},

				{"sha256", DIGEST::SHA256},
				{"sha-256", DIGEST::SHA256},

				{"sha512", DIGEST::SHA512},
				{"sha-512", DIGEST::SHA512},

				{"hmac-ripemd160", DIGEST::HMAC_RIPEMD160},
				{"hmac-ripemd-160", DIGEST::HMAC_RIPEMD160},

				{"hmac-sha1", DIGEST::HMAC_SHA1},
				{"hmac-sha-1", DIGEST::HMAC_SHA1},

				{"hmac-sha256", DIGEST::HMAC_SHA256},
				{"hmac-sha-256", DIGEST::HMAC_SHA256},

				{"hmac-sha512", DIGEST::HMAC_SHA512},
				{"hmac-sha-512", DIGEST::HMAC_SHA512},

				{"siphash", DIGEST::SIPHASH24_64},
				{"siphash24", DIGEST::SIPHASH24_64},
				{"siphash24-64", DIGEST::SIPHASH24_64},
				{"siphash-2-4", DIGEST::SIPHASH24_64},
				{"siphash-2-4-64", DIGEST::SIPHASH24_64},

				{"siphash24-128", DIGEST::SIPHASH24_128},
				{"siphash-2-4-128", DIGEST::SIPHASH24_128},

				{"siphash48", DIGEST::SIPHASH48_64},
				{"siphash48-64", DIGEST::SIPHASH48_64},
				{"siphash-4-8", DIGEST::SIPHASH48_64},
				{"siphash-4-8-64", DIGEST::SIPHASH48_64},

				{"siphash48-128", DIGEST::SIPHASH48_128},
				{"siphash-4-8-128", DIGEST::SIPHASH48_128},
			};

			enum class CRYPTO {
				NONE,
				AES_128_CBC,
				AES_256_CBC,
				_MAX
			};

			const std::map <std::string, CRYPTO> CRYPTO_MAP = {
				{"none", CRYPTO::NONE},

				{"aes", CRYPTO::AES_128_CBC},
				{"aes128", CRYPTO::AES_128_CBC},
				{"aes-128", CRYPTO::AES_128_CBC},
				{"aes-128-cbc", CRYPTO::AES_128_CBC},

				{"aes256", CRYPTO::AES_256_CBC},
				{"aes-256", CRYPTO::AES_256_CBC},
				{"aes-256-cbc", CRYPTO::AES_256_CBC}
			};

#ifdef SHAGA_FULL
			struct CryptoCache {
				std::random_device _rand_rd;
				std::default_random_engine _rand_rng;
				std::uniform_int_distribution<int> _rand_dist;

				mbedtls_aes_context _aes_ctx;
				#ifdef SHAGA_THREADING
					std::once_flag _aes_init_flag;
				#else
					bool _aes_init_flag {false};
				#endif // SHAGA_THREADING

				CryptoCache () : _rand_rng(_rand_rd ()), _rand_dist (0, UINT8_MAX) {}
				CryptoCache (const CryptoCache &) = delete;
				CryptoCache (CryptoCache &&) = delete;
			};
#else
			struct CryptoCache { };
#endif // SHAGA_FULL

			struct DigestCache {
				unsigned char output[64];
				DIGEST digest;
				std::string key;
				std::string ipad;
				std::string opad;

				uint64_t siphash_k0;
				uint64_t siphash_k1;

				DigestCache ();
				DigestCache (const DigestCache &other);
				DigestCache (DigestCache &&other);
				DigestCache& operator= (const DigestCache &other);
				DigestCache& operator= (DigestCache &&other);
			};

		private:
			DIGEST _used_digest;
			CRYPTO _used_crypto;

			CryptoCache _cache_crypto;
			DigestCache _cache_digest;

			unsigned char _temp_iv[MBEDTLS_MAX_IV_LENGTH];

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

			ReDataConfig::DIGEST get_digest (void) const;
			ReDataConfig::CRYPTO get_crypto (void) const;

			std::string get_digest_text (void) const;
			std::string get_crypto_text (void) const;

			std::string describe (void) const;

			/* ReDataConfigDigest.cpp */
			std::string calc_digest (const std::string &plain, const std::string &key);

			size_t get_digest_result_size (void) const;
			size_t get_digest_hmac_block_size (void) const;

			bool has_hmac (void) const;
			bool has_digest_size_at_least_bits (const size_t limit) const;

			/* ReDataConfigCrypto.cpp */
			void calc_crypto_enc (std::string &plain, std::string &out, const std::string &key);
			void calc_crypto_dec (const std::string &msg, size_t offset, std::string &out, const std::string &key);

			size_t get_crypto_block_size (void) const;
			size_t get_crypto_key_size (void) const;

			bool has_crypto (void) const;
			bool has_crypto_size_at_least_bits (const size_t limit) const;
	};

	class ReData {
		private:
			ReDataConfig _conf;
			COMMON_VECTOR _hmac_keys;
			COMMON_VECTOR _crypto_keys;
			bool _use_config_header;
			size_t _key_id;
			std::string _work_msg;

			void decode_message (const std::string &msg, const size_t offset, std::string &plain, const size_t key_id);
			void encode_message (const std::string &plain, std::string &msg, const size_t key_id);

		public:
			ReData ();
			ReData (const ReDataConfig &conf);
			ReData (ReDataConfig &&conf);

			void reset (const bool also_reset_keys);

			void decode (const std::string &msg, size_t &offset, std::string &out, std::function<bool(const ReDataConfig &)> check_callback = nullptr);
			void decode (const std::string &msg, std::string &out, std::function<bool(const ReDataConfig &)> check_callback = nullptr);

			void encode (const std::string &plain, std::string &out);
			std::string encode (const std::string &plain);

			ReDataConfig get_config (void) const;
			void set_config (const ReDataConfig &conf);
			void set_config (ReDataConfig &&conf);
			ReDataConfig& set_config (void);

			void set_hmac_key (const std::string &key);
			void set_crypto_key (const std::string &key);

			void set_hmac_keys (const COMMON_VECTOR &keys, const size_t key_id = SIZE_MAX);
			void set_crypto_keys (const COMMON_VECTOR &keys, const size_t key_id = SIZE_MAX);

			size_t get_key_id (void) const;
			void set_key_id (const size_t id);

			bool config_header_enabled (void) const;
			void use_config_header (const bool enabled);
	};
}

#endif // HEAD_shaga_ReData

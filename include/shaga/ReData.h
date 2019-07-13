/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_ReData
#define HEAD_shaga_ReData

#include "common.h"

#ifndef MBEDTLS_MAX_IV_LENGTH
	#define MBEDTLS_MAX_IV_LENGTH 1
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
				CRC8, /* CRC-8 Dallas/Maxim */
				CRC32, /* CRC-32-Castagnoli */
				CRC64, /* CRC-64-Jones */

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
				randutils::mt19937_rng _rng;
				mbedtls_aes_context _aes_ctx;

				#ifdef SHAGA_THREADING
				std::once_flag _aes_init_flag;
				#else
				bool _aes_init_flag {false};
				#endif // SHAGA_THREADING

				CryptoCache () {}
				CryptoCache (const CryptoCache &) = delete;
				CryptoCache (CryptoCache &&) = delete;
			};
#else
			struct CryptoCache { };
#endif // SHAGA_FULL

			struct DigestCache {
				unsigned char output[64];
				DIGEST digest {DIGEST::CRC32};
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
			DIGEST _used_digest {DIGEST::CRC32};
			CRYPTO _used_crypto {CRYPTO::NONE};

			CryptoCache _cache_crypto;
			DigestCache _cache_digest;

			unsigned char _temp_iv[MBEDTLS_MAX_IV_LENGTH];

			std::string _user_iv;
			bool _user_iv_enabled{false};

		public:
			ReDataConfig ();

			ReDataConfig (const ReDataConfig &conf);
			ReDataConfig (ReDataConfig &&conf);
			ReDataConfig& operator= (const ReDataConfig &conf);
			ReDataConfig& operator= (ReDataConfig &&conf);

			void reset (void);

			void decode (const std::string_view msg, size_t &offset);
			void decode (const std::string_view msg);

			void encode (std::string &msg) const;
			std::string encode (void) const;

			ReDataConfig& set_digest (const ReDataConfig::DIGEST v);
			ReDataConfig& set_digest (const std::string_view str);

			ReDataConfig& set_crypto (const ReDataConfig::CRYPTO v);
			ReDataConfig& set_crypto (const std::string_view str);

			ReDataConfig::DIGEST get_digest (void) const;
			ReDataConfig::CRYPTO get_crypto (void) const;

			std::string get_digest_text (void) const;
			std::string get_crypto_text (void) const;

			std::string describe (void) const;

			/* ReDataConfigDigest.cpp */
			std::string calc_digest (const std::string_view plain, const std::string_view key);

			size_t get_digest_result_size (void) const;
			size_t get_digest_hmac_block_size (void) const;

			bool has_hmac (void) const;
			bool has_digest_size_at_least_bits (const size_t limit) const;

			/* ReDataConfigCrypto.cpp */
			void calc_crypto_enc (std::string &plain, std::string &out, const std::string_view key);
			void calc_crypto_dec (const std::string_view msg, size_t offset, std::string &out, const std::string_view key);

			size_t get_crypto_block_size (void) const;
			size_t get_crypto_key_size (void) const;

			bool has_crypto (void) const;
			bool has_crypto_size_at_least_bits (const size_t limit) const;

			void set_user_iv (const std::string_view iv);
			void unset_user_iv (void);
	};

	class ReData {
		public:
			typedef uint_fast8_t KEY_IDENT;
			typedef std::vector<KEY_IDENT> KEY_IDENT_VECTOR;

			enum class MixKeysUse {
				ONLY_NORMAL, /* Use only un-mixed keys */
				ONLY_MIXED, /* Use only mixed keys */
				BOTH_NORMAL_FIRST, /* Use bot mixed and un-mixed keys, try un-mixed first */
				BOTH_MIXED_FIRST /* Use bot mixed and un-mixed keys, try mixed first */
			};

		private:
			ReDataConfig _conf;

			COMMON_VECTOR _hmac_keys;
			COMMON_VECTOR _crypto_keys;

			COMMON_VECTOR _hmac_keys_mixed;
			COMMON_VECTOR _crypto_keys_mixed;

			KEY_IDENT_VECTOR _key_idents;
			std::unordered_map<KEY_IDENT, size_t> _key_ident_map;

			bool _use_config_header {true};
			bool _use_key_ident {false};
			size_t _key_id {0};
			uint_fast16_t _last_key_ident {UINT_FAST16_MAX};
			std::string _work_msg;

			std::string _mix_key;
			bool _mix_key_enabled {false};
			bool _last_decode_was_mixed {false};

			void decode_message (const std::string_view msg, const size_t offset, std::string &plain, const size_t key_id, const bool key_mixed);
			void encode_message (const std::string_view plain, std::string &msg, const size_t key_id, const bool key_mixed);

			void mix_keys (const COMMON_VECTOR &keys, COMMON_VECTOR &out) const;

		public:
			ReData ();
			ReData (const ReDataConfig &conf);
			ReData (ReDataConfig &&conf);

			void reset (const bool also_reset_keys);

			/* Output is stored in 'out', which is always truncated, even when decode fails */
			void decode (const std::string_view msg, size_t &offset, std::string &out, std::function<bool(const ReDataConfig &)> check_callback = nullptr, const MixKeysUse use = MixKeysUse::ONLY_NORMAL);
			void decode (const std::string_view msg, std::string &out, std::function<bool(const ReDataConfig &)> check_callback = nullptr, const MixKeysUse use = MixKeysUse::ONLY_NORMAL);

			/* Output is appended to 'out' only when encode succeeds, otherwise 'out' is untouched */
			void encode (const std::string_view plain, std::string &out, const bool use_mixed = false);
			std::string encode (const std::string_view plain, const bool use_mixed = false);

			ReDataConfig get_config (void) const;
			void set_config (const ReDataConfig &conf);
			void set_config (ReDataConfig &&conf);
			ReDataConfig& set_config (void);

			void set_hmac_key (const std::string_view key);
			void set_crypto_key (const std::string_view key);

			/* key_id == SIZE_MAX means keep current key_id */
			void set_hmac_keys (const COMMON_VECTOR &keys, const size_t key_id = SIZE_MAX);
			void set_crypto_keys (const COMMON_VECTOR &keys, const size_t key_id = SIZE_MAX);
			void set_key_idents (const KEY_IDENT_VECTOR &idents, const size_t key_id = SIZE_MAX);

			/* Key mixing is implemented using SHAKE256 of SHA-3 family by combining normal key + mix */
			/* Enable use of mix key. Empty mix key is also allowed */
			void set_mix_key (const std::string_view mix);
			/* Stop using mix key */
			void clear_mix_key (void);
			/* Returns true if last successfull decode was using mixed keys, false if normal keys */
			bool get_last_decode_was_mixed (void) const;

			/* Get vectors of mixed keys */
			COMMON_VECTOR get_mixed_hmac_keys (void) const;
			COMMON_VECTOR get_mixed_crypto_keys (void) const;

			size_t get_key_id (void) const;
			void set_key_id (const size_t id);

			bool config_header_enabled (void) const;
			void use_config_header (const bool enabled);

			/* Decode normally loops through all keys until it finds one that works.
			But if you have a lot of keys or want to skip this, enable key ident and set
			different key ident for every key. It adds one byte to the message, but uses only
			one decryption/digest check. */
			bool key_ident_enabled (void) const;
			void use_key_ident (const bool enabled);
	};
}

#endif // HEAD_shaga_ReData

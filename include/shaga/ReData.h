/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

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
			static const uint8_t key_digest_mask   {0b0000'1111};
			static const uint8_t key_crypto_mask   {0b0111'0000};
			static const uint8_t key_highbit_mask  {0b1000'0000};

			static const constexpr uint32_t key_digest_shift = BIN::count_trailing_zeros (key_digest_mask);
			static const constexpr uint32_t key_crypto_shift = BIN::count_trailing_zeros (key_crypto_mask);

			enum class DIGEST_HMAC_TYPE {
				NONE,
				TYPICAL,
				RAWKEY,
				SIPHASH,
				HALFSIPHASH,
				_MAX
			};

			enum class DIGEST {
				NONE,

				CRC8, /* CRC-8 Dallas/Maxim */
				CRC32, /* CRC-32-Castagnoli */
				CRC64, /* CRC-64-Jones */

				SHA256,
				SHA512,

				HMAC_SHA256,
				HMAC_SHA512,

				HALFSIPHASH24_32,
				HALFSIPHASH24_64,
				HALFSIPHASH48_32,
				HALFSIPHASH48_64,

				SIPHASH24_64,
				SIPHASH24_128,
				SIPHASH48_64,
				SIPHASH48_128,

				_MAX
			};

			const std::map <std::string, DIGEST> DIGEST_MAP = {
				{"none"s, DIGEST::NONE},

				{"crc8"s, DIGEST::CRC8},
				{"crc-8"s, DIGEST::CRC8},

				{"crc32"s, DIGEST::CRC32},
				{"crc-32"s, DIGEST::CRC32},
				{"crc32c"s, DIGEST::CRC32},
				{"crc-32c"s, DIGEST::CRC32},

				{"crc64"s, DIGEST::CRC64},
				{"crc-64"s, DIGEST::CRC64},

				{"sha256"s, DIGEST::SHA256},
				{"sha-256"s, DIGEST::SHA256},

				{"sha512"s, DIGEST::SHA512},
				{"sha-512"s, DIGEST::SHA512},

				{"hmac-sha256"s, DIGEST::HMAC_SHA256},
				{"hmac-sha-256"s, DIGEST::HMAC_SHA256},

				{"hmac-sha512"s, DIGEST::HMAC_SHA512},
				{"hmac-sha-512"s, DIGEST::HMAC_SHA512},

				{"halfsiphash24-32"s, DIGEST::HALFSIPHASH24_32},
				{"halfsiphash-2-4-32"s, DIGEST::HALFSIPHASH24_32},

				{"halfsiphash24-64"s, DIGEST::HALFSIPHASH24_64},
				{"halfsiphash-2-4-64"s, DIGEST::HALFSIPHASH24_64},

				{"halfsiphash48-32"s, DIGEST::HALFSIPHASH48_32},
				{"halfsiphash-4-8-32"s, DIGEST::HALFSIPHASH48_32},

				{"halfsiphash48-64"s, DIGEST::HALFSIPHASH48_64},
				{"halfsiphash-4-8-64"s, DIGEST::HALFSIPHASH48_64},

				{"siphash24-64"s, DIGEST::SIPHASH24_64},
				{"siphash-2-4-64"s, DIGEST::SIPHASH24_64},

				{"siphash24-128"s, DIGEST::SIPHASH24_128},
				{"siphash-2-4-128"s, DIGEST::SIPHASH24_128},

				{"siphash48-64"s, DIGEST::SIPHASH48_64},
				{"siphash-4-8-64"s, DIGEST::SIPHASH48_64},

				{"siphash48-128"s, DIGEST::SIPHASH48_128},
				{"siphash-4-8-128"s, DIGEST::SIPHASH48_128},
			};

			enum class CRYPTO {
				NONE,

				AES_128_CBC,
				AES_256_CBC,

				CHACHA20,
				CHACHA20_POLY1305,

				_MAX
			};

			const std::map <std::string, CRYPTO> CRYPTO_MAP = {
				{"none"s, CRYPTO::NONE},

				{"aes"s, CRYPTO::AES_128_CBC},
				{"aes128"s, CRYPTO::AES_128_CBC},
				{"aes-128"s, CRYPTO::AES_128_CBC},
				{"aes-128-cbc"s, CRYPTO::AES_128_CBC},

				{"aes256"s, CRYPTO::AES_256_CBC},
				{"aes-256"s, CRYPTO::AES_256_CBC},
				{"aes-256-cbc"s, CRYPTO::AES_256_CBC},

				{"chacha20"s, CRYPTO::CHACHA20},
				{"chacha"s, CRYPTO::CHACHA20},

				{"chacha20-poly1305"s, CRYPTO::CHACHA20_POLY1305},
				{"chacha-poly"s, CRYPTO::CHACHA20_POLY1305},
				{"chachapoly"s, CRYPTO::CHACHA20_POLY1305},
			};

#ifdef SHAGA_FULL
			struct CryptoCache {
				randutils::mt19937_r_rng _rng;
				mbedtls_aes_context _aes_ctx;
				mbedtls_chacha20_context _chacha20_ctx;
				mbedtls_chachapoly_context _chachapoly_ctx;

				CryptoCache ()
				{
					::mbedtls_aes_init (&_aes_ctx);
					::mbedtls_chacha20_init (&_chacha20_ctx);
					::mbedtls_chachapoly_init (&_chachapoly_ctx);
				}

				~CryptoCache ()
				{
					::mbedtls_chachapoly_free (&_chachapoly_ctx);
					::mbedtls_chacha20_free (&_chacha20_ctx);
					::mbedtls_aes_free (&_aes_ctx);
				}

				CryptoCache (const CryptoCache &) = delete;
				CryptoCache (CryptoCache &&) = delete;
				CryptoCache& operator= (const CryptoCache &other) = delete;
				CryptoCache& operator= (CryptoCache &&other) = delete;
			};
#else
			struct CryptoCache { };
#endif // SHAGA_FULL

			struct DigestCache {
				DIGEST digest {DIGEST::CRC32};
				std::string key;
				std::string ipad;
				std::string opad;
				std::string temp;

				uint64_t siphash_k0;
				uint64_t siphash_k1;
				uint32_t halfsiphash_k0;
				uint32_t halfsiphash_k1;

				DigestCache ();

				void reset (void);

				DigestCache (const DigestCache &other) = delete;
				DigestCache (DigestCache &&other) = delete;
				DigestCache& operator= (const DigestCache &other) = delete;
				DigestCache& operator= (DigestCache &&other) = delete;
			};

		private:
			DIGEST _used_digest {DIGEST::CRC32};
			CRYPTO _used_crypto {CRYPTO::NONE};

			mutable CryptoCache _cache_crypto;
			mutable DigestCache _cache_digest;
			mutable uint8_t _temp_iv[MBEDTLS_MAX_IV_LENGTH];

			std::string _user_iv;
			bool _user_iv_enabled {false};

			mutable uint32_t _counter_iv {0};
			bool _counter_iv_enabled {false};

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
			SHAGA_NODISCARD std::string encode (void) const;

			ReDataConfig& set_digest (const ReDataConfig::DIGEST v);
			ReDataConfig& set_digest (const std::string_view str);

			ReDataConfig& set_crypto (const ReDataConfig::CRYPTO v);
			ReDataConfig& set_crypto (const std::string_view str);

			SHAGA_NODISCARD ReDataConfig::DIGEST get_digest (void) const;
			SHAGA_NODISCARD ReDataConfig::CRYPTO get_crypto (void) const;

			SHAGA_STRV std::string_view get_digest_text (void) const;
			SHAGA_STRV std::string_view get_crypto_text (void) const;

			std::string describe (void) const;

			SHAGA_NODISCARD bool is_compatible_digest (const ReDataConfig &other) const;
			SHAGA_NODISCARD bool is_compatible_crypto (const ReDataConfig &other) const;
			SHAGA_NODISCARD bool is_compatible (const ReDataConfig &other) const;

			/* ReDataConfigDigest.cpp */
			void calc_digest (const std::string_view plain, std::string &out, const std::string_view key) const;

			SHAGA_NODISCARD size_t get_digest_result_size (void) const;
			SHAGA_NODISCARD size_t get_digest_hmac_key_size (void) const;

			SHAGA_NODISCARD bool has_hmac (void) const;
			SHAGA_NODISCARD bool has_digest_result_size_at_least_bits (const size_t limit) const;
			SHAGA_NODISCARD bool has_digest_hmac_key_size_at_least_bits (const size_t limit) const;

			/* ReDataConfigCrypto.cpp */
			void calc_crypto_enc (std::string &plain, std::string &out_append, const std::string_view key) const;
			void calc_crypto_dec (std::string_view msg, std::string &out, const std::string_view key) const;

			SHAGA_NODISCARD size_t get_crypto_block_size (void) const;
			SHAGA_NODISCARD size_t get_crypto_key_size (void) const;
			SHAGA_NODISCARD size_t get_crypto_iv_size (void) const;
			SHAGA_NODISCARD size_t get_crypto_mac_size (void) const;

			SHAGA_NODISCARD bool has_crypto (void) const;
			SHAGA_NODISCARD bool has_mac (void) const;
			SHAGA_NODISCARD bool has_crypto_key_size_at_least_bits (const size_t limit) const;
			SHAGA_NODISCARD bool has_crypto_mac_size_at_least_bits (const size_t limit) const;

			void set_user_iv (const std::string_view iv);
			void unset_user_iv (void);

			void set_iv_counter (const uint32_t counter = 0);
			void unset_iv_counter (void);
	};

	class ReData {
		public:
			typedef uint_fast8_t KEY_IDENT;
			typedef std::vector<KEY_IDENT> KEY_IDENT_VECTOR;

			enum class MixKeysUse {
				ONLY_NORMAL, /* Use only un-mixed keys */
				ONLY_MIXED, /* Use only mixed keys */
				BOTH_NORMAL_FIRST, /* Use both mixed and un-mixed keys, try un-mixed first */
				BOTH_MIXED_FIRST /* Use both mixed and un-mixed keys, try mixed first */
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

			mutable std::string _work_msg;
			mutable std::string _work_msg2;

			std::string _mix_key;
			bool _mix_key_enabled {false};
			bool _last_decode_was_mixed {false};

			void decode_message (std::string_view msg, const size_t offset, std::string &plain, const size_t key_id, const bool key_mixed) const;
			void encode_message (const std::string_view plain, std::string &msg, const size_t key_id, const bool key_mixed) const;

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

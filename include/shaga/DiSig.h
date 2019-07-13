/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_DiSig
#define HEAD_shaga_DiSig

#include "common.h"

#ifdef SHAGA_FULL

namespace shaga {
	class DiSig {
		private:
			enum class _TYPE {
				PEM,
				DER,
				RAW
			};

			class DEC_CTX_ENTRY {
				private:
					int add_raw_key (const std::string_view key, const std::string_view curve_type);

				public:
					mbedtls_pk_context _ctx;
					const std::string _name;

					DEC_CTX_ENTRY (const std::string_view key, const std::string_view name, const DiSig::_TYPE type, const std::string_view curve_type);
					DEC_CTX_ENTRY (const std::string_view key, const std::string_view name, const DiSig::_TYPE type);
					~DEC_CTX_ENTRY ();
			};

			randutils::mt19937_r_rng _rng;
			mbedtls_pk_context _enc_ctx;
			std::list<DEC_CTX_ENTRY> _dec_ctx_list;
			unsigned char _output_buf[16'000];

			static void check_error (const int err, const bool allow_positive = false);
			static void can_do (mbedtls_pk_context &ctx);

			void dump_to_file (const std::string_view fname, const unsigned char *buf) const;
			void dump_to_file (const std::string_view fname, const unsigned char *buf, const size_t len) const;
			void dump_to_file (const std::string_view fname, const std::string_view buf) const;

			void check_hash (const mbedtls_md_type_t md_alg, const std::string_view hsh) const;

		public:
			explicit DiSig ();
			~DiSig ();

			void set_encryption_key_pem (const std::string_view key, const std::string_view pass);
			void set_encryption_key_pem (const std::string_view key);

			void set_encryption_key_der (const std::string_view key, const std::string_view pass);
			void set_encryption_key_der (const std::string_view key);

			void load_encryption_key (const std::string_view fname, const std::string_view pass);
			void load_encryption_key (const std::string_view fname);

			void save_encryption_keypair_pem (const std::string_view fname_priv, const std::string_view fname_pub);
			void save_encryption_keypair_der (const std::string_view fname_priv, const std::string_view fname_pub);
			void save_encryption_keypair_raw (const std::string_view fname_priv, const std::string_view fname_pub);

			std::string get_encryption_key_pem (void);
			std::string get_encryption_key_der (void);
			std::string get_encryption_key_raw (void);

			std::string get_encryption_pubkey_pem (void);
			std::string get_encryption_pubkey_der (void);
			std::string get_encryption_pubkey_raw (void);

			void add_decryption_key_pem (const std::string_view key, const std::string_view name);
			void add_decryption_key_pem (const std::string_view key);

			void add_decryption_key_der (const std::string_view key, const std::string_view name);
			void add_decryption_key_der (const std::string_view key);

			void add_decryption_key_raw (const std::string_view curve_type, const std::string_view key, const std::string_view name);
			void add_decryption_key_raw (const std::string_view curve_type, const std::string_view key);

			void reset_decryption_keys (void);

			std::string sign (const mbedtls_md_type_t md_alg, const std::string_view hsh);
			bool verify (const mbedtls_md_type_t md_alg, const std::string_view hsh, const std::string_view sgn, std::string &key_name_out);

			void generate_new_key (const std::string_view curve_type);
	};
}

#endif // SHAGA_FULL

#endif // HEAD_shaga_DiSig

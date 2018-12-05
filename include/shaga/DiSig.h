/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_DiSig
#define HEAD_shaga_DiSig

#include "common.h"

#ifdef SHAGA_FULL

namespace shaga {

	class DiSig {
		private:
			struct DEC_CTX_ENTRY {
				mbedtls_pk_context _ctx;
				const std::string _name;

				DEC_CTX_ENTRY (const std::string &key, const std::string &name);
				~DEC_CTX_ENTRY ();
			};

			mbedtls_pk_context _enc_ctx;
			std::list<DEC_CTX_ENTRY> _dec_ctx_list;
			unsigned char _output_buf[16'000];

			static void check_error (const int err);
			static void can_do (mbedtls_pk_context &ctx);

			void dump_to_file (const std::string &fname, const unsigned char *buf) const;
			void dump_to_file (const std::string &fname, const std::string &buf) const;

			void check_hash (const mbedtls_md_type_t md_alg, const std::string &hsh) const;

		public:
			explicit DiSig ();
			~DiSig ();

			void set_encryption_key (const std::string &key, const std::string &pass);
			void set_encryption_key (const std::string &key);

			void load_encryption_key (const std::string &fname, const std::string &pass);
			void load_encryption_key (const std::string &fname);

			void save_encryption_keypair (const std::string &fname_priv, const std::string &fname_pub);

			std::string get_encryption_key (void);
			std::string get_encryption_pubkey (void);

			void add_decryption_key (const std::string &key, const std::string &name);
			void add_decryption_key (const std::string &key);

			void reset_decryption_keys (void);

			std::string sign (const mbedtls_md_type_t md_alg, const std::string &hsh);
			bool verify (const mbedtls_md_type_t md_alg, const std::string &hsh, const std::string &sgn, std::string &key_name);

			void generate_new_key (const std::string &curve_type);
	};

	int random_for_mbedtls (void *p_rng, unsigned char *output, size_t output_len);
}

#endif // SHAGA_FULL

#endif // HEAD_shaga_DiSig

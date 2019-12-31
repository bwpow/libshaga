/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_DiSig
#define HEAD_shaga_DiSig

#include "common.h"

#ifdef SHAGA_FULL

namespace shaga {
	class DiSigCommon {
		protected:
			static void check_error (const int err, const bool allow_positive = false);
			static void can_do (const mbedtls_pk_context &ctx);
			static std::string_view get_name (const mbedtls_pk_context &ctx);

			virtual void dump_to_file (const std::string_view fname, const unsigned char *buf) const;
			virtual void dump_to_file (const std::string_view fname, const unsigned char *buf, const size_t len) const;
			virtual void dump_to_file (const std::string_view fname, const std::string_view buf) const;

			virtual void check_hash (const mbedtls_md_type_t md_alg, const std::string_view hsh) const;

		public:
	};

	class DiSigPrivate : public DiSigCommon
	{
		private:
			unsigned char _output_buf[16'000];
			randutils::mt19937_r_rng _rng;
			mbedtls_pk_context _enc_ctx;

		public:
			explicit DiSigPrivate ();
			~DiSigPrivate ();

			virtual void generate_new_keypair (const std::string_view curve_type);
			virtual std::string_view get_curve_type (void) const;

			virtual void set_private_key_pem (const std::string_view key, const std::string_view pass);
			virtual void set_private_key_pem (const std::string_view key);

			virtual void set_private_key_der (const std::string_view key, const std::string_view pass);
			virtual void set_private_key_der (const std::string_view key);

			virtual void load_private_key (const std::string_view fname, const std::string_view pass);
			virtual void load_private_key (const std::string_view fname);

			virtual void save_keypair_pem (const std::string_view fname_priv, const std::string_view fname_pub);
			virtual void save_keypair_der (const std::string_view fname_priv, const std::string_view fname_pub);
			virtual void save_keypair_raw (const std::string_view fname_priv, const std::string_view fname_pub);

			virtual std::string get_private_key_pem (void);
			virtual std::string get_private_key_der (void);
			virtual std::string get_private_key_raw (void);

			virtual std::string get_public_key_pem (void);
			virtual std::string get_public_key_der (void);
			virtual std::string get_public_key_raw (void);

			virtual std::string sign (const mbedtls_md_type_t md_alg, const std::string_view hsh);
			virtual std::string sign_raw (const mbedtls_md_type_t md_alg, const std::string_view hsh);
	};

	class DiSigVerify : public DiSigCommon
	{
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

					DEC_CTX_ENTRY (const std::string_view key, const std::string_view name, const DiSigVerify::_TYPE type, const std::string_view curve_type);
					DEC_CTX_ENTRY (const std::string_view key, const std::string_view name, const DiSigVerify::_TYPE type);
					~DEC_CTX_ENTRY ();
			};

			class RAW_SIGNATURE_ENTRY {
				public:
					mbedtls_pk_context _ctx;
					mbedtls_ecp_keypair *ec {nullptr};

					RAW_SIGNATURE_ENTRY (const std::string_view curve_type, const std::string_view sgn_raw);
					~RAW_SIGNATURE_ENTRY ();
			};

			std::deque<DEC_CTX_ENTRY> _dec_ctx_list;

		public:
			explicit DiSigVerify ();
			~DiSigVerify ();

			virtual std::string_view get_curve_type (const size_t key_id) const;

			virtual void add_public_key_pem (const std::string_view key, const std::string_view name);
			virtual void add_public_key_pem (const std::string_view key);

			virtual void add_public_key_der (const std::string_view key, const std::string_view name);
			virtual void add_public_key_der (const std::string_view key);

			virtual void add_public_key_raw (const std::string_view curve_type, const std::string_view key, const std::string_view name);
			virtual void add_public_key_raw (const std::string_view curve_type, const std::string_view key);

			virtual void clear_keys (void);

			virtual bool verify (const mbedtls_md_type_t md_alg, const std::string_view hsh, const std::string_view sgn, std::string &key_name_out);
			virtual bool verify (const mbedtls_md_type_t md_alg, const std::string_view hsh, const std::string_view sgn);

			virtual bool verify_raw (const std::string_view curve_type, const mbedtls_md_type_t md_alg, const std::string_view hsh, const std::string_view sgn, std::string &key_name_out);
			virtual bool verify_raw (const std::string_view curve_type, const mbedtls_md_type_t md_alg, const std::string_view hsh, const std::string_view sgn);
	};
}

#endif // SHAGA_FULL

#endif // HEAD_shaga_DiSig

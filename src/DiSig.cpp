/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifdef SHAGA_FULL

#include <mbedtls/error.h>

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void DiSig::check_error (const int err) const
	{
		if (err != 0) {
			char buf[256];
			mbedtls_strerror (err, buf, sizeof (buf));
			cThrow ("DiSig error: %s", buf);
		}
	}

	void DiSig::can_do (mbedtls_pk_context &ctx) const
	{
		if (mbedtls_pk_can_do (&ctx, MBEDTLS_PK_ECDSA) != 1) {
			cThrow ("DiSig error: Key is not suitable for ECDSA");
		}
	}

	void DiSig::dump_to_file (const std::string &fname, const unsigned char *buf) const
	{
		ShFile file (fname, ShFile::mWRITE | ShFile::mTRUNC);
		file.write (reinterpret_cast<const char *> (buf));
	}

	void DiSig::check_hash (const mbedtls_md_type_t md_alg, const std::string &hsh) const
	{
		const mbedtls_md_info_t *info = mbedtls_md_info_from_type (md_alg);
		if (info == nullptr) {
			cThrow ("Unknown hash type");
		}

		if (info->size != static_cast<int> (hsh.size ())) {
			cThrow ("Hash %s expects %d bytes but %d bytes provided", info->name, info->size, static_cast<int> (hsh.size ()));
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DiSig::DiSig (const std::string &seed)
	{
		mbedtls_entropy_init (&_entropy);
		mbedtls_ctr_drbg_init (&_ctr_drbg);
		mbedtls_pk_init  (&_enc_ctx);

		int ret = mbedtls_ctr_drbg_seed (&_ctr_drbg, mbedtls_entropy_func, &_entropy, reinterpret_cast<const unsigned char *> (seed.data()), seed.size ());
		check_error (ret);
	}

	DiSig::~DiSig ()
	{
		mbedtls_pk_free (&_enc_ctx);
		mbedtls_ctr_drbg_free (&_ctr_drbg);
		mbedtls_entropy_free (&_entropy);
		reset_decryption_keys ();
	}

	void DiSig::set_encryption_key (const std::string &key, const std::string &pass)
	{
		mbedtls_pk_free (&_enc_ctx);

		int ret = mbedtls_pk_parse_key (&_enc_ctx, reinterpret_cast<const unsigned char *> (key.data ()), key.size () + 1,
			reinterpret_cast<const unsigned char *> (pass.data ()), pass.size ());
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSig::set_encryption_key (const std::string &key)
	{
		set_encryption_key (key, "");
	}

	void DiSig::load_encryption_key (const std::string &fname, const std::string &pass)
	{
		mbedtls_pk_free (&_enc_ctx);

		int ret = mbedtls_pk_parse_keyfile (&_enc_ctx, fname.c_str (), pass.c_str ());
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSig::load_encryption_key (const std::string &fname)
	{
		mbedtls_pk_free (&_enc_ctx);

		int ret = mbedtls_pk_parse_keyfile (&_enc_ctx, fname.c_str (), nullptr);
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSig::save_encryption_keypair (const std::string &fname_priv, const std::string &fname_pub)
	{
		unsigned char output_buf[16000];
		int ret;

		memset (output_buf, 0, sizeof (output_buf));
		ret = mbedtls_pk_write_key_pem (&_enc_ctx, output_buf, sizeof (output_buf));
		check_error (ret);
		dump_to_file (fname_priv, output_buf);

		memset (output_buf, 0, sizeof (output_buf));
		ret = mbedtls_pk_write_pubkey_pem (&_enc_ctx, output_buf, sizeof (output_buf));
		check_error (ret);
		dump_to_file (fname_pub, reinterpret_cast<const unsigned char *> (BIN::to_base64 (std::string (reinterpret_cast<const char *> (output_buf)), true).c_str ()));
	}

	std::string DiSig::get_encryption_key (void)
	{
		unsigned char output_buf[16000];
		int ret;

		memset (output_buf, 0, sizeof (output_buf));
		ret = mbedtls_pk_write_key_pem (&_enc_ctx, output_buf, sizeof (output_buf));
		check_error (ret);

		return std::string (reinterpret_cast<const char *> (output_buf));
	}

	std::string DiSig::get_encryption_pubkey (void)
	{
		unsigned char output_buf[16000];
		int ret;

		memset (output_buf, 0, sizeof (output_buf));
		ret = mbedtls_pk_write_pubkey_pem (&_enc_ctx, output_buf, sizeof (output_buf));
		check_error (ret);

		return BIN::to_base64 (std::string (reinterpret_cast<const char *> (output_buf)), true);
	}

	void DiSig::add_decryption_key (const std::string &key, const std::string &name)
	{
		DEC_CTX_ENTRY entry;

		mbedtls_pk_init  (&(entry.ctx));

		const std::string pubkey = BIN::from_base64 (key, true);
		int ret = mbedtls_pk_parse_public_key (&(entry.ctx), reinterpret_cast<const unsigned char *> (pubkey.data ()), pubkey.size () + 1);
		check_error (ret);

		can_do (entry.ctx);

		entry.name.assign (name);

		_dec_ctx_list.push_back (std::move (entry));
	}

	void DiSig::add_decryption_key (const std::string &key)
	{
		add_decryption_key (key, "deckey" + STR::from_int (_dec_ctx_list.size ()));
	}

	void DiSig::reset_decryption_keys (void)
	{
		for (auto &entry : _dec_ctx_list) {
			mbedtls_pk_free (&(entry.ctx));
		}
		_dec_ctx_list.clear ();
	}


	std::string DiSig::sign (const mbedtls_md_type_t md_alg, const std::string &hsh)
	{
		check_hash (md_alg, hsh);

		unsigned char sgn[MBEDTLS_MPI_MAX_SIZE];
		size_t sgn_len = 0;
		int ret =  mbedtls_pk_sign (&_enc_ctx, md_alg, reinterpret_cast<const unsigned char *> (hsh.data ()), hsh.size (), sgn, &sgn_len, mbedtls_ctr_drbg_random, &_ctr_drbg);
		check_error (ret);

		return std::string (reinterpret_cast<const char *> (sgn), sgn_len);
	}

	bool DiSig::verify (const mbedtls_md_type_t md_alg, const std::string &hsh, const std::string &sgn, std::string &key_name)
	{
		check_hash (md_alg, hsh);

		int ret;

		for(DEC_CTX_ENTRY &entry : _dec_ctx_list) {
			ret = mbedtls_pk_verify (&(entry.ctx), md_alg, reinterpret_cast<const unsigned char *> (hsh.data ()), hsh.size (), reinterpret_cast<const unsigned char *> (sgn.data ()), sgn.size ());
			if (ret == 0) {
				key_name.assign (entry.name);
				return true;
			}
		}

		return false;
	}

	void DiSig::generate_new_key (const std::string &curve_type)
	{
		int ret;

		mbedtls_pk_free (&_enc_ctx);

		const mbedtls_ecp_curve_info *curve_info = mbedtls_ecp_curve_info_from_name (curve_type.c_str ());
		if (curve_info == nullptr) {
			cThrow ("Unknown curve '%s'", curve_type.c_str ());
		}

		ret = mbedtls_pk_setup (&_enc_ctx, mbedtls_pk_info_from_type (MBEDTLS_PK_ECKEY));
		check_error (ret);

		ret = mbedtls_ecp_gen_key (curve_info->grp_id, mbedtls_pk_ec (_enc_ctx), mbedtls_ctr_drbg_random, &_ctr_drbg);
		check_error (ret);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Global functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#ifdef SHAGA_FULL
//#else
//		cThrow ("Digest is not supported in lite version");
//#endif // SHAGA_FULL

}

#endif // SHAGA_FULL

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifdef SHAGA_FULL

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	#ifdef SHAGA_THREADING
	static std::mutex _random_for_mbedtls_mutex;
	#endif // SHAGA_THREADING

	void DiSig::check_error (const int err)
	{
		if (err != 0) {
			char buf[256];
			::memset (buf, 0, sizeof (buf));
			::mbedtls_strerror (err, buf, sizeof (buf) - 1);
			cThrow ("DiSig error: %s", buf);
		}
	}

	void DiSig::can_do (mbedtls_pk_context &ctx)
	{
		if (::mbedtls_pk_can_do (&ctx, MBEDTLS_PK_ECDSA) != 1) {
			cThrow ("DiSig error: Key is not suitable for ECDSA");
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DiSig::DEC_CTX_ENTRY::DEC_CTX_ENTRY (const std::string &key, const std::string &name) try : _name (name)
	{
		::mbedtls_pk_init  (&_ctx);

		std::string pubkey = BIN::from_base64 (key, true);
		pubkey.append (1, '\0');

		int ret = ::mbedtls_pk_parse_public_key (&_ctx, reinterpret_cast<const unsigned char *> (pubkey.data ()), pubkey.size ());
		DiSig::check_error (ret);

		DiSig::can_do (_ctx);
	}
	catch (...) {
		::mbedtls_pk_free (&_ctx);
	}

	DiSig::DEC_CTX_ENTRY::~DEC_CTX_ENTRY ()
	{
		::mbedtls_pk_free (&_ctx);
	}

	void DiSig::dump_to_file (const std::string &fname, const unsigned char *buf) const
	{
		ShFile file (fname, ShFile::mWRITE | ShFile::mTRUNC);
		file.write (reinterpret_cast<const char *> (buf));
	}

	void DiSig::dump_to_file (const std::string &fname, const std::string &buf) const
	{
		ShFile file (fname, ShFile::mWRITE | ShFile::mTRUNC);
		file.write (buf);
	}

	void DiSig::check_hash (const mbedtls_md_type_t md_alg, const std::string &hsh) const
	{
		const mbedtls_md_info_t *info = ::mbedtls_md_info_from_type (md_alg);
		if (nullptr == info) {
			cThrow ("Unknown hash type");
		}

		if (info->size != static_cast<int> (hsh.size ())) {
			cThrow ("Hash %s expects %d bytes but %d bytes provided", info->name, static_cast<int> (info->size), static_cast<int> (hsh.size ()));
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DiSig::DiSig ()
	{
		::mbedtls_pk_init (&_enc_ctx);
	}

	DiSig::~DiSig ()
	{
		_dec_ctx_list.clear ();
		::mbedtls_pk_free (&_enc_ctx);
	}

	void DiSig::set_encryption_key (const std::string &key, const std::string &pass)
	{
		::mbedtls_pk_free (&_enc_ctx);

		int ret = ::mbedtls_pk_parse_key (&_enc_ctx, reinterpret_cast<const unsigned char *> (key.data ()), key.size () + 1,
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
		::mbedtls_pk_free (&_enc_ctx);

		int ret = ::mbedtls_pk_parse_keyfile (&_enc_ctx, fname.c_str (), pass.c_str ());
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSig::load_encryption_key (const std::string &fname)
	{
		::mbedtls_pk_free (&_enc_ctx);

		int ret = ::mbedtls_pk_parse_keyfile (&_enc_ctx, fname.c_str (), nullptr);
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSig::save_encryption_keypair (const std::string &fname_priv, const std::string &fname_pub)
	{
		int ret;

		::memset (_output_buf, 0, sizeof (_output_buf));
		ret = mbedtls_pk_write_key_pem (&_enc_ctx, _output_buf, sizeof (_output_buf) - 1);
		check_error (ret);
		dump_to_file (fname_priv, _output_buf);

		::memset (_output_buf, 0, sizeof (_output_buf));
		ret = mbedtls_pk_write_pubkey_pem (&_enc_ctx, _output_buf, sizeof (_output_buf) - 1);
		check_error (ret);
		dump_to_file (fname_pub, BIN::to_base64 (std::string (reinterpret_cast<const char *> (_output_buf)), true));
	}

	std::string DiSig::get_encryption_key (void)
	{
		::memset (_output_buf, 0, sizeof (_output_buf));
		int ret = ::mbedtls_pk_write_key_pem (&_enc_ctx, _output_buf, sizeof (_output_buf) - 1);
		check_error (ret);

		return std::string (reinterpret_cast<const char *> (_output_buf));
	}

	std::string DiSig::get_encryption_pubkey (void)
	{
		::memset (_output_buf, 0, sizeof (_output_buf));
		int ret = ::mbedtls_pk_write_pubkey_pem (&_enc_ctx, _output_buf, sizeof (_output_buf) - 1);
		check_error (ret);

		return BIN::to_base64 (std::string (reinterpret_cast<const char *> (_output_buf)), true);
	}

	void DiSig::add_decryption_key (const std::string &key, const std::string &name)
	{
		_dec_ctx_list.emplace_back (key, name);
	}

	void DiSig::add_decryption_key (const std::string &key)
	{
		const std::string name = "deckey" + STR::from_int (_dec_ctx_list.size ());
		_dec_ctx_list.emplace_back (key, name);
	}

	void DiSig::reset_decryption_keys (void)
	{
		_dec_ctx_list.clear ();
	}

	std::string DiSig::sign (const mbedtls_md_type_t md_alg, const std::string &hsh)
	{
		check_hash (md_alg, hsh);

		unsigned char sgn[MBEDTLS_MPI_MAX_SIZE];
		size_t sgn_len = 0;
		int ret =  ::mbedtls_pk_sign (&_enc_ctx, md_alg, reinterpret_cast<const unsigned char *> (hsh.data ()), hsh.size (), sgn, &sgn_len, random_for_mbedtls, nullptr);
		check_error (ret);

		return std::string (reinterpret_cast<const char *> (sgn), sgn_len);
	}

	bool DiSig::verify (const mbedtls_md_type_t md_alg, const std::string &hsh, const std::string &sgn, std::string &key_name)
	{
		check_hash (md_alg, hsh);

		int ret;

		for(DEC_CTX_ENTRY &entry : _dec_ctx_list) {
			ret = ::mbedtls_pk_verify (&(entry._ctx), md_alg, reinterpret_cast<const unsigned char *> (hsh.data ()), hsh.size (), reinterpret_cast<const unsigned char *> (sgn.data ()), sgn.size ());
			if (0 == ret) {
				key_name.assign (entry._name);
				return true;
			}
		}

		return false;
	}

	void DiSig::generate_new_key (const std::string &curve_type)
	{
		int ret;

		mbedtls_pk_free (&_enc_ctx);

		const mbedtls_ecp_curve_info *curve_info = ::mbedtls_ecp_curve_info_from_name (curve_type.c_str ());
		if (nullptr == curve_info) {
			cThrow ("Unknown curve '%s'", curve_type.c_str ());
		}

		ret = ::mbedtls_pk_setup (&_enc_ctx, ::mbedtls_pk_info_from_type (MBEDTLS_PK_ECKEY));
		check_error (ret);

		ret = ::mbedtls_ecp_gen_key (curve_info->grp_id, ::mbedtls_pk_ec (_enc_ctx), random_for_mbedtls, nullptr);
		check_error (ret);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Global functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* This is a random generator suitable for mbedtls functions using mt19937 from C++ std library */
	int random_for_mbedtls (void *p_rng, unsigned char *output, size_t output_len)
	{
		#ifdef SHAGA_THREADING
		std::lock_guard<std::mutex> lock (_random_for_mbedtls_mutex);
		#endif // SHAGA_THREADING

		(void) p_rng;

		static std::random_device rd;
		static std::mt19937 mte (rd ());
		static std::uniform_int_distribution<unsigned char> dist (0x00, 0xff);

		std::generate_n (output, output_len, [&] (void) -> unsigned char {
			return dist(mte);
		});

		return 0;
	}
}

#endif // SHAGA_FULL

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifdef SHAGA_FULL

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static std::random_device _mbedtls_rd;
	static std::mt19937 _mbedtls_mte (_mbedtls_rd ());
	static std::uniform_int_distribution<unsigned char> _mbedtls_dist (0x00, 0xff);

	#ifdef SHAGA_THREADING
	static std::mutex _random_for_mbedtls_mutex;
	#endif // SHAGA_THREADING

	void DiSig::check_error (const int err, const bool allow_positive)
	{
		if (true == allow_positive ? err <= 0 : err != 0) {
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

	int DiSig::DEC_CTX_ENTRY::add_raw_key (const std::string &key, const std::string &curve_type)
	{
		int ret;

		const mbedtls_ecp_curve_info *curve_info = ::mbedtls_ecp_curve_info_from_name (curve_type.c_str ());
		if (nullptr == curve_info) {
			cThrow ("DiSig error: Unknown curve '%s'", curve_type.c_str ());
		}

		mbedtls_pk_free (&_ctx);
		ret = ::mbedtls_pk_setup (&_ctx, ::mbedtls_pk_info_from_type (MBEDTLS_PK_ECKEY));
		check_error (ret);

		mbedtls_ecp_keypair *ec = mbedtls_pk_ec (_ctx);

		ret = ::mbedtls_ecp_group_load (&ec->grp, curve_info->grp_id);
		check_error (ret);

		size_t pos = 0;
		const size_t len = ::mbedtls_mpi_size (&ec->grp.P);

		auto func = [&](mbedtls_mpi *P) -> void {
			if (pos + len > key.size ()) {
				cThrow ("DiSig error: Raw data too short");
			}

			ret = ::mbedtls_mpi_read_binary (P, reinterpret_cast<const uint8_t *>(key.data () + pos), len);
			check_error (ret);

			pos += len;
		};

		func (&ec->Q.X);
		func (&ec->Q.Y);
		mbedtls_mpi_lset (&ec->Q.Z, 1);

		if (pos != key.size ()) {
			cThrow ("DiSig error: Extra raw data for selected curve type");
		}

		return 0;
	}

	DiSig::DEC_CTX_ENTRY::DEC_CTX_ENTRY (const std::string &key, const std::string &name, const DiSig::_TYPE type, const std::string &curve_type) try :
		_name (name)
	{
		::mbedtls_pk_init  (&_ctx);

		int ret = (-1);

		switch (type) {
			case DiSig::_TYPE::PEM:
				ret = ::mbedtls_pk_parse_public_key (&_ctx, reinterpret_cast<const unsigned char *> (key.c_str ()), key.size () + 1);
				break;

			case DiSig::_TYPE::DER:
				ret = ::mbedtls_pk_parse_public_key (&_ctx, reinterpret_cast<const unsigned char *> (key.data ()), key.size ());
				break;

			case DiSig::_TYPE::RAW:
				ret = add_raw_key (key, curve_type);
				break;
		}

		DiSig::check_error (ret);
		DiSig::can_do (_ctx);
	}
	catch (...) {
		::mbedtls_pk_free (&_ctx);
	}

	DiSig::DEC_CTX_ENTRY::DEC_CTX_ENTRY (const std::string &key, const std::string &name, const DiSig::_TYPE type) :
		DEC_CTX_ENTRY (key, name, type, "")
	{ }

	DiSig::DEC_CTX_ENTRY::~DEC_CTX_ENTRY ()
	{
		::mbedtls_pk_free (&_ctx);
	}

	void DiSig::dump_to_file (const std::string &fname, const unsigned char *buf) const
	{
		ShFile file (fname, ShFile::mWRITE | ShFile::mTRUNC);
		file.write (reinterpret_cast<const char *> (buf));
	}

	void DiSig::dump_to_file (const std::string &fname, const unsigned char *buf, const size_t len) const
	{
		ShFile file (fname, ShFile::mWRITE | ShFile::mTRUNC);
		file.write (reinterpret_cast<const char *> (buf), len);
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
			cThrow ("DiSig error: Unknown hash type");
		}

		if (info->size != static_cast<int> (hsh.size ())) {
			cThrow ("DiSig error: Hash %s expects %d bytes but %d bytes provided", info->name, static_cast<int> (info->size), static_cast<int> (hsh.size ()));
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

	void DiSig::set_encryption_key_pem (const std::string &key, const std::string &pass)
	{
		::mbedtls_pk_free (&_enc_ctx);

		const int ret = ::mbedtls_pk_parse_key (&_enc_ctx, reinterpret_cast<const unsigned char *> (key.c_str ()), key.size () + 1,
			reinterpret_cast<const unsigned char *> (pass.data ()), pass.size ());
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSig::set_encryption_key_pem (const std::string &key)
	{
		::mbedtls_pk_free (&_enc_ctx);

		const int ret = ::mbedtls_pk_parse_key (&_enc_ctx, reinterpret_cast<const unsigned char *> (key.c_str ()), key.size () + 1, nullptr, 0);
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSig::set_encryption_key_der (const std::string &key, const std::string &pass)
	{
		::mbedtls_pk_free (&_enc_ctx);

		const int ret = ::mbedtls_pk_parse_key (&_enc_ctx, reinterpret_cast<const unsigned char *> (key.data ()), key.size (),
			reinterpret_cast<const unsigned char *> (pass.data ()), pass.size ());
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSig::set_encryption_key_der (const std::string &key)
	{
		::mbedtls_pk_free (&_enc_ctx);

		const int ret = ::mbedtls_pk_parse_key (&_enc_ctx, reinterpret_cast<const unsigned char *> (key.data ()), key.size (), nullptr, 0);
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSig::load_encryption_key (const std::string &fname, const std::string &pass)
	{
		::mbedtls_pk_free (&_enc_ctx);

		const int ret = ::mbedtls_pk_parse_keyfile (&_enc_ctx, fname.c_str (), pass.c_str ());
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSig::load_encryption_key (const std::string &fname)
	{
		::mbedtls_pk_free (&_enc_ctx);

		const int ret = ::mbedtls_pk_parse_keyfile (&_enc_ctx, fname.c_str (), nullptr);
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSig::save_encryption_keypair_pem (const std::string &fname_priv, const std::string &fname_pub)
	{
		int ret;

		::memset (_output_buf, 0, sizeof (_output_buf));
		ret = ::mbedtls_pk_write_key_pem (&_enc_ctx, _output_buf, sizeof (_output_buf) - 1);
		check_error (ret);
		dump_to_file (fname_priv, _output_buf);

		::memset (_output_buf, 0, sizeof (_output_buf));
		ret = ::mbedtls_pk_write_pubkey_pem (&_enc_ctx, _output_buf, sizeof (_output_buf) - 1);
		check_error (ret);
		dump_to_file (fname_pub, _output_buf);
	}

	void DiSig::save_encryption_keypair_der (const std::string &fname_priv, const std::string &fname_pub)
	{
		int ret;

		::memset (_output_buf, 0, sizeof (_output_buf));
		ret = ::mbedtls_pk_write_key_der (&_enc_ctx, _output_buf, sizeof (_output_buf));
		check_error (ret, true);
		dump_to_file (fname_priv, _output_buf + sizeof (_output_buf) - ret, ret);

		::memset (_output_buf, 0, sizeof (_output_buf));
		ret = ::mbedtls_pk_write_pubkey_der (&_enc_ctx, _output_buf, sizeof (_output_buf));
		check_error (ret, true);
		dump_to_file (fname_pub, _output_buf + sizeof (_output_buf) - ret, ret);
	}

	void DiSig::save_encryption_keypair_raw (const std::string &fname_priv, const std::string &fname_pub)
	{
		{
			const std::string buf = get_encryption_key_raw ();
			dump_to_file (fname_priv, buf);
		}
		{
			const std::string buf = get_encryption_pubkey_raw ();
			dump_to_file (fname_pub, buf);
		}
	}

	std::string DiSig::get_encryption_key_pem (void)
	{
		::memset (_output_buf, 0, sizeof (_output_buf));
		const int ret = ::mbedtls_pk_write_key_pem (&_enc_ctx, _output_buf, sizeof (_output_buf) - 1);
		check_error (ret);

		return std::string (reinterpret_cast<const char *> (_output_buf));
	}

	std::string DiSig::get_encryption_key_der (void)
	{
		const int ret = ::mbedtls_pk_write_key_der (&_enc_ctx, _output_buf, sizeof (_output_buf));
		check_error (ret, true);

		return std::string (reinterpret_cast<const char *> (_output_buf) + sizeof (_output_buf) - ret, ret);
	}

	std::string DiSig::get_encryption_key_raw (void)
	{
		if (mbedtls_pk_get_type (&_enc_ctx) != MBEDTLS_PK_ECKEY) {
			cThrow ("DiSig error: Raw export is only supported for EC keys");
		}
		const mbedtls_ecp_keypair *ec = mbedtls_pk_ec (_enc_ctx);

		const size_t expected_len = ::mbedtls_mpi_size (&ec->grp.P);
		const size_t len = ::mbedtls_mpi_size (&ec->d);

		if (len != expected_len) {
			cThrow ("DiSig error: Output size does not match expected length");
		}

		const int ret = ::mbedtls_mpi_write_binary (&ec->d, _output_buf, sizeof (_output_buf));
		check_error (ret);

		return std::string (reinterpret_cast<const char *> (_output_buf) + sizeof (_output_buf) - len, len);
	}

	std::string DiSig::get_encryption_pubkey_pem (void)
	{
		::memset (_output_buf, 0, sizeof (_output_buf));
		const int ret = ::mbedtls_pk_write_pubkey_pem (&_enc_ctx, _output_buf, sizeof (_output_buf) - 1);
		check_error (ret);

		return std::string (reinterpret_cast<const char *> (_output_buf));
	}

	std::string DiSig::get_encryption_pubkey_der (void)
	{
		const int ret = ::mbedtls_pk_write_pubkey_der (&_enc_ctx, _output_buf, sizeof (_output_buf));
		check_error (ret, true);

		return std::string (reinterpret_cast<const char *> (_output_buf) + sizeof (_output_buf) - ret, ret);
	}

	std::string DiSig::get_encryption_pubkey_raw (void)
	{
		if (mbedtls_pk_get_type (&_enc_ctx) != MBEDTLS_PK_ECKEY) {
			cThrow ("DiSig error: Raw export is only supported for EC keys");
		}
		const mbedtls_ecp_keypair *ec = mbedtls_pk_ec (_enc_ctx);

		const size_t expected_len = ::mbedtls_mpi_size (&ec->grp.P) * 2;

		std::string output;
		output.reserve (expected_len);

		auto func = [&](const mbedtls_mpi *P) -> void {
			const size_t len = ::mbedtls_mpi_size (P);
			const int ret = ::mbedtls_mpi_write_binary (P, _output_buf, sizeof (_output_buf));
			check_error (ret);
			output.append (reinterpret_cast<const char *> (_output_buf) + sizeof (_output_buf) - len, len);
		};

		func (&ec->Q.X);
		func (&ec->Q.Y);

		if (output.size () != expected_len) {
			cThrow ("DiSig error: Output size does not match expected length");
		}

		return output;
	}

	void DiSig::add_decryption_key_pem (const std::string &key, const std::string &name)
	{
		_dec_ctx_list.emplace_back (key, name, _TYPE::PEM);
	}

	void DiSig::add_decryption_key_pem (const std::string &key)
	{
		const std::string name = "deckey" + STR::from_int (_dec_ctx_list.size ());
		_dec_ctx_list.emplace_back (key, name, _TYPE::PEM);
	}

	void DiSig::add_decryption_key_der (const std::string &key, const std::string &name)
	{
		_dec_ctx_list.emplace_back (key, name, _TYPE::DER);
	}

	void DiSig::add_decryption_key_der (const std::string &key)
	{
		const std::string name = "deckey" + STR::from_int (_dec_ctx_list.size ());
		_dec_ctx_list.emplace_back (key, name, _TYPE::DER);
	}

	void DiSig::add_decryption_key_raw (const std::string &curve_type, const std::string &key, const std::string &name)
	{
		_dec_ctx_list.emplace_back (key, name, _TYPE::RAW, curve_type);
	}

	void DiSig::add_decryption_key_raw (const std::string &curve_type, const std::string &key)
	{
		const std::string name = "deckey" + STR::from_int (_dec_ctx_list.size ());
		_dec_ctx_list.emplace_back (key, name, _TYPE::RAW, curve_type);
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
		const int ret =  ::mbedtls_pk_sign (&_enc_ctx, md_alg, reinterpret_cast<const unsigned char *> (hsh.data ()), hsh.size (), sgn, &sgn_len, random_for_mbedtls, nullptr);
		check_error (ret);

		return std::string (reinterpret_cast<const char *> (sgn), sgn_len);
	}

	bool DiSig::verify (const mbedtls_md_type_t md_alg, const std::string &hsh, const std::string &sgn, std::string &key_name_out)
	{
		check_hash (md_alg, hsh);

		int ret;

		for(DEC_CTX_ENTRY &entry : _dec_ctx_list) {
			ret = ::mbedtls_pk_verify (&(entry._ctx), md_alg, reinterpret_cast<const unsigned char *> (hsh.data ()), hsh.size (), reinterpret_cast<const unsigned char *> (sgn.data ()), sgn.size ());
			if (0 == ret) {
				key_name_out.assign (entry._name);
				return true;
			}
		}

		return false;
	}

	void DiSig::generate_new_key (const std::string &curve_type)
	{
		int ret;

		::mbedtls_pk_free (&_enc_ctx);

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

		std::generate_n (output, output_len, [&] (void) -> unsigned char {
			return _mbedtls_dist(_mbedtls_mte);
		});

		return 0;
	}
}

#endif // SHAGA_FULL

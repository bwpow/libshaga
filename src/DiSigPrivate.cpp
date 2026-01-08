/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifdef SHAGA_FULL

namespace shaga {
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* This is a random generator suitable for mbedtls functions using mt19937 from C++ std library */
	static int _random_for_mbedtls (void *p_rng, unsigned char *output, size_t output_len)
	{
		if (nullptr == p_rng) {
			cThrow ("DiSig error: Null pointer in random"sv);
		}
		randutils::mt19937_r_rng *const _rng = reinterpret_cast<randutils::mt19937_r_rng *> (p_rng);
		_rng->generate_n<uint8_t> (output, output_len);
		return 0;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DiSigPrivate::DiSigPrivate ()
	{
		::mbedtls_pk_init (&_enc_ctx);
	}

	DiSigPrivate::~DiSigPrivate ()
	{
		::mbedtls_pk_free (&_enc_ctx);
	}

	void DiSigPrivate::generate_new_keypair (const std::string_view curve_type)
	{
		int ret;

		::mbedtls_pk_free (&_enc_ctx);
		::mbedtls_pk_init (&_enc_ctx);

		try {
			const mbedtls_ecp_curve_info *curve_info = ::mbedtls_ecp_curve_info_from_name (s_c_str (curve_type));
			if (nullptr == curve_info) {
				cThrow ("Unknown curve '{}'"sv, curve_type);
			}

			ret = ::mbedtls_pk_setup (&_enc_ctx, ::mbedtls_pk_info_from_type (MBEDTLS_PK_ECKEY));
			check_error (ret);

			ret = ::mbedtls_ecp_gen_key (curve_info->grp_id, ::mbedtls_pk_ec (_enc_ctx), _random_for_mbedtls, &_rng);
			check_error (ret);
		}
		catch (...) {
			::mbedtls_pk_free (&_enc_ctx);
			::mbedtls_pk_init (&_enc_ctx);
			throw;
		}
	}

	SHAGA_STRV std::string_view DiSigPrivate::get_curve_type (void) const
	{
		return get_name (_enc_ctx);
	}

	void DiSigPrivate::set_private_key_pem (const std::string_view key, const std::string_view pass)
	{
		::mbedtls_pk_free (&_enc_ctx);
		::mbedtls_pk_init (&_enc_ctx);

		const int ret = ::mbedtls_pk_parse_key (&_enc_ctx, reinterpret_cast<const unsigned char *> (s_c_str (key)), key.size () + 1,
			reinterpret_cast<const unsigned char *> (pass.data ()), pass.size ());
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSigPrivate::set_private_key_pem (const std::string_view key)
	{
		::mbedtls_pk_free (&_enc_ctx);
		::mbedtls_pk_init (&_enc_ctx);

		const int ret = ::mbedtls_pk_parse_key (&_enc_ctx, reinterpret_cast<const unsigned char *> (s_c_str (key)), key.size () + 1, nullptr, 0);
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSigPrivate::set_private_key_der (const std::string_view key, const std::string_view pass)
	{
		::mbedtls_pk_free (&_enc_ctx);
		::mbedtls_pk_init (&_enc_ctx);

		const int ret = ::mbedtls_pk_parse_key (&_enc_ctx, reinterpret_cast<const unsigned char *> (key.data ()), key.size (),
			reinterpret_cast<const unsigned char *> (pass.data ()), pass.size ());
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSigPrivate::set_private_key_der (const std::string_view key)
	{
		::mbedtls_pk_free (&_enc_ctx);
		::mbedtls_pk_init (&_enc_ctx);

		const int ret = ::mbedtls_pk_parse_key (&_enc_ctx, reinterpret_cast<const unsigned char *> (key.data ()), key.size (), nullptr, 0);
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSigPrivate::load_private_key (const std::string_view fname, const std::string_view pass)
	{
		::mbedtls_pk_free (&_enc_ctx);
		::mbedtls_pk_init (&_enc_ctx);

		const int ret = ::mbedtls_pk_parse_keyfile (&_enc_ctx, s_c_str (fname), s_c_str (pass));
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSigPrivate::load_private_key (const std::string_view fname)
	{
		::mbedtls_pk_free (&_enc_ctx);
		::mbedtls_pk_init (&_enc_ctx);

		const int ret = ::mbedtls_pk_parse_keyfile (&_enc_ctx, s_c_str (fname), nullptr);
		check_error (ret);

		can_do (_enc_ctx);
	}

	void DiSigPrivate::save_keypair_pem (const std::string_view fname_priv, const std::string_view fname_pub)
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

	void DiSigPrivate::save_keypair_der (const std::string_view fname_priv, const std::string_view fname_pub)
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

	void DiSigPrivate::save_keypair_raw (const std::string_view fname_priv, const std::string_view fname_pub)
	{
		{
			const std::string buf = get_private_key_raw ();
			dump_to_file (fname_priv, buf);
		}
		{
			const std::string buf = get_public_key_raw ();
			dump_to_file (fname_pub, buf);
		}
	}

	std::string DiSigPrivate::get_private_key_pem (void)
	{
		::memset (_output_buf, 0, sizeof (_output_buf));
		const int ret = ::mbedtls_pk_write_key_pem (&_enc_ctx, _output_buf, sizeof (_output_buf) - 1);
		check_error (ret);

		return std::string (reinterpret_cast<const char *> (_output_buf));
	}

	std::string DiSigPrivate::get_private_key_der (void)
	{
		const int ret = ::mbedtls_pk_write_key_der (&_enc_ctx, _output_buf, sizeof (_output_buf));
		check_error (ret, true);

		return std::string (reinterpret_cast<const char *> (_output_buf) + sizeof (_output_buf) - ret, ret);
	}

	std::string DiSigPrivate::get_private_key_raw (void)
	{
		if (mbedtls_pk_get_type (&_enc_ctx) != MBEDTLS_PK_ECKEY) {
			cThrow ("DiSig error: Raw export is only supported for EC keys"sv);
		}
		const mbedtls_ecp_keypair *const ec = ::mbedtls_pk_ec (_enc_ctx);

		const size_t expected_len = ::mbedtls_mpi_size (&ec->grp.P);
		const size_t len = ::mbedtls_mpi_size (&ec->d);

		if (len != expected_len) {
			cThrow ("DiSig error: Output size does not match expected length"sv);
		}

		const int ret = ::mbedtls_mpi_write_binary (&ec->d, _output_buf, sizeof (_output_buf));
		check_error (ret);

		return std::string (reinterpret_cast<const char *> (_output_buf) + sizeof (_output_buf) - len, len);
	}

	std::string DiSigPrivate::get_public_key_pem (void)
	{
		::memset (_output_buf, 0, sizeof (_output_buf));
		const int ret = ::mbedtls_pk_write_pubkey_pem (&_enc_ctx, _output_buf, sizeof (_output_buf) - 1);
		check_error (ret);

		return std::string (reinterpret_cast<const char *> (_output_buf));
	}

	std::string DiSigPrivate::get_public_key_der (void)
	{
		const int ret = ::mbedtls_pk_write_pubkey_der (&_enc_ctx, _output_buf, sizeof (_output_buf));
		check_error (ret, true);

		return std::string (reinterpret_cast<const char *> (_output_buf) + sizeof (_output_buf) - ret, ret);
	}

	std::string DiSigPrivate::get_public_key_raw (void)
	{
		if (::mbedtls_pk_get_type (&_enc_ctx) != MBEDTLS_PK_ECKEY) {
			cThrow ("DiSig error: Raw export is only supported for EC keys"sv);
		}
		const mbedtls_ecp_keypair *const ec = ::mbedtls_pk_ec (_enc_ctx);

		const size_t expected_len = ::mbedtls_mpi_size (&ec->grp.P);

		std::string output;
		output.reserve (expected_len * 2);

		auto func = [&](const mbedtls_mpi *P) -> void {
			const size_t len = ::mbedtls_mpi_size (P);
			const int ret = ::mbedtls_mpi_write_binary (P, _output_buf, sizeof (_output_buf));
			check_error (ret);
			output.append (reinterpret_cast<const char *> (_output_buf) + sizeof (_output_buf) - len, len);
		};

		func (&ec->Q.X);
		func (&ec->Q.Y);

		if (output.size () != (expected_len * 2)) {
			cThrow ("DiSig error: Output size does not match expected length"sv);
		}

		return output;
	}

	std::string DiSigPrivate::sign (const mbedtls_md_type_t md_alg, const std::string_view hsh)
	{
		check_hash (md_alg, hsh);

		unsigned char sgn[MBEDTLS_MPI_MAX_SIZE];
		size_t sgn_len {0};
		const int ret =  ::mbedtls_pk_sign (&_enc_ctx, md_alg, reinterpret_cast<const unsigned char *> (hsh.data ()), hsh.size (), sgn, &sgn_len, _random_for_mbedtls, &_rng);
		check_error (ret);

		return std::string (reinterpret_cast<const char *> (sgn), sgn_len);
	}

	std::string DiSigPrivate::sign_raw (const mbedtls_md_type_t md_alg, const std::string_view hsh)
	{
		if (::mbedtls_pk_get_type (&_enc_ctx) != MBEDTLS_PK_ECKEY) {
			cThrow ("DiSig error: Raw export is only supported for EC keys"sv);
		}
		::mbedtls_ecp_keypair *const ec = ::mbedtls_pk_ec (_enc_ctx);

		mbedtls_mpi r;
		mbedtls_mpi s;

		::mbedtls_mpi_init (&r);
		::mbedtls_mpi_init (&s);

		try {
			{
				const int ret = ::mbedtls_ecdsa_sign_det (&(ec->grp), &r, &s, &(ec->d), reinterpret_cast<const unsigned char *> (hsh.data ()), hsh.size (), md_alg);
				check_error (ret);
			}

			const size_t expected_len = ::mbedtls_mpi_size (&ec->grp.P);

			std::string output;
			output.reserve (expected_len * 2);

			auto func = [&](const mbedtls_mpi *P) -> void {
				const size_t len = ::mbedtls_mpi_size (P);
				const int ret = ::mbedtls_mpi_write_binary (P, _output_buf, sizeof (_output_buf));
				check_error (ret);
				output.append (reinterpret_cast<const char *> (_output_buf) + sizeof (_output_buf) - len, len);
			};

			func (&r);
			func (&s);

			if (output.size () != (expected_len * 2)) {
				cThrow ("DiSig error: Output size does not match expected length"sv);
			}

			::mbedtls_mpi_free (&r);
			::mbedtls_mpi_free (&s);

			return output;
		}
		catch (...) {
			::mbedtls_mpi_free (&r);
			::mbedtls_mpi_free (&s);

			throw;
		}
	}
}

#endif // SHAGA_FULL

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

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int DiSigVerify::DEC_CTX_ENTRY::add_raw_key (const std::string_view key, const std::string_view curve_type)
	{
		int ret;

		const mbedtls_ecp_curve_info *curve_info = ::mbedtls_ecp_curve_info_from_name (s_c_str (curve_type));
		if (nullptr == curve_info) {
			cThrow ("DiSig error: Unknown curve '{}'"sv, curve_type);
		}

		ret = ::mbedtls_pk_setup (&_ctx, ::mbedtls_pk_info_from_type (MBEDTLS_PK_ECKEY));
		check_error (ret);

		mbedtls_ecp_keypair *ec = mbedtls_pk_ec (_ctx);

		ret = ::mbedtls_ecp_group_load (&ec->grp, curve_info->grp_id);
		check_error (ret);

		size_t pos = 0;
		const size_t len = ::mbedtls_mpi_size (&ec->grp.P);

		auto func = [&](mbedtls_mpi *P) -> void {
			if (pos + len > key.size ()) {
				cThrow ("DiSig error: Raw data too short"sv);
			}

			ret = ::mbedtls_mpi_read_binary (P, reinterpret_cast<const uint8_t *>(key.data () + pos), len);
			check_error (ret);

			pos += len;
		};

		func (&ec->Q.X);
		func (&ec->Q.Y);
		mbedtls_mpi_lset (&ec->Q.Z, 1);

		if (pos != key.size ()) {
			cThrow ("DiSig error: Extra raw data for selected curve type"sv);
		}

		return 0;
	}

	DiSigVerify::DEC_CTX_ENTRY::DEC_CTX_ENTRY (const std::string_view key, const std::string_view name, const DiSigVerify::_TYPE type, const std::string_view curve_type) try :
		_name (name)
	{
		::mbedtls_pk_init  (&_ctx);

		int ret = (-1);

		switch (type) {
			case DiSigVerify::_TYPE::PEM:
				ret = ::mbedtls_pk_parse_public_key (&_ctx, reinterpret_cast<const unsigned char *> (s_c_str (key)), key.size () + 1);
				break;

			case DiSigVerify::_TYPE::DER:
				ret = ::mbedtls_pk_parse_public_key (&_ctx, reinterpret_cast<const unsigned char *> (key.data ()), key.size ());
				break;

			case DiSigVerify::_TYPE::RAW:
				ret = add_raw_key (key, curve_type);
				break;
		}

		DiSigVerify::check_error (ret);
		DiSigVerify::can_do (_ctx);
	}
	catch (...) {
		::mbedtls_pk_free (&_ctx);
	}

	DiSigVerify::DEC_CTX_ENTRY::DEC_CTX_ENTRY (const std::string_view key, const std::string_view name, const DiSigVerify::_TYPE type) :
		DEC_CTX_ENTRY (key, name, type, ""sv)
	{ }

	DiSigVerify::DEC_CTX_ENTRY::~DEC_CTX_ENTRY ()
	{
		::mbedtls_pk_free (&_ctx);
	}

	DiSigVerify::RAW_SIGNATURE_ENTRY::RAW_SIGNATURE_ENTRY (const std::string_view curve_type, const std::string_view sgn_raw)
	{
		int ret;

		const mbedtls_ecp_curve_info *curve_info = ::mbedtls_ecp_curve_info_from_name (s_c_str (curve_type));
		if (nullptr == curve_info) {
			cThrow ("DiSig error: Unknown curve '{}'"sv, curve_type);
		}

		::mbedtls_pk_init (&_ctx);

		try {
			ret = ::mbedtls_pk_setup (&_ctx, ::mbedtls_pk_info_from_type (MBEDTLS_PK_ECKEY));
			check_error (ret);

			ec = ::mbedtls_pk_ec (_ctx);

			ret = ::mbedtls_ecp_group_load (&ec->grp, curve_info->grp_id);
			check_error (ret);

			size_t pos = 0;
			const size_t len = ::mbedtls_mpi_size (&ec->grp.P);

			auto func = [&](mbedtls_mpi *P) -> void {
				if ((pos + len) > sgn_raw.size ()) {
					cThrow ("DiSig error: Raw data too short"sv);
				}

				ret = ::mbedtls_mpi_read_binary (P, reinterpret_cast<const uint8_t *>(sgn_raw.data () + pos), len);
				check_error (ret);

				pos += len;
			};

			/* We are using Q.X and Q.Y as r and s for signature! */
			func (&ec->Q.X);
			func (&ec->Q.Y);

			if (pos != sgn_raw.size ()) {
				cThrow ("DiSig error: Extra raw data for selected curve type"sv);
			}
		}
		catch (...) {
			ec = nullptr;
			::mbedtls_pk_free (&_ctx);
			throw;
		}
	}

	DiSigVerify::RAW_SIGNATURE_ENTRY::~RAW_SIGNATURE_ENTRY ()
	{
		ec = nullptr;
		::mbedtls_pk_free (&_ctx);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DiSigVerify::DiSigVerify ()
	{
	}

	DiSigVerify::~DiSigVerify ()
	{
		_dec_ctx_list.clear ();
	}

	std::string_view DiSigVerify::get_curve_type (const size_t key_id) const
	{
		return get_name (_dec_ctx_list.at (key_id)._ctx);
	}

	void DiSigVerify::add_public_key_pem (const std::string_view key, const std::string_view name)
	{
		_dec_ctx_list.emplace_back (key, name, _TYPE::PEM);
	}

	void DiSigVerify::add_public_key_pem (const std::string_view key)
	{
		const std::string name = "pubkey"s + STR::from_int (_dec_ctx_list.size ());
		_dec_ctx_list.emplace_back (key, name, _TYPE::PEM);
	}

	void DiSigVerify::add_public_key_der (const std::string_view key, const std::string_view name)
	{
		_dec_ctx_list.emplace_back (key, name, _TYPE::DER);
	}

	void DiSigVerify::add_public_key_der (const std::string_view key)
	{
		const std::string name = "pubkey"s + STR::from_int (_dec_ctx_list.size ());
		_dec_ctx_list.emplace_back (key, name, _TYPE::DER);
	}

	void DiSigVerify::add_public_key_raw (const std::string_view curve_type, const std::string_view key, const std::string_view name)
	{
		_dec_ctx_list.emplace_back (key, name, _TYPE::RAW, curve_type);
	}

	void DiSigVerify::add_public_key_raw (const std::string_view curve_type, const std::string_view key)
	{
		const std::string name = "pubkey"s + STR::from_int (_dec_ctx_list.size ());
		_dec_ctx_list.emplace_back (key, name, _TYPE::RAW, curve_type);
	}

	void DiSigVerify::clear_keys (void)
	{
		_dec_ctx_list.clear ();
	}

	bool DiSigVerify::verify (const mbedtls_md_type_t md_alg, const std::string_view hsh, const std::string_view sgn, std::string &key_name_out)
	{
		check_hash (md_alg, hsh);

		for (DEC_CTX_ENTRY &entry : _dec_ctx_list) {
			const int ret = ::mbedtls_pk_verify (
				&(entry._ctx),
				md_alg,
				reinterpret_cast<const unsigned char *> (hsh.data ()),
				hsh.size (),
				reinterpret_cast<const unsigned char *> (sgn.data ()),
				sgn.size ());

			if (0 == ret) {
				key_name_out.assign (entry._name);
				return true;
			}
		}

		return false;
	}

	bool DiSigVerify::verify (const mbedtls_md_type_t md_alg, const std::string_view hsh, const std::string_view sgn)
	{
		check_hash (md_alg, hsh);

		/* TODO: This can be run in parallel when available */
		return std::any_of (_dec_ctx_list.begin (), _dec_ctx_list.end (), [&md_alg, &hsh, &sgn](DEC_CTX_ENTRY &entry) -> bool {
			const int ret = ::mbedtls_pk_verify (
				&(entry._ctx),
				md_alg,
				reinterpret_cast<const unsigned char *> (hsh.data ()),
				hsh.size (),
				reinterpret_cast<const unsigned char *> (sgn.data ()),
				sgn.size ());

			return (0 == ret);
		});
	}

	bool DiSigVerify::verify_raw (const std::string_view curve_type, const mbedtls_md_type_t md_alg, const std::string_view hsh, const std::string_view sgn_raw, std::string &key_name_out)
	{
		check_hash (md_alg, hsh);

		RAW_SIGNATURE_ENTRY sgn (curve_type, sgn_raw);

		for (DEC_CTX_ENTRY &entry : _dec_ctx_list) {
			if (mbedtls_pk_get_type (&entry._ctx) != MBEDTLS_PK_ECKEY) {
				/* This key is not suitable for ECDSA */
				return false;
			}
			mbedtls_ecp_keypair *ec = ::mbedtls_pk_ec (entry._ctx);
			const int ret = ::mbedtls_ecdsa_verify (
				&(ec->grp),
				reinterpret_cast<const unsigned char *> (hsh.data ()),
				hsh.size (),
				&(ec->Q),
				&sgn.ec->Q.X,
				&sgn.ec->Q.Y);

			if (0 == ret) {
				key_name_out.assign (entry._name);
				return true;
			}
		}

		return false;
	}

	bool DiSigVerify::verify_raw (const std::string_view curve_type, const mbedtls_md_type_t md_alg, const std::string_view hsh, const std::string_view sgn_raw)
	{
		check_hash (md_alg, hsh);

		RAW_SIGNATURE_ENTRY sgn (curve_type, sgn_raw);

		/* TODO: This can be run in parallel when available */
		return std::any_of (_dec_ctx_list.begin (), _dec_ctx_list.end (), [&md_alg, &hsh, &sgn](DEC_CTX_ENTRY &entry) -> bool {
			if (::mbedtls_pk_get_type (&entry._ctx) != MBEDTLS_PK_ECKEY) {
				/* This key is not suitable for ECDSA */
				return false;
			}
			mbedtls_ecp_keypair *ec = ::mbedtls_pk_ec (entry._ctx);
			const int ret = ::mbedtls_ecdsa_verify (
				&(ec->grp),
				reinterpret_cast<const unsigned char *> (hsh.data ()),
				hsh.size (),
				&(ec->Q),
				&sgn.ec->Q.X,
				&sgn.ec->Q.Y);

			return (0 == ret);
		});
	}
}

#endif // SHAGA_FULL

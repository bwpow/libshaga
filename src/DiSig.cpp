/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifdef SHAGA_FULL

namespace shaga {
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void DiSigCommon::check_error (const int err, const bool allow_positive)
	{
		if (true == allow_positive ? err <= 0 : err != 0) {
			char buf[256];
			::memset (buf, 0, sizeof (buf));
			::mbedtls_strerror (err, buf, sizeof (buf) - 1);
			cThrow ("DiSig error: {}"sv, buf);
		}
	}

	void DiSigCommon::can_do (const mbedtls_pk_context &ctx)
	{
		if (::mbedtls_pk_can_do (&ctx, MBEDTLS_PK_ECDSA) != 1) {
			cThrow ("DiSig error: Key is not suitable for ECDSA"sv);
		}
	}

	SHAGA_STRV std::string_view DiSigCommon::get_name (const mbedtls_pk_context &ctx)
	{
		/* mbedtls_ecp_curve_info points to static memory, it is safe to return string_view */
		can_do (ctx);
		mbedtls_ecp_keypair *const ec = ::mbedtls_pk_ec (ctx);
		if (nullptr == ec) {
			cThrow ("DiSig error: null error"sv);
		}

		const mbedtls_ecp_group *const grp = &(ec->grp);
		if (nullptr == grp) {
			cThrow ("DiSig error: null error"sv);
		}

		const mbedtls_ecp_curve_info *const info = ::mbedtls_ecp_curve_info_from_grp_id (grp->id);
		if (nullptr == info) {
			cThrow ("DiSig error: null error"sv);
		}

		return std::string_view (info->name);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Protected class methods  ////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void DiSigCommon::dump_to_file (const std::string_view fname, const void *const buf) const
	{
		ShFile file (fname, ShFile::mWRITE | ShFile::mTRUNC);
		file.write (buf);
	}

	void DiSigCommon::dump_to_file (const std::string_view fname, const void *const buf, const size_t len) const
	{
		ShFile file (fname, ShFile::mWRITE | ShFile::mTRUNC);
		file.write (buf, len);
	}

	void DiSigCommon::dump_to_file (const std::string_view fname, const std::string_view buf) const
	{
		ShFile file (fname, ShFile::mWRITE | ShFile::mTRUNC);
		file.write (buf);
	}

	void DiSigCommon::check_hash (const mbedtls_md_type_t md_alg, const std::string_view hsh) const
	{
		const mbedtls_md_info_t *info = ::mbedtls_md_info_from_type (md_alg);
		if (nullptr == info) {
			cThrow ("DiSig error: Unknown hash type"sv);
		}

		if (info->size != static_cast<int> (hsh.size ())) {
			cThrow ("DiSig error: Hash {} expects {} bytes but {} bytes provided"sv, info->name, info->size, hsh.size ());
		}
	}
}

#endif // SHAGA_FULL

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

	void DiSigCommon::check_error (const int err, const bool allow_positive)
	{
		if (true == allow_positive ? err <= 0 : err != 0) {
			char buf[256];
			::memset (buf, 0, sizeof (buf));
			::mbedtls_strerror (err, buf, sizeof (buf) - 1);
			cThrow ("DiSig error: {}"sv, buf);
		}
	}

	void DiSigCommon::can_do (mbedtls_pk_context &ctx)
	{
		if (::mbedtls_pk_can_do (&ctx, MBEDTLS_PK_ECDSA) != 1) {
			cThrow ("DiSig error: Key is not suitable for ECDSA"sv);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Protected class methods  ////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void DiSigCommon::dump_to_file (const std::string_view fname, const unsigned char *buf) const
	{
		ShFile file (fname, ShFile::mWRITE | ShFile::mTRUNC);
		file.write (reinterpret_cast<const char *> (buf));
	}

	void DiSigCommon::dump_to_file (const std::string_view fname, const unsigned char *buf, const size_t len) const
	{
		ShFile file (fname, ShFile::mWRITE | ShFile::mTRUNC);
		file.write (reinterpret_cast<const char *> (buf), len);
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

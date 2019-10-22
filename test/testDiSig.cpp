/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

/* Private key using NIST P-256 in der format */
static const unsigned char _secp256r1_priv[] = {
  0x30, 0x78, 0x02, 0x01, 0x01, 0x04, 0x21, 0x00, 0x83, 0xad, 0xea, 0x67,
  0x65, 0x2e, 0x92, 0xb0, 0xe8, 0x21, 0xb0, 0xea, 0xd8, 0x04, 0x42, 0x94,
  0x8b, 0xc5, 0x41, 0x17, 0xd4, 0x34, 0xe5, 0x2f, 0x94, 0x71, 0x16, 0xd3,
  0x4f, 0x1f, 0x1d, 0x95, 0xa0, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce,
  0x3d, 0x03, 0x01, 0x07, 0xa1, 0x44, 0x03, 0x42, 0x00, 0x04, 0xfc, 0x01,
  0xf3, 0x28, 0xaa, 0xed, 0x45, 0x50, 0x06, 0xf3, 0xbf, 0xff, 0xcf, 0x9f,
  0xba, 0x78, 0xdd, 0xbf, 0xb7, 0x33, 0x14, 0xa2, 0xe1, 0xc1, 0x80, 0x4e,
  0xd9, 0xfa, 0xdb, 0x17, 0x59, 0x4b, 0xf7, 0x2b, 0xca, 0x3d, 0x08, 0x91,
  0x8e, 0xa1, 0x22, 0xe8, 0x4a, 0x30, 0x9a, 0xa6, 0x83, 0xc6, 0x2a, 0x11,
  0x18, 0xcb, 0x51, 0x8c, 0x17, 0x05, 0x7e, 0xf2, 0x78, 0xa1, 0xe5, 0x90,
  0xab, 0x01
};
static const unsigned int _secp256r1_priv_len = 122;

/* Private key using NIST P-256 in raw (parameter d) format */
static const unsigned char _secp256r1_priv_raw[] = {
  0x83, 0xad, 0xea, 0x67, 0x65, 0x2e, 0x92, 0xb0, 0xe8, 0x21, 0xb0, 0xea,
  0xd8, 0x04, 0x42, 0x94, 0x8b, 0xc5, 0x41, 0x17, 0xd4, 0x34, 0xe5, 0x2f,
  0x94, 0x71, 0x16, 0xd3, 0x4f, 0x1f, 0x1d, 0x95
};
static const unsigned int _secp256r1_priv_raw_len = 32;

/* Public key using NIST P-256 in der format */
static const unsigned char _secp256r1_pub[] = {
  0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
  0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03,
  0x42, 0x00, 0x04, 0xfc, 0x01, 0xf3, 0x28, 0xaa, 0xed, 0x45, 0x50, 0x06,
  0xf3, 0xbf, 0xff, 0xcf, 0x9f, 0xba, 0x78, 0xdd, 0xbf, 0xb7, 0x33, 0x14,
  0xa2, 0xe1, 0xc1, 0x80, 0x4e, 0xd9, 0xfa, 0xdb, 0x17, 0x59, 0x4b, 0xf7,
  0x2b, 0xca, 0x3d, 0x08, 0x91, 0x8e, 0xa1, 0x22, 0xe8, 0x4a, 0x30, 0x9a,
  0xa6, 0x83, 0xc6, 0x2a, 0x11, 0x18, 0xcb, 0x51, 0x8c, 0x17, 0x05, 0x7e,
  0xf2, 0x78, 0xa1, 0xe5, 0x90, 0xab, 0x01
};
static const unsigned int _secp256r1_pub_len = 91;

/* Public key using NIST P-256 in raw (parameters X and Y) format */
static const unsigned char _secp256r1_pub_raw[] = {
  0xfc, 0x01, 0xf3, 0x28, 0xaa, 0xed, 0x45, 0x50, 0x06, 0xf3, 0xbf, 0xff,
  0xcf, 0x9f, 0xba, 0x78, 0xdd, 0xbf, 0xb7, 0x33, 0x14, 0xa2, 0xe1, 0xc1,
  0x80, 0x4e, 0xd9, 0xfa, 0xdb, 0x17, 0x59, 0x4b, 0xf7, 0x2b, 0xca, 0x3d,
  0x08, 0x91, 0x8e, 0xa1, 0x22, 0xe8, 0x4a, 0x30, 0x9a, 0xa6, 0x83, 0xc6,
  0x2a, 0x11, 0x18, 0xcb, 0x51, 0x8c, 0x17, 0x05, 0x7e, 0xf2, 0x78, 0xa1,
  0xe5, 0x90, 0xab, 0x01
};
static const unsigned int _secp256r1_pub_raw_len = 64;


TEST (DiSig, sign_raw)
{
	/* Private key for signature using NIST P-256 in PEM format */
	const constexpr std::string_view privkey_pem =
	"-----BEGIN EC PRIVATE KEY-----\n"
	"MHgCAQEEIQDu8/a3fnHmJntEYKvRsmWWS6YKG1ztE3GaLORB9nWcTaAKBggqhkjO\n"
	"PQMBB6FEA0IABIfNjQbUACbt/xMGK+RMA9/jdHMtHBjTHQP0it+9ad/KdsYSQPQC\n"
	"VTs/OlxpRCpgG4k9HYTKqPmBwRFBM/PsCes=\n"
	"-----END EC PRIVATE KEY-----\n"sv;

	const constexpr std::string_view pubkey_pem =
	"-----BEGIN PUBLIC KEY-----\n"
	"MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEh82NBtQAJu3/EwYr5EwD3+N0cy0c\n"
	"GNMdA/SK371p38p2xhJA9AJVOz86XGlEKmAbiT0dhMqo+YHBEUEz8+wJ6w==\n"
	"-----END PUBLIC KEY-----\n"sv;

	const constexpr std::string_view curve_type = "secp256r1"sv;

	/* Signature using NIST P-256 in raw (parameters r and s) format */
	/* Source hash is 32 bytes of 0x59 */
	const uint8_t signature_raw[64] = {
		0xac, 0x85, 0x97, 0x6c, 0x61, 0x19, 0xba, 0x60, 0xdf, 0x8e, 0x6d, 0x4d, 0x68, 0x45, 0xad, 0x5d,
		0x7e, 0x96, 0xe1, 0xff, 0x41, 0xbe, 0x54, 0x2a, 0xc9, 0x2e, 0x79, 0xa4, 0x23, 0x74, 0x20, 0xd4,
		0x89, 0x0d, 0x6f, 0xa9, 0x22, 0xf4, 0x6d, 0xd9, 0xba, 0x91, 0x3a, 0x8a, 0x98, 0x8e, 0x55, 0x59,
		0x3e, 0x8c, 0xe2, 0x4a, 0x48, 0x12, 0x51, 0xcb, 0xcd, 0xad, 0x0c, 0x64, 0x8b, 0x88, 0xf7, 0x91
	};

	const std::string_view signature_raw_original (reinterpret_cast<const char *> (signature_raw), sizeof (signature_raw));

	DiSigPrivate priv;
	priv.set_private_key_pem (privkey_pem);

	DiSigVerify pub;
	pub.add_public_key_pem (pubkey_pem);

	EXPECT_TRUE (priv.get_curve_type () == curve_type);
	EXPECT_TRUE (pub.get_curve_type (0) == curve_type);
	EXPECT_THROW (pub.get_curve_type (1), std::exception);

	const std::string digest (32, 0x59);
	const std::string signature_raw_current = priv.sign_raw (MBEDTLS_MD_SHA256, digest);

	ASSERT_TRUE (signature_raw_current.size () == signature_raw_original.size ());

	EXPECT_TRUE (pub.verify_raw (curve_type, MBEDTLS_MD_SHA256, digest, signature_raw_original));
	EXPECT_TRUE (pub.verify_raw (curve_type, MBEDTLS_MD_SHA256, digest, signature_raw_current));

	std::string name1, name2;
	EXPECT_TRUE (pub.verify_raw (curve_type, MBEDTLS_MD_SHA256, digest, signature_raw_original, name1));
	EXPECT_TRUE (pub.verify_raw (curve_type, MBEDTLS_MD_SHA256, digest, signature_raw_current, name2));

	EXPECT_TRUE (name1 == name2);
	EXPECT_FALSE (name1.empty ());
}

TEST (DiSig, raw)
{
	const std::string ref_priv = std::string (reinterpret_cast<const char *> (_secp256r1_priv), _secp256r1_priv_len);
	const std::string ref_priv_raw = std::string (reinterpret_cast<const char *> (_secp256r1_priv_raw), _secp256r1_priv_raw_len);
	const std::string ref_pub = std::string (reinterpret_cast<const char *> (_secp256r1_pub), _secp256r1_pub_len);
	const std::string ref_pub_raw = std::string (reinterpret_cast<const char *> (_secp256r1_pub_raw), _secp256r1_pub_raw_len);

	const std::string_view curve_type ("secp256r1"sv);

	std::string digest (32, '\0');
	std::string signature;

	/* Fill digest with some data */
	std::iota (digest.begin (), digest.end (), 0x33);

	/* Try import DER private key and export other types of key */
	{
		DiSigPrivate disig;
		disig.set_private_key_der (ref_priv);

		const std::string pub = disig.get_public_key_der ();
		EXPECT_TRUE (pub.compare (ref_pub) == 0);

		const std::string priv_raw = disig.get_private_key_raw ();
		EXPECT_TRUE (priv_raw.compare (ref_priv_raw) == 0);

		const std::string pub_raw = disig.get_public_key_raw ();
		EXPECT_TRUE (pub_raw.compare (ref_pub_raw) == 0);

		signature = disig.sign (MBEDTLS_MD_SHA256, digest);
	}

	/* Import raw pubkey and try verify */
	{
		DiSigVerify disig;
		disig.add_public_key_raw (curve_type, ref_pub_raw);

		EXPECT_TRUE (disig.get_curve_type (0) == curve_type);
		EXPECT_THROW (disig.get_curve_type (1), std::exception);

		std::string name;
		EXPECT_TRUE (disig.verify (MBEDTLS_MD_SHA256, digest, signature, name));
		EXPECT_FALSE (name.empty ());

		EXPECT_TRUE (disig.verify (MBEDTLS_MD_SHA256, digest, signature));
	}
}

TEST (DiSig, genkey)
{
	std::string name;
	const std::string hsh_ok = CRC::sha256 ("test string"sv);
	const std::string hsh_other = CRC::sha256 ("different test string"sv);
	const std::string hsh_bad = "abcd"s;

	DiSigPrivate priv;
	DiSigVerify pub;
	COMMON_VECTOR vec;

	const mbedtls_ecp_curve_info *curve_info;
	for (curve_info = mbedtls_ecp_curve_list(); curve_info->grp_id != MBEDTLS_ECP_DP_NONE; curve_info++) {
		{
			priv.generate_new_keypair (curve_info->name);
			pub.add_public_key_pem (priv.get_public_key_pem ());

			std::string test = priv.sign (MBEDTLS_MD_SHA256, hsh_ok);
			EXPECT_THROW (priv.sign (MBEDTLS_MD_SHA256, hsh_bad), CommonException);

			vec.push_back (test);
		}

		{
			priv.generate_new_keypair (curve_info->name);
			pub.add_public_key_der (priv.get_public_key_der ());

			std::string test = priv.sign (MBEDTLS_MD_SHA256, hsh_ok);
			EXPECT_THROW (priv.sign (MBEDTLS_MD_SHA256, hsh_bad), CommonException);

			vec.push_back (test);
		}
	}

	for (const std::string &test: vec) {
		EXPECT_THROW (pub.verify (MBEDTLS_MD_SHA256, hsh_bad, test), CommonException);
		EXPECT_TRUE (pub.verify (MBEDTLS_MD_SHA256, hsh_ok, test));
		EXPECT_FALSE (pub.verify (MBEDTLS_MD_SHA256, hsh_other, test));

		EXPECT_THROW (pub.verify (MBEDTLS_MD_SHA256, hsh_bad, test, name), CommonException);
		EXPECT_TRUE (pub.verify (MBEDTLS_MD_SHA256, hsh_ok, test, name));
		EXPECT_FALSE (pub.verify (MBEDTLS_MD_SHA256, hsh_other, test, name));
	}

	EXPECT_FALSE (pub.verify (MBEDTLS_MD_SHA256, hsh_ok, hsh_bad));
	EXPECT_FALSE (pub.verify (MBEDTLS_MD_SHA256, hsh_other, hsh_bad));
	EXPECT_FALSE (pub.verify (MBEDTLS_MD_SHA256, hsh_ok, hsh_other));

	EXPECT_FALSE (pub.verify (MBEDTLS_MD_SHA256, hsh_ok, hsh_bad, name));
	EXPECT_FALSE (pub.verify (MBEDTLS_MD_SHA256, hsh_other, hsh_bad, name));
	EXPECT_FALSE (pub.verify (MBEDTLS_MD_SHA256, hsh_ok, hsh_other, name));
}

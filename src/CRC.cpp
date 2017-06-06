/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifdef SHAGA_FULL
	#include <mbedtls/sha256.h>
	#include <mbedtls/sha512.h>
#endif // SHAGA_FULL

namespace shaga {

	extern const uint_fast64_t _crc64_table[256] = {
		UINT64_C(0x0000000000000000), UINT64_C(0x7ad870c830358979),
		UINT64_C(0xf5b0e190606b12f2), UINT64_C(0x8f689158505e9b8b),
		UINT64_C(0xc038e5739841b68f), UINT64_C(0xbae095bba8743ff6),
		UINT64_C(0x358804e3f82aa47d), UINT64_C(0x4f50742bc81f2d04),
		UINT64_C(0xab28ecb46814fe75), UINT64_C(0xd1f09c7c5821770c),
		UINT64_C(0x5e980d24087fec87), UINT64_C(0x24407dec384a65fe),
		UINT64_C(0x6b1009c7f05548fa), UINT64_C(0x11c8790fc060c183),
		UINT64_C(0x9ea0e857903e5a08), UINT64_C(0xe478989fa00bd371),
		UINT64_C(0x7d08ff3b88be6f81), UINT64_C(0x07d08ff3b88be6f8),
		UINT64_C(0x88b81eabe8d57d73), UINT64_C(0xf2606e63d8e0f40a),
		UINT64_C(0xbd301a4810ffd90e), UINT64_C(0xc7e86a8020ca5077),
		UINT64_C(0x4880fbd87094cbfc), UINT64_C(0x32588b1040a14285),
		UINT64_C(0xd620138fe0aa91f4), UINT64_C(0xacf86347d09f188d),
		UINT64_C(0x2390f21f80c18306), UINT64_C(0x594882d7b0f40a7f),
		UINT64_C(0x1618f6fc78eb277b), UINT64_C(0x6cc0863448deae02),
		UINT64_C(0xe3a8176c18803589), UINT64_C(0x997067a428b5bcf0),
		UINT64_C(0xfa11fe77117cdf02), UINT64_C(0x80c98ebf2149567b),
		UINT64_C(0x0fa11fe77117cdf0), UINT64_C(0x75796f2f41224489),
		UINT64_C(0x3a291b04893d698d), UINT64_C(0x40f16bccb908e0f4),
		UINT64_C(0xcf99fa94e9567b7f), UINT64_C(0xb5418a5cd963f206),
		UINT64_C(0x513912c379682177), UINT64_C(0x2be1620b495da80e),
		UINT64_C(0xa489f35319033385), UINT64_C(0xde51839b2936bafc),
		UINT64_C(0x9101f7b0e12997f8), UINT64_C(0xebd98778d11c1e81),
		UINT64_C(0x64b116208142850a), UINT64_C(0x1e6966e8b1770c73),
		UINT64_C(0x8719014c99c2b083), UINT64_C(0xfdc17184a9f739fa),
		UINT64_C(0x72a9e0dcf9a9a271), UINT64_C(0x08719014c99c2b08),
		UINT64_C(0x4721e43f0183060c), UINT64_C(0x3df994f731b68f75),
		UINT64_C(0xb29105af61e814fe), UINT64_C(0xc849756751dd9d87),
		UINT64_C(0x2c31edf8f1d64ef6), UINT64_C(0x56e99d30c1e3c78f),
		UINT64_C(0xd9810c6891bd5c04), UINT64_C(0xa3597ca0a188d57d),
		UINT64_C(0xec09088b6997f879), UINT64_C(0x96d1784359a27100),
		UINT64_C(0x19b9e91b09fcea8b), UINT64_C(0x636199d339c963f2),
		UINT64_C(0xdf7adabd7a6e2d6f), UINT64_C(0xa5a2aa754a5ba416),
		UINT64_C(0x2aca3b2d1a053f9d), UINT64_C(0x50124be52a30b6e4),
		UINT64_C(0x1f423fcee22f9be0), UINT64_C(0x659a4f06d21a1299),
		UINT64_C(0xeaf2de5e82448912), UINT64_C(0x902aae96b271006b),
		UINT64_C(0x74523609127ad31a), UINT64_C(0x0e8a46c1224f5a63),
		UINT64_C(0x81e2d7997211c1e8), UINT64_C(0xfb3aa75142244891),
		UINT64_C(0xb46ad37a8a3b6595), UINT64_C(0xceb2a3b2ba0eecec),
		UINT64_C(0x41da32eaea507767), UINT64_C(0x3b024222da65fe1e),
		UINT64_C(0xa2722586f2d042ee), UINT64_C(0xd8aa554ec2e5cb97),
		UINT64_C(0x57c2c41692bb501c), UINT64_C(0x2d1ab4dea28ed965),
		UINT64_C(0x624ac0f56a91f461), UINT64_C(0x1892b03d5aa47d18),
		UINT64_C(0x97fa21650afae693), UINT64_C(0xed2251ad3acf6fea),
		UINT64_C(0x095ac9329ac4bc9b), UINT64_C(0x7382b9faaaf135e2),
		UINT64_C(0xfcea28a2faafae69), UINT64_C(0x8632586aca9a2710),
		UINT64_C(0xc9622c4102850a14), UINT64_C(0xb3ba5c8932b0836d),
		UINT64_C(0x3cd2cdd162ee18e6), UINT64_C(0x460abd1952db919f),
		UINT64_C(0x256b24ca6b12f26d), UINT64_C(0x5fb354025b277b14),
		UINT64_C(0xd0dbc55a0b79e09f), UINT64_C(0xaa03b5923b4c69e6),
		UINT64_C(0xe553c1b9f35344e2), UINT64_C(0x9f8bb171c366cd9b),
		UINT64_C(0x10e3202993385610), UINT64_C(0x6a3b50e1a30ddf69),
		UINT64_C(0x8e43c87e03060c18), UINT64_C(0xf49bb8b633338561),
		UINT64_C(0x7bf329ee636d1eea), UINT64_C(0x012b592653589793),
		UINT64_C(0x4e7b2d0d9b47ba97), UINT64_C(0x34a35dc5ab7233ee),
		UINT64_C(0xbbcbcc9dfb2ca865), UINT64_C(0xc113bc55cb19211c),
		UINT64_C(0x5863dbf1e3ac9dec), UINT64_C(0x22bbab39d3991495),
		UINT64_C(0xadd33a6183c78f1e), UINT64_C(0xd70b4aa9b3f20667),
		UINT64_C(0x985b3e827bed2b63), UINT64_C(0xe2834e4a4bd8a21a),
		UINT64_C(0x6debdf121b863991), UINT64_C(0x1733afda2bb3b0e8),
		UINT64_C(0xf34b37458bb86399), UINT64_C(0x8993478dbb8deae0),
		UINT64_C(0x06fbd6d5ebd3716b), UINT64_C(0x7c23a61ddbe6f812),
		UINT64_C(0x3373d23613f9d516), UINT64_C(0x49aba2fe23cc5c6f),
		UINT64_C(0xc6c333a67392c7e4), UINT64_C(0xbc1b436e43a74e9d),
		UINT64_C(0x95ac9329ac4bc9b5), UINT64_C(0xef74e3e19c7e40cc),
		UINT64_C(0x601c72b9cc20db47), UINT64_C(0x1ac40271fc15523e),
		UINT64_C(0x5594765a340a7f3a), UINT64_C(0x2f4c0692043ff643),
		UINT64_C(0xa02497ca54616dc8), UINT64_C(0xdafce7026454e4b1),
		UINT64_C(0x3e847f9dc45f37c0), UINT64_C(0x445c0f55f46abeb9),
		UINT64_C(0xcb349e0da4342532), UINT64_C(0xb1eceec59401ac4b),
		UINT64_C(0xfebc9aee5c1e814f), UINT64_C(0x8464ea266c2b0836),
		UINT64_C(0x0b0c7b7e3c7593bd), UINT64_C(0x71d40bb60c401ac4),
		UINT64_C(0xe8a46c1224f5a634), UINT64_C(0x927c1cda14c02f4d),
		UINT64_C(0x1d148d82449eb4c6), UINT64_C(0x67ccfd4a74ab3dbf),
		UINT64_C(0x289c8961bcb410bb), UINT64_C(0x5244f9a98c8199c2),
		UINT64_C(0xdd2c68f1dcdf0249), UINT64_C(0xa7f41839ecea8b30),
		UINT64_C(0x438c80a64ce15841), UINT64_C(0x3954f06e7cd4d138),
		UINT64_C(0xb63c61362c8a4ab3), UINT64_C(0xcce411fe1cbfc3ca),
		UINT64_C(0x83b465d5d4a0eece), UINT64_C(0xf96c151de49567b7),
		UINT64_C(0x76048445b4cbfc3c), UINT64_C(0x0cdcf48d84fe7545),
		UINT64_C(0x6fbd6d5ebd3716b7), UINT64_C(0x15651d968d029fce),
		UINT64_C(0x9a0d8ccedd5c0445), UINT64_C(0xe0d5fc06ed698d3c),
		UINT64_C(0xaf85882d2576a038), UINT64_C(0xd55df8e515432941),
		UINT64_C(0x5a3569bd451db2ca), UINT64_C(0x20ed197575283bb3),
		UINT64_C(0xc49581ead523e8c2), UINT64_C(0xbe4df122e51661bb),
		UINT64_C(0x3125607ab548fa30), UINT64_C(0x4bfd10b2857d7349),
		UINT64_C(0x04ad64994d625e4d), UINT64_C(0x7e7514517d57d734),
		UINT64_C(0xf11d85092d094cbf), UINT64_C(0x8bc5f5c11d3cc5c6),
		UINT64_C(0x12b5926535897936), UINT64_C(0x686de2ad05bcf04f),
		UINT64_C(0xe70573f555e26bc4), UINT64_C(0x9ddd033d65d7e2bd),
		UINT64_C(0xd28d7716adc8cfb9), UINT64_C(0xa85507de9dfd46c0),
		UINT64_C(0x273d9686cda3dd4b), UINT64_C(0x5de5e64efd965432),
		UINT64_C(0xb99d7ed15d9d8743), UINT64_C(0xc3450e196da80e3a),
		UINT64_C(0x4c2d9f413df695b1), UINT64_C(0x36f5ef890dc31cc8),
		UINT64_C(0x79a59ba2c5dc31cc), UINT64_C(0x037deb6af5e9b8b5),
		UINT64_C(0x8c157a32a5b7233e), UINT64_C(0xf6cd0afa9582aa47),
		UINT64_C(0x4ad64994d625e4da), UINT64_C(0x300e395ce6106da3),
		UINT64_C(0xbf66a804b64ef628), UINT64_C(0xc5bed8cc867b7f51),
		UINT64_C(0x8aeeace74e645255), UINT64_C(0xf036dc2f7e51db2c),
		UINT64_C(0x7f5e4d772e0f40a7), UINT64_C(0x05863dbf1e3ac9de),
		UINT64_C(0xe1fea520be311aaf), UINT64_C(0x9b26d5e88e0493d6),
		UINT64_C(0x144e44b0de5a085d), UINT64_C(0x6e963478ee6f8124),
		UINT64_C(0x21c640532670ac20), UINT64_C(0x5b1e309b16452559),
		UINT64_C(0xd476a1c3461bbed2), UINT64_C(0xaeaed10b762e37ab),
		UINT64_C(0x37deb6af5e9b8b5b), UINT64_C(0x4d06c6676eae0222),
		UINT64_C(0xc26e573f3ef099a9), UINT64_C(0xb8b627f70ec510d0),
		UINT64_C(0xf7e653dcc6da3dd4), UINT64_C(0x8d3e2314f6efb4ad),
		UINT64_C(0x0256b24ca6b12f26), UINT64_C(0x788ec2849684a65f),
		UINT64_C(0x9cf65a1b368f752e), UINT64_C(0xe62e2ad306bafc57),
		UINT64_C(0x6946bb8b56e467dc), UINT64_C(0x139ecb4366d1eea5),
		UINT64_C(0x5ccebf68aecec3a1), UINT64_C(0x2616cfa09efb4ad8),
		UINT64_C(0xa97e5ef8cea5d153), UINT64_C(0xd3a62e30fe90582a),
		UINT64_C(0xb0c7b7e3c7593bd8), UINT64_C(0xca1fc72bf76cb2a1),
		UINT64_C(0x45775673a732292a), UINT64_C(0x3faf26bb9707a053),
		UINT64_C(0x70ff52905f188d57), UINT64_C(0x0a2722586f2d042e),
		UINT64_C(0x854fb3003f739fa5), UINT64_C(0xff97c3c80f4616dc),
		UINT64_C(0x1bef5b57af4dc5ad), UINT64_C(0x61372b9f9f784cd4),
		UINT64_C(0xee5fbac7cf26d75f), UINT64_C(0x9487ca0fff135e26),
		UINT64_C(0xdbd7be24370c7322), UINT64_C(0xa10fceec0739fa5b),
		UINT64_C(0x2e675fb4576761d0), UINT64_C(0x54bf2f7c6752e8a9),
		UINT64_C(0xcdcf48d84fe75459), UINT64_C(0xb71738107fd2dd20),
		UINT64_C(0x387fa9482f8c46ab), UINT64_C(0x42a7d9801fb9cfd2),
		UINT64_C(0x0df7adabd7a6e2d6), UINT64_C(0x772fdd63e7936baf),
		UINT64_C(0xf8474c3bb7cdf024), UINT64_C(0x829f3cf387f8795d),
		UINT64_C(0x66e7a46c27f3aa2c), UINT64_C(0x1c3fd4a417c62355),
		UINT64_C(0x935745fc4798b8de), UINT64_C(0xe98f353477ad31a7),
		UINT64_C(0xa6df411fbfb21ca3), UINT64_C(0xdc0731d78f8795da),
		UINT64_C(0x536fa08fdfd90e51), UINT64_C(0x29b7d047efec8728),
	};

	/*
		CRC32 implementation - Copyright (C) 1986 Gary S. Brown.  You may use this program, or
		code or tables extracted from it, as desired without restriction.
		CRC polynomial 0xedb88320
	*/

	extern const uint_fast32_t _crc32_table[256] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
		0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
		0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
		0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
		0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
		0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
		0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
		0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
		0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
		0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
		0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
		0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
		0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
		0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
		0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
		0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
		0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
		0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
		0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
		0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
		0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
		0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
		0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
		0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
		0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
		0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
		0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
	};

	uint64_t CRC::crc64 (const char *buf, const size_t len, const uint64_t startval)
	{
		uint_fast64_t val = startval;

		#define UPDC64(octet,crc) crc=(_crc64_table[((crc) ^ (octet)) & 0xff] ^ ((crc) >> 8))
		for (size_t pos = 0; pos < len; ++pos) {
			UPDC64 (static_cast<uint_fast64_t> (buf[pos]), val);
		}
		#undef UPDC64

		return val & UINT64_MAX;
	}

	uint64_t CRC::crc64 (const std::string &plain, const uint64_t startval)
	{
		return crc64 (plain.data (), plain.size (), startval);
	}

	size_t CRC::crc64_check (const std::string &plain, const uint64_t startval)
	{
		size_t offset = plain.size ();
		if (offset < sizeof (uint64_t)) {
			cThrow ("CRC64: String is too short");
		}
		const size_t endpos = (offset -= sizeof (uint64_t));
		const uint64_t crc_a = crc64 (plain.data (), offset, startval);
		const uint64_t crc_b = BIN::to_uint64 (plain, offset);
		if (crc_a != crc_b) {
			cThrow ("CRC64: String and signature does not match");
		}
		return endpos;
	}

	void CRC::crc64_add (std::string &plain, const uint64_t startval)
	{
		const uint64_t crc = crc64 (plain, startval);
		BIN::from_uint64 (crc, plain);
	}

	uint32_t CRC::crc32 (const char *buf, const size_t len, const uint32_t startval)
	{
		uint_fast32_t val = startval;

		#define UPDC32(octet,crc) crc=(_crc32_table[((crc) ^ (octet)) & 0xff] ^ ((crc) >> 8))
		for (size_t pos = 0; pos < len; ++pos) {
			UPDC32 (static_cast<uint_fast32_t> (buf[pos]), val);
		}
		#undef UPDC32

		return (~val) & UINT32_MAX;
	}

	uint32_t CRC::crc32 (const std::string &plain, const uint32_t startval)
	{
		return crc32 (plain.data (), plain.size (), startval);
	}

	size_t CRC::crc32_check (const std::string &plain, const uint32_t startval)
	{
		size_t offset = plain.size ();
		if (offset < sizeof (uint32_t)) {
			cThrow ("CRC32: String is too short");
		}
		const size_t endpos = (offset -= sizeof (uint32_t));
		const uint32_t crc_a = crc32 (plain.data (), offset, startval);
		const uint32_t crc_b = BIN::to_uint32 (plain, offset);
		if (crc_a != crc_b) {
			cThrow ("CRC32: String and signature does not match");
		}
		return endpos;
	}

	void CRC::crc32_add (std::string &plain, const uint32_t startval)
	{
		const uint32_t crc = crc32 (plain, startval);
		BIN::from_uint32 (crc, plain);
	}

	uint8_t CRC::crc8 (const char *buf, const size_t len, const uint8_t startval)
	{
		uint_fast8_t val = startval;

		for (size_t pos = 0; pos < len; ++pos) {
			val = _crc8_table[(static_cast<uint_fast8_t> (buf[pos]) ^ val) & 0xFF];
		}

		return val & UINT8_MAX;
	}

	uint8_t CRC::crc8 (const std::string &plain, const uint8_t startval)
	{
		return crc8 (plain.data (), plain.size (), startval);
	}

	size_t CRC::crc8_check (const std::string &plain, const uint8_t startval)
	{
		size_t offset = plain.size ();
		if (offset < sizeof (uint8_t)) {
			cThrow ("CRC8: String is too short");
		}
		const size_t endpos = (offset -= sizeof (uint8_t));
		const uint8_t crc_a = crc8 (plain.data (), offset, startval);
		const uint8_t crc_b = BIN::to_uint8 (plain, offset);
		if (crc_a != crc_b) {
			cThrow ("CRC8: String and signature does not match");
		}
		return endpos;
	}

	void CRC::crc8_add (std::string &plain, const uint8_t startval)
	{
		const uint8_t crc = crc8 (plain, startval);
		BIN::from_uint8 (crc, plain);
	}

	std::string CRC::sha256 (const std::string &plain)
	{
#ifdef SHAGA_FULL
		const size_t sze = 32;
		unsigned char output[sze];
		mbedtls_sha256 (reinterpret_cast<const unsigned char *> (plain.data ()), plain.size (), output, 0);
		return std::string (reinterpret_cast<const char *> (output), sze);
#else
		(void) plain;
		cThrow ("Digest is not supported in lite version");
#endif // SHAGA_FULL
	}

	std::string CRC::sha512 (const std::string &plain)
	{
#ifdef SHAGA_FULL
		const size_t sze = 64;
		unsigned char output[sze];
		mbedtls_sha512 (reinterpret_cast<const unsigned char *> (plain.data ()), plain.size (), output, 0);
		return std::string (reinterpret_cast<const char *> (output), sze);
#else
		(void) plain;
		cThrow ("Digest is not supported in lite version");
#endif // SHAGA_FULL
	}
};

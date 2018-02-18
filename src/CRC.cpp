/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#ifdef SHAGA_FULL
	#include <mbedtls/sha256.h>
	#include <mbedtls/sha512.h>
#endif // SHAGA_FULL

namespace shaga {

	/* Bit reverse table is used for CRC-32 Atmel compatible algorithm */
	static const uint_fast32_t _bit_reverse_table[256] = {
		0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
		0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
		0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
		0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
		0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
		0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
		0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
		0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
		0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
		0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
		0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
		0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
		0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
		0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
		0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
		0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
	};

	/*
	 * Specification of this CRC64 variant follows:
	 * Name: crc-64-jones
	 * Width: 64 bites
	 * Poly: 0xad93d23594c935a9
	 * Reflected In: True
	 * Xor_In: 0xffffffffffffffff
	 * Reflected_Out: True
	 * Xor_Out: 0x0
	 * Check("123456789"): 0xe9c6d914c4b8d9ca
	 *
	 * Copyright (c) 2012, Salvatore Sanfilippo <antirez at gmail dot com>
	 * All rights reserved.
	 */
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

	/* CRC32 implementation - Copyright (C) 1986 Gary S. Brown.  You may use this program, or
	 * code or tables extracted from it, as desired without restriction.
	 * CRC polynomial 0xedb88320
	 */
	extern const uint_fast32_t _crc32_table[256] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,	0xe963a535, 0x9e6495a3,
		0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
		0x1db71064, 0x6ab020f2,	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,	0xfa0f3d63, 0x8d080df5,
		0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
		0x35b5a8fa, 0x42b2986c,	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,	0xcfba9599, 0xb8bda50f,
		0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
		0x76dc4190, 0x01db7106,	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,	0x91646c97, 0xe6635c01,
		0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
		0x65b0d9c6, 0x12b7e950,	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,	0xa4d1c46d, 0xd3d6f4fb,
		0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
		0x5005713c, 0x270241aa,	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,	0xb7bd5c3b, 0xc0ba6cad,
		0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
		0xe3630b12, 0x94643b84,	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,	0x196c3671, 0x6e6b06e7,
		0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
		0xd6d6a3e8, 0xa1d1937e,	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,	0x316e8eef, 0x4669be79,
		0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
		0xc5ba3bbe, 0xb2bd0b28,	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,	0x72076785, 0x05005713,
		0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
		0x86d3d2d4, 0xf1d4e242,	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,	0x616bffd3, 0x166ccf45,
		0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
		0xaed16a4a, 0xd9d65adc,	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,	0x54de5729, 0x23d967bf,
		0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
	};

	/* Polynomial 0x04C11DB7 */
	extern const uint_fast32_t _crc32_atmel_table[256] = {
		0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,	0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
		0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
		0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,	0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
		0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,	0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
		0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
		0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,	0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
		0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,	0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
		0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
		0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,	0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
		0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,	0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
		0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
		0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,	0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
		0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,	0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
		0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
		0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,	0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
		0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,	0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
		0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
		0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,	0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
		0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,	0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
		0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
		0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,	0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
		0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,	0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
		0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
		0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,	0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
		0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,	0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
		0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
		0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,	0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
		0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,	0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
		0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
		0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,	0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
		0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,	0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
		0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  CRC-64  /////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	uint64_t CRC::crc64 (const char *buf, const size_t len, const uint64_t startval)
	{
		uint_fast64_t val = startval;

		#define UPDC64(octet,crc) crc=(_crc64_table[((crc) ^ (octet)) & 0xff] ^ ((crc) >> 8))
		for (uint_fast32_t pos = 0; pos < len; ++pos) {
			UPDC64 (static_cast<uint_fast64_t> (buf[pos]), val);
		}
		#undef UPDC64

		return val & UINT64_MAX;
	}

	uint64_t CRC::crc64 (const uint8_t *buf, const size_t len, const uint64_t startval)
	{
		uint_fast64_t val = startval;

		#define UPDC64(octet,crc) crc=(_crc64_table[((crc) ^ (octet)) & 0xff] ^ ((crc) >> 8))
		for (uint_fast32_t pos = 0; pos < len; ++pos) {
			UPDC64 (static_cast<uint_fast64_t> (buf[pos]), val);
		}
		#undef UPDC64

		return val & UINT64_MAX;
	}

	size_t CRC::crc64_check (const std::string &plain, const uint64_t startval)
	{
		size_t offset = plain.size ();
		if (offset < sizeof (uint64_t)) {
			cThrow ("CRC-64: String is too short");
		}
		const size_t endpos = (offset -= sizeof (uint64_t));
		const uint64_t crc_a = crc64 (plain.data (), offset, startval);
		const uint64_t crc_b = BIN::to_uint64 (plain, offset);
		if (crc_a != crc_b) {
			cThrow ("CRC-64: String and signature does not match");
		}
		return endpos;
	}

	void CRC::crc64_add (std::string &plain, const uint64_t startval)
	{
		const uint64_t crc = crc64 (plain.data (), plain.size (), startval);
		BIN::from_uint64 (crc, plain);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  CRC-32 zlib compatible  /////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	uint32_t CRC::crc32 (const char *buf, const size_t len, const uint32_t startval)
	{
		uint_fast32_t val = startval ^ UINT32_MAX;

		#define UPDC32(octet,crc) crc=(_crc32_table[((crc) ^ (octet)) & 0xff] ^ ((crc) >> 8))
		for (uint_fast32_t pos = 0; pos < len; ++pos) {
			UPDC32 (static_cast<uint_fast32_t> (buf[pos]), val);
		}
		#undef UPDC32

		return val ^ UINT32_MAX;
	}

	uint32_t CRC::crc32 (const uint8_t *buf, const size_t len, const uint32_t startval)
	{
		uint_fast32_t val = startval ^ UINT32_MAX;

		#define UPDC32(octet,crc) crc=(_crc32_table[((crc) ^ (octet)) & 0xff] ^ ((crc) >> 8))
		for (uint_fast32_t pos = 0; pos < len; ++pos) {
			UPDC32 (static_cast<uint_fast32_t> (buf[pos]), val);
		}
		#undef UPDC32

		return val ^ UINT32_MAX;
	}

	size_t CRC::crc32_check (const std::string &plain, const uint32_t startval)
	{
		size_t offset = plain.size ();
		if (offset < sizeof (uint32_t)) {
			cThrow ("CRC-32: String is too short");
		}
		const size_t endpos = (offset -= sizeof (uint32_t));
		const uint32_t crc_a = crc32 (plain.data (), offset, startval);
		const uint32_t crc_b = BIN::to_uint32 (plain, offset);
		if (crc_a != crc_b) {
			cThrow ("CRC-32: String and signature does not match");
		}
		return endpos;
	}

	void CRC::crc32_add (std::string &plain, const uint32_t startval)
	{
		const uint32_t crc = crc32 (plain.data (), plain.size (), startval);
		BIN::from_uint32 (crc, plain);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  CRC-32 Atmel CRCCU CCITT802.3 compatible  ///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	uint32_t CRC::crc32_atmel (const char *buf, const size_t len, const uint32_t startval)
	{
		uint_fast32_t val = startval;

		#define UPDC32(octet,crc) crc=(_crc32_atmel_table[((crc >> 24) ^ (_bit_reverse_table[octet])) & 0xff] ^ (crc << 8))
		for (uint_fast32_t pos = 0; pos < len; ++pos) {
			UPDC32 (static_cast<uint_fast32_t> (buf[pos]), val);
		}

		return val & UINT32_MAX;
	}

	uint32_t CRC::crc32_atmel (const uint8_t *buf, const size_t len, const uint32_t startval)
	{
		uint_fast32_t val = startval;

		#define UPDC32(octet,crc) crc=(_crc32_atmel_table[((crc >> 24) ^ (_bit_reverse_table[octet])) & 0xff] ^ (crc << 8))
		for (uint_fast32_t pos = 0; pos < len; ++pos) {
			UPDC32 (static_cast<uint_fast32_t> (buf[pos]), val);
		}

		return val & UINT32_MAX;
	}

	size_t CRC::crc32_atmel_check (const std::string &plain, const uint32_t startval)
	{
		size_t offset = plain.size ();
		if (offset < sizeof (uint32_t)) {
			cThrow ("CRC-32: String is too short");
		}
		const size_t endpos = (offset -= sizeof (uint32_t));
		const uint32_t crc_a = crc32_atmel (plain.data (), offset, startval);
		const uint32_t crc_b = BIN::to_uint32 (plain, offset);
		if (crc_a != crc_b) {
			cThrow ("CRC-32: String and signature does not match");
		}
		return endpos;
	}

	void CRC::crc32_atmel_add (std::string &plain, const uint32_t startval)
	{
		const uint32_t crc = crc32_atmel (plain.data (), plain.size (), startval);
		BIN::from_uint32 (crc, plain);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  CRC-8  //////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	uint8_t CRC::crc8 (const char *buf, const size_t len, const uint8_t startval)
	{
		uint_fast8_t val = startval;

		for (uint_fast32_t pos = 0; pos < len; ++pos) {
			val = _crc8_table[(static_cast<uint_fast8_t> (buf[pos]) ^ val) & 0xFF];
		}

		return val & UINT8_MAX;
	}

	uint8_t CRC::crc8 (const uint8_t *buf, const size_t len, const uint8_t startval)
	{
		uint_fast8_t val = startval;

		for (uint_fast32_t pos = 0; pos < len; ++pos) {
			val = _crc8_table[(static_cast<uint_fast8_t> (buf[pos]) ^ val) & 0xFF];
		}

		return val & UINT8_MAX;
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
			cThrow ("CRC-8: String and signature does not match");
		}
		return endpos;
	}

	void CRC::crc8_add (std::string &plain, const uint8_t startval)
	{
		const uint8_t crc = crc8 (plain, startval);
		BIN::from_uint8 (crc, plain);
	}

	std::string CRC::sha256 (const char *buf, const size_t len)
	{
#ifdef SHAGA_FULL
		const size_t sze = 32;
		unsigned char output[sze];
		mbedtls_sha256 (reinterpret_cast<const unsigned char *> (buf), len, output, 0);
		return std::string (reinterpret_cast<const char *> (output), sze);
#else
		(void) buf;
		(void) len;
		cThrow ("Digest is not supported in lite version");
#endif // SHAGA_FULL
	}

	std::string CRC::sha256 (const uint8_t *buf, const size_t len)
	{
#ifdef SHAGA_FULL
		const size_t sze = 32;
		unsigned char output[sze];
		mbedtls_sha256 (buf, len, output, 0);
		return std::string (reinterpret_cast<const char *> (output), sze);
#else
		(void) buf;
		(void) len;
		cThrow ("Digest is not supported in lite version");
#endif // SHAGA_FULL
	}

	std::string CRC::sha512 (const char *buf, const size_t len)
	{
#ifdef SHAGA_FULL
		const size_t sze = 64;
		unsigned char output[sze];
		mbedtls_sha512 (reinterpret_cast<const unsigned char *> (buf), len, output, 0);
		return std::string (reinterpret_cast<const char *> (output), sze);
#else
		(void) buf;
		(void) len;
		cThrow ("Digest is not supported in lite version");
#endif // SHAGA_FULL
	}

	std::string CRC::sha512 (const uint8_t *buf, const size_t len)
	{
#ifdef SHAGA_FULL
		const size_t sze = 64;
		unsigned char output[sze];
		mbedtls_sha512 (buf, len, output, 0);
		return std::string (reinterpret_cast<const char *> (output), sze);
#else
		(void) buf;
		(void) len;
		cThrow ("Digest is not supported in lite version");
#endif // SHAGA_FULL
	}

	namespace CRC {
		#define SIPHASH_DATA _data
		#define SIPHASH_DATA_SIZE _sze
		#define SIPHASH_PARAMS (const uint8_t *_data, const size_t _sze, const uint64_t siphash_k0, const uint64_t siphash_k1)

		#define SIPHASH_BEGIN_KEYS \
			uint64_t vv0 = 0x736f6d6570736575ULL ^ siphash_k0;	\
			uint64_t vv1 = 0x646f72616e646f6dULL ^ siphash_k1;	\
			uint64_t vv2 = 0x6c7967656e657261ULL ^ siphash_k0;	\
			uint64_t vv3 = 0x7465646279746573ULL ^ siphash_k1;

		#include "internal_siphash.h"

		#undef SIPHASH_BEGIN_KEYS
		#undef SIPHASH_PARAMS
		#undef SIPHASH_DATA_SIZE
		#undef SIPHASH_DATA
	};

	std::pair<uint64_t, uint64_t> CRC::siphash_extract_key (const std::string &key)
	{
		if (key.size () != 16) {
			cThrow ("SipHash key must be exactly 16 bytes (128 bit) long");
		}

		std::pair<uint64_t, uint64_t> out;

		out.first = _siphash_to_uint64 (reinterpret_cast<const uint8_t *> (key.data ()));
		out.second = _siphash_to_uint64 (reinterpret_cast<const uint8_t *> (key.data () + 8));

		return out;
	}

	uint64_t CRC::siphash24 (const char *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key)
	{
		return _calc_siphash24 (reinterpret_cast<const uint8_t *> (buf), len, key.first, key.second);
	}

	uint64_t CRC::siphash24 (const uint8_t *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key)
	{
		return _calc_siphash24 (buf, len, key.first, key.second);
	}

	uint64_t CRC::siphash48 (const char *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key)
	{
		return _calc_siphash48 (reinterpret_cast<const uint8_t *> (buf), len, key.first, key.second);
	}

	uint64_t CRC::siphash48 (const uint8_t *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key)
	{
		return _calc_siphash48 (buf, len, key.first, key.second);
	}

	std::string CRC::siphash24_128 (const char *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key)
	{
		return _calc_siphash24_128 (reinterpret_cast<const uint8_t *> (buf), len, key.first, key.second);
	}

	std::string CRC::siphash24_128 (const uint8_t *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key)
	{
		return _calc_siphash24_128 (buf, len, key.first, key.second);
	}

	std::string CRC::siphash48_128 (const char *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key)
	{
		return _calc_siphash48_128 (reinterpret_cast<const uint8_t *> (buf), len, key.first, key.second);
	}

	std::string CRC::siphash48_128 (const uint8_t *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key)
	{
		return _calc_siphash48_128 (buf, len, key.first, key.second);
	}

};

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_CRC
#define HEAD_shaga_CRC

#include "common.h"

namespace shaga {

	static const uint_fast8_t _crc8_dallas_table[256] = {
		0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
		0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
		0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
		0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
		0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,
		0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
		0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,
		0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
		0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
		0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
		0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,
		0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
		0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,
		0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
		0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
		0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
		0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,
		0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
		0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,
		0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
		0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,
		0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
		0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,
		0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
		0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,
		0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
		0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,
		0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
		0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,
		0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
		0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,
		0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35
	};

	static const uint_fast16_t _crc16_modbus_table[256] = {
		0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
		0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
		0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
		0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
		0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
		0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
		0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
		0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
		0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
		0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
		0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
		0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
		0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
		0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
		0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
		0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
		0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
		0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
		0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
		0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
		0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
		0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
		0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
		0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
		0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
		0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
		0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
		0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
		0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
		0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
		0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
		0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
	};

	namespace CRC
	{
		/*** CRC-64 Jones ***/
		static const uint64_t crc64_magic {0};

		uint64_t crc64 (const char *buf, const size_t len, const uint64_t startval = 0);
		uint64_t crc64 (const uint8_t *buf, const size_t len, const uint64_t startval = 0);

		template<typename T>
		uint64_t crc64 (const T &plain, const uint64_t startval = 0)
		{
			return crc64 (plain.data (), plain.size (), startval);
		}

		size_t crc64_check (const std::string &plain, const uint64_t startval = 0);
		void crc64_append (std::string &plain, const uint64_t startval = 0);

		/*** CRC-32 zlib compatible ***/
		static const uint32_t crc32_zlib_magic {0x2144df1c};

		uint32_t crc32_zlib (const char *buf, const size_t len, const uint32_t startval = 0);
		uint32_t crc32_zlib (const uint8_t *buf, const size_t len, const uint32_t startval = 0);

		template<typename T>
		uint32_t crc32_zlib (const T &plain, const uint32_t startval = 0)
		{
			return crc32_zlib (plain.data (), plain.size (), startval);
		}

		size_t crc32_zlib_check (const std::string &plain, const uint32_t startval = 0);
		void crc32_zlib_append (std::string &plain, const uint32_t startval = 0);

		/*** CRC-32 Atmel CRCCU CCITT802.3 compatible ***/
		uint32_t crc32_atmel (const char *buf, const size_t len, const uint32_t startval = UINT32_MAX);
		uint32_t crc32_atmel (const uint8_t *buf, const size_t len, const uint32_t startval = UINT32_MAX);

		template<typename T>
		uint32_t crc32_atmel (const T &plain, const uint32_t startval = UINT32_MAX)
		{
			return crc32_atmel (plain.data (), plain.size (), startval);
		}

		size_t crc32_atmel_check (const std::string &plain, const uint32_t startval = UINT32_MAX);
		void crc32_atmel_append (std::string &plain, const uint32_t startval = UINT32_MAX);

		/*** CRC-32-Castagnoli ***/
		static const uint32_t crc32c_magic {0x48674bc7};

		uint32_t crc32c (const char *buf, const size_t len, const uint32_t startval = 0);
		uint32_t crc32c (const uint8_t *buf, const size_t len, const uint32_t startval = 0);

		template<typename T>
		uint32_t crc32c (const T &plain, const uint32_t startval = 0)
		{
			return crc32c (plain.data (), plain.size (), startval);
		}

		size_t crc32c_check (const std::string &plain, const uint32_t startval = 0);
		void crc32c_append (std::string &plain, const uint32_t startval = 0);

		/*** CRC-16 Modbus compatible ***/
		static const uint16_t crc16_modbus_magic {0};

		uint16_t crc16_modbus (const char *buf, const size_t len, const uint16_t startval = UINT16_MAX);
		uint16_t crc16_modbus (const uint8_t *buf, const size_t len, const uint16_t startval = UINT16_MAX);

		template<typename T>
		uint16_t crc16_modbus (const T &plain, const uint16_t startval = UINT16_MAX)
		{
			return crc16_modbus (plain.data (), plain.size (), startval);
		}

		size_t crc16_modbus_check (const std::string &plain, const uint16_t startval = UINT16_MAX);
		void crc16_modbus_append (std::string &plain, const uint16_t startval = UINT16_MAX);

		/*** CRC-8 Dallas/Maxim ***/
		static const uint8_t crc8_dallas_magic {0};

		uint8_t crc8_dallas (const char *buf, const size_t len, const uint8_t startval = 0);
		uint8_t crc8_dallas (const uint8_t *buf, const size_t len, const uint8_t startval = 0);

		template<typename T>
		uint8_t crc8_dallas (const T &plain, const uint8_t startval = 0)
		{
			return crc8_dallas (plain.data (), plain.size (), startval);
		}

		size_t crc8_dallas_check (const std::string &plain, const uint8_t startval = 0);
		void crc8_dallas_append (std::string &plain, const uint8_t startval = 0);

		/*** SHA-256 ***/
		std::string sha256 (const char *buf, const size_t len);
		std::string sha256 (const uint8_t *buf, const size_t len);

		template<class T>
		std::string sha256 (const T &plain)
		{
			return sha256 (plain.data (), plain.size ());
		}

		/*** SHA-512 ***/
		std::string sha512 (const char *buf, const size_t len);
		std::string sha512 (const uint8_t *buf, const size_t len);

		template<class T>
		std::string sha512 (const T &plain)
		{
			return sha512 (plain.data (), plain.size ());
		}

		/*** SipHash ***/
		typedef std::pair<uint64_t, uint64_t> SipHash128_t;

		SipHash128_t siphash_extract_key (const std::string &key);

		uint64_t siphash24 (const char *buf, const size_t len, const SipHash128_t &key);
		uint64_t siphash24 (const uint8_t *buf, const size_t len, const SipHash128_t &key);
		uint64_t siphash48 (const char *buf, const size_t len, const SipHash128_t &key);
		uint64_t siphash48 (const uint8_t *buf, const size_t len, const SipHash128_t &key);

		template<typename T>
		uint64_t siphash24 (const T &plain, const SipHash128_t &key)
		{
			return siphash24 (plain.data (), plain.size (), key);
		}

		template<typename T>
		uint64_t siphash48 (const T &plain, const SipHash128_t &key)
		{
			return siphash48 (plain.data (), plain.size (), key);
		}

		std::string siphash24_128 (const char *buf, const size_t len, const SipHash128_t &key);
		std::string siphash24_128 (const uint8_t *buf, const size_t len, const SipHash128_t &key);
		std::string siphash48_128 (const char *buf, const size_t len, const SipHash128_t &key);
		std::string siphash48_128 (const uint8_t *buf, const size_t len, const SipHash128_t &key);

		template<typename T>
		std::string siphash24_128 (const T &plain, const SipHash128_t &key)
		{
			return siphash24_128 (plain.data (), plain.size (), key);
		}

		template<typename T>
		std::string siphash48_128 (const T &plain, const SipHash128_t &key)
		{
			return siphash48_128 (plain.data (), plain.size (), key);
		}

		SipHash128_t siphash24_128t (const char *buf, const size_t len, const SipHash128_t &key);
		SipHash128_t siphash24_128t (const uint8_t *buf, const size_t len, const SipHash128_t &key);
		SipHash128_t siphash48_128t (const char *buf, const size_t len, const SipHash128_t &key);
		SipHash128_t siphash48_128t (const uint8_t *buf, const size_t len, const SipHash128_t &key);

		template<typename T>
		SipHash128_t siphash24_128t (const T &plain, const SipHash128_t &key)
		{
			return siphash24_128t (plain.data (), plain.size (), key);
		}

		template<typename T>
		SipHash128_t siphash48_128t (const T &plain, const SipHash128_t &key)
		{
			return siphash48_128t (plain.data (), plain.size (), key);
		}

	};
};

#endif // HEAD_shaga_CRC

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_CRC
#define HEAD_shaga_CRC

#include "common.h"

namespace shaga {

	static const uint_fast8_t _crc8_table[256] = {
		0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
		157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
		35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
		190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
		70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
		219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
		101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
		248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
		140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
		17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
		175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
		50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
		202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
		87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
		233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
		116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
	};

	namespace CRC
	{
		/*** CRC-64 ***/
		uint64_t crc64 (const char *buf, const size_t len, const uint64_t startval = 0);
		uint64_t crc64 (const uint8_t *buf, const size_t len, const uint64_t startval = 0);

		template<typename T>
		uint64_t crc64 (const T &plain, const uint64_t startval = 0)
		{
			return crc64 (plain.data (), plain.size (), startval);
		}

		size_t crc64_check (const std::string &plain, const uint64_t startval = 0);
		void crc64_add (std::string &plain, const uint64_t startval = 0);

		/*** CRC-32 zlib compatible ***/
		uint32_t crc32 (const char *buf, const size_t len, const uint32_t startval = 0);
		uint32_t crc32 (const uint8_t *buf, const size_t len, const uint32_t startval = 0);

		template<typename T>
		uint32_t crc32 (const T &plain, const uint32_t startval = 0)
		{
			return crc32 (plain.data (), plain.size (), startval);
		}

		size_t crc32_check (const std::string &plain, const uint32_t startval = 0);
		void crc32_add (std::string &plain, const uint32_t startval = 0);

		/*** CRC-32 Atmel CRCCU CCITT802.3 compatible ***/
		uint32_t crc32_atmel (const char *buf, const size_t len, const uint32_t startval = UINT32_MAX);
		uint32_t crc32_atmel (const uint8_t *buf, const size_t len, const uint32_t startval = UINT32_MAX);

		template<typename T>
		uint32_t crc32_atmel (const T &plain, const uint32_t startval = UINT32_MAX)
		{
			return crc32_atmel (plain.data (), plain.size (), startval);
		}

		size_t crc32_atmel_check (const std::string &plain, const uint32_t startval = UINT32_MAX);
		void crc32_atmel_add (std::string &plain, const uint32_t startval = UINT32_MAX);

		/*** CRC-8 ***/
		uint8_t crc8 (const char *buf, const size_t len, const uint8_t startval = 0);
		uint8_t crc8 (const uint8_t *buf, const size_t len, const uint8_t startval = 0);

		template<typename T>
		uint8_t crc8 (const T &plain, const uint8_t startval = 0)
		{
			return crc8 (plain.data (), plain.size (), startval);
		}

		size_t crc8_check (const std::string &plain, const uint8_t startval = 0);
		void crc8_add (std::string &plain, const uint8_t startval = 0);

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
		std::pair<uint64_t, uint64_t> siphash_extract_key (const std::string &key);

		uint64_t siphash24 (const char *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key);
		uint64_t siphash24 (const uint8_t *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key);
		uint64_t siphash48 (const char *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key);
		uint64_t siphash48 (const uint8_t *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key);

		template<typename T>
		uint64_t siphash24 (const T &plain, const std::pair<uint64_t, uint64_t> &key)
		{
			return siphash24 (plain.data (), plain.size (), key);
		}

		template<typename T>
		uint64_t siphash48 (const T &plain, const std::pair<uint64_t, uint64_t> &key)
		{
			return siphash48 (plain.data (), plain.size (), key);
		}

		std::string siphash24_128 (const char *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key);
		std::string siphash24_128 (const uint8_t *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key);
		std::string siphash48_128 (const char *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key);
		std::string siphash48_128 (const uint8_t *buf, const size_t len, const std::pair<uint64_t, uint64_t> &key);

		template<typename T>
		std::string siphash24_128 (const T &plain, const std::pair<uint64_t, uint64_t> &key)
		{
			return siphash24_128 (plain.data (), plain.size (), key);
		}

		template<typename T>
		std::string siphash48_128 (const T &plain, const std::pair<uint64_t, uint64_t> &key)
		{
			return siphash48_128 (plain.data (), plain.size (), key);
		}
	};
};

#endif // HEAD_shaga_CRC

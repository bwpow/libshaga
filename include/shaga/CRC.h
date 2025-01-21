/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_CRC
#define HEAD_shaga_CRC

#include "common.h"

namespace shaga::CRC {
	/* All tables - look into src/CRC.cpp for more details */
	extern const uint_fast32_t _bit_reverse_table[256];
	extern const uint_fast64_t _crc64_table[256];
	extern const uint_fast32_t _crc32_zlib_table[256];
	extern const uint_fast32_t _crc32_atmel_table[256];
	extern const uint_fast32_t _crc32c_table[256];
	extern const uint_fast16_t _crc16_modbus_table[256];
	extern const uint_fast8_t _crc8_dallas_table[256];

	/*** CRC-64 Jones ***/
	static const uint64_t crc64_magic {0};
	static const uint64_t crc64_startval {0};

	uint64_t crc64 (const void *const buf, const size_t len, const uint64_t startval = crc64_startval);

	template<class T, SHAGA_TYPE_IS_CLASS(T)>
	uint64_t crc64 (const T &plain, const uint64_t startval = crc64_startval)
	{
		return crc64 (plain.data (), plain.size (), startval);
	}

	size_t crc64_check (const std::string_view plain, const uint64_t startval = crc64_startval);
	void crc64_check_and_trim (std::string_view &plain, const uint64_t startval = crc64_startval);
	void crc64_append (std::string &plain, const uint64_t startval = crc64_startval);

	/*** CRC-32 zlib compatible ***/
	static const uint32_t crc32_zlib_magic {0x2144df1c};
	static const uint32_t crc32_zlib_startval {0};

	uint32_t crc32_zlib (const void *const buf, const size_t len, const uint32_t startval = crc32_zlib_startval);

	template<class T, SHAGA_TYPE_IS_CLASS(T)>
	uint32_t crc32_zlib (const T &plain, const uint32_t startval = crc32_zlib_startval)
	{
		return crc32_zlib (plain.data (), plain.size (), startval);
	}

	size_t crc32_zlib_check (const std::string_view plain, const uint32_t startval = crc32_zlib_startval);
	void crc32_zlib_check_and_trim (std::string_view &plain, const uint32_t startval = crc32_zlib_startval);
	void crc32_zlib_append (std::string &plain, const uint32_t startval = crc32_zlib_startval);

	/*** CRC-32 Atmel CRCCU CCITT802.3 compatible ***/
	static const uint32_t crc32_atmel_startval {UINT32_MAX};

	uint32_t crc32_atmel (const void *const buf, const size_t len, const uint32_t startval = crc32_atmel_startval);

	template<class T, SHAGA_TYPE_IS_CLASS(T)>
	uint32_t crc32_atmel (const T &plain, const uint32_t startval = crc32_atmel_startval)
	{
		return crc32_atmel (plain.data (), plain.size (), startval);
	}

	size_t crc32_atmel_check (const std::string_view plain, const uint32_t startval = crc32_atmel_startval);
	void crc32_atmel_check_and_trim (std::string_view &plain, const uint32_t startval = crc32_atmel_startval);
	void crc32_atmel_append (std::string &plain, const uint32_t startval = crc32_atmel_startval);

	/*** CRC-32-Castagnoli ***/
	static const uint32_t crc32c_magic {0x48674bc7};
	static const uint32_t crc32c_startval {0};

	uint32_t crc32c (const void *const buf, const size_t len, const uint32_t startval = crc32c_startval);

	template<class T, SHAGA_TYPE_IS_CLASS(T)>
	uint32_t crc32c (const T &plain, const uint32_t startval = crc32c_startval)
	{
		return crc32c (plain.data (), plain.size (), startval);
	}

	size_t crc32c_check (const std::string_view plain, const uint32_t startval = crc32c_startval);
	void crc32c_check_and_trim (std::string_view &plain, const uint32_t startval = crc32c_startval);
	void crc32c_append (std::string &plain, const uint32_t startval = crc32c_startval);

	/*** CRC-16 Modbus compatible ***/
	static const uint16_t crc16_modbus_magic {0};
	static const uint16_t crc16_modbus_startval {UINT16_MAX};

	uint16_t crc16_modbus (const void *const buf, const size_t len, const uint16_t startval = crc16_modbus_startval);

	template<class T, SHAGA_TYPE_IS_CLASS(T)>
	uint16_t crc16_modbus (const T &plain, const uint16_t startval = crc16_modbus_startval)
	{
		return crc16_modbus (plain.data (), plain.size (), startval);
	}

	size_t crc16_modbus_check (const std::string_view plain, const uint16_t startval = crc16_modbus_startval);
	void crc16_modbus_check_and_trim (std::string_view &plain, const uint16_t startval = crc16_modbus_startval);
	void crc16_modbus_append (std::string &plain, const uint16_t startval = crc16_modbus_startval);

	/*** CRC-8 Dallas/Maxim ***/
	static const uint8_t crc8_dallas_magic {0};
	static const uint8_t crc8_dallas_startval {0};

	uint8_t crc8_dallas (const void *const buf, const size_t len, const uint8_t startval = crc8_dallas_startval);

	template<class T, SHAGA_TYPE_IS_CLASS(T)>
	uint8_t crc8_dallas (const T &plain, const uint8_t startval = crc8_dallas_startval)
	{
		return crc8_dallas (plain.data (), plain.size (), startval);
	}

	size_t crc8_dallas_check (const std::string_view plain, const uint8_t startval = crc8_dallas_startval);
	void crc8_dallas_check_and_trim (std::string_view &plain, const uint8_t startval = crc8_dallas_startval);
	void crc8_dallas_append (std::string &plain, const uint8_t startval = crc8_dallas_startval);
}

#endif // HEAD_shaga_CRC

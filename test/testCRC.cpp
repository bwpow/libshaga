/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2022, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

TEST (CRC, CRC64)
{
	/* Some common strings */
	EXPECT_TRUE (CRC::crc64 (std::string ("123456789")) == UINT64_C(0xe9c6d914c4b8d9ca));

	/* Append and check */
	std::string str ("1234567890xyz");
	const size_t sze = str.size ();
	EXPECT_NO_THROW (CRC::crc64_append (str));
	EXPECT_NO_THROW (CRC::crc64_check (str));

	std::string_view view (str);
	EXPECT_NO_THROW (CRC::crc64_check_and_trim (view));
	EXPECT_TRUE (view.size () == sze);
	EXPECT_THROW (CRC::crc64_check_and_trim (view), CommonException);

	str.append ("xy");
	EXPECT_THROW (CRC::crc64_check (str), CommonException);

	str.clear ();
	EXPECT_THROW (CRC::crc64_check (str), CommonException);
}

TEST (CRC, CRC32_zlib)
{
	std::array <uint8_t, 32> data;

	/* Some common strings */
	EXPECT_TRUE (CRC::crc32_zlib (std::string ("The quick brown fox jumps over the lazy dog")) == 0x41'4F'A3'39);

	std::fill (data.begin (), data.end (), 0x00);
	EXPECT_TRUE (CRC::crc32_zlib (data) == 0x19'0A'55'AD);

	std::fill (data.begin (), data.end (), 0xff);
	EXPECT_TRUE (CRC::crc32_zlib (data) == 0xFF'6C'AB'0B);

	std::iota (data.begin (), data.end (), 0x00);
	EXPECT_TRUE (CRC::crc32_zlib (data) == 0x91'26'7E'8A);

	/* Append and check */
	std::string str ("1234567890xyz");
	const size_t sze = str.size ();
	EXPECT_NO_THROW (CRC::crc32_zlib_append (str));
	EXPECT_NO_THROW (CRC::crc32_zlib_check (str));

	std::string_view view (str);
	EXPECT_NO_THROW (CRC::crc32_zlib_check_and_trim (view));
	EXPECT_TRUE (view.size () == sze);
	EXPECT_THROW (CRC::crc32_zlib_check_and_trim (view), CommonException);

	str.append ("xy");
	EXPECT_THROW (CRC::crc32_zlib_check (str), CommonException);

	str.clear ();
	EXPECT_THROW (CRC::crc32_zlib_check (str), CommonException);
}

TEST (CRC, CRC32_atmel)
{
	std::array <uint8_t, 64> data;

	/* Some common strings */
	std::iota (data.begin (), data.end (), 0x00);
	EXPECT_TRUE (CRC::crc32_atmel (data) == 0xCE'8C'8F'F7);

	std::iota (data.begin (), data.end (), 0x05);
	EXPECT_TRUE (CRC::crc32_atmel (data) == 0x63'0C'49'64);

	/* Append and check */
	std::string str ("1234567890xyz");
	const size_t sze = str.size ();
	EXPECT_NO_THROW (CRC::crc32_atmel_append (str));
	EXPECT_NO_THROW (CRC::crc32_atmel_check (str));

	std::string_view view (str);
	EXPECT_NO_THROW (CRC::crc32_atmel_check_and_trim (view));
	EXPECT_TRUE (view.size () == sze);
	EXPECT_THROW (CRC::crc32_atmel_check_and_trim (view), CommonException);

	str.append ("xy");
	EXPECT_THROW (CRC::crc32_atmel_check (str), CommonException);

	str.clear ();
	EXPECT_THROW (CRC::crc32_atmel_check (str), CommonException);
}

TEST (CRC, CRC32C)
{
	std::array <uint8_t, 32> data;

	/* Some common strings */
	EXPECT_TRUE (CRC::crc32c (std::string ("123456789")) == 0xe3'06'92'83);

	std::fill (data.begin (), data.end (), 0x00);
	EXPECT_TRUE (CRC::crc32c (data) == 0x8a'91'36'aa);

	std::fill (data.begin (), data.end (), 0xff);
	EXPECT_TRUE (CRC::crc32c (data) == 0x62'a8'ab'43);

	std::iota (data.begin (), data.end (), 0x00);
	EXPECT_TRUE (CRC::crc32c (data) == 0x46'dd'79'4e);

	/* Append and check */
	std::string str ("1234567890xyz");
	const size_t sze = str.size ();
	EXPECT_NO_THROW (CRC::crc32c_append (str));
	EXPECT_NO_THROW (CRC::crc32c_check (str));

	std::string_view view (str);
	EXPECT_NO_THROW (CRC::crc32c_check_and_trim (view));
	EXPECT_TRUE (view.size () == sze);
	EXPECT_THROW (CRC::crc32c_check_and_trim (view), CommonException);

	str.append ("xy");
	EXPECT_THROW (CRC::crc32c_check (str), CommonException);

	str.clear ();
	EXPECT_THROW (CRC::crc32c_check (str), CommonException);
}

TEST (CRC, CRC16_modbus)
{
	auto _ref_implementation = [](const uint8_t* data, const size_t length) -> uint16_t
	{
		/* Other implementation of Modbus CRC-16 calculation */
		uint16_t crc = 0xFF'FF;

		for (size_t pos = 0; pos < length; ++pos) {
			crc ^= static_cast<uint16_t> (data[pos]);
			for (int i = 0; i < 8; ++i) {
				if(crc & 1) {
					/* LSB is set - Shift right and XOR 0xA001 */
					crc >>= 1;
					crc ^= 0xA0'01;
				} else {
					/* LSB is not set */
					crc >>= 1;
				}
			}
		}
		return crc;
	};

	/* Check reference too */
	const std::string refdata = BIN::from_hex ("55450A000900030106320000");
	EXPECT_TRUE (CRC::crc16_modbus (refdata) == 0x3d'90);
	EXPECT_TRUE (_ref_implementation (reinterpret_cast<const uint8_t *> (refdata.data ()), refdata.size ()) == 0x3d'90);

	/* Some common strings */
	EXPECT_TRUE (CRC::crc16_modbus (std::string ("1234567890")) == 0xc2'0a);
	EXPECT_TRUE (CRC::crc16_modbus (std::string ("test")) == 0xdc'2e);

	std::array <uint8_t, 256> data;

	std::fill (data.begin (), data.end (), 0x00);
	EXPECT_TRUE (CRC::crc16_modbus (data) == _ref_implementation (data.data (), data.size ()));

	std::fill (data.begin (), data.end (), 0xff);
	EXPECT_TRUE (CRC::crc16_modbus (data) == _ref_implementation (data.data (), data.size ()));

	std::iota (data.begin (), data.end (), 0x00);
	EXPECT_TRUE (CRC::crc16_modbus (data) == _ref_implementation (data.data (), data.size ()));

	/* Append and check */
	std::string str ("1234567890xyz");
	const size_t sze = str.size ();
	EXPECT_NO_THROW (CRC::crc16_modbus_append (str));
	EXPECT_NO_THROW (CRC::crc16_modbus_check (str));

	std::string_view view (str);
	EXPECT_NO_THROW (CRC::crc16_modbus_check_and_trim (view));
	EXPECT_TRUE (view.size () == sze);
	EXPECT_THROW (CRC::crc16_modbus_check_and_trim (view), CommonException);

	str.append ("xy");
	EXPECT_THROW (CRC::crc16_modbus_check (str), CommonException);

	str.clear ();
	EXPECT_THROW (CRC::crc16_modbus_check (str), CommonException);
}

TEST (CRC, CRC8_dallas)
{
	auto _ref_implementation = [](const uint8_t* data, const size_t length) -> uint8_t
	{
		uint8_t crc = 0;
		for (size_t pos = 0; pos < length; ++pos) {
			uint8_t inbyte = data[pos];
			for (uint8_t j = 0; j < 8; ++j) {
				unsigned char mix = (crc ^ inbyte) & 0x01;
				crc >>= 1;
				if ( mix ) crc ^= 0x8C;
				inbyte >>= 1;
			}
		}
		return crc;
	};

	std::array <uint8_t, 256> data;

	std::fill (data.begin (), data.end (), 0x00);
	EXPECT_TRUE (CRC::crc8_dallas (data) == _ref_implementation (data.data (), data.size ()));

	std::fill (data.begin (), data.end (), 0xff);
	EXPECT_TRUE (CRC::crc8_dallas (data) == _ref_implementation (data.data (), data.size ()));

	std::iota (data.begin (), data.end (), 0x00);
	EXPECT_TRUE (CRC::crc8_dallas (data) == _ref_implementation (data.data (), data.size ()));

	/* Append and check */
	std::string str ("1234567890xyz");
	const size_t sze = str.size ();
	EXPECT_NO_THROW (CRC::crc8_dallas_append (str));
	EXPECT_NO_THROW (CRC::crc8_dallas_check (str));

	std::string_view view (str);
	EXPECT_NO_THROW (CRC::crc8_dallas_check_and_trim (view));
	EXPECT_TRUE (view.size () == sze);
	EXPECT_THROW (CRC::crc8_dallas_check_and_trim (view), CommonException);

	str.append ("xy");
	EXPECT_THROW (CRC::crc8_dallas_check (str), CommonException);

	str.clear ();
	EXPECT_THROW (CRC::crc8_dallas_check (str), CommonException);
}

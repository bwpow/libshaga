/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

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

	/* Hardware CRC loads must accept buffers at every byte alignment. */
	const std::string payload ("unaligned CRC32C payload");
	const uint32_t payload_crc = CRC::crc32c (payload);
	std::array <uint8_t, 64> storage;
	for (size_t offset = 0; offset < 8; ++offset) {
		std::fill (storage.begin (), storage.end (), 0);
		std::copy (payload.begin (), payload.end (), storage.begin () + offset);
		EXPECT_EQ (CRC::crc32c (storage.data () + offset, payload.size ()), payload_crc);
	}

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
	EXPECT_EQ (CRC::crc16_modbus (refdata), 0x3d'90);
	EXPECT_EQ (_ref_implementation (reinterpret_cast<const uint8_t *> (refdata.data ()), refdata.size ()), 0x3d'90);

	/* Some common strings */
	EXPECT_EQ (CRC::crc16_modbus ("1234567890"s), 0xc2'0a);
	EXPECT_EQ (CRC::crc16_modbus ("test"s), 0xdc'2e);

	std::array <uint8_t, 256> data;

	std::fill (data.begin (), data.end (), 0x00);
	EXPECT_EQ (CRC::crc16_modbus (data), _ref_implementation (data.data (), data.size ()));

	std::fill (data.begin (), data.end (), 0xff);
	EXPECT_EQ (CRC::crc16_modbus (data), _ref_implementation (data.data (), data.size ()));

	std::iota (data.begin (), data.end (), 0x00);
	EXPECT_EQ (CRC::crc16_modbus (data), _ref_implementation (data.data (), data.size ()));

	/* Append and check */
	std::string str ("1234567890xyz");
	const size_t sze = str.size ();
	EXPECT_NO_THROW (CRC::crc16_modbus_append (str));
	EXPECT_NO_THROW (CRC::crc16_modbus_check (str));

	std::string_view view (str);
	EXPECT_NO_THROW (CRC::crc16_modbus_check_and_trim (view));
	EXPECT_EQ (view.size (), sze);
	EXPECT_THROW (CRC::crc16_modbus_check_and_trim (view), CommonException);

	str.append ("xy");
	EXPECT_THROW (CRC::crc16_modbus_check (str), CommonException);

	str.clear ();
	EXPECT_THROW (CRC::crc16_modbus_check (str), CommonException);
}

TEST (CRC, CRC16_ccitt_false)
{
	auto _ref_implementation = [](const uint8_t* data, const size_t length) -> uint16_t
	{
		/* CRC-16/CCITT-FALSE */
		/* poly = 0x1021, init = 0xFFFF, refin=false, refout=false, xorout=0x0000 */
		uint16_t crc = 0xFFFF;

		for (size_t pos = 0; pos < length; ++pos) {
			crc ^= static_cast<uint16_t> (data[pos]) << 8; // align byte to MSB
			for (int i = 0; i < 8; ++i) {
				if (crc & 0x8000) {
					crc = static_cast<uint16_t>((crc << 1) ^ 0x1021);
				}
				else {
					crc = static_cast<uint16_t>(crc << 1);
				}
			}
		}
		return crc;
	};

	/* Check reference too */
	const std::string refdata = "123456789"s;
	EXPECT_EQ (CRC::crc16_ccitt_false (refdata), 0x29B1);
	EXPECT_EQ (_ref_implementation (reinterpret_cast<const uint8_t *> (refdata.data ()), refdata.size ()), 0x29B1);

	{
		std::array <uint8_t, 256> data;

		std::fill (data.begin (), data.end (), 0x00);
		EXPECT_EQ (CRC::crc16_ccitt_false (data), _ref_implementation (data.data (), data.size ()));

		std::fill (data.begin (), data.end (), 0xff);
		EXPECT_EQ (CRC::crc16_ccitt_false (data), _ref_implementation (data.data (), data.size ()));

		std::iota (data.begin (), data.end (), 0x00);
		EXPECT_EQ (CRC::crc16_ccitt_false (data), _ref_implementation (data.data (), data.size ()));
	}

	{
		/* bytes with high bit set */
		const std::array<uint8_t, 8> data = {0x80, 0x81, 0xFE, 0xFF, 0x90, 0xA5, 0xC3, 0x7F};
		EXPECT_EQ (CRC::crc16_ccitt_false (data), _ref_implementation (data.data (), data.size ()));
	}

	/* Append and check */
	std::string str = "1234567890xyz"s;
	const size_t sze = str.size ();
	EXPECT_NO_THROW (CRC::crc16_ccitt_false_append (str));
	EXPECT_NO_THROW (CRC::crc16_ccitt_false_check (str));

	std::string_view view (str);
	EXPECT_NO_THROW (CRC::crc16_ccitt_false_check_and_trim (view));
	EXPECT_EQ (view.size (), sze);
	EXPECT_THROW (CRC::crc16_ccitt_false_check_and_trim (view), CommonException);

	str.append ("xy");
	EXPECT_THROW (CRC::crc16_ccitt_false_check (str), CommonException);

	str.clear ();
	EXPECT_THROW (CRC::crc16_ccitt_false_check (str), CommonException);
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

TEST (CRC, CRC32_atmel_high_bit_bytes)
{
	/* Regression: crc32_atmel indexed _bit_reverse_table with a sign-extended char.
	On platforms where char is signed, bytes >= 0x80 produced an out-of-bounds read
	and platform-dependent results. This uses an independent, table-free reference
	(bit-reversed input, MSB-first CRC-32 poly 0x04C11DB7, init 0xFFFFFFFF). */
	auto _ref_implementation = [](const uint8_t *data, const size_t length) -> uint32_t
	{
		uint32_t crc = UINT32_MAX;
		for (size_t pos = 0; pos < length; ++pos) {
			/* Reverse the 8 input bits */
			uint8_t rb = 0;
			uint8_t b = data[pos];
			for (int k = 0; k < 8; ++k) {
				rb = static_cast<uint8_t> ((rb << 1) | (b & 1));
				b >>= 1;
			}

			crc ^= static_cast<uint32_t> (rb) << 24;
			for (int k = 0; k < 8; ++k) {
				if (crc & 0x80000000u) {
					crc = (crc << 1) ^ 0x04C11DB7u;
				}
				else {
					crc <<= 1;
				}
			}
		}
		return crc;
	};

	std::array <uint8_t, 64> data;

	/* Validate the reference against the known-good low-byte vectors used above.
	If these hold, the reference is trustworthy for the high-bit comparisons below. */
	std::iota (data.begin (), data.end (), 0x00);
	EXPECT_EQ (_ref_implementation (data.data (), data.size ()), 0xCE'8C'8F'F7u);
	std::iota (data.begin (), data.end (), 0x05);
	EXPECT_EQ (_ref_implementation (data.data (), data.size ()), 0x63'0C'49'64u);

	/* Regression cases: data with the high bit set */
	std::fill (data.begin (), data.end (), 0x80);
	EXPECT_EQ (CRC::crc32_atmel (data), _ref_implementation (data.data (), data.size ()));

	std::fill (data.begin (), data.end (), 0xFF);
	EXPECT_EQ (CRC::crc32_atmel (data), _ref_implementation (data.data (), data.size ()));

	std::iota (data.begin (), data.end (), 0xC0);
	EXPECT_EQ (CRC::crc32_atmel (data), _ref_implementation (data.data (), data.size ()));

	const std::array<uint8_t, 12> mixed = {0x00, 0x80, 0x7F, 0xFF, 0x81, 0xA5, 0xC3, 0x01, 0xFE, 0x90, 0x88, 0xBB};
	EXPECT_EQ (CRC::crc32_atmel (mixed), _ref_implementation (mixed.data (), mixed.size ()));

	/* Append and check must round-trip even with high-bit payload */
	std::string str;
	for (int i = 0; i < 40; ++i) {
		str.push_back (static_cast<char> (0x80 | (i & 0x7F)));
	}
	const size_t sze = str.size ();
	EXPECT_NO_THROW (CRC::crc32_atmel_append (str));
	EXPECT_NO_THROW (CRC::crc32_atmel_check (str));

	std::string_view view (str);
	EXPECT_NO_THROW (CRC::crc32_atmel_check_and_trim (view));
	EXPECT_EQ (view.size (), sze);
}

TEST (CRC, CRC16_ccitt_false_zero_in_crc_bytes)
{
	/* Regression: crc16_ccitt_false_check(_and_trim) read the trailing CRC bytes through
	a raw pointer, which constructed a string_view using strlen. A CRC byte equal to 0x00
	terminated the view early, causing valid messages to be rejected (or an out-of-bounds
	read). Build payloads whose CRC contains a 0x00 byte and verify the check succeeds. */

	auto _find_payload_with_crc_byte = [](const bool low_byte_zero) -> std::string {
		for (uint32_t i = 0; i < 1'000'000; ++i) {
			std::string s = "ccitt_regression_"s + STR::from_int (i);
			const uint16_t crc = CRC::crc16_ccitt_false (s);
			if (true == low_byte_zero) {
				if ((crc & 0xFF) == 0x00) {
					return s;
				}
			}
			else {
				if (((crc >> 8) & 0xFF) == 0x00) {
					return s;
				}
			}
		}
		return std::string ();
	};

	for (const bool low_byte_zero : {true, false}) {
		std::string str = _find_payload_with_crc_byte (low_byte_zero);
		ASSERT_FALSE (str.empty ());

		const uint16_t crc = CRC::crc16_ccitt_false (str);
		if (true == low_byte_zero) {
			ASSERT_EQ (crc & 0xFF, 0x00);
		}
		else {
			ASSERT_EQ ((crc >> 8) & 0xFF, 0x00);
		}

		const size_t sze = str.size ();
		EXPECT_NO_THROW (CRC::crc16_ccitt_false_append (str));

		/* Both check variants must accept the message despite the embedded 0x00 CRC byte */
		EXPECT_NO_THROW (CRC::crc16_ccitt_false_check (str));

		std::string_view view (str);
		EXPECT_NO_THROW (CRC::crc16_ccitt_false_check_and_trim (view));
		EXPECT_EQ (view.size (), sze);
	}
}

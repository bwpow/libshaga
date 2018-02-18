/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.txt):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <shaga.h>
#include <gtest/gtest.h>

using namespace shaga;

TEST (CRC, CRC64)
{
	EXPECT_TRUE (CRC::crc64 (std::string ("123456789")) == UINT64_C(0xe9c6d914c4b8d9ca));
}

TEST (CRC, CRC32)
{
	std::array <uint8_t, 32> data;

	EXPECT_TRUE (CRC::crc32 (std::string ("The quick brown fox jumps over the lazy dog")) == 0x41'4F'A3'39);

	std::fill (data.begin (), data.end (), 0x00);
	EXPECT_TRUE (CRC::crc32 (data) == 0x19'0A'55'AD);

	std::fill (data.begin (), data.end (), 0xff);
	EXPECT_TRUE (CRC::crc32 (data) == 0xFF'6C'AB'0B);

	std::iota (data.begin (), data.end (), 0x00);
	EXPECT_TRUE (CRC::crc32 (data) == 0x91'26'7E'8A);
}

TEST (CRC, CRC32_atmel)
{
	std::array <uint8_t, 64> data;

	std::iota (data.begin (), data.end (), 0x00);
	EXPECT_TRUE (CRC::crc32_atmel (data) == 0xCE'8C'8F'F7);

	std::iota (data.begin (), data.end (), 0x05);
	EXPECT_TRUE (CRC::crc32_atmel (data) == 0x63'0C'49'64);
}

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

TEST (BIN, count_trailing_zeros)
{
	EXPECT_EQ (BIN::count_trailing_zeros (static_cast<uint8_t> (UINT8_MAX)), 0);
	EXPECT_EQ (BIN::count_trailing_zeros (static_cast<uint16_t> (UINT16_MAX)), 0);
	EXPECT_EQ (BIN::count_trailing_zeros (static_cast<uint32_t> (UINT32_MAX)), 0);
	EXPECT_EQ (BIN::count_trailing_zeros (static_cast<uint64_t> (UINT64_MAX)), 0);

	EXPECT_EQ (BIN::count_trailing_zeros (static_cast<uint8_t> (0)), 8);
	EXPECT_EQ (BIN::count_trailing_zeros (static_cast<uint16_t> (0)), 16);
	EXPECT_EQ (BIN::count_trailing_zeros (static_cast<uint32_t> (0)), 32);
	EXPECT_EQ (BIN::count_trailing_zeros (static_cast<uint64_t> (0)), 64);

	for (size_t i = 0; i < 32; ++i) {
		EXPECT_EQ (BIN::count_trailing_zeros (static_cast<uint32_t> (1) << i), i);
	}
}

TEST (BIN, count_zeros)
{
	EXPECT_EQ (BIN::count_zeros (static_cast<uint8_t> (UINT8_MAX)), 0);
	EXPECT_EQ (BIN::count_zeros (static_cast<uint16_t> (UINT16_MAX)), 0);
	EXPECT_EQ (BIN::count_zeros (static_cast<uint32_t> (UINT32_MAX)), 0);
	EXPECT_EQ (BIN::count_zeros (static_cast<uint64_t> (UINT64_MAX)), 0);

	EXPECT_EQ (BIN::count_zeros (static_cast<uint8_t> (0)), 8);
	EXPECT_EQ (BIN::count_zeros (static_cast<uint16_t> (0)), 16);
	EXPECT_EQ (BIN::count_zeros (static_cast<uint32_t> (0)), 32);
	EXPECT_EQ (BIN::count_zeros (static_cast<uint64_t> (0)), 64);

	for (size_t i = 0; i <= UINT16_MAX ; ++i) {
		size_t cnt = 0;
		for (size_t j = 0; j < 16; ++j) {
			if ((i & (1 << j)) == 0) {
				++cnt;
			}
		}
		EXPECT_EQ (BIN::count_zeros (static_cast<uint16_t> (i)), cnt);
	}
}

TEST (BIN, count_trailing_ones)
{
	EXPECT_EQ (BIN::count_trailing_ones (static_cast<uint8_t> (UINT8_MAX)), 8);
	EXPECT_EQ (BIN::count_trailing_ones (static_cast<uint16_t> (UINT16_MAX)), 16);
	EXPECT_EQ (BIN::count_trailing_ones (static_cast<uint32_t> (UINT32_MAX)), 32);
	EXPECT_EQ (BIN::count_trailing_ones (static_cast<uint64_t> (UINT64_MAX)), 64);

	EXPECT_EQ (BIN::count_trailing_ones (static_cast<uint8_t> (0)), 0);
	EXPECT_EQ (BIN::count_trailing_ones (static_cast<uint16_t> (0)), 0);
	EXPECT_EQ (BIN::count_trailing_ones (static_cast<uint32_t> (0)), 0);
	EXPECT_EQ (BIN::count_trailing_ones (static_cast<uint64_t> (0)), 0);

	for (size_t i = 0; i < 2; ++i) {
		EXPECT_EQ (BIN::count_trailing_ones ((static_cast<uint32_t> (0b10) << i) - 1), i + 1);
	}
}

TEST (BIN, count_ones)
{
	EXPECT_EQ (BIN::count_ones (static_cast<uint8_t> (UINT8_MAX)), 8);
	EXPECT_EQ (BIN::count_ones (static_cast<uint16_t> (UINT16_MAX)), 16);
	EXPECT_EQ (BIN::count_ones (static_cast<uint32_t> (UINT32_MAX)), 32);
	EXPECT_EQ (BIN::count_ones (static_cast<uint64_t> (UINT64_MAX)), 64);

	EXPECT_EQ (BIN::count_ones (static_cast<uint8_t> (0)), 0);
	EXPECT_EQ (BIN::count_ones (static_cast<uint16_t> (0)), 0);
	EXPECT_EQ (BIN::count_ones (static_cast<uint32_t> (0)), 0);
	EXPECT_EQ (BIN::count_ones (static_cast<uint64_t> (0)), 0);

	for (size_t i = 0; i <= UINT16_MAX ; ++i) {
		size_t cnt = 0;
		for (size_t j = 0; j < 16; ++j) {
			if ((i & (1 << j)) != 0) {
				++cnt;
			}
		}
		EXPECT_EQ (BIN::count_ones (static_cast<uint16_t> (i)), cnt);
	}
}

TEST (BIN, check_bit_coverage)
{
	EXPECT_TRUE (BIN::check_bit_coverage (UINT8_MAX, UINT8_MAX));
	EXPECT_TRUE (BIN::check_bit_coverage (UINT8_MAX, UINT8_MAX ^ 0b101, 0b100, 0b001));
	EXPECT_FALSE (BIN::check_bit_coverage (UINT8_MAX, UINT8_MAX ^ 0b101, 0b100));
	EXPECT_FALSE (BIN::check_bit_coverage (UINT8_MAX, UINT8_MAX ^ 0b101, 0b100, 0b011));

	EXPECT_TRUE (BIN::check_bit_coverage (UINT16_MAX, UINT16_MAX));
	EXPECT_TRUE (BIN::check_bit_coverage (UINT16_MAX, UINT16_MAX ^ 0b101, 0b100, 0b001));
	EXPECT_FALSE (BIN::check_bit_coverage (UINT16_MAX, UINT16_MAX ^ 0b101, 0b100));
	EXPECT_FALSE (BIN::check_bit_coverage (UINT16_MAX, UINT16_MAX ^ 0b101, 0b100, 0b011));

	EXPECT_TRUE (BIN::check_bit_coverage (UINT32_MAX, UINT32_MAX));
	EXPECT_TRUE (BIN::check_bit_coverage (UINT32_MAX, UINT32_MAX ^ 0b101, 0b100, 0b001));
	EXPECT_FALSE (BIN::check_bit_coverage (UINT32_MAX, UINT32_MAX ^ 0b101, 0b100));
	EXPECT_FALSE (BIN::check_bit_coverage (UINT32_MAX, UINT32_MAX ^ 0b101, 0b100, 0b011));

	EXPECT_TRUE (BIN::check_bit_coverage (UINT64_MAX, UINT64_MAX));
	EXPECT_TRUE (BIN::check_bit_coverage (UINT64_MAX, UINT64_MAX ^ 0b101, 0b100, 0b001));
	EXPECT_FALSE (BIN::check_bit_coverage (UINT64_MAX, UINT64_MAX ^ 0b101, 0b100));
	EXPECT_FALSE (BIN::check_bit_coverage (UINT64_MAX, UINT64_MAX ^ 0b101, 0b100, 0b011));
}

TEST (BIN, big_endian_from)
{
	uint64_t val = 0;
	for (size_t i = 0; i < 8; i++) {
		val |= static_cast<uint64_t> ('A' + i) << (i * 8);
	}

	EXPECT_TRUE (BIN::be_from_uint8 (val & 0xff) == "A");
	EXPECT_TRUE (BIN::be_from_uint16 (val & 0xff'ff) == "BA");
	EXPECT_TRUE (BIN::be_from_uint24 (val & 0xff'ff'ff) == "CBA");
	EXPECT_TRUE (BIN::be_from_uint32 (val & 0xff'ff'ff'ff) == "DCBA");
	EXPECT_TRUE (BIN::be_from_uint64 (val) == "HGFEDCBA");
}

template <typename T>
static void _big_endian_to (void)
{
	uint64_t val = 0;
	uint64_t vbl = 0;
	size_t offset;

	uint64_t testval = 0;
	for (size_t i = 0; i < 8; i++) {
		testval |= static_cast<uint64_t> ('A' + i) << (i * 8);
	}

	val |= BIN::be_to_uint8 (T("A"));
	EXPECT_TRUE (val == (testval & 0xff));

	offset = 0;
	vbl |= BIN::be_to_uint8 (T("A"), offset);
	EXPECT_TRUE (vbl == (testval & 0xff));

	val |= BIN::be_to_uint16 (T("BA"));
	EXPECT_TRUE (val == (testval & 0xff'ff));

	offset = 0;
	vbl |= BIN::be_to_uint16 (T("BA"), offset);
	EXPECT_TRUE (vbl == (testval & 0xff'ff));

	val |= BIN::be_to_uint24 (T("CBA"));
	EXPECT_TRUE (val == (testval & 0xff'ff'ff));

	offset = 0;
	vbl |= BIN::be_to_uint24 (T("CBA"), offset);
	EXPECT_TRUE (vbl == (testval & 0xff'ff'ff));

	val |= BIN::be_to_uint32 (T("DCBA"));
	EXPECT_TRUE (val == (testval & 0xff'ff'ff'ff));

	offset = 0;
	vbl |= BIN::be_to_uint32 (T("DCBA"), offset);
	EXPECT_TRUE (vbl == (testval & 0xff'ff'ff'ff));

	val |= BIN::be_to_uint64 (T("HGFEDCBA"));
	EXPECT_TRUE (val == testval);

	offset = 0;
	vbl |= BIN::be_to_uint64 (T("HGFEDCBA"), offset);
	EXPECT_TRUE (vbl == testval);
};

TEST (BIN, big_endian_to)
{
	_big_endian_to<std::string>();
	_big_endian_to<std::string_view>();
}

TEST (BIN, little_endian_from)
{
	uint64_t val = 0;
	for (size_t i = 0; i < 8; i++) {
		val |= static_cast<uint64_t> ('A' + i) << (i * 8);
	}

	EXPECT_TRUE (BIN::from_uint8 (val & 0xff) == "A");
	EXPECT_TRUE (BIN::from_uint16 (val & 0xff'ff) == "AB");
	EXPECT_TRUE (BIN::from_uint24 (val & 0xff'ff'ff) == "ABC");
	EXPECT_TRUE (BIN::from_uint32 (val & 0xff'ff'ff'ff) == "ABCD");
	EXPECT_TRUE (BIN::from_uint64 (val) == "ABCDEFGH");
}

template <typename T>
static void _little_endian_to (void)
{
	uint64_t val = 0;
	uint64_t vbl = 0;
	size_t offset;

	uint64_t testval = 0;
	for (size_t i = 0; i < 8; i++) {
		testval |= static_cast<uint64_t> ('A' + i) << (i * 8);
	}

	val |= BIN::to_uint8 (T("A"));
	EXPECT_TRUE (val == (testval & 0xff));

	offset = 0;
	vbl |= BIN::to_uint8 (T("A"), offset);
	EXPECT_TRUE (vbl == (testval & 0xff));

	val |= BIN::to_uint16 (T("AB"));
	EXPECT_TRUE (val == (testval & 0xff'ff));

	offset = 0;
	vbl |= BIN::to_uint16 (T("AB"), offset);
	EXPECT_TRUE (vbl == (testval & 0xff'ff));

	val |= BIN::to_uint24 (T("ABC"));
	EXPECT_TRUE (val == (testval & 0xff'ff'ff));

	offset = 0;
	vbl |= BIN::to_uint24 (T("ABC"), offset);
	EXPECT_TRUE (vbl == (testval & 0xff'ff'ff));

	val |= BIN::to_uint32 (T("ABCD"));
	EXPECT_TRUE (val == (testval & 0xff'ff'ff'ff));

	offset = 0;
	vbl |= BIN::to_uint32 (T("ABCD"), offset);
	EXPECT_TRUE (vbl == (testval & 0xff'ff'ff'ff));

	val |= BIN::to_uint64 (T("ABCDEFGH"));
	EXPECT_TRUE (val == testval);

	offset = 0;
	vbl |= BIN::to_uint64 (T("ABCDEFGH"), offset);
	EXPECT_TRUE (vbl == testval);
}

TEST (BIN, little_endian_to)
{
	_little_endian_to<std::string>();
	_little_endian_to<std::string_view>();
}

template <typename T>
static void _size (void)
{
	for (size_t i = 1; i < 29; i++) {
		const size_t cnt = (1 << i) - 1;
		const std::string str = BIN::from_size (cnt);

		size_t offset = 0;
		const size_t output = BIN::to_size (T(str), offset);

		EXPECT_TRUE (str.size () == offset);
		EXPECT_TRUE (cnt == output);
	}
}

TEST (BIN, size)
{
	_size<std::string> ();
	_size<std::string_view> ();
}

//TEST (BIN, special_chars)
//{
//	const uint8_t stx = 0x45;
//	const uint8_t etx = 0x4A;
//	const uint8_t ntx = 0x10;
//
//
//    std::string chars (3, 0);
//    chars.at (0) = ntx;
//    chars.at (1) = stx;
//    chars.at (2) = etx;
//
//    std::string input (2048, 0);
//    for (size_t i = 0; i < input.size (); ++i) {
//    	input.at (i) = i & 0xff;
//    }
//
//    EXPECT_FALSE (input.find_first_of (ntx) == std::string::npos);
//    EXPECT_FALSE (input.find_first_of (stx) == std::string::npos);
//    EXPECT_FALSE (input.find_first_of (etx) == std::string::npos);
//
//    const std::string escaped = BIN::escape_special_chars (input, chars);
//
//    EXPECT_FALSE (escaped.find_first_of (ntx) == std::string::npos);
//    EXPECT_TRUE (escaped.find_first_of (stx) == std::string::npos);
//    EXPECT_TRUE (escaped.find_first_of (etx) == std::string::npos);
//
//    const std::string unescaped = BIN::unescape_special_chars (escaped, ntx);
//    EXPECT_TRUE (input == unescaped);
//
//    uint8_t *str_unescaped = reinterpret_cast<uint8_t *> (malloc (escaped.size ()));
//	ASSERT_FALSE (str_unescaped == nullptr);
//
//    const size_t sze_unescaped = BIN::unescape_special_chars_preallocated (reinterpret_cast<const uint8_t *> (escaped.data ()), escaped.size (), str_unescaped, ntx);
//    EXPECT_TRUE (sze_unescaped == input.size ());
//	EXPECT_TRUE (memcmp (str_unescaped, input.data (), input.size ()) == 0);
//    free (str_unescaped);
//
//    uint8_t *str_escaped = reinterpret_cast<uint8_t *> (strdup (escaped.c_str ()));
//	ASSERT_FALSE (str_escaped == nullptr);
//
////	const size_t sze_unescaped2 = BIN::unescape_special_chars_inline (str_escaped, escaped.size (), ntx);
//    //EXPECT_TRUE (sze_unescaped2 == input.size ());
//	//EXPECT_TRUE (memcmp (str_escaped, input.data (), input.size ()) == 0);
//    free (str_escaped);
//}

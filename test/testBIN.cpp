/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

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

		size_t offset {0};
		const size_t output = BIN::to_size (T(str), offset);

		EXPECT_TRUE (str.size () == offset);
		EXPECT_TRUE (cnt == output);
	}

	// Test random values
	for (size_t i = 0; i < 1'000'000; i++) {
		const size_t cnt = ((rand() & 0x0FFF) << 16) + (rand () & 0xFFFF); // Up to 29 bits
		const std::string str = BIN::from_size (cnt);

		size_t offset {0};
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

TEST (BIN, size_invalid_prefix)
{
	// Construct invalid 4-byte prefix: 1111xxxx which should not be accepted
	std::string invalid4 (4, '\0');
	invalid4[0] = static_cast<char> (0xF0); // 1111 0000
	invalid4[1] = '\0';
	invalid4[2] = '\0';
	invalid4[3] = '\0';

	size_t offset = 0;
	EXPECT_THROW ({
		(void) shaga::BIN::to_size (invalid4, offset);
	}, CommonException);
}

template <typename T>
static void _size_utf8 (void)
{
	// Test valid UTF-8 range (U+0000 to U+10FFFF)
	for (size_t i = 0; i <= 21; i++) { // 21 bits covers U+0000 to U+10FFFF
		const size_t cnt = (i == 0) ? 0 : (1ULL << i) - 1; // Start with 0, then 2^i - 1
		if (cnt > 0x10FFFF) continue; // Skip values beyond UTF-8 range

		std::string str;
		ASSERT_NO_THROW ({
			BIN::from_size_utf8 (cnt, str);
		}) << "Failed to encode size: " << cnt;

		size_t offset {0};
		size_t output {0};
		ASSERT_NO_THROW({
			output = BIN::to_size_utf8(T(str), offset);
		}) << "Failed to decode size: " << cnt;

		EXPECT_EQ (str.size(), offset) << "Offset mismatch for size: " << cnt;
		EXPECT_EQ (cnt, output) << "Value mismatch for size: " << cnt;
	}

	{
		// Test invalid input (> U+10FFFF)
		std::string str;
		EXPECT_THROW ({
			BIN::from_size_utf8 (0x110000, str);
		}, CommonException) << "Expected exception for size > U+10FFFF";
	}

	{
		// Test surrogate range (U+D800 to U+DFFF)
		std::string str;
		EXPECT_THROW ({
			BIN::from_size_utf8 (0xD800, str);
		}, CommonException) << "Expected exception for surrogate code point U+D800";

		str.clear();
		EXPECT_THROW ({
			BIN::from_size_utf8 (0xDFFF, str);
		}, CommonException) << "Expected exception for surrogate code point U+DFFF";

		str.clear();
		EXPECT_THROW ({
			BIN::from_size_utf8 (0xDBFF, str);
		}, CommonException) << "Expected exception for surrogate code point U+DBFF";
	}

	{
		// Test empty string for decoding
		size_t offset {0};
		EXPECT_THROW ({
			BIN::to_size_utf8 (T (""), offset);
		}, std::exception) << "Expected exception for empty string";
	}

	{
		// Test invalid UTF-8 sequence (e.g., lone continuation byte)
		std::string invalid_str;
		invalid_str.push_back (static_cast<char> (0x80)); // Invalid: 10xxxxxx
		size_t offset {0};
		EXPECT_THROW ({
			BIN::to_size_utf8 (T(invalid_str), offset);
		}, CommonException) << "Expected exception for invalid UTF-8 sequence";
	}
}

TEST (BIN, size_utf8)
{
	_size_utf8<std::string> ();
	_size_utf8<std::string_view> ();
}

TEST (BIN, size_utf8_roundtrip)
{
	// Define test cases: each pair contains a UTF-8 string and its expected code points
	struct TestCase {
		std::string input;
		std::vector<size_t> expected_code_points;
	};

	const std::vector<TestCase> test_cases = {
		// Test 1: ASCII (1-byte), 2-byte, 3-byte, and 4-byte sequences
		{
			std::string("zŒϕ𐀀"), // U+007A, U+0152, U+03D5, U+10000
			{0x7A, 0x152, 0x3D5, 0x10000}
		},
		// Test 2: Empty string
		{
			std::string(""),
			{}
		},
		// Test 3: Only ASCII
		{
			std::string("Hello!"),
			{0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x21}
		},
		// Test 4: Mixed multi-byte (Latin, Cyrillic, CJK)
		{
			std::string("éД汉"), // U+00E9, U+0414, U+6C49
			{0xE9, 0x414, 0x6C49}
		},
		// Test 5: Max valid Unicode code point (using raw UTF-8 bytes)
		{
			std::string("\xF4\x8F\xBF\xBF"), // U+10FFFF (raw bytes)
			{0x10FFFF}
		}
	};

	// Test decoding: UTF-8 string to code points
	for (size_t i = 0; i < test_cases.size(); ++i) {
		const auto& test = test_cases[i];
		std::vector<size_t> decoded_points;
		size_t offset = 0;

		// Decode each code point
		while (offset < test.input.size()) {
			size_t code_point = 0;
			ASSERT_NO_THROW({
				code_point = BIN::to_size_utf8(test.input, offset);
			}) << "Failed to decode UTF-8 string at test case " << i << ", offset " << offset;
			decoded_points.push_back(code_point);
		}

		// Verify decoded code points match expected
		EXPECT_EQ(decoded_points, test.expected_code_points)
			<< "Decoded code points mismatch for test case " << i;

		// Verify all bytes were consumed
		EXPECT_EQ(offset, test.input.size())
			<< "Not all bytes consumed for test case " << i;

		// Test encoding: code points back to UTF-8 string
		std::string encoded_str;
		for (size_t code_point : test.expected_code_points) {
			ASSERT_NO_THROW({
				BIN::from_size_utf8(code_point, encoded_str);
			}) << "Failed to encode code point 0x" << std::hex << code_point << " in test case " << i;
		}

		// Verify round-trip: encoded string matches input
		EXPECT_EQ(encoded_str, test.input)
			<< "Round-trip encoding mismatch for test case " << i;
	}

	// Test invalid UTF-8 sequences
	std::vector<std::string> invalid_inputs = {
		std::string("\x80"),           // Lone continuation byte
		std::string("\xC2"),           // Incomplete 2-byte sequence
		std::string("\xE0\x80"),       // Incomplete 3-byte sequence
		std::string("\xF0\x80\x80"),   // Incomplete 4-byte sequence
		std::string("\xC0\x80"),       // Overlong encoding of U+0000
		std::string("\xF5\x80\x80\x80") // Beyond U+10FFFF
	};

	for (size_t i = 0; i < invalid_inputs.size(); ++i) {
		size_t offset = 0;
		EXPECT_THROW({
			BIN::to_size_utf8(invalid_inputs[i], offset);
		}, std::exception) << "Expected exception for invalid UTF-8 sequence in test case " << i;
	}

	// Test encoding invalid code point
	std::string str;
	EXPECT_THROW({
		BIN::from_size_utf8 (0x110000, str);
	}, CommonException) << "Expected exception for code point > U+10FFFF";
}

TEST (BIN, base64)
{
	// Test empty input
	EXPECT_TRUE(BIN::to_base64("") == "");
	EXPECT_TRUE(BIN::from_base64("") == "");

	// Test standard base64
	EXPECT_TRUE(BIN::to_base64("A") == "QQ==");
	EXPECT_TRUE(BIN::to_base64("AB") == "QUI=");
	EXPECT_TRUE(BIN::to_base64("ABC") == "QUJD");
	EXPECT_TRUE(BIN::to_base64("Hello World!") == "SGVsbG8gV29ybGQh");

	// Test alternate base64
	EXPECT_TRUE(BIN::to_base64("A", true) == "QQ--");
	EXPECT_TRUE(BIN::to_base64("AB", true) == "QUI-");
	EXPECT_TRUE(BIN::to_base64("ABC", true) == "QUJD");
	EXPECT_TRUE(BIN::to_base64("Hello World!", true) == "SGVsbG8gV29ybGQh");

	// Test decoding
	EXPECT_TRUE(BIN::from_base64("QQ==") == "A");
	EXPECT_TRUE(BIN::from_base64("QUI=") == "AB");
	EXPECT_TRUE(BIN::from_base64("QUJD") == "ABC");
	EXPECT_TRUE(BIN::from_base64("SGVsbG8gV29ybGQh") == "Hello World!");

	// Test alternate decoding
	EXPECT_TRUE(BIN::from_base64("QQ--", true) == "A");
	EXPECT_TRUE(BIN::from_base64("QUI-", true) == "AB");
	EXPECT_TRUE(BIN::from_base64("QUJD", true) == "ABC");
	EXPECT_TRUE(BIN::from_base64("SGVsbG8gV29ybGQh", true) == "Hello World!");

	// Test binary data
	std::string binary;
	for (size_t i = 0; i < 256; ++i) {
		binary.push_back(static_cast<char>(i));
	}
	const std::string encoded = BIN::to_base64(binary);
	EXPECT_TRUE(BIN::from_base64(encoded) == binary);

	// Test error cases
	EXPECT_THROW(BIN::from_base64("QQ="), CommonException);  // Invalid length
	EXPECT_THROW(BIN::from_base64("Q==="), CommonException); // Invalid padding
	EXPECT_THROW(BIN::from_base64("===="), CommonException); // Invalid input
	EXPECT_THROW(BIN::from_base64("!@#$"), CommonException); // Invalid characters

	// Test append functionality
	std::string append_test;
	BIN::to_base64("Test", append_test);
	BIN::to_base64(" append", append_test);
	EXPECT_TRUE(append_test == "VGVzdA==IGFwcGVuZA==");

	std::string decoded;
	BIN::from_base64("VGVzdA==", decoded);
	BIN::from_base64("IGFwcGVuZA==", decoded);
	EXPECT_TRUE(decoded == "Test append");
}

// Test base64 with random data of various lengths
TEST (BIN, base64_random)
{
	std::string input;
	for (size_t len = 1; len < 1000; len += 111) {
		input.clear();
		for (size_t i = 0; i < len; ++i) {
			input.push_back(static_cast<char>(rand() & 0xFF));
		}

		// Test standard base64
		const std::string encoded = BIN::to_base64(input);
		const std::string decoded = BIN::from_base64(encoded);
		EXPECT_TRUE(decoded == input);

		// Test alternate base64
		const std::string alt_encoded = BIN::to_base64(input, true);
		const std::string alt_decoded = BIN::from_base64(alt_encoded, true);
		EXPECT_TRUE(alt_decoded == input);
	}
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

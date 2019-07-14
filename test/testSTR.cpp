/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <shaga.h>
#include <gtest/gtest.h>

using namespace shaga;

TEST (STR, icompare)
{
	EXPECT_TRUE (STR::icompare ("AHOJ"sv, "ahoj"sv));
	EXPECT_TRUE (STR::icompare ("01_3"sv, "01_3"sv));
	EXPECT_FALSE (STR::icompare ("AHOJ"sv, "ahoi"sv));
	EXPECT_FALSE (STR::icompare ("AHOJ"sv, "ahoj "sv));
	EXPECT_FALSE (STR::icompare ("AHOJ"sv, ""sv));
	EXPECT_FALSE (STR::icompare ("", "AHOJ"));
}

TEST (STR, has_prefix)
{
	EXPECT_TRUE (STR::has_prefix ("toto je veta", "toto"));
	EXPECT_TRUE (STR::has_prefix ("toto je veta", "toto "));
	EXPECT_TRUE (STR::has_prefix ("toto je veta", ""));
	EXPECT_TRUE (STR::has_prefix ("toto", "toto"));
	EXPECT_TRUE (STR::has_prefix ("", ""));

	EXPECT_FALSE (STR::has_prefix ("toto je veta", " toto"));
	EXPECT_FALSE (STR::has_prefix ("tot", "toto"));
	EXPECT_FALSE (STR::has_prefix ("", "toto"));
}

TEST (STR, has_iprefix)
{
	EXPECT_TRUE (STR::has_iprefix ("toto je veta", "TOTO"));
	EXPECT_TRUE (STR::has_iprefix ("toto je veta", "TOTO "));
	EXPECT_TRUE (STR::has_iprefix ("toto je veta", ""));
	EXPECT_TRUE (STR::has_iprefix ("toto", "TOTO"));
	EXPECT_TRUE (STR::has_iprefix ("", ""));

	EXPECT_FALSE (STR::has_iprefix ("toto je veta", " toto"));
	EXPECT_FALSE (STR::has_iprefix ("tot", "toto"));
	EXPECT_FALSE (STR::has_iprefix ("", "toto"));
}

TEST (STR, has_suffix)
{
	EXPECT_TRUE (STR::has_suffix ("toto je veta", "veta"));
	EXPECT_TRUE (STR::has_suffix ("toto je veta", " veta"));
	EXPECT_TRUE (STR::has_suffix ("toto je veta", ""));
	EXPECT_TRUE (STR::has_suffix ("toto", "toto"));
	EXPECT_TRUE (STR::has_suffix ("", ""));

	EXPECT_FALSE (STR::has_suffix ("toto je veta", "veta "));
	EXPECT_FALSE (STR::has_suffix ("tot", "toto"));
	EXPECT_FALSE (STR::has_suffix ("", "toto"));
}

TEST (STR, has_isuffix)
{
	EXPECT_TRUE (STR::has_isuffix ("toto je veta", "VETA"));
	EXPECT_TRUE (STR::has_isuffix ("toto je veta", " VETA"));
	EXPECT_TRUE (STR::has_isuffix ("toto je veta", ""));
	EXPECT_TRUE (STR::has_isuffix ("toto", "TOTO"));
	EXPECT_TRUE (STR::has_isuffix ("", ""));

	EXPECT_FALSE (STR::has_isuffix ("toto je veta", "VETA "));
	EXPECT_FALSE (STR::has_isuffix ("tot", "TOTO"));
	EXPECT_FALSE (STR::has_isuffix ("", "TOTO"));
}

TEST (STR, sprintf)
{
	std::string str;

	str = STR::sprintf ("%d %X %g %c", 100, 0x100, 100.100d, 'x');
	EXPECT_TRUE (str == "100 100 100.1 x") << str;

	str = STR::sprintf ("1234567");
	EXPECT_TRUE (str == "1234567") << str;

	str = STR::sprintf ("%s", "12345678");
	EXPECT_TRUE (str == "12345678") << str;
}

TEST (STR, format)
{
	std::string str;

	str = STR::format ("123456789");
	EXPECT_TRUE (str == "123456789") << str;

	str = STR::format ("{}", "123456789"sv);
	EXPECT_TRUE (str == "123456789") << str;
}

TEST (STR, bool)
{
	EXPECT_TRUE (STR::to_bool ("oN"));
	EXPECT_TRUE (STR::to_bool ("yeS"));
	EXPECT_TRUE (STR::to_bool ("TruE"));
	EXPECT_TRUE (STR::to_bool ("1"));

	EXPECT_FALSE (STR::to_bool ("ofF"));
	EXPECT_FALSE (STR::to_bool ("nO"));
	EXPECT_FALSE (STR::to_bool ("FaLse"));
	EXPECT_FALSE (STR::to_bool ("0"));

	EXPECT_THROW (STR::to_bool ("something else"), CommonException);
	EXPECT_THROW (STR::to_bool (""), CommonException);

	EXPECT_TRUE (STR::from_int (static_cast <bool> (true)) == "true");
	EXPECT_TRUE (STR::from_int (static_cast <bool> (false)) == "false");
}

TEST (STR, uint8)
{
	EXPECT_TRUE (STR::to_uint8 ("0") == 0);
	EXPECT_TRUE (STR::to_uint8 ("255") == 255);
	EXPECT_TRUE (STR::to_uint8 ("ff", 16) == 255);

	EXPECT_TRUE (STR::from_int (static_cast <uint8_t> (0)) == "0");
	EXPECT_TRUE (STR::from_int (static_cast <uint8_t> (255)) == "255");

	EXPECT_THROW (STR::to_uint8 ("-1"), CommonException);
	EXPECT_THROW (STR::to_uint8 ("256"), CommonException);
}

TEST (STR, uint16)
{
	EXPECT_TRUE (STR::to_uint16 ("0") == 0);
	EXPECT_TRUE (STR::to_uint16 ("65535") == 65535);
	EXPECT_TRUE (STR::to_uint16 ("fFfF", 16) == 65535);

	EXPECT_TRUE (STR::from_int (static_cast <uint16_t> (0)) == "0");
	EXPECT_TRUE (STR::from_int (static_cast <uint16_t> (65535)) == "65535");

	EXPECT_THROW (STR::to_uint16 ("-1"), CommonException);
	EXPECT_THROW (STR::to_uint16 ("65536"), CommonException);

}

TEST (STR, uint32)
{
	EXPECT_TRUE (STR::to_uint32 ("0") == 0);
	EXPECT_TRUE (STR::to_uint32 ("4294967295") == 4294967295LL);
	EXPECT_TRUE (STR::to_uint32 ("ffffFFff", 16) == 4294967295LL);

	EXPECT_TRUE (STR::from_int (static_cast <uint32_t> (0)) == "0");
	EXPECT_TRUE (STR::from_int (static_cast <uint32_t> (4294967295LL)) == "4294967295");

	EXPECT_THROW (STR::to_uint32 ("-1"), CommonException);
	EXPECT_THROW (STR::to_uint32 ("4294967296"), CommonException);

}

TEST (STR, uint64)
{
	EXPECT_TRUE (STR::to_uint64 ("0") == 0);
	EXPECT_TRUE (STR::to_uint64 ("18446744073709551615") == 18446744073709551615ULL);
	EXPECT_TRUE (STR::to_uint64 ("   ff'FF'ff'ff'ff'ff'ff'ff   ", 16) == 18446744073709551615ULL);

	EXPECT_TRUE (STR::from_int (static_cast <uint64_t> (0)) == "0");
	EXPECT_TRUE (STR::from_int (static_cast <uint64_t> (18446744073709551615ULL)) == "18446744073709551615");

	EXPECT_THROW (STR::to_uint64 ("-1"), CommonException);
	EXPECT_THROW (STR::to_uint64 ("18446744073709551616"), CommonException);
	EXPECT_THROW (STR::to_uint64 ("ff'ff'ff'FF'ff'ff'ff'ff'00", 16), CommonException);
	EXPECT_THROW (STR::to_uint64 ("a", 10), CommonException);
	EXPECT_THROW (STR::to_uint64 ("0", 1), CommonException);
}

TEST (STR, int8)
{
	EXPECT_TRUE (STR::to_int8 ("-128") == (-128));
	EXPECT_TRUE (STR::to_int8 ("127") == 127);
	EXPECT_TRUE (STR::to_int8 ("7f", 16) == 127);
	EXPECT_TRUE (STR::to_int8 ("7F", 16) == 127);

	EXPECT_TRUE (STR::from_int (static_cast <int8_t> (-128)) == "-128");
	EXPECT_TRUE (STR::from_int (static_cast <int8_t> (127)) == "127");

	EXPECT_THROW (STR::to_int8 ("-129"), CommonException);
	EXPECT_THROW (STR::to_int8 ("128"), CommonException);
}

TEST (STR, int16)
{
	EXPECT_TRUE (STR::to_int16 ("-32768") == (-32768));
	EXPECT_TRUE (STR::to_int16 ("32767") == 32767);
	EXPECT_TRUE (STR::to_int16 ("7ffF", 16) == 32767);

	EXPECT_TRUE (STR::from_int (static_cast <int16_t> (-32768)) == "-32768");
	EXPECT_TRUE (STR::from_int (static_cast <int16_t> (32767)) == "32767");

	EXPECT_THROW (STR::to_int16 ("-32769"), CommonException);
	EXPECT_THROW (STR::to_int16 ("32768"), CommonException);

}

TEST (STR, int32)
{
	EXPECT_TRUE (STR::to_int32 ("-2147483648") == (-2147483648LL));
	EXPECT_TRUE (STR::to_int32 ("2147483647") == 2147483647LL);
	EXPECT_TRUE (STR::to_int32 ("7ffffFff", 16) == 2147483647LL);

	EXPECT_TRUE (STR::from_int (static_cast <int32_t> (-2147483648LL)) == "-2147483648");
	EXPECT_TRUE (STR::from_int (static_cast <int32_t> (2147483647LL)) == "2147483647");

	EXPECT_THROW (STR::to_int32 ("-2147483649"), CommonException);
	EXPECT_THROW (STR::to_int32 ("2147483648"), CommonException);

}

TEST (STR, int64)
{
	EXPECT_TRUE (STR::to_int64 ("-9223372036854775808") == (-9223372036854775807LL - 1));
	EXPECT_TRUE (STR::to_int64 ("9223372036854775807") == 9223372036854775807LL);
	EXPECT_TRUE (STR::to_int64 ("7fffffFFFFffffff", 16) == 9223372036854775807LL);

	EXPECT_TRUE (STR::from_int (static_cast <int64_t> (-9223372036854775807LL - 1)) == "-9223372036854775808");
	EXPECT_TRUE (STR::from_int (static_cast <int64_t> (9223372036854775807LL)) == "9223372036854775807");

	EXPECT_THROW (STR::to_int64 ("-9223372036854775809"), CommonException);
	EXPECT_THROW (STR::to_int64 ("9223372036854775808"), CommonException);
}

TEST (STR, trim)
{
	const std::string_view chars ("\n\r\t\0 ", 5);
	const char arr[] = {' ', ' ', 'a', 'b', ' ', 'c', 'd', ' ', ' ', '\0', '\0', '\0'};
	const char arr2[] = {' ', ' ', '\t', '\t', ' ', '\t', '\t', ' ', ' ', '\0', '\0', '\0'};

	auto ltrim = [](const char *arr, const size_t len, const std::string_view chars) -> size_t
	{
		std::string s (arr, len);
		std::string_view v (arr, len);

		STR::ltrim (s, chars);
		STR::ltrim (v, chars);

		EXPECT_TRUE (s.size () == v.size ());
		return s.size ();
	};

	auto rtrim = [](const char *arr, const size_t len, const std::string_view chars) -> size_t
	{
		std::string s (arr, len);
		std::string_view v (arr, len);

		STR::rtrim (s, chars);
		STR::rtrim (v, chars);

		EXPECT_TRUE (s.size () == v.size ()) << s.size () << " " << v.size ();
		return s.size ();
	};

	auto trim = [](const char *arr, const size_t len, const std::string_view chars) -> size_t
	{
		std::string s (arr, len);
		std::string_view v (arr, len);

		STR::trim (s, chars);
		STR::trim (v, chars);

		EXPECT_TRUE (s.size () == v.size ());
		return s.size ();
	};

	EXPECT_TRUE (ltrim (arr, sizeof (arr), chars) == sizeof (arr) - 2);
	EXPECT_TRUE (rtrim (arr, sizeof (arr), chars) == sizeof (arr) - 5);
	EXPECT_TRUE (trim (arr, sizeof (arr), chars) == sizeof (arr) - 7);

	EXPECT_TRUE (ltrim (arr2, sizeof (arr2), chars) == 0);
	EXPECT_TRUE (rtrim (arr2, sizeof (arr2), chars) == 0);
	EXPECT_TRUE (trim (arr2, sizeof (arr2), chars) == 0);
}

TEST (STR, split_join)
{
	const std::string original {",Aa,Bb,Cc,:Dd,:Ee,:::,Zz,"};
	const std::string joined {"Aa,Bb,Cc,Dd,Ee,Zz"};
	const std::string nothing = "";
	const std::string delimiters = ",:";
	const std::string delimiter = ",";
	COMMON_VECTOR v;
	COMMON_DEQUE d;
	COMMON_LIST l;

	v = STR::split<COMMON_VECTOR> (original, delimiters);
	d = STR::split<COMMON_DEQUE> (original, delimiters);
	l = STR::split<COMMON_LIST> (original, delimiters);

	EXPECT_TRUE (v.size () == 6);
	EXPECT_TRUE (d.size () == 6);
	EXPECT_TRUE (l.size () == 6);

	EXPECT_TRUE (STR::join (v, ",") == joined);
	EXPECT_TRUE (STR::join (d, ",") == joined);
	EXPECT_TRUE (STR::join (l, ",") == joined);

	STR::split (v, original, delimiter);
	STR::split (d, original, delimiter);
	STR::split (l, original, delimiter);

	EXPECT_TRUE (v.size () == 13);
	EXPECT_TRUE (d.size () == 13);
	EXPECT_TRUE (l.size () == 13);

	STR::split (v, nothing, delimiter);
	STR::split (d, nothing, delimiter);
	STR::split (l, nothing, delimiter);

	EXPECT_TRUE (v.size () == 13);
	EXPECT_TRUE (d.size () == 13);
	EXPECT_TRUE (l.size () == 13);
}

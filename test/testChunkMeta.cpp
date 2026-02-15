/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

TEST (ChunkMeta, add_value)
{
	ChunkMeta meta;

	EXPECT_NO_THROW (meta.add_value ("ABC", "abc"sv));
	EXPECT_NO_THROW (meta.add_value ("XYZ", "abc"sv));
	EXPECT_NO_THROW (meta.add_value ("ABC", "def"sv));
	EXPECT_NO_THROW (meta.add_value ("XYZ", "def"sv));

	EXPECT_TRUE (meta.size () == 4);
	EXPECT_TRUE (meta.count ("ABC") == 2);
	EXPECT_TRUE (meta.count ("XYZ") == 2);
	EXPECT_TRUE (meta.count ("DEF") == 0);
}

TEST (ChunkMeta, add_numeric_getters)
{
	ChunkMeta meta;

	const uint16_t key_u8 = ChunkMeta::key_to_bin ("UAA");
	const uint16_t key_i8 = ChunkMeta::key_to_bin ("IAB");
	const uint16_t key_u16 = ChunkMeta::key_to_bin ("UAC");
	const uint16_t key_i16 = ChunkMeta::key_to_bin ("IAD");
	const uint16_t key_u32 = ChunkMeta::key_to_bin ("UAE");
	const uint16_t key_i32 = ChunkMeta::key_to_bin ("IAF");
	const uint16_t key_u64 = ChunkMeta::key_to_bin ("UAG");
	const uint16_t key_i64 = ChunkMeta::key_to_bin ("IAH");

	EXPECT_NO_THROW (meta.add_uint8 ("UAA", static_cast<uint8_t>(200)));
	EXPECT_NO_THROW (meta.add_int8 (key_i8, static_cast<int8_t>(-5)));

	EXPECT_NO_THROW (meta.add_uint16 ("UAC", static_cast<uint16_t>(65530)));
	EXPECT_NO_THROW (meta.add_int16 (key_i16, static_cast<int16_t>(-1234)));

	EXPECT_NO_THROW (meta.add_uint32 ("UAE", 0x10203040u));
	EXPECT_NO_THROW (meta.add_int32 (key_i32, static_cast<int32_t>(-123456)));

	EXPECT_NO_THROW (meta.add_uint64 ("UAG", 0x1020304050607080ULL));
	EXPECT_NO_THROW (meta.add_int64 (key_i64, static_cast<int64_t>(-1234567890123LL)));

	EXPECT_EQ (meta.get_uint8 ("UAA", 0), 200);
	EXPECT_EQ (meta.get_uint8 (key_u8, 0), 200);
	EXPECT_EQ (meta.get_int8 ("IAB", 0), -5);
	EXPECT_EQ (meta.get_int8 (key_i8, 0), -5);

	EXPECT_EQ (meta.get_uint16 ("UAC", 0), 65530);
	EXPECT_EQ (meta.get_uint16 (key_u16, 0), 65530);
	EXPECT_EQ (meta.get_int16 ("IAD", 0), -1234);
	EXPECT_EQ (meta.get_int16 (key_i16, 0), -1234);

	EXPECT_EQ (meta.get_uint32 ("UAE", 0), 0x10203040u);
	EXPECT_EQ (meta.get_uint32 (key_u32, 0), 0x10203040u);
	EXPECT_EQ (meta.get_int32 ("IAF", 0), -123456);
	EXPECT_EQ (meta.get_int32 (key_i32, 0), -123456);

	EXPECT_EQ (meta.get_uint64 ("UAG", 0), 0x1020304050607080ULL);
	EXPECT_EQ (meta.get_uint64 (key_u64, 0), 0x1020304050607080ULL);
	EXPECT_EQ (meta.get_int64 ("IAH", 0), -1234567890123LL);
	EXPECT_EQ (meta.get_int64 (key_i64, 0), -1234567890123LL);
}

TEST (ChunkMeta, add_value_numeric)
{
	ChunkMeta meta;

	EXPECT_NO_THROW (meta.add_value ("TUA", static_cast<uint8_t>(9)));
	EXPECT_NO_THROW (meta.add_value ("TIA", static_cast<int16_t>(-9)));
	EXPECT_NO_THROW (meta.add_value ("TUB", static_cast<uint32_t>(1234)));
	EXPECT_NO_THROW (meta.add_value ("TIC", static_cast<int64_t>(-123456)));

	EXPECT_EQ (meta.get_uint8 ("TUA", 0), 9);
	EXPECT_EQ (meta.get_int16 ("TIA", 0), -9);
	EXPECT_EQ (meta.get_uint32 ("TUB", 0), 1234u);
	EXPECT_EQ (meta.get_int64 ("TIC", 0), -123456);
}

TEST (ChunkMeta, ctor_numeric)
{
	ChunkMeta meta_u ("CUA", static_cast<uint16_t>(42));
	EXPECT_EQ (meta_u.get_uint16 ("CUA", 0), 42);

	ChunkMeta meta_i (ChunkMeta::key_to_bin ("CIB"), static_cast<int32_t>(-42));
	EXPECT_EQ (meta_i.get_int32 ("CIB", 0), -42);
}

TEST (ChunkMeta, get_defaults_optional)
{
	ChunkMeta meta;

	EXPECT_EQ (meta.get_int32 ("DEF", -7), -7);
	EXPECT_FALSE (meta.get_int32_optional ("DEF").has_value ());

	EXPECT_NO_THROW (meta.add_value ("SML", "A"sv));
	EXPECT_EQ (meta.get_uint32 ("SML", 55u), 55u);
	EXPECT_FALSE (meta.get_uint32_optional ("SML").has_value ());

	EXPECT_NO_THROW (meta.add_int16 ("OPT", static_cast<int16_t>(-12)));
	const auto opt = meta.get_int16_optional ("OPT");
	EXPECT_TRUE (opt.has_value ());
	EXPECT_EQ (opt.value (), -12);
}

TEST (ChunkMeta, get_bool_optional)
{
	ChunkMeta meta;

	const uint16_t key_false = ChunkMeta::key_to_bin ("BFA");
	const uint16_t key_true = ChunkMeta::key_to_bin ("BTA");
	const uint16_t key_nonzero = ChunkMeta::key_to_bin ("BNZ");

	EXPECT_FALSE (meta.get_bool_optional ("MIS").has_value ());
	EXPECT_FALSE (meta.get_bool_optional (ChunkMeta::key_to_bin ("MIS")).has_value ());

	EXPECT_NO_THROW (meta.add_bool (key_false, false));
	EXPECT_NO_THROW (meta.add_bool ("BTA", true));
	EXPECT_NO_THROW (meta.add_uint8 (key_nonzero, static_cast<uint8_t>(2)));

	const auto opt_false_sv = meta.get_bool_optional ("BFA");
	const auto opt_false_u16 = meta.get_bool_optional (key_false);
	EXPECT_TRUE (opt_false_sv.has_value ());
	EXPECT_TRUE (opt_false_u16.has_value ());
	EXPECT_FALSE (opt_false_sv.value ());
	EXPECT_FALSE (opt_false_u16.value ());

	const auto opt_true_sv = meta.get_bool_optional ("BTA");
	const auto opt_true_u16 = meta.get_bool_optional (key_true);
	EXPECT_TRUE (opt_true_sv.has_value ());
	EXPECT_TRUE (opt_true_u16.has_value ());
	EXPECT_TRUE (opt_true_sv.value ());
	EXPECT_TRUE (opt_true_u16.value ());

	const auto opt_nonzero_sv = meta.get_bool_optional ("BNZ");
	const auto opt_nonzero_u16 = meta.get_bool_optional (key_nonzero);
	EXPECT_TRUE (opt_nonzero_sv.has_value ());
	EXPECT_TRUE (opt_nonzero_u16.has_value ());
	EXPECT_TRUE (opt_nonzero_sv.value ());
	EXPECT_TRUE (opt_nonzero_u16.value ());
}

TEST (ChunkMeta, get_bool)
{
	ChunkMeta meta;

	const uint16_t key_false = ChunkMeta::key_to_bin ("NFA");
	const uint16_t key_true = ChunkMeta::key_to_bin ("NTA");
	const uint16_t key_nonzero = ChunkMeta::key_to_bin ("NNZ");

	EXPECT_TRUE (meta.get_bool ("MIS", true));
	EXPECT_FALSE (meta.get_bool (ChunkMeta::key_to_bin ("MIS"), false));

	EXPECT_NO_THROW (meta.add_bool (key_false, false));
	EXPECT_NO_THROW (meta.add_bool ("NTA", true));
	EXPECT_NO_THROW (meta.add_uint8 (key_nonzero, static_cast<uint8_t>(7)));

	EXPECT_FALSE (meta.get_bool ("NFA", true));
	EXPECT_FALSE (meta.get_bool (key_false, true));

	EXPECT_TRUE (meta.get_bool ("NTA", false));
	EXPECT_TRUE (meta.get_bool (key_true, false));

	EXPECT_TRUE (meta.get_bool ("NNZ", false));
	EXPECT_TRUE (meta.get_bool (key_nonzero, false));
}

TEST (ChunkMeta, modify_value_missing)
{
	ChunkMeta meta;
	bool called = false;

	EXPECT_NO_THROW (meta.modify_value ("ABC", [&](std::string &value) {
		called = true;
		value = "init";
	}));

	EXPECT_TRUE (called);
	EXPECT_EQ (meta.count ("ABC"), 1);
	EXPECT_EQ (meta.get_value ("ABC"), "init"sv);
}

TEST (ChunkMeta, modify_values_missing)
{
	ChunkMeta meta;
	bool called = false;

	EXPECT_NO_THROW (meta.modify_values (ChunkMeta::key_to_bin ("XYZ"), [&](std::string &value) {
		called = true;
		value = "seed";
		return true;
	}));

	EXPECT_TRUE (called);
	EXPECT_EQ (meta.count ("XYZ"), 1);
	EXPECT_EQ (meta.get_value ("XYZ"), "seed"sv);
}

TEST (ChunkMeta, erase_find)
{
	ChunkMeta meta;

	meta.add_value ("ABC", "one"sv);
	meta.add_value ("ABC", "two"sv);
	meta.add_value ("XYZ", "three"sv);

	EXPECT_EQ (meta.count ("ABC"), 2);
	EXPECT_EQ (meta.count ("XYZ"), 1);

	EXPECT_EQ (meta.erase ("ABC"), 2);
	EXPECT_EQ (meta.count ("ABC"), 0);

	EXPECT_NE (meta.find ("XYZ"), meta.cend ());
	EXPECT_EQ (meta.find ("ABC"), meta.cend ());
}

TEST (ChunkMeta, bin_roundtrip_mixed)
{
	ChunkMeta meta;

	meta.add_value ("AAA", "alpha"sv);
	meta.add_uint16 ("BBB", static_cast<uint16_t>(123));
	meta.add_int32 ("CCC", static_cast<int32_t>(-456));
	meta.add_value ("RPT", "first"sv);
	meta.add_value ("RPT", "second"sv);

	const std::string bin = meta.to_bin ();

	ChunkMeta restored;
	size_t offset = 0;
	EXPECT_NO_THROW (restored.from_bin (bin, offset));
	EXPECT_EQ (offset, bin.size ());

	EXPECT_EQ (restored.count ("AAA"), 1);
	EXPECT_EQ (restored.get_value ("AAA"), "alpha"sv);
	EXPECT_EQ (restored.get_uint16 ("BBB", 0), 123);
	EXPECT_EQ (restored.get_int32 ("CCC", 0), -456);

	EXPECT_EQ (restored.count ("RPT"), 2);
	auto rpt_values = restored.get_values<std::vector<std::string>> ("RPT");
	std::sort (rpt_values.begin (), rpt_values.end ());
	EXPECT_EQ (rpt_values.size (), 2u);
	EXPECT_EQ (rpt_values[0], "first");
	EXPECT_EQ (rpt_values[1], "second");
}

TEST (ChunkMeta, modify_value)
{
	ChunkMeta meta;
	meta.add_value ("ABC", "abc"sv);
	meta.add_value ("XYZ", "abc"sv);
	meta.add_value ("ABC", "def"sv);
	meta.add_value ("XYZ", "def"sv);

	auto func = [](std::string &str) -> void {
		if (str == "abc") {
			str = "def";
		}
		else if (str == "def") {
			str = "abc";
		}
		else {
			cThrow ("Error");
		}
	};

	auto func_true = [&](std::string &str) -> bool {
		func (str);
		return true;
	};

	auto func_false = [&](std::string &str) -> bool {
		func (str);
		return false;
	};

	auto test = [&](const std::string &key) -> std::string {
		auto res = meta.get_values (key);
		EXPECT_TRUE (res.size () == 2);
		EXPECT_TRUE (res.front () == res.back ());

		return res.front ();
	};

	auto test_diff = [&](const std::string &key) -> void {
		auto res = meta.get_values (key);
		EXPECT_TRUE (res.size () == 2);
		EXPECT_FALSE (res.front () == res.back ());
	};

	/* Modify only one entry pre key */
	EXPECT_NO_THROW (meta.modify_value ("ABC", func));
	EXPECT_NO_THROW (meta.modify_value (ChunkMeta::key_to_bin ("XYZ"), func));

	const std::string result_abc = test ("ABC");
	const std::string result_xyz = test ("XYZ");

	/* Now modify all entries for key */
	EXPECT_NO_THROW (meta.modify_values ("ABC", func_true));
	EXPECT_NO_THROW (meta.modify_values (ChunkMeta::key_to_bin ("XYZ"), func_true));

	/* All entries should be swapped again */
	const std::string result_abc2 = test ("ABC");
	const std::string result_xyz2 = test ("XYZ");

	/* It has to be different than in first try */
	EXPECT_FALSE (result_abc == result_abc2);
	EXPECT_FALSE (result_xyz == result_xyz2);

	/* Now modify only one entry for key */
	EXPECT_NO_THROW (meta.modify_values ("ABC", func_false));
	EXPECT_NO_THROW (meta.modify_values (ChunkMeta::key_to_bin ("XYZ"), func_false));

	/* Now the results should be different */
	test_diff ("ABC");
	test_diff ("XYZ");
}

TEST (ChunkMeta, merge)
{
	ChunkMeta meta[3];

	for (int id = 0; id < 3; ++id) {
		for (int i = 0; i < 10; ++i) {
			meta[id].add_value ("ABC", std::string_view("ahoj"));
			meta[id].add_value ("ABC", std::string_view("bhoj"));
			meta[id].add_value ("ABC");
			meta[id].add_value ("BBC", std::string_view("ahoj"));
			meta[id].add_value ("CBC", std::string_view("ahoj"));

			meta[id].add_value ((id * 10 + i) + 1);
		}

		meta[id].unique ();

		/* 5 shared between all and 10 unique for each id */
		EXPECT_TRUE (meta[id].size () == 15);
	}

	meta[0].merge (meta[1]);
	EXPECT_TRUE (meta[0].size () == 25);
	EXPECT_TRUE (meta[1].size () == 15);

	meta[0].merge (std::move (meta[2]));
	EXPECT_TRUE (meta[0].size () == 35);
	EXPECT_TRUE (meta[2].size () == 0);
}

TEST (ChunkMeta, unique)
{
	ChunkMeta meta;

	for (int i = 0; i < 10; ++i) {
		meta.add_value ("ABC", std::string_view("ahoj"));
		meta.add_value ("ABC", std::string_view("bhoj"));
		meta.add_value ("ABC");
		meta.add_value ("BBC", std::string_view("ahoj"));
		meta.add_value ("CBC", std::string_view("ahoj"));
	}

	EXPECT_TRUE (meta.size () > 5);
	meta.unique ();
	EXPECT_TRUE (meta.size () == 5);

	meta.add_value ("CBC", std::string_view("ahoj"));
	meta.add_value ("CBC");

	EXPECT_TRUE (meta.size () > 6);
	meta.unique ();
	EXPECT_TRUE (meta.size () == 6);
}

TEST (ChunkMeta, keys)
{
	const char chars[] = "_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	ChunkMeta meta;

	for (int64_t i = 0; i < 19683; i++) {
		std::string key;
		key.append (1, chars[(i) % 27]);
		key.append (1, chars[(i/27) % 27]);
		key.append (1, chars[(i/729) % 27]);

		EXPECT_NO_THROW (meta.add_value (key, std::string_view())) << "Key value: " << key;
		EXPECT_NO_THROW (meta.add_value (key, BIN::from_int64 (i))) << "Key value: " << key;

		const uint16_t k = ChunkMeta::key_to_bin (key);
		const uint16_t ck = _chunkmeta_key_to_bin (key.c_str ());

		EXPECT_TRUE (k == ck);

		const std::string skey = ChunkMeta::bin_to_key (k);
		EXPECT_TRUE (skey == key);
	}

	EXPECT_TRUE (meta.size () == (19683 * 2));

	for (int64_t i = 0; i < 19683; i++) {
		std::string key;
		key.append (1, chars[(i/729) % 27]);
		key.append (1, chars[(i/27) % 27]);
		key.append (1, chars[(i) % 27]);

		EXPECT_TRUE (meta.count (key) == 2);
	}

	EXPECT_THROW (meta.add_value ("", std::string_view()), CommonException);
	EXPECT_THROW (meta.add_value ("A", std::string_view()), CommonException);
	EXPECT_THROW (meta.add_value ("x", std::string_view()), CommonException);
	EXPECT_THROW (meta.add_value ("xyz", std::string_view()), CommonException);
	EXPECT_THROW (meta.add_value ("ABCD", std::string_view()), CommonException);
}

TEST (ChunkMeta, bin)
{
	const char chars[] = "_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	ChunkMeta meta1, meta2;

	for (int64_t i = 0; i < 19683; i++) {
		std::string key;
		key.append (1, chars[(i) % 27]);
		key.append (1, chars[(i/27) % 27]);
		key.append (1, chars[(i/729) % 27]);
		EXPECT_NO_THROW (meta1.add_value (key, BIN::from_int64 (i))) << "Key value: " << key;
		EXPECT_NO_THROW (meta1.add_value (key, BIN::from_int64 (i))) << "Key value: " << key;
	}

	for (int64_t i = 19682; i >= 0; i--) {
		std::string key;
		key.append (1, chars[(i) % 27]);
		key.append (1, chars[(i/27) % 27]);
		key.append (1, chars[(i/729) % 27]);
		EXPECT_NO_THROW (meta2.add_value (key, BIN::from_int64 (i))) << "Key value: " << key;
		EXPECT_NO_THROW (meta2.add_value (key, BIN::from_int64 (i))) << "Key value: " << key;
	}

	std::string bin1, bin2;
	EXPECT_NO_THROW (meta1.to_bin (bin1));
	EXPECT_NO_THROW (meta2.to_bin (bin2));

	EXPECT_TRUE (bin1.size () <= meta1.get_max_bytes ());
	EXPECT_TRUE (bin2.size () <= meta2.get_max_bytes ());

	meta1.reset ();
	EXPECT_TRUE (meta1.size () == 0);

	size_t offset = 0;
	EXPECT_NO_THROW (meta1.from_bin (bin1, offset));
	EXPECT_TRUE (offset == bin1.size ());

	EXPECT_TRUE (meta1.size () == (19683 * 2));

	for (int64_t i = 0; i < 19683; i++) {
		std::string key;
		key.append (1, chars[(i) % 27]);
		key.append (1, chars[(i/27) % 27]);
		key.append (1, chars[(i/729) % 27]);

		EXPECT_TRUE (meta1.count (key) == 2);
	}
}

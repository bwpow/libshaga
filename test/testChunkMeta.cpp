/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

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
		const uint16_t ck = ChMetaKEY (key.c_str ());

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

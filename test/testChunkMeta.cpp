/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.txt):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <shaga.h>
#include <gtest/gtest.h>

using namespace shaga;

TEST (ChunkMeta, merge)
{
	ChunkMeta meta[3];

	for (int id = 0; id < 3; ++id) {
		for (int i = 0; i < 10; ++i) {
			meta[id].add_value ("ABC", "ahoj");
			meta[id].add_value ("ABC", "bhoj");
			meta[id].add_value ("ABC");
			meta[id].add_value ("BBC", "ahoj");
			meta[id].add_value ("CBC", "ahoj");

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
		meta.add_value ("ABC", "ahoj");
		meta.add_value ("ABC", "bhoj");
		meta.add_value ("ABC");
		meta.add_value ("BBC", "ahoj");
		meta.add_value ("CBC", "ahoj");
	}

	EXPECT_TRUE (meta.size () > 5);
	meta.unique ();
	EXPECT_TRUE (meta.size () == 5);

	meta.add_value ("CBC", "ahoj");
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

		EXPECT_NO_THROW (meta.add_value (key, "")) << "Key value: " << key;
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

	EXPECT_THROW (meta.add_value ("", ""), CommonException);
	EXPECT_THROW (meta.add_value ("A", ""), CommonException);
	EXPECT_THROW (meta.add_value ("x", ""), CommonException);
	EXPECT_THROW (meta.add_value ("xyz", ""), CommonException);
	EXPECT_THROW (meta.add_value ("ABCD", ""), CommonException);
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
	//EXPECT_TRUE (bin1.compare (bin2) == 0);

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

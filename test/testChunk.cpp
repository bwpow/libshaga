/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <shaga.h>
#include <gtest/gtest.h>

using namespace shaga;

TEST (Chunk, constructors)
{
	Chunk c1 (0, "AAAA");
	Chunk c2 (c1);
	EXPECT_TRUE (c2.compare (c1) == 0);

	c1 = c2;
	EXPECT_TRUE (c2.compare (c1) == 0);

	Chunk c3 (std::move (c2));
	EXPECT_TRUE (c3.compare (c1) == 0);
}

TEST (Chunk, keys)
{
	const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for (int64_t i = 0; i < 456976; ++i) {
		std::string key;
		key.append (1, chars[(i) % 26]);
		key.append (1, chars[(i/26) % 26]);
		key.append (1, chars[(i/676) % 26]);
		key.append (1, chars[(i/17576) % 26]);

		const uint32_t k = Chunk::key_to_bin (key);
		const uint32_t ck = ChKEY (key.c_str ());

		EXPECT_TRUE (k == ck);

		const std::string skey = Chunk::bin_to_key (k);
		EXPECT_TRUE (skey == key);
	}

	EXPECT_THROW (Chunk::key_to_bin (""), CommonException);
	EXPECT_THROW (Chunk::key_to_bin ("A"), CommonException);
	EXPECT_THROW (Chunk::key_to_bin ("x"), CommonException);
	EXPECT_THROW (Chunk::key_to_bin ("xyz"), CommonException);
	EXPECT_THROW (Chunk::key_to_bin ("ABCDE"), CommonException);
}

TEST (Chunk, compare)
{
	CHUNKLIST cl;
	cl.emplace_back (0, "AAAA", Chunk::Priority::pDEBUG, Chunk::TrustLevel::TRUSTED);
	cl.emplace_back (0, "AAAA", Chunk::Priority::pDEBUG, Chunk::TrustLevel::INTERNAL);

	cl.emplace_back (0, "AAAA", Chunk::Priority::pOPTIONAL, Chunk::TrustLevel::TRUSTED);
	cl.emplace_back (0, "AAAA", Chunk::Priority::pOPTIONAL, Chunk::TrustLevel::INTERNAL);

	cl.emplace_back (0, "AAAA", Chunk::Priority::pMANDATORY, Chunk::TrustLevel::TRUSTED);
	cl.emplace_back (0, "AAAA", Chunk::Priority::pMANDATORY, Chunk::TrustLevel::INTERNAL);

	cl.emplace_back (0, "AAAA", Chunk::Priority::pCRITICAL, Chunk::TrustLevel::TRUSTED);
	cl.emplace_back (0, "AAAA", Chunk::Priority::pCRITICAL, Chunk::TrustLevel::INTERNAL);

	cl.reverse ();

	for (CHUNKLIST::const_iterator iter1 = cl.begin (); iter1 != cl.end (); ++iter1) {
		for (CHUNKLIST::const_iterator iter2 = iter1; iter2 != cl.end (); ++iter2) {
			if (iter1 == iter2) {
				EXPECT_TRUE (iter1->compare (*iter2) == 0);
				EXPECT_TRUE ((*iter1) == (*iter2));
				EXPECT_FALSE ((*iter1) < (*iter2));
			}
			else {
				EXPECT_TRUE (iter1->compare (*iter2) < 0);
				EXPECT_TRUE (iter2->compare (*iter1) > 0);
				EXPECT_TRUE ((*iter1) < (*iter2));
				EXPECT_FALSE ((*iter1) == (*iter2));
			}
		}
	}
}

TEST (Chunk, trim)
{
	const size_t sze = 1000;
	std::vector<Chunk> v;

	for (size_t i = 0; i < sze; ++i) {
		v.emplace_back (0, "AAAA", Chunk::Priority::pCRITICAL);
		v.emplace_back (0, "AAAA", Chunk::Priority::pMANDATORY);
		v.emplace_back (0, "AAAA", Chunk::Priority::pOPTIONAL);
		v.emplace_back (0, "AAAA", Chunk::Priority::pDEBUG);
	}

	std::random_shuffle (v.begin (), v.end ());

	CHUNKSET cset;
	for (const auto &c : v) {
		cset.insert (c);
	}

	ASSERT_TRUE (cset.size () == sze * 4);

	{
		CHUNKSET cs = cset;
		chunkset_trim (cs, sze * 3, Chunk::Priority::pOPTIONAL);
		EXPECT_TRUE (cs.size () == (sze * 3));
	}

	{
		CHUNKSET cs = cset;
		chunkset_trim (cs, 0, Chunk::Priority::pOPTIONAL);
		EXPECT_TRUE (cs.size () == (sze * 2));
	}

	{
		CHUNKSET cs = cset;
		chunkset_trim (cs, 0, Chunk::Priority::pCRITICAL);
		EXPECT_TRUE (cs.size () == 0);
	}
}

TEST (Chunk, tracert)
{
	Chunk c1 (0, "TRAC");
	Chunk c2 (0, "ABCD");

	c1.tracert_hops_add (0, 0);
	c1.tracert_hops_add (1, 1);

	c2.tracert_hops_add (0, 0);
	c2.tracert_hops_add (1, 1);

	size_t offset = 0;
	Chunk d1 (c1.to_bin (), offset);
	offset = 0;
	Chunk d2 (c2.to_bin (), offset);

	EXPECT_TRUE (c1.tracert_hops_get ().size () == 2);
	EXPECT_TRUE (d1.tracert_hops_get ().size () == 2);

	const auto v1 = c1.tracert_hops_get ();
	const auto v2 = d1.tracert_hops_get ();

	EXPECT_TRUE (std::equal (v1.cbegin (), v1.cend (), v2.cbegin (), v2.cend (), [](const auto &a, const auto &b) -> bool {
		return (a.hwid == b.hwid) && (a.metric == b.metric);
	}));

	EXPECT_TRUE (c2.tracert_hops_get ().size () == 0);
	EXPECT_TRUE (d2.tracert_hops_get ().size () == 0);
}

TEST (Chunk, channel)
{
	const size_t sze = 1000;
	std::vector<Chunk> v1, v2;
	std::string out;

	/* Generate different types of chunks */
	for (size_t i = 0; i < sze; ++i) {
		v1.emplace_back (i, "AAAA");
		v1.back ().set_channel (false);

		v1.emplace_back (i, "ZZZZ");
		v1.back ().set_channel (Chunk::Channel::SECONDARY);

		v1.emplace_back (i, "AAAA");
		if ((i % 2) == 0) {
			/* Channel == true is default */
			v1.back ().set_channel (true);
		}

		v1.emplace_back (i, "ZZZZ", std::string_view ("payload"));
		if ((i % 2) == 0) {
			/* Channel == true is default */
			v1.back ().set_channel (Chunk::Channel::PRIMARY);
		}

		v1.emplace_back (i, "TRAC");
		v1.back ().set_channel (false);

		v1.emplace_back (i, "TRAC");
		v1.back ().set_channel (true);
	}

	/* Create one long string */
	for (const Chunk &c : v1) {
		EXPECT_NO_THROW (c.to_bin (out));
	}

	/* Create new vector of chunks from the long string */
	size_t pos = 0;
	while (pos < out.size ()) {
		EXPECT_NO_THROW (v2.emplace_back (out, pos));
	}

	/* We have read whole string and got the same number of chunks */
	EXPECT_TRUE (pos == out.size ());
	EXPECT_TRUE (v1.size () == v2.size ());

	pos = 0;
	for (size_t i = 0; i < sze; ++i) {
		EXPECT_FALSE (v2[pos].is_primary_channel ());
		EXPECT_TRUE (v2[pos++].is_secondary_channel ());

		EXPECT_FALSE (v2[pos].is_primary_channel ());
		EXPECT_TRUE (v2[pos++].is_secondary_channel ());

		EXPECT_TRUE (v2[pos++].is_primary_channel ());
		EXPECT_TRUE (v2[pos++].is_primary_channel ());

		EXPECT_FALSE (v2[pos++].is_primary_channel ());
		EXPECT_TRUE (v2[pos++].is_primary_channel ());
	}
	EXPECT_TRUE (pos == v2.size ());

	for (size_t i = 0; i < v1.size (); ++i) {
		EXPECT_TRUE (v1[i].get_source_hwid () == v2[i].get_source_hwid ());
		EXPECT_TRUE (v1[i].get_num_type () == v2[i].get_num_type ());
		EXPECT_TRUE (v1[i].get_channel_bitmask () == v2[i].get_channel_bitmask ());
		EXPECT_TRUE (v1[i].get_channel () == v2[i].get_channel ());
		EXPECT_TRUE (v1[i].get_payload () == v2[i].get_payload ());
	}
}

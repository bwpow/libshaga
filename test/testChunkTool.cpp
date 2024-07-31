/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2024, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

TEST (ChunkTool, change_source_hwid)
{
	const size_t sze = 1000;
	CHUNKLIST lst;
	const uint32_t key = ChKEY ("AAAA");

	for (size_t i = 0; i < sze; ++i) {
		lst.emplace_back (1, key + 1, Chunk::Priority::pCRITICAL);
		lst.emplace_back (2, key + 2, Chunk::Priority::pMANDATORY);
		lst.emplace_back (3, key + 3, Chunk::Priority::pOPTIONAL);
		lst.emplace_back (0, key + 4, Chunk::Priority::pDEBUG);
	}

	ChunkTool::change_source_hwid (lst, 4, true);
	for (const Chunk &chunk : lst) {
		EXPECT_TRUE (chunk.get_num_type () == (chunk.get_source_hwid () + key));
	}

	ChunkTool::change_source_hwid (lst, 5, false);
	for (const Chunk &chunk : lst) {
		EXPECT_TRUE (5 == chunk.get_source_hwid ());
	}
}

TEST (ChunkTool, bin)
{
	const size_t sze = 1000;
	std::vector<Chunk> v, nv1, nv2;

	for (size_t i = 0; i < sze; ++i) {
		v.emplace_back (0, "AAAA", Chunk::Priority::pCRITICAL);
		v.emplace_back (0, "BBBB", Chunk::Priority::pMANDATORY);
		v.emplace_back (0, "CCCC", Chunk::Priority::pOPTIONAL);
		v.emplace_back (0, "DDDD", Chunk::Priority::pDEBUG);
	}

	CHUNKSET cset;
	CHUNKLIST lst;
	for (const auto &c : v) {
		cset.insert (c);
		lst.push_back (c);
	}

	Chunk::SPECIAL_TYPES special_types { ChKEY("AAAA"), ChKEY("DDDD"), 0, 0, 0, 0, 0 };
	ChunkTool tool (true, &special_types);

	auto test = [&](const Chunk::Priority max_priority) -> void
	{
		const auto cset_bin = tool.to_bin (cset, 0, max_priority);
		const auto lst_bin = tool.to_bin (lst, 0, max_priority);

		EXPECT_TRUE (cset_bin.size () == lst_bin.size ());

		const auto v1 = tool.from_bin<CHUNKSET> (lst_bin);
		const auto v2 = tool.from_bin<CHUNKLIST> (cset_bin);

		EXPECT_TRUE (v1.size () == v2.size ());

		for (const auto &x : v1) {
			nv1.push_back (x);
		}

		for (const auto &x : v2) {
			nv2.push_back (x);
		}

		EXPECT_TRUE (nv1.size () == nv2.size ());
	};

	test (Chunk::Priority::pCRITICAL);
	test (Chunk::Priority::pMANDATORY);
	test (Chunk::Priority::pOPTIONAL);
	test (Chunk::Priority::pDEBUG);

	EXPECT_TRUE (nv1.size () == nv2.size ());
	EXPECT_TRUE (nv1.size () == v.size ());

	auto is_same_chunk = [&](const Chunk &ch1, const Chunk &ch2) -> bool
	{
		return (ch1.get_num_type () == ch2.get_num_type ()) && (ch1.get_prio () == ch2.get_prio ());
	};

	for (const auto &chunk : v) {
		EXPECT_TRUE (std::any_of (nv1.begin (), nv1.end (), [&](const auto &ch2) -> bool { return is_same_chunk (chunk, ch2); }));
		EXPECT_TRUE (std::any_of (nv2.begin (), nv2.end (), [&](const auto &ch2) -> bool { return is_same_chunk (chunk, ch2); }));
	}
}

TEST (ChunkTool, purge)
{
	const size_t sze = 1000;
	std::vector<Chunk> v;

	for (size_t i = 0; i < sze; ++i) {
		v.emplace_back (0, "AAAA", Chunk::Priority::pCRITICAL);
		v.emplace_back (0, "AAAA", Chunk::Priority::pMANDATORY);
		v.emplace_back (0, "AAAA", Chunk::Priority::pOPTIONAL);
		v.emplace_back (0, "AAAA", Chunk::Priority::pDEBUG);
	}

	CHUNKSET cset;
	CHUNKLIST lst;
	for (const auto &c : v) {
		cset.insert (c);
		lst.push_back (c);
	}

	auto func = [](const Chunk &chunk) -> bool {
		/* Leave only optional chunks */
		return (chunk.get_prio () == Chunk::Priority::pOPTIONAL);
	};

	ASSERT_NO_THROW (ChunkTool::purge (cset, func));
	ASSERT_NO_THROW (ChunkTool::purge (lst, func));

	EXPECT_TRUE (cset.size () == sze);
	EXPECT_TRUE (lst.size () == sze);
}

TEST (ChunkTool, trim)
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
		ChunkTool::trim (cs, sze * 3, Chunk::Priority::pOPTIONAL);
		EXPECT_TRUE (cs.size () == (sze * 3));
	}

	{
		CHUNKSET cs = cset;
		ChunkTool::trim (cs, 0, Chunk::Priority::pOPTIONAL);
		EXPECT_TRUE (cs.size () == (sze * 2));
	}

	{
		CHUNKSET cs = cset;
		ChunkTool::trim (cs, 0, Chunk::Priority::pCRITICAL);
		EXPECT_TRUE (cs.size () == 0);
	}
}

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

TEST (SPSC, empty)
{
	const int sze = 16;
	const int loops = 128;
	SPSC<int> ring (sze);

	for (int i = 0; i < loops; ++i) {
		EXPECT_TRUE (ring.empty ());
		ring.push_back (i);
		EXPECT_FALSE (ring.empty ());

		int val;
		EXPECT_TRUE (ring.pop_front (val));
		EXPECT_TRUE (val == i);
		EXPECT_FALSE (ring.pop_front (val));
	}
}

TEST (SPSC, full)
{
	const int sze = 16;
	const int loops = 128;
	SPSC<int> ring (sze);

	for (int i = 0; i < loops; ++i) {
		if (i < sze - 1) {
			EXPECT_FALSE (ring.full ());
			EXPECT_TRUE (ring.push_back (i));
		}
		else {
			EXPECT_TRUE (ring.full ());
			EXPECT_FALSE (ring.push_back (i));
		}
		EXPECT_FALSE (ring.empty ());
	}

	for (int i = 0; i < loops; ++i) {
		if (i < sze - 1) {
			EXPECT_FALSE (ring.empty ());
			ASSERT_TRUE (ring.front () != nullptr);
			const auto frnt = ring.front ();
			ASSERT_TRUE (frnt != nullptr);
			EXPECT_TRUE (*frnt == i);
			EXPECT_TRUE (ring.pop_front ());
		}
		else {
			EXPECT_TRUE (ring.empty ());
			EXPECT_FALSE (ring.front () != nullptr);
			EXPECT_FALSE (ring.pop_front ());
		}
		EXPECT_FALSE (ring.full ());
	}
}

TEST (SPSC, non_trivial_container)
{
	const int sze = 16;
	const int loops = 128;

	SPSC<Chunk> ring (sze);

	for (int i = 0; i < loops; ++i) {
		Chunk c (i, "AAAA");
		EXPECT_TRUE (ring.push_back (c));
		EXPECT_TRUE (ring.emplace_back (i, "BBBB"));

		ASSERT_TRUE (ring.front () != nullptr);
		EXPECT_TRUE (ring.front ()->get_source_hwid () == static_cast<HWID>(i));
		EXPECT_TRUE (ring.front ()->get_type () == "AAAA");
		EXPECT_TRUE (ring.pop_front ());

		EXPECT_TRUE (ring.pop_front (c));
		EXPECT_TRUE (c.get_source_hwid () == static_cast<HWID>(i));
		EXPECT_TRUE (c.get_type () == "BBBB");

		EXPECT_FALSE (ring.pop_front ());
		EXPECT_FALSE (ring.front () != nullptr);
	}
}

TEST (PreAllocSPSC, empty)
{
	const int sze = 16;
	const int loops = 128;
	PreAllocSPSC<int> ring (sze);

	for (int i = 0; i < loops; ++i) {
		ASSERT_TRUE (ring.empty ());
		ASSERT_NO_THROW (ring.back () = i);
		ASSERT_NO_THROW (ring.push_back ());
		ASSERT_FALSE (ring.empty ());

		int val {0};
		ASSERT_NO_THROW (val = ring.front ());
		ASSERT_NO_THROW (ring.pop_front ());
		ASSERT_TRUE (val == i);
		ASSERT_THROW (ring.pop_front (), CommonException);
	}
}

TEST (PreAllocSPSC, full)
{
	const int sze = 16;
	const int loops = 128;
	PreAllocSPSC<int> ring (sze);

	for (int i = 0; i < loops; ++i) {
		if (i < sze - 1) {
			EXPECT_FALSE (ring.full ());
			EXPECT_NO_THROW (ring.back () = i);
			EXPECT_NO_THROW (ring.push_back ());
		}
		else {
			EXPECT_TRUE (ring.full ());
			EXPECT_THROW (ring.back (), CommonException);
			EXPECT_THROW (ring.push_back (), CommonException);
		}
		EXPECT_FALSE (ring.empty ());
	}

	for (int i = 0; i < loops; ++i) {
		if (i < sze - 1) {
			EXPECT_FALSE (ring.empty ());
			EXPECT_NO_THROW (ring.front ());
			EXPECT_TRUE (ring.front () == i);
			EXPECT_NO_THROW (ring.pop_front ());
		}
		else {
			EXPECT_TRUE (ring.empty ());
			EXPECT_THROW (ring.pop_front (), CommonException);
		}
		EXPECT_FALSE (ring.full ());
	}
}

TEST (PreAllocSPSC, non_trivial_container)
{
	const int sze = 16;
	const int loops = 128;

	PreAllocSPSC<Chunk> ring (sze, 0, "AAAA");

	for (uint32_t i = 0; i < loops; ++i) {
		EXPECT_NO_THROW (ring.back ().set_ttl (i & 0x7));
		EXPECT_NO_THROW (ring.back ().set_destination_hwid (i + 1));
		EXPECT_NO_THROW (ring.push_back ());

		EXPECT_NO_THROW (ring.front ());
		EXPECT_TRUE (ring.front ().get_ttl () == (i & 0x7));
		EXPECT_TRUE (ring.front ().get_type () == "AAAA");
		EXPECT_TRUE (ring.front ().is_for_destination (i + 1));
		EXPECT_FALSE (ring.front ().is_for_destination (i + 2));
		EXPECT_NO_THROW (ring.pop_front ());

		EXPECT_THROW (ring.front (), CommonException);
		EXPECT_THROW (ring.pop_front (), CommonException);
	}
}

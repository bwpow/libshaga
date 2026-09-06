/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

#include <future>

using namespace shaga;

namespace
{
	struct alignas (128) AlignedValue
	{
		int value {0};
	};

	struct ThrowingValue
	{
		static int alive;
		static int attempts;

		ThrowingValue ()
		{
			if (++attempts == 4) {
				throw std::runtime_error ("constructor failure");
			}
			++alive;
		}

		~ThrowingValue ()
		{
			--alive;
		}
	};

	int ThrowingValue::alive {0};
	int ThrowingValue::attempts {0};
}

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

TEST (SPSC, push_back_move)
{
	/* Regression: push_back(T&&) default-constructed the slot and then move-assigned into it,
	which needlessly required T to be default-constructible and leaked on a throwing move.
	It now move-constructs the slot directly. Verify the moved value arrives intact and the
	source is left in a moved-from state. */
	const uint_fast32_t sze = 16;
	SPSC<std::string> ring (sze);

	for (int i = 0; i < 64; ++i) {
		std::string src (48, static_cast<char> ('A' + (i & 0x0F)));
		const std::string expected = src;

		ASSERT_TRUE (ring.push_back (std::move (src)));

		std::string out;
		ASSERT_TRUE (ring.pop_front (out));
		ASSERT_EQ (out, expected);

		ASSERT_FALSE (ring.pop_front (out));
	}
}

TEST (SPSC, checked_aligned_storage_and_wraparound)
{
	EXPECT_THROW (SPSC<AlignedValue> (std::numeric_limits<uint_fast32_t>::max ()), CommonException);

	SPSC<AlignedValue> ring (4);
	for (int i = 0; i < 64; ++i) {
		AlignedValue value;
		value.value = i;
		ASSERT_TRUE (ring.push_back (value));
		AlignedValue *front = ring.front ();
		ASSERT_NE (nullptr, front);
		EXPECT_EQ (0U, reinterpret_cast<uintptr_t> (front) % alignof (AlignedValue));
		AlignedValue out;
		ASSERT_TRUE (ring.pop_front (out));
		EXPECT_EQ (i, out.value);
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

TEST (PreAllocSPSC, clear)
{
	/* Regression: clear() was written as `while (pop_front () == true);` but pop_front()
	returns void, so it did not compile once instantiated, and would have thrown on empty.
	It must drain the ring and leave it empty. */
	const uint_fast32_t sze = 16;
	PreAllocSPSC<int> ring (sze);

	for (int i = 0; i < 5; ++i) {
		ASSERT_NO_THROW (ring.back () = i);
		ASSERT_NO_THROW (ring.push_back ());
	}
	ASSERT_FALSE (ring.empty ());

	ASSERT_NO_THROW (ring.clear ());
	ASSERT_TRUE (ring.empty ());
	ASSERT_THROW (ring.pop_front (), CommonException);

	/* clear() on an already empty ring must be a no-op, not throw */
	ASSERT_NO_THROW (ring.clear ());
	ASSERT_TRUE (ring.empty ());

	/* The ring must remain usable after clear() */
	ASSERT_NO_THROW (ring.back () = 42);
	ASSERT_NO_THROW (ring.push_back ());
	ASSERT_FALSE (ring.empty ());
	ASSERT_EQ (ring.front (), 42);
}

TEST (PreAllocSPSC, constructor_args_not_moved_from)
{
	/* Regression: the constructor used std::forward<Args>(args)... inside the element loop,
	so when an rvalue was supplied, the first slot moved from it and every following slot was
	constructed from a moved-from object. All pre-constructed slots must hold an equal copy. */
	const uint_fast32_t sze = 16;
	const std::string expected (32, 'x');

	PreAllocSPSC<std::string> ring (sze, std::string (32, 'x'));

	/* back() exposes the pre-constructed write slot; walk through sze-1 of them */
	for (uint_fast32_t i = 0; i + 1 < sze; ++i) {
		ASSERT_EQ (ring.back (), expected);
		ASSERT_NO_THROW (ring.push_back ());
	}
}

TEST (PreAllocSPSC, checked_aligned_storage_and_exception_cleanup)
{
	EXPECT_THROW (PreAllocSPSC<AlignedValue> (std::numeric_limits<uint_fast32_t>::max ()), CommonException);

	PreAllocSPSC<AlignedValue> ring (4);
	EXPECT_EQ (0U, reinterpret_cast<uintptr_t> (&ring.back ()) % alignof (AlignedValue));
	ring.back ().value = 17;
	ring.push_back ();
	EXPECT_EQ (17, ring.front ().value);
	ring.pop_front ();

	ThrowingValue::alive = 0;
	ThrowingValue::attempts = 0;
	EXPECT_THROW (PreAllocSPSC<ThrowingValue> (8), std::runtime_error);
	EXPECT_EQ (0, ThrowingValue::alive);
}

TEST (SPSCDataDynAlloc, free_resets_logical_size)
{
	SPSCDataDynAlloc data (32);
	data.alloc (16);
	data.set_size (8);
	ASSERT_FALSE (data.empty ());
	data.free ();
	EXPECT_TRUE (data.empty ());
	EXPECT_EQ (0U, data.size ());
	EXPECT_EQ (nullptr, data.buffer);
}

#ifdef SHAGA_THREADING
TEST (Semaphore, reset_wakes_waiters)
{
	/* Regression: reset(n) with n > 0 set the counter but did not notify the condition
	variable, so a thread already blocked in wait() stayed blocked until the next notify().
	reset() with a positive count must wake a waiting thread. The test uses a bounded wait
	and releases the worker on failure so it can never hang the suite. */
	Semaphore sem (0);

	std::future<bool> fut = std::async (std::launch::async, [&sem]() -> bool {
		sem.wait ();
		return true;
	});

	/* Give the worker time to actually block inside wait() */
	std::this_thread::sleep_for (std::chrono::milliseconds (100));

	/* This must wake the blocked worker */
	sem.reset (1);

	if (fut.wait_for (std::chrono::seconds (5)) != std::future_status::ready) {
		/* Regression present: release the worker so the test does not hang, then fail */
		sem.notify ();
		fut.wait ();
		FAIL () << "reset() did not wake the waiting thread";
	}
	else {
		EXPECT_TRUE (fut.get ());
	}
}

TEST (Semaphore, count_semantics)
{
	Semaphore sem (0);

	/* reset(n) must make exactly n tokens available */
	sem.reset (3);
	EXPECT_TRUE (sem.wait_for (std::chrono::milliseconds (0)));
	EXPECT_TRUE (sem.wait_for (std::chrono::milliseconds (0)));
	EXPECT_TRUE (sem.wait_for (std::chrono::milliseconds (0)));
	EXPECT_FALSE (sem.wait_for (std::chrono::milliseconds (0)));

	/* reset(0) must clear all tokens */
	sem.notify ();
	sem.reset (0);
	EXPECT_FALSE (sem.wait_for (std::chrono::milliseconds (0)));
}
#endif // SHAGA_THREADING

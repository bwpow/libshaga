/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2023, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
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

	cl.emplace_back (0, "AAAA", Chunk::Priority::pDEBUG, Chunk::TrustLevel::INTERNAL, HWIDMASK (0x20, HWID_MAX));
	cl.emplace_back (0, "AAAA", Chunk::Priority::pDEBUG, Chunk::TrustLevel::INTERNAL, HWIDMASK (0x10, HWID_MAX));
	cl.emplace_back (0, "AAAA", Chunk::Priority::pDEBUG, Chunk::TrustLevel::INTERNAL, HWIDMASK (0x10, 0));

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

TEST (Chunk, special_types)
{
	const auto key = ChKEY ("ABCD");

	Chunk::SPECIAL_TYPES special_types;
	std::iota (special_types.begin (), special_types.end (), key);

	std::vector<Chunk> cv1, cv2;
	COMMON_VECTOR cs1, cs2;

	for (size_t id = 0; id < (Chunk::num_special_types + 128); ++id) {
		cv1.emplace_back (0, key + id);
	}

	for (const auto &item : cv1) {
		ASSERT_NO_THROW (cs1.push_back (item.to_bin (&special_types)));
		ASSERT_NO_THROW (cs2.push_back (item.to_bin ()));
	}

	/* Fill cv2 from binary */
	for (const auto &item : cs1) {
		size_t offset = 0;
		ASSERT_NO_THROW (cv2.emplace_back (item, offset, &special_types));
	}

	{
		/* Can't decode from bin without special types */
		size_t offset = 0;
		ASSERT_THROW (cv2.emplace_back (cs1.front (), offset), CommonException);
	}

	ASSERT_TRUE (cs1.size () == cs2.size ());
	ASSERT_TRUE (cv1.size () == cv2.size ());
	ASSERT_TRUE (cv1.size () == cs1.size ());

	/* First Chunk::num_special_types should be 2 bytes smaller generated with special_types */
	for (size_t id = 0; id < cs1.size (); ++id) {
		if (id < Chunk::num_special_types) {
			EXPECT_TRUE ((cs1.at (id).size () + 2) == cs2.at (id).size ());
		}
		else {
			EXPECT_TRUE (cs1.at (id).size () == cs2.at (id).size ());
		}

		/* Check original and decoded types */
		EXPECT_TRUE (cv1.at (id).get_num_type () == cv2.at (id).get_num_type ());
	}
}

TEST (Chunk, cbor)
{
	const auto data = "{ \"happy\": true, \"pi\": 3.141 }"_json;
	auto cbor = nlohmann::json::to_cbor (data);
	std::string payload {"some payload"s};

	std::vector<Chunk> cv;
	COMMON_VECTOR cs;

	cv.emplace_back (0, "ABCD", payload);
	cv.emplace_back (0, "ABCD", payload, data);
	cv.emplace_back (0, "ABCD", payload, cbor);
	cv.emplace_back (0, "ABCD", std::move (payload), std::move (cbor));

	/* Convert to binary representation */
	for (size_t id = 0; id < cv.size (); id++) {
		if (0 == id) {
			EXPECT_FALSE (cv[id].has_cbor ());
		}
		else {
			EXPECT_TRUE (cv[id].has_cbor ());
		}

		cs.push_back (cv[id].to_bin ());
	}

	for (size_t id = 2; id < cs.size (); id++) {
		EXPECT_TRUE (cs[1] == cs[id]);
	}

	cv.clear ();

	/* Convert back from binary representation */
	for (size_t id = 0; id < cs.size (); id++) {
		size_t offset = 0;
		cv.emplace_back (cs[id], offset);
	}

	/* Check restored chunks */
	for (size_t id = 0; id < cv.size (); id++) {
		EXPECT_TRUE (cv[id].get_type () == "ABCD"s);

		if (0 == id) {
			EXPECT_FALSE (cv[id].has_cbor ());
		}
		else {
			EXPECT_TRUE (cv[id].has_cbor ());
			EXPECT_TRUE (cbor == cv[id].get_cbor ());

			const auto data2 = cv[id].get_json ();
			EXPECT_TRUE (data == data2);

			EXPECT_TRUE (data2.contains ("happy"s));
			EXPECT_TRUE (data2.contains ("pi"s));
			EXPECT_FALSE (data2.contains ("sad"s));
		}
	}
}

TEST (Chunk, tracert)
{
	Chunk c1 (0, "TRAC");
	Chunk c2 (0, "ABCD");

	EXPECT_TRUE (c1.tracert_hops_add (0, 0));
	EXPECT_TRUE (c1.tracert_hops_add (1, 1));
	EXPECT_TRUE (c1.tracert_hops_add (2, 2));

	EXPECT_FALSE (c2.tracert_hops_add (0, 0));

	size_t offset = 0;
	Chunk d1 (c1.to_bin (), offset);
	offset = 0;
	Chunk d2 (c2.to_bin (), offset);

	EXPECT_TRUE (c1.tracert_hops_count () == 3);
	EXPECT_TRUE (d1.tracert_hops_count () == 3);

	const auto v1 = c1.tracert_hops_get ();
	const auto v2 = d1.tracert_hops_get ();

	EXPECT_TRUE (v1.size () == 3);
	EXPECT_TRUE (v2.size () == 3);

	EXPECT_TRUE (std::equal (v1.cbegin (), v1.cend (), v2.cbegin (), v2.cend (), [](const auto &a, const auto &b) -> bool {
		return (a.hwid == b.hwid) && (a.metric == b.metric);
	}));

	EXPECT_TRUE (c2.tracert_hops_count () == 0);
	EXPECT_TRUE (d2.tracert_hops_count () == 0);
}

TEST (Chunk, stored_binary)
{
	ChunkMeta meta;
	meta.add_value ("ABC", "abc"sv);

	Chunk orig_chunk (0x1234, "ABCD", "payload"sv, "{ \"happy\": true, \"pi\": 3.141 }"_json, std::move (meta));
	std::string bin = orig_chunk.to_bin ();

	ASSERT_TRUE (bin.size () <= orig_chunk.get_max_bytes ());

	size_t offset = 0;
	Chunk stored_chunk (bin, offset, nullptr, true);

	/* Used whole string */
	ASSERT_TRUE (offset = bin.size ());

	ASSERT_TRUE (stored_chunk.get_max_bytes () == orig_chunk.get_max_bytes ());

	{
		/* Is stored version the same as internal? */
		const auto bin2 = stored_chunk.get_stored_binary_representation ();
		ASSERT_TRUE (bin.compare (bin2) == 0);
	}

	auto test = [&]() -> void
	{
		stored_chunk.set_prio (Chunk::_Priority_first);
		stored_chunk.set_trustlevel (Chunk::_TrustLevel_first);
		stored_chunk.set_ttl (Chunk::max_ttl);

		orig_chunk.set_prio (stored_chunk.get_prio ());
		orig_chunk.set_trustlevel (stored_chunk.get_trustlevel ());
		orig_chunk.set_ttl (stored_chunk.get_ttl ());

		orig_chunk.set_payload (stored_chunk.get_payload ());
		orig_chunk.set_cbor (stored_chunk.get_cbor ());
		orig_chunk.set_destination_hwid (stored_chunk.get_destination_hwidmask ());

		bin.resize (0);
		orig_chunk.to_bin (bin);

		std::string out1;
		std::string out2;
		const char *ptr = nullptr;

		/* First call should be empty */
		{
			const auto bin2 = stored_chunk.get_stored_binary_representation ();
			ASSERT_TRUE (bin2.empty ());
		}

		/* This will generate new version */
		stored_chunk.restore_bin (out1, true, nullptr);
		ASSERT_TRUE (bin.compare (out1) == 0);

		/* Second call should return stored version */
		{
			const auto bin2 = stored_chunk.get_stored_binary_representation ();
			ASSERT_TRUE (bin.compare (bin2) == 0);

			/* Store pointer to cached data */
			ptr = bin2.data ();
		}

		/* Change some data in header in both chunks */
		stored_chunk.set_prio (Chunk::_Priority_last);
		stored_chunk.set_trustlevel (Chunk::_TrustLevel_last);
		stored_chunk.set_ttl (3);

		orig_chunk.set_prio (stored_chunk.get_prio ());
		orig_chunk.set_trustlevel (stored_chunk.get_trustlevel ());
		orig_chunk.set_ttl (stored_chunk.get_ttl ());

		bin.resize (0);
		orig_chunk.to_bin (bin);

		/* This will return swapped version and with updated header */
		stored_chunk.restore_bin (out2, true, nullptr);
		ASSERT_TRUE (bin.compare (out2) == 0);

		/* Since this string is just swapped, it should have the same pointer as previously stored */
		ASSERT_TRUE (out2.data () == ptr);
		ptr = nullptr;

		/* Should be empty again */
		{
			const auto bin2 = stored_chunk.get_stored_binary_representation ();
			ASSERT_TRUE (bin2.empty ());
		}

		/* This will generate new version again */
		stored_chunk.restore_bin (out1, false, nullptr);
		ASSERT_TRUE (bin.compare (out1) == 0);

		/* Should return stored version */
		{
			const auto bin2 = stored_chunk.get_stored_binary_representation ();
			ASSERT_TRUE (bin.compare (bin2) == 0);
		}

		/* This will return copy of stored version */
		stored_chunk.restore_bin (out2, false, nullptr);
		ASSERT_TRUE (bin.compare (out2) == 0);

		/* Should return stored version */
		{
			const auto bin2 = stored_chunk.get_stored_binary_representation ();
			ASSERT_TRUE (bin.compare (bin2) == 0);
		}

		/* At this point, stored_chunk has binary version stored */
	};

	/* Now test several methods to invalidate stored binary */
	stored_chunk.invalidate_stored_binary_representation ();
	test ();

	stored_chunk.set_payload ("some other payload"sv);
	test ();

	stored_chunk.set_json ("{ \"happy\": false, \"pi\": 4 }"_json);
	test ();

	stored_chunk.set_destination_hwid (0x4567);
	test ();
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

		v1.emplace_back (i, "ZZZZ", "payload"sv);
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

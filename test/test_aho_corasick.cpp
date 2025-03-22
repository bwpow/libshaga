/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

/* Based on tests retreived from: https://github.com/cjgdev/aho_corasick/blob/master/test/ */
/* Date: 2025-03-22 */

namespace ac = aho_corasick;

TEST (AhoCorasickEmitTest, Equals)
{
	using emit_t = ac::emit<char>;
	{
		emit_t one (13, 42, "");
		emit_t two (13, 42, "");
		ASSERT_TRUE (one == two);
	}
	{
		emit_t one (13, 42, "", 0);
		emit_t two (13, 42, "", 0);
		ASSERT_TRUE (one == two);
	}
}

TEST (AhoCorasickEmitTest, NotEquals)
{
	using emit_t = ac::emit<char>;
	{
		emit_t one (13, 42, "");
		emit_t two (13, 43, "");
		ASSERT_TRUE (one != two);
	}
	{
		emit_t one (13, 42, "", 0);
		emit_t two (13, 43, "", 0);
		ASSERT_TRUE (one != two);
	}
}

TEST (AhoCorasickEmitTest, IndexSupport)
{
	using emit_t = ac::emit<char>;
	emit_t with_index (10, 20, "test", 42);
	ASSERT_EQ (10, with_index.get_start ());
	ASSERT_EQ (20, with_index.get_end ());
	ASSERT_EQ ("test", with_index.get_keyword ());
	ASSERT_EQ (42, with_index.get_index ());

	emit_t without_index (10, 20, "test");
	ASSERT_EQ (0, without_index.get_index ());
}

TEST (AhoCorasickIntervalTest, Construct)
{
	const ac::interval i (1, 3);
	ASSERT_EQ (1, i.get_start ());
	ASSERT_EQ (3, i.get_end ());
}

TEST (AhoCorasickIntervalTest, Size)
{
	ASSERT_EQ (3, ac::interval (0, 2).size ());
}

TEST (AhoCorasickIntervalTest, IntervalOverlaps)
{
	ASSERT_TRUE (ac::interval (1, 3).overlaps_with (ac::interval (2, 4)));
}

TEST (AhoCorasickIntervalTest, IntervalDoesNotOverlap)
{
	ASSERT_FALSE (ac::interval (1, 13).overlaps_with (ac::interval (27, 42)));
}

TEST (AhoCorasickIntervalTest, PointOverlaps)
{
	ASSERT_TRUE (ac::interval (1, 3).overlaps_with (2));
}

TEST (AhoCorasickIntervalTest, PointDoesNotOverlap)
{
	ASSERT_FALSE (ac::interval (1, 13).overlaps_with (42));
}

TEST (AhoCorasickIntervalTest, Comparable)
{
	std::set<ac::interval> intervals{
		ac::interval (4, 6),
		ac::interval (2, 7),
		ac::interval (3, 4),
	};
	auto it = intervals.begin ();
	ASSERT_EQ (2, it++->get_start ());
	ASSERT_EQ (3, it++->get_start ());
	ASSERT_EQ (4, it++->get_start ());
}

TEST (AhoCorasickIntervalTreeTest, FindOverlaps)
{
	const auto assert_interval = [] (const ac::interval& interval, size_t expect_start, size_t expect_end) -> void {
		ASSERT_EQ (expect_start, interval.get_start ());
		ASSERT_EQ (expect_end, interval.get_end ());
	};
	const std::vector<ac::interval> intervals{
		ac::interval (0, 2),
		ac::interval (1, 3),
		ac::interval (2, 4),
		ac::interval (3, 5),
		ac::interval (4, 6),
		ac::interval (5, 7),
	};
	ac::interval_tree<ac::interval> tree (intervals);
	auto overlaps = tree.find_overlaps (ac::interval (1, 3));
	ASSERT_EQ (3, overlaps.size ());
	auto it = overlaps.begin ();
	assert_interval (*it++, 2, 4);
	assert_interval (*it++, 3, 5);
	assert_interval (*it++, 0, 2);
}

TEST (AhoCorasickIntervalTreeTest, RemoveOverlaps)
{
	const std::vector<ac::interval> intervals{
		ac::interval (0, 2),
		ac::interval (4, 5),
		ac::interval (2, 10),
		ac::interval (6, 13),
		ac::interval (9, 15),
		ac::interval (12, 16),
	};
	ac::interval_tree<ac::interval> tree (intervals);
	const auto result = tree.remove_overlaps (intervals);
	ASSERT_EQ (2, result.size ());
}

TEST (AhoCorasickStateTest, ConstructCharacterSequence)
{
	auto root = new ac::state<char> ();
	root
		->add_state ('a')
		->add_state ('b')
		->add_state ('c');
	auto cur_state = root->next_state ('a');
	ASSERT_EQ (1, cur_state->get_depth ());
	cur_state = cur_state->next_state ('b');
	ASSERT_EQ (2, cur_state->get_depth ());
	cur_state = cur_state->next_state ('c');
	ASSERT_EQ (3, cur_state->get_depth ());
	delete root;
}

TEST (AhoCorasickTrieTest, KeywordAndTextAreTheSame)
{
	const auto check_emit = [] (const ac::emit<char>& next, size_t expect_start, size_t expect_end, std::string expect_keyword) -> void {
		ASSERT_EQ (expect_start, next.get_start ());
		ASSERT_EQ (expect_end, next.get_end ());
		ASSERT_EQ (expect_keyword, next.get_keyword ());
	};
	ac::trie t;
	t.insert ("abc");
	auto emits = t.parse_text ("abc");
	const auto it = emits.begin ();
	check_emit (*it, 0, 2, "abc");
}

TEST (AhoCorasickTrieTest, TextIsLongerThanKeyword)
{
	const auto check_emit = [] (const ac::emit<char>& next, size_t expect_start, size_t expect_end, std::string expect_keyword) -> void {
		ASSERT_EQ (expect_start, next.get_start ());
		ASSERT_EQ (expect_end, next.get_end ());
		ASSERT_EQ (expect_keyword, next.get_keyword ());
	};
	ac::trie t;
	t.insert ("abc");

	auto emits = t.parse_text (" abc");

	const auto it = emits.begin ();
	check_emit (*it, 1, 3, "abc");
}

TEST (AhoCorasickTrieTest, VariousKeywordsOneMatch)
{
	const auto check_emit = [] (const ac::emit<char>& emit_val, size_t expect_start, size_t expect_end, std::string expect_keyword) -> void {
		ASSERT_EQ (expect_start, emit_val.get_start ());
		ASSERT_EQ (expect_end, emit_val.get_end ());
		ASSERT_EQ (expect_keyword, emit_val.get_keyword ());
	};
	ac::trie t;
	t.insert ("abc");
	t.insert ("bcd");
	t.insert ("cde");

	auto emits = t.parse_text ("bcd");

	const auto it = emits.begin ();
	check_emit (*it, 0, 2, "bcd");
}

TEST (AhoCorasickTrieTest, UshersTest)
{
	const auto check_emit = [] (const ac::emit<char>& next, size_t expect_start, size_t expect_end, std::string expect_keyword) -> void {
		ASSERT_EQ (expect_start, next.get_start ());
		ASSERT_EQ (expect_end, next.get_end ());
		ASSERT_EQ (expect_keyword, next.get_keyword ());
	};
	ac::trie t;
	t.insert ("hers");
	t.insert ("his");
	t.insert ("she");
	t.insert ("he");

	auto emits = t.parse_text ("ushers");
	ASSERT_EQ (3, emits.size ());

	auto it = emits.begin ();
	check_emit (*it++, 2, 3, "he");
	check_emit (*it++, 1, 3, "she");
	check_emit (*it++, 2, 5, "hers");
}

TEST (AhoCorasickTrieTest, MisleadingTest)
{
	const auto check_emit = [] (const ac::emit<char>& next, size_t expect_start, size_t expect_end, std::string expect_keyword) -> void {
		ASSERT_EQ (expect_start, next.get_start ());
		ASSERT_EQ (expect_end, next.get_end ());
		ASSERT_EQ (expect_keyword, next.get_keyword ());
	};
	ac::trie t;
	t.insert ("hers");

	auto emits = t.parse_text ("h he her hers");

	auto it = emits.begin ();
	check_emit (*it++, 9, 12, "hers");
}

TEST (AhoCorasickTrieTest, Recipes)
{
	const auto check_emit = [] (const ac::emit<char>& next, size_t expect_start, size_t expect_end, std::string expect_keyword) -> void {
		ASSERT_EQ (expect_start, next.get_start ());
		ASSERT_EQ (expect_end, next.get_end ());
		ASSERT_EQ (expect_keyword, next.get_keyword ());
	};
	ac::trie t;
	t.insert ("veal");
	t.insert ("cauliflower");
	t.insert ("broccoli");
	t.insert ("tomatoes");

	auto emits = t.parse_text ("2 cauliflowers, 3 tomatoes, 4 slices of veal, 100g broccoli");
	ASSERT_EQ (4, emits.size ());

	auto it = emits.begin ();
	check_emit (*it++, 2, 12, "cauliflower");
	check_emit (*it++, 18, 25, "tomatoes");
	check_emit (*it++, 40, 43, "veal");
	check_emit (*it++, 51, 58, "broccoli");
}

TEST (AhoCorasickTrieTest, LongAndShortOverlappingMatch)
{
	const auto check_emit = [] (const ac::emit<char>& next, size_t expect_start, size_t expect_end, std::string expect_keyword) -> void {
		ASSERT_EQ (expect_start, next.get_start ());
		ASSERT_EQ (expect_end, next.get_end ());
		ASSERT_EQ (expect_keyword, next.get_keyword ());
	};
	ac::trie t;
	t.insert ("he");
	t.insert ("hehehehe");

	auto emits = t.parse_text ("hehehehehe");
	ASSERT_EQ (7, emits.size ());

	auto it = emits.begin ();
	check_emit (*it++, 0, 1, "he");
	check_emit (*it++, 2, 3, "he");
	check_emit (*it++, 4, 5, "he");
	check_emit (*it++, 6, 7, "he");
	check_emit (*it++, 0, 7, "hehehehe");
	check_emit (*it++, 8, 9, "he");
	check_emit (*it++, 2, 9, "hehehehe");
}

TEST (AhoCorasickTrieTest, NonOverlapping)
{
	const auto check_emit = [] (const ac::emit<char>& next, size_t expect_start, size_t expect_end, std::string expect_keyword) -> void {
		ASSERT_EQ (expect_start, next.get_start ());
		ASSERT_EQ (expect_end, next.get_end ());
		ASSERT_EQ (expect_keyword, next.get_keyword ());
	};
	ac::trie t;
	t.remove_overlaps ();
	t.insert ("ab");
	t.insert ("cba");
	t.insert ("ababc");

	auto emits = t.parse_text ("ababcbab");
	ASSERT_EQ (2, emits.size ());

	auto it = emits.begin ();
	check_emit (*it++, 0, 4, "ababc");
	check_emit (*it++, 6, 7, "ab");
}

TEST (AhoCorasickTrieTest, PartialMatch)
{
	const auto check_emit = [] (const ac::emit<char>& next, size_t expect_start, size_t expect_end, std::string expect_keyword) -> void {
		ASSERT_EQ (expect_start, next.get_start ());
		ASSERT_EQ (expect_end, next.get_end ());
		ASSERT_EQ (expect_keyword, next.get_keyword ());
	};
	ac::trie t;
	t.only_whole_words ();
	t.insert ("sugar");

	auto emits = t.parse_text ("sugarcane sugarcane sugar canesugar");
	ASSERT_EQ (1, emits.size ());

	const auto it = emits.begin ();
	check_emit (*it, 20, 24, "sugar");
}

TEST (AhoCorasickTrieTest, TokeniseTokensInSequence)
{
	// Define lambda only if we're going to use it
	ac::trie t;
	t.insert ("Alpha");
	t.insert ("Beta");
	t.insert ("Gamma");

	const auto tokens = t.tokenise ("Alpha Beta Gamma");
	ASSERT_EQ (5, tokens.size ());

	// Verify token content by checking first, middle, and last tokens
	auto it = tokens.begin ();
	ASSERT_EQ ("Alpha", it->get_fragment ());
	ASSERT_TRUE (it->is_match ());

	std::advance (it, 2);
	ASSERT_EQ ("Beta", it->get_fragment ());
	ASSERT_TRUE (it->is_match ());

	std::advance (it, 2);
	ASSERT_EQ ("Gamma", it->get_fragment ());
	ASSERT_TRUE (it->is_match ());
}

TEST (AhoCorasickTrieTest, TokeniseFullSentence)
{
	const auto check_token = [] (const ac::trie::token_type& token_val, std::string expect_fragment) -> void {
		ASSERT_EQ (expect_fragment, token_val.get_fragment ());
	};
	ac::trie t;
	t.only_whole_words ();
	t.insert ("Alpha");
	t.insert ("Beta");
	t.insert ("Gamma");

	auto tokens = t.tokenise ("Hear: Alpha team first, Beta from the rear, Gamma in reserve");
	ASSERT_EQ (7, tokens.size ());

	auto it = tokens.begin ();
	check_token (*it++, "Hear: ");
	check_token (*it++, "Alpha");
	check_token (*it++, " team first, ");
	check_token (*it++, "Beta");
	check_token (*it++, " from the rear, ");
	check_token (*it++, "Gamma");
	check_token (*it++, " in reserve");
}

TEST (AhoCorasickTrieTest, WtrieCaseInsensitive)
{
	const auto check_wemit = [] (const ac::emit<wchar_t>& next, size_t expect_start, size_t expect_end, std::wstring expect_keyword) -> void {
		ASSERT_EQ (expect_start, next.get_start ());
		ASSERT_EQ (expect_end, next.get_end ());
		ASSERT_EQ (expect_keyword, next.get_keyword ());
	};
	ac::wtrie t;
	t.case_insensitive ().only_whole_words ();
	t.insert (L"turning");
	t.insert (L"once");
	t.insert (L"again");

	auto emits = t.parse_text (L"TurninG OnCe AgAiN");
	ASSERT_EQ (3, emits.size ());

	auto it = emits.begin ();
	check_wemit (*it++, 0, 6, L"turning");
	check_wemit (*it++, 8, 11, L"once");
	check_wemit (*it++, 13, 17, L"again");
}

TEST (AhoCorasickTrieTest, TrieCaseInsensitive)
{
	const auto check_emit = [] (const ac::emit<char>& next, size_t expect_start, size_t expect_end, std::string expect_keyword) -> void {
		ASSERT_EQ (expect_start, next.get_start ());
		ASSERT_EQ (expect_end, next.get_end ());
		ASSERT_EQ (expect_keyword, next.get_keyword ());
	};
	ac::trie t;
	t.case_insensitive ();
	t.insert ("turning");
	t.insert ("once");
	t.insert ("again");

	auto emits = t.parse_text ("TurninG OnCe AgAiN");
	ASSERT_EQ (3, emits.size ());

	auto it = emits.begin ();
	check_emit (*it++, 0, 6, "turning");
	check_emit (*it++, 8, 11, "once");
	check_emit (*it++, 13, 17, "again");
}

TEST (AhoCorasickTrieTest, GithubIssue7)
{
	ac::trie t;

	t.insert ("hers");
	t.insert ("his");
	t.insert ("he");

	auto result = t.parse_text ("she");
	EXPECT_EQ (1, result.size ());

	t.insert ("she");

	result = t.parse_text ("something");
	EXPECT_TRUE (result.empty ());
}

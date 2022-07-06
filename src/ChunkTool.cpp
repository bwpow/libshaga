/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2022, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ChunkTool::ChunkTool (const bool enable_thr, const Chunk::SPECIAL_TYPES *const special_types) :
		_special_types (special_types),
		_enable_thr (enable_thr)
	{ }

	ChunkTool::ChunkTool (const Chunk::SPECIAL_TYPES *const special_types) :
		_special_types (special_types)
	{ }

	void ChunkTool::set_enable_thr (const bool enable_thr)
	{
		_enable_thr = enable_thr;
	}

	void ChunkTool::set_store_binary (const bool store_binary)
	{
		_store_binary = store_binary;
	}

	/*** From binary string ***/
	void ChunkTool::from_bin (const std::string_view buf, size_t &offset, CHUNKLIST &out_append) const
	{
		while (offset != buf.size ()) {
			out_append.emplace_back (buf, offset, _special_types, _store_binary);
		}
	}

	void ChunkTool::from_bin (const std::string_view buf, size_t &offset, CHUNKSET &out_append) const
	{
		while (offset != buf.size ()) {
			out_append.emplace (buf, offset, _special_types, _store_binary);
		}
	}

	/*** To binary string ***/
	void ChunkTool::to_bin (CHUNKLIST &lst_erase, std::string &out_append, const size_t max_size, const Chunk::Priority max_priority, const bool erase_skipped)
	{
		size_t sze = 0;
		size_t cnt = 0;

		for (CHUNKLIST::iterator iter = lst_erase.begin (); iter != lst_erase.end ();) {
			if (iter->get_prio () > max_priority) {
				if (true == erase_skipped) {
					iter = lst_erase.erase (iter);
				}
				else {
					++iter;
				}
				continue;
			}

			_temp_str.resize (0);
			iter->to_bin (_temp_str, _special_types);
			if (max_size > 0 && (_temp_str.size () + sze) > max_size) {
				if (true == _enable_thr) {
					if (cnt == 0) {
						cThrow ("Unable to add first chunk."sv);
					}
				}
				break;
			}

			out_append.append (_temp_str);
			sze += _temp_str.size ();

			iter = lst_erase.erase (iter);
			++cnt;
		}
	}

	std::string ChunkTool::to_bin (CHUNKLIST &lst_erase, const size_t max_size, const Chunk::Priority max_priority,	const bool erase_skipped)
	{
		_out_str.resize (0);
		to_bin (lst_erase, _out_str, max_size, max_priority, erase_skipped);
		return _out_str;
	}

	void ChunkTool::to_bin (CHUNKSET &cs_erase, std::string &out_append, const size_t max_size, const Chunk::Priority max_priority)
	{
		size_t sze = 0;
		size_t cnt = 0;

		for (CHUNKSET::iterator iter = cs_erase.begin (); iter != cs_erase.end ();) {
			if (iter->get_prio () > max_priority) {
				break;
			}

			_temp_str.resize (0);
			iter->to_bin (_temp_str, _special_types);
			if (max_size > 0 && (_temp_str.size () + sze) > max_size) {
				if (true == _enable_thr) {
					if (0 == cnt) {
						cThrow ("Unable to add first chunk."sv);
					}
					if (iter->get_prio () == Chunk::Priority::pCRITICAL || iter->get_prio () == Chunk::Priority::pMANDATORY) {
						cThrow ("Unable to add critical and mandatory chunks. Buffer is full."sv);
					}
				}
				break;
			}

			out_append.append (_temp_str);
			sze += _temp_str.size ();

			iter = cs_erase.erase (iter);
			++cnt;
		}
	}

	std::string ChunkTool::to_bin (CHUNKSET &cs_erase, const size_t max_size, const Chunk::Priority max_priority)
	{
		_out_str.resize (0);
		to_bin (cs_erase, _out_str, max_size, max_priority);
		return _out_str;
	}

	void ChunkTool::change_source_hwid (CHUNKLIST &lst, const HWID new_source_hwid, const bool replace_only_zero)
	{
		/* TODO: add std::execution::par */
		std::for_each (lst.begin (), lst.end (), [new_source_hwid, replace_only_zero](Chunk &chunk) -> void {
			if (false == replace_only_zero || 0 == chunk._hwid_source) {
				chunk._hwid_source = new_source_hwid;
			}
		});
	}

	void ChunkTool::trim (CHUNKSET &cset, const size_t treshold_size, const Chunk::Priority treshold_prio)
	{
		size_t sze = cset.size ();

		if (sze <= treshold_size) {
			/* Set contains less elements than is treshold_size, nothing to do */
			return;
		}

		/* At this point, we have at least one element in set */
		if (0 == treshold_size && cset.begin ()->get_prio () >= treshold_prio) {
			/* Treshold size is zero and the first element has priority greater or equal as treshold_prio, so delete everything */
			cset.clear ();
			return;
		}

		/* Start from the last element */
		CHUNKSET::iterator iter = cset.end ();
		--iter;

		while (sze > treshold_size && iter->get_prio () >= treshold_prio) {
			/* Iterate from back to front until there are more set elements than treshold_size and prio is
			greater or equal than treshold_prio. */
			--sze;
			--iter;
		}

		/* Erase everything after last tested element till the end of list */
		cset.erase (++iter, cset.end ());
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Global functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*** TTL ***/
	Chunk::TTL uint8_to_ttl (const uint8_t v)
	{
		const Chunk::TTL t = convert<Chunk::TTL>(v);
		if (Chunk::_TTL_first <= t && t <= Chunk::_TTL_last) {
			return t;
		}
		cThrow ("Value out of range"sv);
	}

	uint8_t ttl_to_uint8 (const Chunk::TTL v)
	{
		return +v;
	}

	/*** TrustLevel ***/
	Chunk::TrustLevel operator++ (Chunk::TrustLevel &x)
	{
		return x = convert<Chunk::TrustLevel>(+x + 1);
	}

	Chunk::TrustLevel operator++ (Chunk::TrustLevel &x, int r)
	{
		return x = convert<Chunk::TrustLevel>(+x + r);
	}

	Chunk::TrustLevel operator* (Chunk::TrustLevel c)
	{
		return c;
	}

	Chunk::TrustLevel begin ([[maybe_unused]] Chunk::TrustLevel r)
	{
		return Chunk::_TrustLevel_first;
	}

	Chunk::TrustLevel end ([[maybe_unused]] Chunk::TrustLevel r)
	{
		Chunk::TrustLevel l = Chunk::_TrustLevel_last;
		return ++l;
	}

	Chunk::TrustLevel uint8_to_trustlevel (const uint8_t v)
	{
		const Chunk::TrustLevel t = convert<Chunk::TrustLevel> (v);
		if (Chunk::_TrustLevel_first <= t && t <= Chunk::_TrustLevel_last) {
			return t;
		}
		cThrow ("Value out of range"sv);
	}

	uint8_t trustlevel_to_uint8 (const Chunk::TrustLevel v)
	{
		return +v;
	}

	SHAGA_STRV std::string_view trustlevel_to_string (const Chunk::TrustLevel level)
	{
		switch (level) {
			case Chunk::TrustLevel::INTERNAL:
				return "internal"sv;
			case Chunk::TrustLevel::TRUSTED:
				return "trusted"sv;
			case Chunk::TrustLevel::FRIEND:
				return "friend"sv;
			case Chunk::TrustLevel::UNTRUSTED:
				return "untrusted"sv;
		}
		cThrow ("Unknown Trustlevel"sv);
	}

	Chunk::TrustLevel string_to_trustlevel (const std::string_view str)
	{
		for (const auto &t : Chunk::TrustLevel ()) {
			if (true == STR::icompare (str, trustlevel_to_string (t))) {
				return t;
			}
		}
		cThrow ("Value out of range"sv);
	}

	/*** Priority ***/
	Chunk::Priority uint8_to_priority (const uint8_t v)
	{
		const Chunk::Priority t = convert<Chunk::Priority> (v);
		if (Chunk::_Priority_first <= t && t <= Chunk::_Priority_last) {
			return t;
		}
		cThrow ("Value out of range"sv);
	}

	uint8_t priority_to_uint8 (const Chunk::Priority v)
	{
		return +v;
	}

	SHAGA_STRV std::string_view priority_to_string (const Chunk::Priority prio)
	{
		switch (prio) {
			case Chunk::Priority::pCRITICAL:
				return "CRITICAL"sv;
			case Chunk::Priority::pMANDATORY:
				return "MANDATORY"sv;
			case Chunk::Priority::pOPTIONAL:
				return "OPTIONAL"sv;
			case Chunk::Priority::pDEBUG:
				return "DEBUG"sv;
		}
		cThrow ("Unknown Priority"sv);
	}

	/*** Channel ***/
	Chunk::Channel bool_to_channel (const bool val)
	{
		if (true == val) {
			return Chunk::Channel::PRIMARY;
		}
		else {
			return Chunk::Channel::SECONDARY;
		}
	}

	bool channel_to_bool (const Chunk::Channel channel)
	{
		switch (channel) {
			case Chunk::Channel::PRIMARY:
				return true;

			case Chunk::Channel::SECONDARY:
				return false;
		}
	}

	SHAGA_STRV std::string_view channel_to_string (const Chunk::Channel channel)
	{
		switch (channel) {
			case Chunk::Channel::PRIMARY:
				return "Primary"sv;

			case Chunk::Channel::SECONDARY:
				return "Secondary"sv;
		}
		cThrow ("Unknown Channel"sv);
	}
}

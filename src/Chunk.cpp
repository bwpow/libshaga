/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	#ifdef SHAGA_THREADING
		static std::atomic<uint_fast64_t> _chunk_global_counter (0);
	#else
		static uint_fast64_t _chunk_global_counter {0};
	#endif // SHAGA_THREADING

	static inline uint32_t _key_char_to_val (const unsigned char c)
	{
		if (c >= 'A' && c <= 'Z') {
			return c - 'A';
		}
		else {
			cThrow ("Key must contain only capital letters"sv);
		}
	}

	static void _check_key_validity (uint32_t bkey, uint8_t *const out_chars = nullptr)
	{
		if (bkey < Chunk::key_type_min || bkey > Chunk::key_type_max) {
			cThrow ("Unrecognized key value {:X}"sv, bkey);
		}

		if (out_chars != nullptr) {
			--bkey;

			uint8_t val[4];
			val[0] = (bkey % 26);
			val[1] = ((bkey / 26) % 26);
			val[2] = ((bkey / 676) % 26);
			val[3] = ((bkey / 17'576) % 26);

			out_chars[0] = val[0] + 'A';
			out_chars[1] = val[1] + 'A';
			out_chars[2] = val[2] + 'A';
			out_chars[3] = val[3] + 'A';
		}
	}

	uint32_t Chunk::key_to_bin (const std::string_view key)
	{
		if (key.size () != 4) {
			cThrow ("Key must be 4 characters long"sv);
		}

		uint32_t val = 0;

		val += _key_char_to_val (key[3]);
		val *= 26;
		val += _key_char_to_val (key[2]);
		val *= 26;
		val += _key_char_to_val (key[1]);
		val *= 26;
		val += _key_char_to_val (key[0]);
		++val;

		return val;
	}

	std::string Chunk::bin_to_key (const uint32_t bkey)
	{
		std::string out (4, '\0');
		_check_key_validity (bkey, reinterpret_cast<uint8_t*> (out.data ()));
		return out;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Chunk::should_continue (const std::string_view s, const size_t offset) const
	{
		if (offset >= s.size ()) {
			return false;
		}
		/* If highest bit is set, we should continue */
		return (static_cast<uint8_t> (s[offset]) & 0x80) != 0;
	}

	void Chunk::_reset (void)
	{
		_construct ();
		_channel = true;
		_hwid_source = 0;
		_type = 0;
		_payload.resize (0);
		_prio = Priority::pMANDATORY;
		_trust = TrustLevel::INTERNAL;
		_ttl = max_ttl;
		_hwid_dest = HWIDMASK ();
		_tracert_hops.clear ();
		meta.reset ();
	}

	void Chunk::_construct (void)
	{
		#ifdef SHAGA_THREADING
			_counter = _chunk_global_counter.fetch_add (1, std::memory_order_relaxed);
		#else
			_counter = _chunk_global_counter++;
		#endif // SHAGA_THREADING
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Chunk::Chunk ()
	{
		_reset ();
		_type = key_type_min;
	}

	Chunk::Chunk (const std::string_view bin, size_t &offset)
	{
		if (should_continue (bin, offset) == false) {
			cThrow ("Buffer is empty"sv);
		}
		_reset ();

		/* Read high 16-bit first (big endian) */
		const uint32_t val = BIN::be_to_uint16 (bin, offset) << 16;

		_prio = uint8_to_priority ((val & key_prio_mask) >> key_prio_shift);
		_trust = uint8_to_trustlevel ((val & key_trust_mask) >> key_trust_shift);
		_ttl = ((val & key_ttl_mask) >> key_ttl_shift);

		if (val & key_channel_mask) {
			_channel = true;
		}
		else {
			_channel = false;
		}

		if (val & key_is_tracert_mask) {
			_type = key_type_tracert;
			const int hop_counter = ((val & key_tracert_hop_mask) >> key_tracert_hop_shift);

			_hwid_source = bin_to_hwid (bin, offset);

			for (int i = 0; i < hop_counter; ++i) {
				TRACERT_HOP hop;
				hop.hwid = bin_to_hwid (bin, offset);
				hop.metric = BIN::to_uint8 (bin, offset);
				_tracert_hops.push_back (std::move (hop));
			}
		}
		else {
			/* This is not tracert type, so read the rest of the 32-bit header */
			const uint32_t lowval = BIN::be_to_uint16 (bin, offset);
			_type = (val | lowval) & key_type_mask;

			_hwid_source = bin_to_hwid (bin, offset);
		}

		_check_key_validity (_type);

		if (val & key_has_dest_mask) {
			_hwid_dest.mask = bin_to_hwid (bin, offset);
			_hwid_dest.hwid = bin_to_hwid (bin, offset);
		}

		if (val & key_has_payload_mask) {
			size_t len = BIN::to_size (bin, offset);
			_payload.assign (bin.substr (offset, len));
			offset += len;
		}

		if (offset > bin.size ()) {
			cThrow ("Not enough data in buffer"sv);
		}

		meta.from_bin (bin, offset);
	}

	Chunk::Chunk (const HWID hwid_source, const std::string_view type)
	{
		_reset ();
		_hwid_source = hwid_source;
		_type = key_to_bin (type);
	}

	Chunk::Chunk (const HWID hwid_source, const uint32_t type)
	{
		if (type < Chunk::key_type_min || type > Chunk::key_type_max) {
			cThrow ("Unrecognized key value {:X}"sv, type);
		}

		_reset ();
		_hwid_source = hwid_source;
		_type = type;
	}

	void Chunk::set_channel (const bool is_primary)
	{
		_channel = is_primary;
	}

	void Chunk::set_channel (const Channel channel)
	{
		switch (channel) {
			case Channel::PRIMARY:
				_channel = true;
				break;

			case Channel::SECONDARY:
				_channel = false;
				break;
		}
	}

	bool Chunk::is_primary_channel (void) const
	{
		return (true == _channel);
	}

	bool Chunk::is_secondary_channel (void) const
	{
		return (false == _channel);
	}

	Chunk::Channel Chunk::get_channel (void) const
	{
		if (true == _channel) {
			return Channel::PRIMARY;
		}
		else {
			return Channel::SECONDARY;
		}
	}

	uint_fast8_t Chunk::get_channel_bitmask (void) const
	{
		return (true == _channel) ? channel_primary : channel_secondary;
	}

	HWID Chunk::get_source_hwid (void) const
	{
		return _hwid_source;
	}

	bool Chunk::is_for_me (const HWID hwid) const
	{
		return _hwid_dest.check (hwid);
	}

	bool Chunk::is_for_me (const HWID_LIST &lst) const
	{
		return std::any_of (lst.begin (), lst.end (), [this](const HWID hwid) {
			return _hwid_dest.check (hwid);
		});
	}

	std::string Chunk::get_type (void) const
	{
		return bin_to_key (_type);
	}

	uint32_t Chunk::get_num_type (void) const
	{
		return _type;
	}

	void Chunk::set_payload (const std::string_view payload)
	{
		_payload.assign (payload);
	}

	void Chunk::set_payload (std::string &&payload)
	{
		std::swap (_payload, payload);
	}

	void Chunk::swap_payload (std::string &other)
	{
		_payload.swap (other);
	}

	void Chunk::set_prio (const Chunk::Priority prio)
	{
		_prio = prio;
	}

	Chunk::Priority Chunk::get_prio (void) const
	{
		return _prio;
	}

	SHAGA_STRV std::string_view Chunk::get_prio_text (void) const
	{
		return priority_to_string (_prio);
	}

	void Chunk::set_minimal_trustlevel (const Chunk::TrustLevel trust)
	{
		if (_trust < trust) {
			_trust = trust;
		}
	}

	void Chunk::set_trustlevel (const Chunk::TrustLevel trust)
	{
		_trust = trust;
	}

	Chunk::TrustLevel Chunk::get_trustlevel (void) const
	{
		return _trust;
	}

	SHAGA_STRV std::string_view Chunk::get_trustlevel_text (void) const
	{
		return trustlevel_to_string (_trust);
	}

	bool Chunk::check_maximal_trustlevel (const Chunk::TrustLevel trust) const
	{
		return (_trust <= trust);
	}

	void Chunk::set_ttl (const uint8_t ttl)
	{
		/* This is deliberately so it never returns error. */
		if (ttl > max_ttl) {
			_ttl = max_ttl;
		}
		else {
			_ttl = ttl;
		}
	}

	void Chunk::set_ttl (const Chunk::TTL ttl)
	{
		set_ttl (ttl_to_uint8 (ttl));
	}

	uint8_t Chunk::get_ttl (void) const
	{
		return _ttl;
	}

	bool Chunk::is_zero_ttl (void) const
	{
		return (_ttl == 0);
	}

	bool Chunk::hop_ttl (void)
	{
		if (_ttl > 0) {
			--_ttl;
			return true;
		}
		else {
			return false;
		}
	}

	bool Chunk::tracert_hops_add (const shaga::HWID hwid, const uint8_t metric)
	{
		if (key_type_tracert != _type || _tracert_hops.size () >= max_hop_counter) {
			return false;
		}

		TRACERT_HOP hop;
		hop.hwid = hwid;
		hop.metric = metric;
		_tracert_hops.push_back (std::move (hop));

		return true;
	}

	std::list<Chunk::TRACERT_HOP> Chunk::tracert_hops_get (void) const
	{
		return _tracert_hops;
	}

	size_t Chunk::tracert_hops_count (void) const
	{
		return _tracert_hops.size ();
	}

	HWIDMASK Chunk::get_destination_hwidmask (void) const
	{
		return _hwid_dest;
	}

	void Chunk::set_destination_hwid (const HWID hwid)
	{
		_hwid_dest = HWIDMASK (hwid);
	}

	void Chunk::set_destination_hwid (const HWIDMASK &hwidmask)
	{
		_hwid_dest = hwidmask;
	}

	void Chunk::set_destination_broadcast (void)
	{
		_hwid_dest = HWIDMASK ();
	}

	int Chunk::compare (const Chunk &c) const
	{
		if (_prio < c._prio) return (-1);
		if (_prio > c._prio) return 1;

		if (_trust < c._trust) return (-1);
		if (_trust > c._trust) return 1;

		switch (_prio) {
			case Priority::pCRITICAL:
			case Priority::pMANDATORY:
				if (_counter < c._counter) return (-1);
				if (_counter > c._counter) return 1;
				break;

			default:
				if (_counter < c._counter) return 1;
				if (_counter > c._counter) return (-1);
				break;
		}

		if (_hwid_source < c._hwid_source) return (-1);
		if (_hwid_source > c._hwid_source) return 1;

		if (_type < c._type) return (-1);
		if (_type > c._type) return 1;

		if (_hwid_dest < c._hwid_dest) return (-1);
		if (_hwid_dest > c._hwid_dest) return 1;

		return 0;
	}

	void Chunk::to_bin (std::string &out) const
	{
		uint32_t val = key_highbit_mask;

		val |= (static_cast<uint32_t>(_prio) << key_prio_shift) & key_prio_mask;
		val |= (static_cast<uint32_t>(_trust) << key_trust_shift) & key_trust_mask;
		val |= (static_cast<uint32_t>(_ttl) << key_ttl_shift) & key_ttl_mask;

		if (true == _channel) {
			val |= key_channel_mask;
		}

		bool has_payload = false;
		if (_payload.empty () == false) {
			val |= key_has_payload_mask;
			has_payload = true;
		}

		bool has_dest = false;
		if (_hwid_dest.empty () == false) {
			val |= key_has_dest_mask;
			has_dest = true;
		}

		if (key_type_tracert == _type) {
			/* This is the tracert type, so store only high 16-bits and set flag */
			val |= key_is_tracert_mask;
			val |= (static_cast<uint32_t> (_tracert_hops.size ()) << key_tracert_hop_shift) & key_tracert_hop_mask;

			BIN::be_from_uint16 (val >> 16, out);
			bin_from_hwid (_hwid_source, out);

			for (const auto &hop : _tracert_hops) {
				bin_from_hwid (hop.hwid, out);
				BIN::from_uint8 (hop.metric, out);
			}
		}
		else {
			val |= _type & key_type_mask;
			BIN::be_from_uint32 (val, out);
			bin_from_hwid (_hwid_source, out);
		}

		if (true == has_dest) {
			bin_from_hwid (_hwid_dest.mask, out);
			bin_from_hwid (_hwid_dest.hwid, out);
		}

		if (true == has_payload) {
			BIN::from_size (_payload.size (), out);
			out.append (_payload);
		}

		meta.to_bin (out);
	}

	std::string Chunk::to_bin (void) const
	{
		std::string out;
		to_bin (out);
		return out;
	}

	bool operator== (const Chunk &a, const Chunk &b)
	{
		return a.compare (b) == 0;
	}

	bool operator!= (const Chunk &a, const Chunk &b)
	{
		return a.compare (b) != 0;
	}

	bool operator< (const Chunk &a, const Chunk &b)
	{
		return a.compare (b) < 0;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Global functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Chunk::TrustLevel operator++ (Chunk::TrustLevel &x)
	{
		return x = static_cast<Chunk::TrustLevel>(std::underlying_type<Chunk::TrustLevel>::type (x) + 1);
	}

	Chunk::TrustLevel operator++ (Chunk::TrustLevel &x, int r)
	{
		return x = static_cast<Chunk::TrustLevel>(std::underlying_type<Chunk::TrustLevel>::type (x) + r);
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
		const Chunk::TrustLevel t = static_cast<Chunk::TrustLevel> (v);
		if (Chunk::_TrustLevel_first <= t && t <= Chunk::_TrustLevel_last) {
			return t;
		}
		cThrow ("Value out of range"sv);
	}

	uint8_t trustlevel_to_uint8 (const Chunk::TrustLevel v)
	{
		return std::underlying_type<Chunk::TrustLevel>::type (v);
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

	Chunk::Priority uint8_to_priority (const uint8_t v)
	{
		const Chunk::Priority t = static_cast<Chunk::Priority> (v);
		if (Chunk::_Priority_first <= t && t <= Chunk::_Priority_last) {
			return t;
		}
		cThrow ("Value out of range"sv);
	}

	uint8_t priority_to_uint8 (const Chunk::Priority v)
	{
		return std::underlying_type<Chunk::Priority>::type (v);
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

	Chunk::TTL uint8_to_ttl (const uint8_t v)
	{
		const Chunk::TTL t = static_cast<Chunk::TTL> (v);
		if (Chunk::_TTL_first <= t && t <= Chunk::_TTL_last) {
			return t;
		}
		cThrow ("Value out of range"sv);
	}

	uint8_t ttl_to_uint8 (const Chunk::TTL v)
	{
		return std::underlying_type<Chunk::TTL>::type (v);
	}

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

	CHUNKLIST chunkset_extract_type (const CHUNKSET &cs, const std::string_view type)
	{
		CHUNKLIST out;
		const uint32_t t = Chunk::key_to_bin (type);

		for (const auto &c : cs) {
			if (c.get_num_type () == t) {
				out.push_back (c);
			}
		}

		return out;
	}

	size_t chunkset_count_type (const CHUNKSET &cs, const std::string_view type)
	{
		size_t out = 0;
		const uint32_t t = Chunk::key_to_bin (type);

		for (const auto &c : cs) {
			if (c.get_num_type () == t) {
				++out;
			}
		}

		return out;
	}

	CHUNKLIST bin_to_chunklist (const std::string_view s, size_t &offset)
	{
		CHUNKLIST cs;

		while (offset != s.size ()) {
			cs.emplace_back (s, offset);
		}

		return cs;
	}

	CHUNKLIST bin_to_chunklist (const std::string_view s)
	{
		size_t offset = 0;
		return bin_to_chunklist (s, offset);
	}

	void bin_to_chunklist (const std::string_view s, size_t &offset, CHUNKLIST &cs)
	{
		while (offset != s.size ()) {
			cs.emplace_back (s, offset);
		}
	}

	void bin_to_chunklist (const std::string_view s, CHUNKLIST &cs)
	{
		size_t offset = 0;

		while (offset != s.size ()) {
			cs.emplace_back (s, offset);
		}
	}

	void chunklist_to_bin (CHUNKLIST &cs, std::string &out, const size_t max_size, const Chunk::Priority max_priority, const bool erase_skipped)
	{
		size_t sze = 0;
		size_t cnt = 0;
		std::string temp;

		for (CHUNKLIST::iterator iter = cs.begin (); iter != cs.end ();) {
			if (iter->get_prio () > max_priority) {
				if (erase_skipped == true) {
					cs.erase (iter++);
				}
				else {
					++iter;
				}
				continue;
			}

			temp.resize (0);
			iter->to_bin (temp);
			if (max_size > 0 && (temp.size () + sze) > max_size) {
				if (cnt == 0) {
					cThrow ("Unable to add first chunk."sv);
				}
				break;
			}

			out.append (temp);
			sze += temp.size ();

			cs.erase (iter++);
			++cnt;
		}
	}

	std::string chunklist_to_bin (CHUNKLIST &cs, const size_t max_size, const Chunk::Priority max_priority, const bool erase_skipped)
	{
		std::string out;
		out.reserve (max_size);
		chunklist_to_bin (cs, out, max_size, max_priority, erase_skipped);
		return out;
	}

	CHUNKSET bin_to_chunkset (const std::string_view s, size_t &offset)
	{
		CHUNKSET cs;

		while (offset != s.size ()) {
			cs.emplace (s, offset);
		}

		return cs;
	}

	CHUNKSET bin_to_chunkset (const std::string_view s)
	{
		size_t offset = 0;
		CHUNKSET cs;

		while (offset != s.size ()) {
			cs.emplace (s, offset);
		}

		return cs;
	}

	void bin_to_chunkset (const std::string_view s, size_t &offset, CHUNKSET &cs)
	{
		while (offset != s.size ()) {
			cs.emplace (s, offset);
		}
	}

	void bin_to_chunkset (const std::string_view s, CHUNKSET &cs)
	{
		size_t offset = 0;

		while (offset != s.size ()) {
			cs.emplace (s, offset);
		}
	}

	void chunkset_to_bin (CHUNKSET &cs, std::string &out, const size_t max_size, const Chunk::Priority max_priority, const bool thr)
	{
		size_t sze = 0;
		size_t cnt = 0;
		std::string temp;

		for (CHUNKSET::iterator iter = cs.begin (); iter != cs.end ();) {
			if (iter->get_prio () > max_priority) {
				break;
			}

			temp.resize (0);
			iter->to_bin (temp);
			if (max_size > 0 && (temp.size () + sze) > max_size) {
				if (true == thr) {
					if (0 == cnt) {
						cThrow ("Unable to add first chunk."sv);
					}
					if (iter->get_prio () == Chunk::Priority::pCRITICAL || iter->get_prio () == Chunk::Priority::pMANDATORY) {
						cThrow ("Unable to add critical and mandatory chunks. Buffer is full."sv);
					}
				}
				break;
			}

			out.append (temp);
			sze += temp.size ();

			cs.erase (iter++);
			++cnt;
		}
	}

	std::string chunkset_to_bin (CHUNKSET &cs, const size_t max_size, const Chunk::Priority max_priority, const bool thr)
	{
		std::string out;
		chunkset_to_bin (cs, out, max_size, max_priority, thr);
		return out;
	}

	void chunkset_trim (CHUNKSET &cset, const size_t treshold_size, const Chunk::Priority treshold_prio)
	{
		size_t sze = cset.size ();
		if (sze <= treshold_size) {
			return;
		}
		if (treshold_size == 0 && cset.begin ()->get_prio () >= treshold_prio) {
			cset.clear ();
			return;
		}

		CHUNKSET::iterator iter = cset.end ();
		--iter;

		while (sze > treshold_size && iter->get_prio () >= treshold_prio) {
			--sze;
			--iter;
		}

		cset.erase (++iter, cset.end ());
	}

	void chunklist_change_source_hwid (CHUNKLIST &lst, const HWID new_source_hwid, const bool replace_only_zero)
	{
		/* When execution is properly implemented, add std::execution::par here */
		std::for_each (lst.begin (), lst.end (), [new_source_hwid, replace_only_zero](Chunk &chunk) {
			if (false == replace_only_zero || 0 == chunk._hwid_source) {
				chunk._hwid_source = new_source_hwid;
			}
		});
	}
}

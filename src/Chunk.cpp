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
			cThrow ("Unrecognized key value {}"sv, bkey);
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
		_hwid_source = HWID_UNKNOWN;
		_type = 0;
		_payload.resize (0);
		_cbor.resize (0);
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

	Chunk::Chunk (const std::string_view bin, size_t &offset, const SPECIAL_TYPES *const special_types)
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

		if ((val & key_tracert_mask) == key_tracert_mask) {
			_type = key_type_tracert;

			const uint_fast8_t hop_counter = BIN::to_uint8 (bin, offset);
			_tracert_hops.reserve (hop_counter);

			for (uint_fast8_t i = 0; i < hop_counter; ++i) {
				TRACERT_HOP hop;
				hop.hwid = bin_to_hwid (bin, offset);
				hop.metric = BIN::to_uint8 (bin, offset);
				_tracert_hops.push_back (std::move (hop));
			}
		}
		else if (val & key_special_type_mask) {
			if (special_types == nullptr) {
				cThrow ("Required special_types are not defined"sv);
			}
			_type = special_types->at ((val >> 16) & num_special_types);
		}
		else {
			/* This is not tracert type, so read the rest of the 32-bit header */
			const uint32_t lowval = BIN::be_to_uint16 (bin, offset);
			_type = (val | lowval) & key_type_mask;
		}

		_check_key_validity (_type);

		_hwid_source = bin_to_hwid (bin, offset);

		if (val & key_has_dest_mask) {
			_hwid_dest.mask = bin_to_hwid (bin, offset);
			_hwid_dest.hwid = bin_to_hwid (bin, offset);
		}

		if (val & key_has_payload_mask) {
			const size_t len = BIN::to_size (bin, offset);
			_payload.assign (bin.substr (offset, len));
			offset += len;
		}

		if (val & key_has_cbor_mask) {
			const size_t len = BIN::to_size (bin, offset);
			_cbor.resize (len);
			::memcpy (_cbor.data (), bin.data () + offset, len);
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

	bool Chunk::is_for_destination (const HWID hwid) const
	{
		return _hwid_dest.check (hwid);
	}

	bool Chunk::is_for_destination (const HWID_LIST &lst) const
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

	bool Chunk::has_payload (void) const
	{
		return _payload.empty () == false;
	}

	void Chunk::reset_payload (void)
	{
		_payload.resize (0);
	}

	void Chunk::set_payload (const std::string_view payload)
	{
		_payload.assign (payload);
	}

	void Chunk::set_payload (std::string &&payload)
	{
		_payload = std::move (payload);
	}

	void Chunk::swap_payload (std::string &other)
	{
		_payload.swap (other);
	}

	bool Chunk::has_cbor (void) const
	{
		return _cbor.empty () == false;
	}

	void Chunk::reset_cbor (void)
	{
		_cbor.resize (0);
	}

	void Chunk::set_json (const nlohmann::json &data)
	{
		_cbor = nlohmann::json::to_cbor (data);
	}

	void Chunk::set_cbor (const std::vector<uint8_t> &cbor)
	{
		_cbor = cbor;
	}

	void Chunk::set_cbor (std::vector<uint8_t> &&cbor)
	{
		_cbor = std::move (cbor);
	}

	void Chunk::swap_cbor (std::vector<uint8_t> &other)
	{
		_cbor.swap (other);
	}

	nlohmann::json Chunk::get_json (void) const
	{
		return nlohmann::json::from_cbor (_cbor);
	}

	std::vector<uint8_t> Chunk::get_cbor (void) const
	{
		return _cbor;
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

	std::vector<Chunk::TRACERT_HOP> Chunk::tracert_hops_get (void) const
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
				break;
		}

		if (_hwid_source < c._hwid_source) return (-1);
		if (_hwid_source > c._hwid_source) return 1;

		if (_type < c._type) return (-1);
		if (_type > c._type) return 1;

		if (_hwid_dest < c._hwid_dest) return (-1);
		if (_hwid_dest > c._hwid_dest) return 1;

		if (_counter < c._counter) return 1;
		if (_counter > c._counter) return (-1);

		return 0;
	}

	void Chunk::to_bin (std::string &out_append, const SPECIAL_TYPES *const special_types) const
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

		bool has_cbor = false;
		if (_cbor.empty () == false) {
			val |= key_has_cbor_mask;
			has_cbor = true;
		}

		bool has_dest = false;
		if (_hwid_dest.empty () == false) {
			val |= key_has_dest_mask;
			has_dest = true;
		}

		if (key_type_tracert == _type) {
			/* This is the tracert type, so store only high 16-bits and set flag */
			val |= key_tracert_mask;
			BIN::be_from_uint16 (val >> 16, out_append);

			BIN::from_uint8 (_tracert_hops.size (), out_append);
			for (const auto &hop : _tracert_hops) {
				bin_from_hwid (hop.hwid, out_append);
				BIN::from_uint8 (hop.metric, out_append);
			}
		}
		else if (special_types != nullptr) {
			/* If special_types is provided, it may contain most used types */
			if (auto result = std::find (special_types->cbegin (), special_types->cend (), _type); result != special_types->cend ()) {
				/* Type found in special types. Store it in the upper 16 bits. */
				val |= key_special_type_mask;
				val |= std::distance (special_types->cbegin (), result) << 16;
				BIN::be_from_uint16 (val >> 16, out_append);
			}
			else {
				val |= _type & key_type_mask;
				BIN::be_from_uint32 (val, out_append);
			}
		}
		else {
			val |= _type & key_type_mask;
			BIN::be_from_uint32 (val, out_append);
		}

		bin_from_hwid (_hwid_source, out_append);

		if (true == has_dest) {
			bin_from_hwid (_hwid_dest.mask, out_append);
			bin_from_hwid (_hwid_dest.hwid, out_append);
		}

		if (true == has_payload) {
			BIN::from_size (_payload.size (), out_append);
			out_append.append (_payload);
		}

		if (true == has_cbor) {
			BIN::from_size (_cbor.size (), out_append);
			out_append.append (reinterpret_cast<const char *> (_cbor.data ()), _cbor.size ());
		}

		meta.to_bin (out_append);
	}

	std::string Chunk::to_bin (const SPECIAL_TYPES *const special_types) const
	{
		std::string out;
		out.reserve (32 + _payload.size () + _cbor.size ());
		to_bin (out, special_types);
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
}

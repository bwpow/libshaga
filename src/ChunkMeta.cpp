/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static inline uint16_t _key_char_to_val (const unsigned char c)
	{
		if (c == '_') {
			return 26;
		}
		else if (c >= 'A' && c <= 'Z') {
			return c - 'A';
		}
		else {
			cThrow ("Key must contain only capital letters or underscore"sv);
		}
	}

	static void _check_key_validity (uint16_t bkey, uint8_t *const out_chars = nullptr)
	{
		if (bkey < ChunkMeta::key_type_min || bkey > ChunkMeta::key_type_max) {
			cThrow ("Unrecognized key value {}"sv, bkey);
		}

		if (out_chars != nullptr) {
			--bkey;

			uint8_t val[3];
			val[0] = (bkey % 27);
			val[1] = ((bkey / 27) % 27);
			val[2] = ((bkey / 729) % 27);

			out_chars[0] = (val[0] == 26) ? '_' : (val[0] + 'A');
			out_chars[1] = (val[1] == 26) ? '_' : (val[1] + 'A');
			out_chars[2] = (val[2] == 26) ? '_' : (val[2] + 'A');
		}
	}

	uint16_t ChunkMeta::key_to_bin (const std::string_view key)
	{
		if (key.size () != 3) {
			cThrow ("Key must be 3 characters long"sv);
		}

		uint16_t val = 0;

		val += _key_char_to_val (key[2]);
		val *= 27;
		val += _key_char_to_val (key[1]);
		val *= 27;
		val += _key_char_to_val (key[0]);
		++val;

		return val;
	}

	std::string ChunkMeta::bin_to_key (const uint16_t bkey)
	{
		std::string out (3, '\0');
		_check_key_validity (bkey, reinterpret_cast<uint8_t*> (out.data ()));
		return out;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ChunkMeta::should_continue (const std::string_view s, const size_t offset) const
	{
		if (offset >= s.size ()) {
			return false;
		}
		/* If highest bit is not set, we should continue */
		return (static_cast<uint8_t> (s[offset]) & 0x80) == 0;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ChunkMeta::ChunkMeta ()
	{ }

	ChunkMeta::ChunkMeta (const uint16_t key)
	{
		_check_key_validity (key, nullptr);
		_data.insert (std::make_pair (key, std::string ()));
	}

	ChunkMeta::ChunkMeta (const uint16_t key, const std::string_view value)
	{
		_check_key_validity (key, nullptr);
		_data.insert (std::make_pair (key, value));
	}

	ChunkMeta::ChunkMeta (const uint16_t key, std::string &&value)
	{
		_check_key_validity (key, nullptr);
		_data.insert (std::make_pair (key, std::move (value)));
	}

	ChunkMeta::ChunkMeta (const std::string_view key)
	{
		_data.insert (std::make_pair (key_to_bin (key), std::string ()));
	}

	ChunkMeta::ChunkMeta (const std::string_view key, const std::string_view value)
	{
		_data.insert (std::make_pair (key_to_bin (key), value));
	}

	ChunkMeta::ChunkMeta (const std::string_view key, std::string &&value)
	{
		_data.insert (std::make_pair (key_to_bin (key), std::move (value)));
	}

	ChunkMetaData::const_iterator ChunkMeta::begin () const
	{
		return _data.cbegin ();
	}

	ChunkMetaData::const_iterator ChunkMeta::end () const
	{
		return _data.cend ();
	}

	ChunkMetaData::const_iterator ChunkMeta::cbegin () const
	{
		return _data.cbegin ();
	}

	ChunkMetaData::const_iterator ChunkMeta::cend () const
	{
		return _data.cend ();
	}

	ChunkMetaData::const_iterator ChunkMeta::find (const std::string_view key) const
	{
		return _data.find (key_to_bin (key));
	}

	ChunkMetaData::const_iterator ChunkMeta::find (const uint16_t key) const
	{
		return _data.find (key);
	}

	void ChunkMeta::clear (void)
	{
		_data.clear ();
	}

	void ChunkMeta::reset (void)
	{
		_data.clear ();
	}

	void ChunkMeta::add_value (const uint16_t key)
	{
		_check_key_validity (key, nullptr);
		_data.insert (std::make_pair (key, std::string ()));
	}

	void ChunkMeta::add_value (const std::string_view key)
	{
		_data.insert (std::make_pair (key_to_bin (key), std::string ()));
	}

	void ChunkMeta::add_value (const uint16_t key, const std::string_view value)
	{
		_check_key_validity (key, nullptr);
		_data.insert (std::make_pair (key, value));
	}

	void ChunkMeta::add_value (const std::string_view key, const std::string_view value)
	{
		_data.insert (std::make_pair (key_to_bin (key), value));
	}

	void ChunkMeta::add_value (const uint16_t key, std::string &&value)
	{
		_check_key_validity (key, nullptr);
		_data.insert (std::make_pair (key, std::move (value)));
	}

	void ChunkMeta::add_value (const std::string_view key, std::string &&value)
	{
		_data.insert (std::make_pair (key_to_bin (key), std::move (value)));
	}

	void ChunkMeta::add_uint8 (const uint16_t key, const uint8_t value)
	{
		auto str = BIN::from_uint8 (value);
		add_value (key, std::move (str));
	}

	void ChunkMeta::add_uint8 (const std::string_view key, const uint8_t value)
	{
		auto str = BIN::from_uint8 (value);
		add_value (key, std::move (str));
	}

	void ChunkMeta::add_uint16 (const uint16_t key, const uint16_t value)
	{
		auto str = BIN::from_uint16 (value);
		add_value (key, std::move (str));
	}

	void ChunkMeta::add_uint16 (const std::string_view key, const uint16_t value)
	{
		auto str = BIN::from_uint16 (value);
		add_value (key, std::move (str));
	}

	void ChunkMeta::add_uint24 (const uint16_t key, const uint32_t value)
	{
		auto str = BIN::from_uint24 (value);
		add_value (key, std::move (str));
	}

	void ChunkMeta::add_uint24 (const std::string_view key, const uint32_t value)
	{
		auto str = BIN::from_uint24 (value);
		add_value (key, std::move (str));
	}

	void ChunkMeta::add_uint32 (const uint16_t key, const uint32_t value)
	{
		auto str = BIN::from_uint32 (value);
		add_value (key, std::move (str));
	}

	void ChunkMeta::add_uint32 (const std::string_view key, const uint32_t value)
	{
		auto str = BIN::from_uint32 (value);
		add_value (key, std::move (str));
	}

	void ChunkMeta::add_uint64 (const uint16_t key, const uint64_t value)
	{
		auto str = BIN::from_uint64 (value);
		add_value (key, std::move (str));
	}

	void ChunkMeta::add_uint64 (const std::string_view key, const uint64_t value)
	{
		auto str = BIN::from_uint64 (value);
		add_value (key, std::move (str));
	}

	size_t ChunkMeta::get_max_bytes (void) const
	{
		size_t len {0};

		for (const auto &entry : _data) {
			/* 2B type + 4B size + data */
			len += 6 + entry.second.size ();
		}

		return len;
	}

	size_t ChunkMeta::size (void) const
	{
		return _data.size ();
	}

	size_t ChunkMeta::count (const std::string_view key) const
	{
		return _data.count (key_to_bin (key));
	}

	size_t ChunkMeta::count (const uint16_t key) const
	{
		return _data.count (key);
	}

	bool ChunkMeta::empty (void) const
	{
		return _data.empty ();
	}

	size_t ChunkMeta::erase (const uint16_t key)
	{
		_check_key_validity (key, nullptr);
		return _data.erase (key);
	}

	size_t ChunkMeta::erase (const std::string_view key)
	{
		return erase (key_to_bin (key));
	}

	void ChunkMeta::unique (void)
	{
		ChunkMetaDataSet temp;
		std::move (_data.begin (), _data.end (), std::inserter (temp, temp.begin ()));
		_data.clear ();
		std::move (temp.begin (), temp.end (), std::inserter (_data, _data.begin ()));
	}

	void ChunkMeta::merge (const ChunkMeta &other)
	{
		ChunkMetaDataSet temp;
		std::move (_data.begin (), _data.end (), std::inserter (temp, temp.begin ()));
		std::copy (other._data.cbegin (), other._data.cend (), std::inserter (temp, temp.begin ()));

		_data.clear ();
		std::move (temp.begin (), temp.end (), std::inserter (_data, _data.begin ()));
	}

	void ChunkMeta::merge (ChunkMeta &&other)
	{
		ChunkMetaDataSet temp;
		std::move (_data.begin (), _data.end (), std::inserter (temp, temp.end ()));
		std::move (other._data.begin (), other._data.end (), std::inserter (temp, temp.end ()));
		other.clear ();

		_data.clear ();
		std::move (temp.begin (), temp.end (), std::inserter (_data, _data.end ()));
	}

	ChunkMetaDataRange ChunkMeta::equal_range (const std::string_view key) const
	{
		return _data.equal_range (key_to_bin (key));
	}

	ChunkMetaDataRange ChunkMeta::equal_range (const uint16_t key) const
	{
		return _data.equal_range (key);
	}

	void ChunkMeta::modify_values (const std::string_view key, ValuesCallback callback)
	{
		modify_values (key_to_bin (key), callback);
	}

	void ChunkMeta::modify_values (const uint16_t key, ValuesCallback callback)
	{
		auto [i_begin, i_end] = _data.equal_range (key);

		if (i_begin == i_end) {
			/* No entries, add a new one */
			std::string str;
			callback (str);
			_data.insert (std::make_pair (key, std::move (str)));
		}
		else {
			for (auto iter = i_begin; iter != i_end; ++iter) {
				if (callback (iter->second) == false) {
					/* If the callback return false, don't continue */
					break;
				}
			}
		}
	}

#define get_uint_helper(type,sze) \
	const auto val = get_value (key); \
	if (val.size () < sze) { \
		return default_value; \
	} \
	else { \
		return BIN::type (val); \
	}

	uint8_t ChunkMeta::get_uint8 (const std::string_view key, const uint8_t default_value) const
	{
		get_uint_helper (to_uint8, 1)
	}

	uint8_t ChunkMeta::get_uint8 (const uint16_t key, const uint8_t default_value) const
	{
		get_uint_helper (to_uint8, 1)
	}

	uint16_t ChunkMeta::get_uint16 (const std::string_view key, const uint16_t default_value) const
	{
		get_uint_helper (to_uint16, 2)
	}

	uint16_t ChunkMeta::get_uint16 (const uint16_t key, const uint16_t default_value) const
	{
		get_uint_helper (to_uint16, 2)
	}

	uint32_t ChunkMeta::get_uint24 (const std::string_view key, const uint32_t default_value) const
	{
		get_uint_helper (to_uint24, 3)
	}

	uint32_t ChunkMeta::get_uint24 (const uint16_t key, const uint32_t default_value) const
	{
		get_uint_helper (to_uint24, 3)
	}

	uint32_t ChunkMeta::get_uint32 (const std::string_view key, const uint32_t default_value) const
	{
		get_uint_helper (to_uint32, 4)
	}

	uint32_t ChunkMeta::get_uint32 (const uint16_t key, const uint32_t default_value) const
	{
		get_uint_helper (to_uint32, 4)
	}

	uint64_t ChunkMeta::get_uint64 (const std::string_view key, const uint64_t default_value) const
	{
		get_uint_helper (to_uint64, 8)
	}

	uint64_t ChunkMeta::get_uint64 (const uint16_t key, const uint64_t default_value) const
	{
		get_uint_helper (to_uint64, 8)
	}

#undef get_uint_helper

	void ChunkMeta::modify_value (const std::string_view key, ValueCallback callback)
	{
		modify_value (key_to_bin (key), callback);
	}

	void ChunkMeta::modify_value (const uint16_t key, ValueCallback callback)
	{
		ChunkMetaData::iterator iter = _data.find (key);
		if (iter != _data.end ()) {
			callback (iter->second);
		}
		else {
			std::string str;
			callback (str);
			_data.insert (std::make_pair (key, std::move (str)));
		}
	}

	void ChunkMeta::to_bin (std::string &out_append) const
	{
		uint16_t last_key {UINT16_MAX};
		for (const auto &[key, value] : _data) {
			if (last_key != key) {
				last_key = key;
				BIN::be_from_uint16 (last_key, out_append);
			}
			else {
				/* Store only high 8-bit value meaning that the key is repeating */
				BIN::from_uint8 (key_repeat_byte, out_append);
			}

			BIN::from_size (value.size (), out_append);
			out_append.append (value);
		}
	}

	std::string ChunkMeta::to_bin (void) const
	{
		std::string out;
		out.reserve (get_max_bytes ());
		to_bin (out);
		return out;
	}

	void ChunkMeta::from_bin (const std::string_view s, size_t &offset)
	{
		reset ();

		uint16_t last_key = UINT16_MAX;
		uint16_t next_key;

		while (should_continue (s, offset)) {
			/* Read top byte first (big endian) */
			next_key = BIN::to_uint8 (s, offset) << 8;

			if (key_repeat_mask == next_key) {
				next_key = last_key;
			} else {
				/* Read the rest of key, low byte */
				next_key |= BIN::to_uint8 (s, offset);
				last_key = next_key;
			}

			_check_key_validity (next_key);

			const size_t len = BIN::to_size (s, offset);
			_data.insert (std::make_pair (next_key, s.substr (offset, len)));
			offset += len;
			if (offset > s.size ()) {
				cThrow ("Not enough data in buffer"sv);
			}
		}
	}
}

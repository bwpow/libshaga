/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

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
			cThrow ("Key must contain only capital letters or underscore");
		}
	}

	static void _check_key_validity (uint16_t bkey, uint8_t *chars = nullptr)
	{
		if (bkey < ChunkMeta::key_type_min || bkey > ChunkMeta::key_type_max) {
			cThrow ("Unrecognized key value %" PRIX16, bkey);
		}

		if (chars != nullptr) {
			--bkey;

			uint8_t val[3];
			val[0] = (bkey % 27);
			val[1] = ((bkey / 27) % 27);
			val[2] = ((bkey / 729) % 27);

			chars[0] = (val[0] == 26) ? '_' : (val[0] + 'A');
			chars[1] = (val[1] == 26) ? '_' : (val[1] + 'A');
			chars[2] = (val[2] == 26) ? '_' : (val[2] + 'A');
		}
	}

	uint16_t ChunkMeta::key_to_bin (const std::string &key)
	{
		if (key.size () != 3) {
			cThrow ("Key must be 3 characters long");
		}

		uint16_t val = 0;

		val += _key_char_to_val (key.at (2));
		val *= 27;
		val += _key_char_to_val (key.at (1));
		val *= 27;
		val += _key_char_to_val (key.at (0));
		++val;

		return val;
	}

	std::string ChunkMeta::bin_to_key (const uint16_t bkey)
	{
		uint8_t buf[3];
		_check_key_validity (bkey, buf);
		return std::string (reinterpret_cast<const char*> (buf), 3);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ChunkMeta::should_continue (const std::string &s, const size_t offset) const
	{
		if (offset == s.size ()) {
			return false;
		}
		/* If highest bit is not set, we should continue */
		return (static_cast<uint8_t> (s.at (offset)) & 0x80) == 0;
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

	ChunkMeta::ChunkMeta (const uint16_t key, const std::string &value)
	{
		_check_key_validity (key, nullptr);
		_data.insert (std::make_pair (key, value));
	}

	ChunkMeta::ChunkMeta (const uint16_t key, std::string &&value)
	{
		_check_key_validity (key, nullptr);
		_data.insert (std::make_pair (key, std::move (value)));
	}

	ChunkMeta::ChunkMeta (const std::string &key)
	{
		_data.insert (std::make_pair (key_to_bin (key), std::string ()));
	}

	ChunkMeta::ChunkMeta (const std::string &key, const std::string &value)
	{
		_data.insert (std::make_pair (key_to_bin (key), value));
	}

	ChunkMeta::ChunkMeta (const std::string &key, std::string &&value)
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

	ChunkMetaData::const_iterator ChunkMeta::find (const std::string &key) const
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

	void ChunkMeta::add_value (const std::string &key)
	{
		_data.insert (std::make_pair (key_to_bin (key), std::string ()));
	}

	void ChunkMeta::add_value (const uint16_t key, const std::string &value)
	{
		_check_key_validity (key, nullptr);
		_data.insert (std::make_pair (key, value));
	}

	void ChunkMeta::add_value (const std::string &key, const std::string &value)
	{
		_data.insert (std::make_pair (key_to_bin (key), value));
	}

	void ChunkMeta::add_value (const uint16_t key, std::string &&value)
	{
		_check_key_validity (key, nullptr);
		_data.insert (std::make_pair (key, std::move (value)));
	}

	void ChunkMeta::add_value (const std::string &key, std::string &&value)
	{
		_data.insert (std::make_pair (key_to_bin (key), std::move (value)));
	}

	size_t ChunkMeta::size (void) const
	{
		return _data.size ();
	}

	size_t ChunkMeta::count (const std::string &key) const
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

	size_t ChunkMeta::erase (const std::string &key)
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

	ChunkMetaDataRange ChunkMeta::equal_range (const std::string &key) const
	{
		return _data.equal_range (key_to_bin (key));
	}

	ChunkMetaDataRange ChunkMeta::equal_range (const uint16_t key) const
	{
		return _data.equal_range (key);
	}

	COMMON_LIST ChunkMeta::get_values (const std::string &key) const
	{
		return get_values (key_to_bin (key));
	}

	COMMON_LIST ChunkMeta::get_values (const uint16_t key) const
	{
		COMMON_LIST v;
		std::pair <ChunkMetaData::const_iterator, ChunkMetaData::const_iterator> p = _data.equal_range (key);

		for (ChunkMetaData::const_iterator iter = p.first; iter != p.second; ++iter) {
			v.push_back (iter->second);
		}

		return v;
	}

	void ChunkMeta::modify_values (const std::string &key, std::function<bool(std::string&)> callback)
	{
		modify_values (key_to_bin (key), callback);
	}

	void ChunkMeta::modify_values (const uint16_t key, std::function<bool(std::string&)> callback)
	{
		std::pair <ChunkMetaData::iterator, ChunkMetaData::iterator> p = _data.equal_range (key);

		if (p.first == p.second) {
			/* No entries, add a new one */
			std::string str;
			callback (str);
			_data.insert (std::make_pair (key, std::move (str)));
		}
		else {
			for (ChunkMetaData::iterator iter = p.first; iter != p.second; ++iter) {
				if (callback (iter->second) == false) {
					/* If the callback return false, don't continue */
					break;
				}
			}
		}
	}

	std::string ChunkMeta::get_value (const std::string &key) const
	{
		return get_value (key_to_bin (key));
	}

	std::string ChunkMeta::get_value (const uint16_t key) const
	{
		std::string s;
		ChunkMetaData::const_iterator iter = _data.find (key);
		if (iter != _data.end ()) {
			s = iter->second;
		}

		return s;
	}

	void ChunkMeta::modify_value (const std::string &key, std::function<void(std::string&)> callback)
	{
		modify_value (key_to_bin (key), callback);
	}

	void ChunkMeta::modify_value (const uint16_t key, std::function<void(std::string&)> callback)
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

	COMMON_LIST ChunkMeta::get_keys (void) const
	{
		COMMON_LIST v;
		uint16_t last_key = UINT16_MAX;

		for (ChunkMetaData::const_iterator iter = _data.begin (); iter != _data.end (); ++iter) {
			if (iter->first != last_key) {
				last_key = iter->first;
				v.push_back (bin_to_key (last_key));
			}
		}

		return v;
	}

	void ChunkMeta::to_bin (std::string &s) const
	{
		uint16_t last_key = UINT16_MAX;
		for (ChunkMetaData::const_iterator iter = _data.begin (); iter != _data.end (); ++iter) {
			if (last_key != iter->first) {
				last_key = iter->first;
				BIN::be_from_uint16 (last_key, s);
			}
			else {
				/* Store only high 8-bit value meaning that the key is repeating */
				BIN::from_uint8 (key_repeat_byte, s);
			}

			BIN::from_size (iter->second.size (), s);
			s.append (iter->second);
		}
	}

	std::string ChunkMeta::to_bin (void) const
	{
		std::string s;
		to_bin (s);
		return s;
	}

	void ChunkMeta::from_bin (const std::string &s, size_t &offset)
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

			size_t len = BIN::to_size (s, offset);
			_data.insert (std::make_pair (next_key, s.substr (offset, len)));
			offset += len;
			if (offset > s.size ()) {
				cThrow ("Not enough data in buffer");
			}
		}

	}

}

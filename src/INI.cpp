/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#include <unistd.h>

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  INI_KEY  ////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	INI::INI_KEY::INI_KEY (const std::string_view _section, const std::string_view _line) :
		section (_section),
		line (_line)
	{ }

	bool INI::INI_KEY::operator< (const INI_KEY &other) const noexcept
	{
		if (section < other.section) return true;
		if (section > other.section) return false;
		return line < other.line;
	}

	bool INI::INI_KEY::operator== (const INI_KEY &other) const noexcept
	{
		return (section == other.section) && (line == other.line);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string INI::get_last_nested_realpath (void) const
	{
		if (_nested_parse_file.empty () == true) {
			return std::string ();
		}

		return FS::realpath (_nested_parse_file.back (), true);
	}

	COMMON_LIST & INI::get_list_ref (INI_MAP &ini_map, const INI_KEY &key, const bool create) const
	{
		INI_MAP::iterator iter = ini_map.find (key);
		if (iter != ini_map.end ()) {
			return iter->second;
		}

		if (true == create) {
			std::pair <INI_MAP::iterator, bool> res = ini_map.insert (std::make_pair (key, COMMON_LIST ()));
			if (false == res.second) {
				cThrow ("Unable to add key {}/{}"sv, key.section, key.line);
			}

			return res.first->second;
		}

		cThrow ("Nonexistent entry {}/{} requested"sv, key.section, key.line);
	}

	const COMMON_LIST & INI::get_list_ref (const INI_MAP &ini_map, const INI_KEY &key) const
	{
		INI_MAP::const_iterator iter = ini_map.find (key);
		if (iter != ini_map.end ()) {
			return iter->second;
		}

		cThrow ("Nonexistent entry {}/{} requested"sv, key.section, key.line);
	}

	COMMON_LIST INI::get_list_copy (const INI_MAP &ini_map, const INI_KEY &key) const
	{
		INI_MAP::const_iterator iter = ini_map.find (key);
		if (iter != ini_map.end ()) {
			return iter->second;
		}

		cThrow ("Nonexistent entry {}/{} requested"sv, key.section, key.line);
	}

	INI::INI_MAP::iterator INI::begin_of_section (INI_MAP &ini_map, const std::string_view section) const
	{
		return ini_map.lower_bound (INI_KEY {section, ""sv});
	}

	bool INI::is_same_section (const INI_KEY &key, const std::string_view section) const noexcept
	{
		return (key.section == section);
	}

	void INI::parse_line (INI_MAP &ini_map, std::string_view line, std::string &active_section, const bool allow_include)
	{
		STR::trim (line);

		if (line.empty () == true) {
			return;
		}

		if (line.front () == ';' || line.front () == '#') {
			return;
		}

		if (line.front () == '@') {
			if (active_section.empty () == false) {
				cThrow ("Special directives allowed only in global section"sv);
			}

			const size_t pos = line.find_first_of ('=');
			if (pos == std::string_view::npos) {
				cThrow ("Malformed line '{}'"sv, line);
			}

			std::string_view line_key = line.substr (0, pos);
			std::string_view line_val = line.substr (pos + 1);

			STR::rtrim (line_key);
			STR::ltrim (line_val);

			if (STR::icompare (line_key, "@include"sv) == true) {
				if (false == allow_include) {
					cThrow ("Including files is not allowed"sv);
				}

				if (line_val.substr (0, 1) == "/"sv) {
					parse_file (ini_map, line_val, allow_include, true);
				}
				else {
					parse_file (ini_map, get_last_nested_realpath () + std::string (line_val), allow_include, true);
				}
			}
			else if (STR::icompare (line_key, "@require"sv) == true) {
				if (false == allow_include) {
					cThrow ("Including files is not allowed"sv);
				}

				if (line_val.substr (0, 1) == "/"sv) {
					parse_file (ini_map, line_val, allow_include, false);
				}
				else {
					parse_file (ini_map, get_last_nested_realpath () + std::string (line_val), allow_include, false);
				}
			}
			else if (STR::icompare (line_key, "@glob"sv) == true) {
				if (false == allow_include) {
					cThrow ("Including files is not allowed"sv);
				}

				if (line_val.substr (0, 1) == "/"sv) {
					FS::glob (line_val, [&](const std::string_view fname) -> void {
						parse_file (ini_map, fname, allow_include, false);
					});
				}
				else {
					FS::glob (get_last_nested_realpath () + std::string (line_val), [&](const std::string_view fname) -> void {
						parse_file (ini_map, fname, allow_include, false);
					});
				}
			}
			else {
				cThrow ("Unknown directive '{}'"sv, line_key);
			}

			return;
		}

		if (line.front () == '[') {
			if (line.back () != ']') {
				cThrow ("Malformed line '{}'"sv, line);
			}

			line.remove_prefix (1);
			line.remove_suffix (1);

			active_section.assign (line);
			return;
		}

		const size_t pos = line.find_first_of ('=');
		if (pos == std::string_view::npos || pos == 0 || pos == (line.size () - 1)) {
			cThrow ("Malformed line '{}'"sv, line);
		}

		std::string_view line_key = line.substr (0, pos);
		std::string_view line_val = line.substr (pos + 1);

		STR::rtrim (line_key);
		STR::ltrim (line_val);

		bool append = false;
		bool refer = false;

		if (line_key.size () > 1 && line_key.back () == '&') {
			/* This is reference to some other value */
			line_key.remove_suffix (1);
			STR::trim (line_key);
			refer = true;
		}

		if (line_key.size () > 2 && line_key.substr (line_key.size () - 2) == "[]"sv) {
			line_key.remove_suffix (2);
			append = true;
		}

		COMMON_LIST &vec = get_list_ref (ini_map, INI_KEY {active_section, line_key}, true);

		if (append == false) {
			vec.clear ();
		}

		if (true == refer) {
			COMMON_VECTOR refval = STR::split<COMMON_VECTOR> (line_val, "/"sv);

			INI_KEY key = [] (const auto &_active_section, const auto &_refval, const auto &_line_val) -> INI_KEY {
				if (_refval.size () == 1) {
					return INI_KEY{_active_section, _line_val};
				}
				else if (_refval.size () == 2) {
					return INI_KEY{_refval[0], _refval[1]};
				}
				else {
					cThrow ("Malformed reference. Reference has to contain either key from current section or section/key."sv);
				}
			} (active_section, refval, line_val);

			COMMON_LIST referenced_list = get_list_copy (ini_map, key);
			vec.splice (vec.end (), std::move (referenced_list));
		}
		else {
			vec.emplace_back (line_val);
		}
	}

	void INI::parse_file (INI_MAP &ini_map, const std::string_view fname, const bool allow_include, const bool allow_missing)
	{
		if (_nested_parse_file.size () > _max_nested_files) {
			cThrow ("Maximum nesting of include directive reached"sv);
		}

		for (const auto &tname : _nested_parse_file) {
			if (tname == fname) {
				cThrow ("Infinite nesting loop detected"sv);
			}
		}

		_nested_parse_file.emplace_back (fname);
		P::debug_print ("INI: Parsing file '{}' nest level {}"sv, fname, _nested_parse_file.size ());

		std::string active_section;
		active_section.clear ();

		if (FS::is_file (fname) == false) {
			if (true == allow_missing) {
				P::debug_print ("INI: File '{}' missing, ignoring..."sv, fname);
			}
			else {
				cThrow ("File '{}' does not exist!"sv, fname);
			}
		}
		else {
			FS::read_file (fname, [&](const std::string_view line) -> void {
				try {
					parse_line (ini_map, line, active_section, allow_include);
				}
				catch (const std::exception &e) {
					if (false == _ignore_broken_lines) {
						throw;
					}
					P::debug_print ("INI: Ignoring malformed line in '{}': {}"sv, fname, e.what ());
				}
			});
		}

		_nested_parse_file.pop_back ();
	}

	void INI::parse_file (const std::string_view fname, const bool allow_include, const bool allow_missing)
	{
		_nested_parse_file.clear ();
		INI_MAP ini_map = _map;
		parse_file (ini_map, fname, allow_include, allow_missing);
		_map = ini_map;
	}

	void INI::parse_buffer (INI_MAP &ini_map, const std::string_view buf, const bool allow_include)
	{
		std::string active_section;
		active_section.clear ();

		STR::split (buf, "\n\r"sv, [&](const std::string_view line) -> void {
			try {
				parse_line (ini_map, line, active_section, allow_include);
			}
			catch (const std::exception &e) {
				if (false == _ignore_broken_lines) {
					throw;
				}
				P::debug_print ("INI: Ignoring malformed line from buffer: {}"sv, e.what ());
			}
		});
	}

	void INI::parse_buffer (const std::string_view buf, const bool allow_include)
	{
		_nested_parse_file.clear ();
		INI_MAP ini_map = _map;
		parse_buffer (ini_map, buf, allow_include);
		_map = ini_map;
	}

	std::optional<std::string_view> INI::get_last_value (const INI_MAP &m, const INI_KEY &key) const noexcept
	{
		try {
			const COMMON_LIST &lst = get_list_ref (m, key);
			if (lst.empty () == false) {
				/* lst is reference, it is safe to return string_view */
				return std::optional<std::string_view> (lst.back ());
			}
		}
		catch (...) {
			/* Do nothing */
		}
		return std::nullopt;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	INI::INI ()
	{
		reset ();
	}

	INI::INI (const INI &other) : _map (other._map), _ignore_broken_lines (other._ignore_broken_lines)
	{
	}

	INI::INI (INI &&other) : _map (std::move (other._map)), _ignore_broken_lines (other._ignore_broken_lines)
	{
	}

	INI& INI::operator= (const INI &other)
	{
		if (this != &other) {
			_map = other._map;
			_ignore_broken_lines = other._ignore_broken_lines;
		}
		return *this;
	}

	INI& INI::operator= (INI &&other)
	{
		if (this != &other) {
			_map = std::move (other._map);
			_ignore_broken_lines = other._ignore_broken_lines;
		}
		return *this;
	}

	INI::INI (const std::string_view fname, const bool allow_include, const bool ignore_broken_lines)
	{
		_ignore_broken_lines = ignore_broken_lines;
		reset ();
		parse_file (fname, allow_include, false);
	}

	void INI::reset (void)
	{
		_map.clear ();
	}

	void INI::load_file (const std::string_view fname, const bool append, const bool allow_include, const bool ignore_broken_lines)
	{
		if (append == false) {
			reset ();
		}

		const bool old_ignore_broken_lines = _ignore_broken_lines;
		_ignore_broken_lines = ignore_broken_lines;
		try {
			parse_file (fname, allow_include, false);
			_ignore_broken_lines = old_ignore_broken_lines;
		}
		catch (...) {
			_ignore_broken_lines = old_ignore_broken_lines;
			throw;
		}
	}

	void INI::save_to_file (ShFile &file) const
	{
		file.set_mode (ShFile::mWRITE | ShFile::mTRUNC);
		file.open ();

		try {
			std::string active_section = ""s;
			bool lines_written = false;

			for (INI_MAP::const_iterator iter = _map.begin (); iter != _map.end (); ++iter) {
				const INI_KEY &key = iter->first;
				const COMMON_LIST &v = iter->second;

				if (active_section != key.section) {
					active_section.assign (key.section);

					if (true == lines_written) {
						file.write ("\n"sv);
					}

					file.write ("["sv);
					file.write (active_section);
					file.write ("]\n"sv);
				}

				const bool append = v.size () > 1;
				for (COMMON_LIST::const_iterator vter = v.cbegin (); vter != v.cend (); ++vter) {
					file.print ("{}{}={}\n", key.line, append ? "[]"sv : ""sv, (*vter));
				}

				lines_written = true;
			}

			file.close ();
		}
		catch (...) {
			file.unlink ();
			throw;
		}
	}

	void INI::save_to_file (const std::string_view fname) const
	{
		ShFile file;
		file.set_file_name (fname);
		save_to_file (file);
	}

	void INI::load_buffer (const std::string_view buf, const bool append, const bool allow_include, const bool ignore_broken_lines)
	{
		if (append == false) {
			reset ();
		}

		const bool old_ignore_broken_lines = _ignore_broken_lines;
		_ignore_broken_lines = ignore_broken_lines;
		try {
			parse_buffer (buf, allow_include);
			_ignore_broken_lines = old_ignore_broken_lines;
		}
		catch (...) {
			_ignore_broken_lines = old_ignore_broken_lines;
			throw;
		}
	}

	void INI::save_to_buffer (std::string &out) const
	{
		std::string active_section = ""s;
		bool lines_written = false;

		for (INI_MAP::const_iterator iter = _map.cbegin (); iter != _map.cend (); ++iter) {
			const INI_KEY &key = iter->first;
			const COMMON_LIST &v = iter->second;

			if (active_section != key.section) {
				active_section.assign (key.section);

				if (lines_written) {
					out.append ("\n"s);
				}
				out.append ("["s + active_section + "]\n"s);
			}

			const bool append = v.size () > 1;
			for (COMMON_LIST::const_iterator vter = v.cbegin (); vter != v.cend (); ++vter) {
				STR::sprint (out, "{}{}={}\n"sv, key.line, append ? "[]"sv : ""sv, (*vter));
			}

			lines_written = true;
		}
	}

	std::string INI::save_to_buffer (void) const
	{
		std::string out;
		save_to_buffer (out);
		return out;
	}

	void INI::save_to_json (nlohmann::json &out) const
	{
		out = nlohmann::json::object ();

		for (INI_MAP::const_iterator iter = _map.cbegin (); iter != _map.cend (); ++iter) {
			const INI_KEY &key = iter->first;
			const COMMON_LIST &v = iter->second;

			nlohmann::json &section = out[key.section];
			if (false == section.is_object ()) {
				section = nlohmann::json::object ();
			}

			if (v.size () <= 1) {
				if (v.empty () == true) {
					section[key.line] = nlohmann::json::array ();
				}
				else {
					section[key.line] = v.front ();
				}
			}
			else {
				nlohmann::json arr = nlohmann::json::array ();
				for (const auto &entry : v) {
					arr.push_back (entry);
				}
				section[key.line] = std::move (arr);
			}
		}
	}

	nlohmann::json INI::save_to_json (void) const
	{
		nlohmann::json out;
		save_to_json (out);
		return out;
	}

	COMMON_LIST INI::get_section_list (void) const
	{
		COMMON_LIST out;

		for (const auto &m : _map) {
			out.push_back (m.first.section);
		}

		out.sort ();
		out.unique ();

		return out;
	}

	const COMMON_VECTOR INI::get_vector (const std::string_view section, const std::string_view key) const
	{
		COMMON_VECTOR out;
		try {
			const COMMON_LIST &lst = get_list_ref (_map, INI_KEY {section, key});
			out.reserve (lst.size ());
			std::copy (lst.cbegin(), lst.cend(), std::back_inserter(out));
		}
		catch (...) {
			return out;
		}

		return out;
	}

	size_t INI::get_vector_size (const std::string_view section, const std::string_view key, const bool thr) const
	{
		return get_list_size (section, key, thr);
	}

	const COMMON_LIST & INI::get_list (const std::string_view section, const std::string_view key, const COMMON_LIST &defvalue, const bool thr) const
	{
		try {
			return get_list_ref (_map, INI_KEY {section, key});
		}
		catch (...) {
			if (true == thr) {
				throw;
			}
		}
		return defvalue;
	}

	const COMMON_LIST INI::get_list (const std::string_view section, const std::string_view key) const
	{
		try {
			return get_list_copy (_map, INI_KEY {section, key});
		}
		catch (...) {
		}
		return COMMON_LIST ();
	}

	size_t INI::get_list_size (const std::string_view section, const std::string_view key, const bool thr) const
	{
		try {
			return get_list_ref (_map, INI_KEY {section, key}).size ();
		}
		catch (...) {
			if (true == thr) {
				throw;
			}
		}
		return 0;
	}

	float INI::get_float (const std::string_view section, const std::string_view key, const float defvalue, const bool thr) const
	{
		try {
			const std::string_view src = get_string<std::string_view> (section, key, ""sv, true);
			return STR::to_float (src);
		}
		catch (const std::exception &e) {
			if (true == thr) {
				cThrow ("{}", e.what ());
			}
			else {
				return defvalue;
			}
		}
	}

	std::optional<float> INI::get_float_optional (const std::string_view section, const std::string_view key) const noexcept
	{
		if (auto val = get_last_value (_map, INI_KEY {section, key})) {
			try {
				return STR::to_float (*val);
			}
			catch (...) {
				return std::nullopt;
			}
		}

		return std::nullopt;
	}

	double INI::get_double (const std::string_view section, const std::string_view key, const double defvalue, const bool thr) const
	{
		try {
			const std::string_view src = get_string<std::string_view> (section, key, ""sv, true);
			return STR::to_double (src);
		}
		catch (const std::exception &e) {
			if (true == thr) {
				cThrow ("{}", e.what ());
			}
			else {
				return defvalue;
			}
		}
	}

	std::optional<double> INI::get_double_optional (const std::string_view section, const std::string_view key) const noexcept
	{
		if (auto val = get_last_value (_map, INI_KEY {section, key})) {
			try {
				return STR::to_double (*val);
			}
			catch (...) {
				return std::nullopt;
			}
		}

		return std::nullopt;
	}

	long double INI::get_long_double (const std::string_view section, const std::string_view key, const long double defvalue, const bool thr) const
	{
		try {
			const std::string_view src = get_string<std::string_view> (section, key, ""sv, true);
			return STR::to_long_double (src);
		}
		catch (const std::exception &e) {
			if (true == thr) {
				cThrow ("{}", e.what ());
			}
			else {
				return defvalue;
			}
		}
	}

	std::optional<long double> INI::get_long_double_optional (const std::string_view section, const std::string_view key) const noexcept
	{
		if (auto val = get_last_value (_map, INI_KEY {section, key})) {
			try {
				return STR::to_long_double (*val);
			}
			catch (...) {
				return std::nullopt;
			}
		}

		return std::nullopt;
	}

	bool INI::get_bool (const std::string_view section, const std::string_view key, const bool defvalue, const bool thr) const
	{
		return get_int<bool> (section, key, defvalue, thr);
	}

	std::optional<bool> INI::get_bool_optional (const std::string_view section, const std::string_view key) const noexcept
	{
		return get_int_optional<bool> (section, key);
	}

	uint8_t INI::get_uint8 (const std::string_view section, const std::string_view key, const uint8_t defvalue, const bool thr, const int base) const
	{
		return get_int<uint8_t> (section, key, defvalue, thr, base);
	}

	std::optional<uint8_t> INI::get_uint8_optional (const std::string_view section, const std::string_view key, const int base) const noexcept
	{
		return get_int_optional<uint8_t> (section, key, base);
	}

	uint16_t INI::get_uint16 (const std::string_view section, const std::string_view key, const uint16_t defvalue, const bool thr, const int base) const
	{
		return get_int<uint16_t> (section, key, defvalue, thr, base);
	}

	std::optional<uint16_t> INI::get_uint16_optional (const std::string_view section, const std::string_view key, const int base) const noexcept
	{
		return get_int_optional<uint16_t> (section, key, base);
	}

	uint32_t INI::get_uint32 (const std::string_view section, const std::string_view key, const uint32_t defvalue, const bool thr, const int base) const
	{
		return get_int<uint32_t> (section, key, defvalue, thr, base);
	}

	std::optional<uint32_t> INI::get_uint32_optional (const std::string_view section, const std::string_view key, const int base) const noexcept
	{
		return get_int_optional<uint32_t> (section, key, base);
	}

	uint64_t INI::get_uint64 (const std::string_view section, const std::string_view key, const uint64_t defvalue, const bool thr, const int base) const
	{
		return get_int<uint64_t> (section, key, defvalue, thr, base);
	}

	std::optional<uint64_t> INI::get_uint64_optional (const std::string_view section, const std::string_view key, const int base) const noexcept
	{
		return get_int_optional<uint64_t> (section, key, base);
	}

	int8_t INI::get_int8 (const std::string_view section, const std::string_view key, const int8_t defvalue, const bool thr, const int base) const
	{
		return get_int<int8_t> (section, key, defvalue, thr, base);
	}

	std::optional<int8_t> INI::get_int8_optional (const std::string_view section, const std::string_view key, const int base) const noexcept
	{
		return get_int_optional<int8_t> (section, key, base);
	}

	int16_t INI::get_int16 (const std::string_view section, const std::string_view key, const int16_t defvalue, const bool thr, const int base) const
	{
		return get_int<int16_t> (section, key, defvalue, thr, base);
	}

	std::optional<int16_t> INI::get_int16_optional (const std::string_view section, const std::string_view key, const int base) const noexcept
	{
		return get_int_optional<int16_t> (section, key, base);
	}

	int32_t INI::get_int32 (const std::string_view section, const std::string_view key, const int32_t defvalue, const bool thr, const int base) const
	{
		return get_int<int32_t> (section, key, defvalue, thr, base);
	}

	std::optional<int32_t> INI::get_int32_optional (const std::string_view section, const std::string_view key, const int base) const noexcept
	{
		return get_int_optional<int32_t> (section, key, base);
	}

	int64_t INI::get_int64 (const std::string_view section, const std::string_view key, const int64_t defvalue, const bool thr, const int base) const
	{
		return get_int<int64_t> (section, key, defvalue, thr, base);
	}

	std::optional<int64_t> INI::get_int64_optional (const std::string_view section, const std::string_view key, const int base) const noexcept
	{
		return get_int_optional<int64_t> (section, key, base);
	}

	void INI::set_string (const std::string_view section, const std::string_view key, const std::string_view val, const bool append)
	{
		COMMON_LIST &v = get_list_ref (_map, INI_KEY {section, key}, true);
		if (append == false) {
			v.clear ();
		}
		v.emplace_back (val);
	}

	void INI::set_vector (const std::string_view section, const std::string_view key, const COMMON_VECTOR &val, const bool append)
	{
		COMMON_LIST &v = get_list_ref (_map, INI_KEY {section, key}, true);
		if (false == append) {
			v.clear ();
		}
		v.insert (v.end (), val.begin (), val.end ());
	}

	void INI::set_list (const std::string_view section, const std::string_view key, const COMMON_LIST &val, const bool append)
	{
		COMMON_LIST &v = get_list_ref (_map, INI_KEY {section, key}, true);
		if (false == append) {
			v.clear ();
		}
		v.insert (v.end (), val.begin (), val.end ());
	}

	size_t INI::erase (const std::string_view section, const std::string_view key)
	{
		return _map.erase (INI_KEY {section, key});
	}

	size_t INI::erase (INI::EraseCallback should_delete)
	{
		if (nullptr == should_delete) {
			return 0;
		}

		size_t erased {0};
		for (auto it = _map.begin (); it != _map.end (); /* no increment */) {
			const INI_KEY &k = it->first;

			if (should_delete (std::string_view (k.section), std::string_view (k.line))) {
				it = _map.erase (it);
				++erased;
				continue;
			}

			++it;
		}

		return erased;
	}
}

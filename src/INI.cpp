/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#include <unistd.h>

namespace shaga {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool operator< (const INI_KEY &a, const INI_KEY &b)
	{
		if (a.section < b.section) return true;
		if (a.section > b.section) return false;

		return a.line < b.line;
	}

	bool operator== (const INI_KEY &a, const INI_KEY &b)
	{
		return (a.section == b.section) && (a.line == b.line);
	}

	//#define FOR_SECTION(iter, section) for (INI_MAP::iterator iter = begin_of_section (_map, section); iter != _map.end () && is_same_section (iter->first, section); ++iter)

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

	INI_KEY INI::get_key (const std::string &section, const std::string &line) const
	{
		INI_KEY k;
		k.section.assign (section);
		k.line.assign (line);

		return k;
	}

	COMMON_LIST & INI::_get_list (INI_MAP &m, const INI_KEY &key, const bool create) const
	{
		INI_MAP::iterator iter = m.find (key);
		if (iter != m.end ()) {
			return iter->second;
		}

		if (create == true) {
			std::pair <INI_MAP::iterator, bool> res = m.insert (std::make_pair (key, COMMON_LIST ()));
			if (res.second == false) {
				cThrow ("Unable to add key %s/%s", key.section.c_str (), key.line.c_str ());
			}

			return res.first->second;
		}

		cThrow ("Nonexistent entry %s/%s requested", key.section.c_str (), key.line.c_str ());
	}

	const COMMON_LIST & INI::_get_list (const INI_MAP &m, const INI_KEY &key) const
	{
		INI_MAP::const_iterator iter = m.find (key);
		if (iter != m.end ()) {
			return iter->second;
		}

		cThrow ("Nonexistent entry %s/%s requested", key.section.c_str (), key.line.c_str ());
	}

	COMMON_LIST INI::_get_list_copy (const INI_MAP &m, const INI_KEY &key) const
	{
		INI_MAP::const_iterator iter = m.find (key);
		if (iter != m.end ()) {
			return iter->second;
		}

		cThrow ("Nonexistent entry %s/%s requested", key.section.c_str (), key.line.c_str ());
	}

	INI_MAP::iterator INI::begin_of_section (INI_MAP &m, const std::string &section) const
	{
		return m.lower_bound (get_key (section, ""));
	}

	bool INI::is_same_section (const INI_KEY &key, const std::string &section) const
	{
		return key.section == section;
	}

	void INI::parse_line (INI_MAP &m, const std::string &line, std::string &active_section, const bool allow_include)
	{
		if (line.empty () == true) {
			return;
		}

		const size_t firstchar = line.find_first_not_of (" \t\n\r");
		if (firstchar == std::string::npos) {
			return;
		}

		if (line.at (firstchar) == ';' || line.at (firstchar) == '#') {
			return;
		}

		if (line.at (firstchar) == '@') {
			if (active_section.empty () == false) {
				cThrow ("Special directives allowed only in global section");
			}

			const size_t pos = line.find_first_of ("=");
			if (pos == std::string::npos) {
				cThrow ("Malformed line '%s'", line.c_str ());
			}

			std::string line_key = line.substr (0, pos);
			std::string line_val = line.substr (pos + 1);

			STR::trim (line_key);
			STR::trim (line_val);

			if (line_key == "@include") {
				if (allow_include == false) {
					cThrow ("Including files is not allowed");
				}

				parse_file (m, get_last_nested_realpath () + line_val, allow_include);
			}
			else if (line_key == "@glob") {
				if (allow_include == false) {
					cThrow ("Including files is not allowed");
				}

				FS::glob (get_last_nested_realpath () + line_val, [&](const std::string &fname) {
					parse_file (m, fname, allow_include);
				});
			}
			else {
				cThrow ("Unknown directive '%s'", line_key.c_str ());
			}

			return;
		}

		if (line.at (firstchar) == '[') {
			active_section.assign (line);
			STR::trim (active_section);
			if (active_section.at (0) != '[' || active_section.at (active_section.size () - 1) != ']') {
				cThrow ("Malformed line '%s'", line.c_str ());
			}
			active_section.erase (0, 1);
			active_section.erase (active_section.size () - 1);

			return;
		}

		const size_t pos = line.find_first_of ("=");
		if (pos == std::string::npos || pos == 0 || pos == (line.size () - 1)) {
			cThrow ("Malformed line '%s'", line.c_str ());
		}

		std::string line_key = line.substr (0, pos);
		std::string line_val = line.substr (pos + 1);

		STR::trim (line_key);
		STR::trim (line_val);

		bool append = false;
		bool refer = false;

		if (line_key.size () > 1 && line_key.back () == '&') {
			/* This is reference to some other value */
			line_key.pop_back ();
			STR::trim (line_key);
			refer = true;
		}

		if (line_key.size () > 2 && line_key.substr (line_key.size () - 2) == "[]") {
			line_key.pop_back ();
			line_key.pop_back ();
			append = true;
		}

		COMMON_LIST &v = _get_list (m, get_key (active_section, line_key), true);

		if (append == false) {
			v.clear ();
		}

		if (true == refer) {
			COMMON_VECTOR refval = STR::split<COMMON_VECTOR> (line_val, "/");

			INI_KEY key;
			if (refval.size () == 1) {
				key = get_key (active_section, line_val);
			}
			else if (refval.size () == 2) {
				key = get_key (refval[0], refval[1]);
			}
			else {
				cThrow ("Malformed reference. Reference has to contain either key from current section or section/key.");
			}

			COMMON_LIST referenced_list = _get_list_copy (m, key);
			v.splice (v.end (), std::move (referenced_list));
		}
		else {
			v.push_back (line_val);
		}
	}

	void INI::parse_file (INI_MAP &m, const std::string &fname, const bool allow_include)
	{
		if (_nested_parse_file.size () > _max_nested_files) {
			cThrow ("Maximum nesting of include directive reached");
		}

		for (const auto &tname : _nested_parse_file) {
			if (tname == fname) {
				cThrow ("Infinite nesting loop detected");
			}
		}

		_nested_parse_file.push_back (fname);
		P::debug_printf ("INI: Parsing file '%s' nest level %zu", fname.c_str (), _nested_parse_file.size ());

		std::string active_section;
		active_section.clear ();

		FS::read_file (fname, [&](const std::string &line) -> void {
			parse_line (m, line, active_section, allow_include);
		});

		_nested_parse_file.pop_back ();
	}

	void INI::parse_file (const std::string &fname, const bool allow_include)
	{
		_nested_parse_file.clear ();
		INI_MAP m = _map;
		parse_file (m, fname, allow_include);
		_map = m;
	}

	void INI::parse_buffer (INI_MAP &m, const std::string &buf, const bool allow_include)
	{
		std::string active_section;
		active_section.clear ();

		STR::split (buf, "\n\r", [&](const std::string &line) -> void {
			parse_line (m, line, active_section, allow_include);
		});
	}

	void INI::parse_buffer (const std::string &buf, const bool allow_include)
	{
		_nested_parse_file.clear ();
		INI_MAP m = _map;
		parse_buffer (m, buf, allow_include);
		_map = m;
	}

	bool INI::get_last_value (const INI_MAP &m, const INI_KEY &key, std::string &val) const
	{
		try {
			const COMMON_LIST &v = _get_list (m, key);
			if (v.empty () == true) {
				return false;
			}

			val.assign (v.back ());
		}
		catch (...) {
			return false;
		}

		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	INI::INI ()
	{
		reset ();
	}

	INI::INI (const INI &other) : _map (other._map)
	{
	}

	INI::INI (INI &&other) : _map (std::move (other._map))
	{
	}

	INI& INI::operator= (const INI &other)
	{
		if (this != &other) {
			_map = other._map;
		}
		return *this;
	}

	INI& INI::operator= (INI &&other)
	{
		if (this != &other) {
			_map = std::move (other._map);
		}
		return *this;
	}


	INI::INI (const std::string &fname, const bool allow_include)
	{
		reset ();
		parse_file (fname, allow_include);
	}

	void INI::reset (void)
	{
		_map.clear ();
	}

	void INI::load_file (const std::string &fname, const bool append, const bool allow_include)
	{
		if (append == false) {
			reset ();
		}
		parse_file (fname, allow_include);
	}

	void INI::save_to_file (const std::string &fname) const
	{
		FILE *s = ::fopen (fname.c_str (), "w");
		if (s == NULL) {
			cThrow ("Unable to write to file '%s'", fname.c_str ());
		}

		try {

			std::string active_section = "";
			bool lines_written = false;

			for (INI_MAP::const_iterator iter = _map.begin (); iter != _map.end (); ++iter) {
				const INI_KEY &key = iter->first;
				const COMMON_LIST &v = iter->second;

				if (active_section != key.section) {
					active_section.assign (key.section);

					if (lines_written) {
						::fprintf (s, "\n");
						if (::ferror (s)) {
							cThrow ("Error while writing to file '%s'", fname.c_str ());
						}
					}

					::fprintf (s, "[%s]\n", active_section.c_str ());
					if (::ferror (s)) {
						cThrow ("Error while writing to file '%s'", fname.c_str ());
					}
				}

				const bool append = v.size () > 1;
				for (COMMON_LIST::const_iterator vter = v.cbegin (); vter != v.cend (); ++vter) {
					::fprintf (s, "%s%s = %s\n", key.line.c_str (), append ? "[]" : "", vter->c_str ());
				}
				if (::ferror (s)) {
					cThrow ("Error while writing to file '%s'", fname.c_str ());
				}

				lines_written = true;
			}
		}
		catch (...) {
			::fclose (s);
			::unlink (fname.c_str ());
			throw;
		}

		::fclose (s);
	}

	void INI::load_buffer (const std::string &buf, const bool append, const bool allow_include)
	{
		if (append == false) {
			reset ();
		}
		parse_buffer (buf, allow_include);
	}

	void INI::save_to_buffer (std::string &out) const
	{

		std::string active_section = "";
		bool lines_written = false;

		for (INI_MAP::const_iterator iter = _map.begin (); iter != _map.end (); ++iter) {
			const INI_KEY &key = iter->first;
			const COMMON_LIST &v = iter->second;

			if (active_section != key.section) {
				active_section.assign (key.section);

				if (lines_written) {
					out.append ("\n");
				}
				out.append ("[" + active_section + "]\n");
			}

			const bool append = v.size () > 1;
			for (COMMON_LIST::const_iterator vter = v.cbegin (); vter != v.cend (); ++vter) {
				out.append (key.line);
				if (append) {
					out.append ("[]");
				}
				out.append ("=" + (*vter) + "\n");
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

	const COMMON_VECTOR INI::get_vector (const std::string &section, const std::string &key) const
	{
		COMMON_VECTOR out;
		try {
			const COMMON_LIST &lst = _get_list (_map, get_key (section, key));
			out.reserve (lst.size ());
			std::copy (lst.cbegin(), lst.cend(), std::back_inserter(out));
		}
		catch (...) {
			return out;
		}

		return out;
	}

	size_t INI::get_vector_size (const std::string &section, const std::string &key, const bool thr) const
	{
		return get_list_size (section, key, thr);
	}

	const COMMON_LIST & INI::get_list (const std::string &section, const std::string &key, const COMMON_LIST &defvalue, const bool thr) const
	{
		try {
			return _get_list (_map, get_key (section, key));
		}
		catch (...) {
			if (thr == true) {
				throw;
			}
			return defvalue;
		}
	}

	const COMMON_LIST INI::get_list (const std::string &section, const std::string &key) const
	{
		try {
			return _get_list (_map, get_key (section, key));
		}
		catch (...) {
			return COMMON_LIST ();
		}
	}

	size_t INI::get_list_size (const std::string &section, const std::string &key, const bool thr) const
	{
		try {
			return _get_list (_map, get_key (section, key)).size ();
		}
		catch (...) {
			if (thr == true) {
				throw;
			}
			return 0;
		}
	}

	bool INI::get_bool (const std::string &section, const std::string &key, const bool defvalue, const bool thr) const
	{
		return get_value (section, key, defvalue, thr);
	}

	uint8_t INI::get_uint8 (const std::string &section, const std::string &key, const uint8_t defvalue, const bool thr) const
	{
		return get_value (section, key, defvalue, thr);
	}

	uint16_t INI::get_uint16 (const std::string &section, const std::string &key, const uint16_t defvalue, const bool thr) const
	{
		return get_value (section, key, defvalue, thr);
	}

	uint32_t INI::get_uint32 (const std::string &section, const std::string &key, const uint32_t defvalue, const bool thr) const
	{
		return get_value (section, key, defvalue, thr);
	}

	uint64_t INI::get_uint64 (const std::string &section, const std::string &key, const uint64_t defvalue, const bool thr) const
	{
		return get_value (section, key, defvalue, thr);
	}

	int8_t INI::get_int8 (const std::string &section, const std::string &key, const int8_t defvalue, const bool thr) const
	{
		return get_value (section, key, defvalue, thr);
	}

	int16_t INI::get_int16 (const std::string &section, const std::string &key, const int16_t defvalue, const bool thr) const
	{
		return get_value (section, key, defvalue, thr);
	}

	int32_t INI::get_int32 (const std::string &section, const std::string &key, const int32_t defvalue, const bool thr) const
	{
		return get_value (section, key, defvalue, thr);
	}

	int64_t INI::get_int64 (const std::string &section, const std::string &key, const int64_t defvalue, const bool thr) const
	{
		return get_value (section, key, defvalue, thr);
	}

	std::string INI::get_string (const std::string &section, const std::string &key, const std::string &defvalue, const bool thr) const
	{
		std::string out;
		if (get_last_value (_map, get_key (section, key), out) == true) {
			return out;
		}

		if (thr == true) {
			cThrow ("Nonexistent entry %s/%s requested", section.c_str (), key.c_str ());
		}

		return defvalue;
	}

	std::string INI::get_string (const std::string &section, const std::string &key) const
	{
		std::string out;
		if (get_last_value (_map, get_key (section, key), out) == true) {
			return out;
		}
		return std::string ();
	}

	void INI::set_string (const std::string &section, const std::string &key, const std::string &val, const bool append)
	{
		COMMON_LIST &v = _get_list (_map, get_key (section, key), true);
		if (append == false) {
			v.clear ();
		}
		v.push_back (val);
	}

	void INI::set_vector (const std::string &section, const std::string &key, const COMMON_VECTOR &val, const bool append)
	{
		COMMON_LIST &v = _get_list (_map, get_key (section, key), true);
		if (append == false) {
			v.clear ();
		}
		v.insert (v.end (), val.begin (), val.end ());
	}

	void INI::set_list (const std::string &section, const std::string &key, const COMMON_LIST &val, const bool append)
	{
		COMMON_LIST &v = _get_list (_map, get_key (section, key), true);
		if (append == false) {
			v.clear ();
		}
		v.insert (v.end (), val.begin (), val.end ());
	}

}

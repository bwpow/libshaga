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

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string ArgTable::Entry::get_str (void) const
	{
		std::string s;

		if (key_short != 0) {
			s.append (fmt::sprintf ("-%c", key_short));
			if (has_param == true) {
				s.append ("<" + param_type + ">");
			}
		}

		if (key_long.empty () == false) {
			if (s.empty () == false) {
				s.append ("|");
			}
			s.append ("--" + key_long);
			if (has_param == true) {
				s.append ("=<" + param_type + ">");
			}
		}

		return s;
	}

	ArgTable::Entries::iterator ArgTable::find_entry_by_key (const std::string_view key_long, const char key_short)
	{
		for (Entries::iterator iter = _entries.begin (); iter != _entries.end (); ++iter) {
			if (key_long.empty () == false && iter->key_long.compare (key_long) == 0) {
				return iter;
			}
			if (key_short > 0 && iter->key_short == key_short) {
				return iter;
			}
		}

		if (key_long.empty () == true) {
			cThrow ("Unknown option '-%c'", key_short);
		}
		cThrow ("Unknown option '--%s'", key_long);
	}

	void ArgTable::process_entry (Entry &e, const std::string_view var)
	{
		if (var.empty () == true) {
			e.vars.push_back (STR::from_int (true));
		}
		else {
			e.vars.emplace_back (var);
		}

		_actual_entry = _entries.end ();
		_next_entry_is_param = false;
	}

	void ArgTable::process_entry (const std::string_view data)
	{
		if (true == _next_entry_is_param) {
			if (_actual_entry == _entries.end ()) {
				cThrow ("Expected parameter, but no option is selected");
			}
			process_entry (*_actual_entry, data);
		}
		else if (data.substr (0, 2) == "--") {
			/* This measn we have --key_long */
			const size_t pos = data.find_first_of ("=", 2);
			if (pos == std::string::npos) {
				/* There is no '=', so let's check if there should be parameter */
				_actual_entry = find_entry_by_key (data.substr (2), 0);
				if (true == _actual_entry->has_param) {
					cThrow ("Option '%s' is missing parameter", data);
				}
				process_entry (*_actual_entry, "");
			}
			else {
				_actual_entry = find_entry_by_key (data.substr (2, pos - 2), 0);
				if (false == _actual_entry->has_param) {
					cThrow ("Option '%s' shouldn't have parameter", data);
				}
				process_entry (*_actual_entry, data.substr (pos + 1));
			}
		}
		else if (data.size () >= 2 && data.at (0) == '-') {
			/* We have -key_short */
			size_t pos = 1;
			_next_entry_is_param = true;

			while (pos < data.size ()) {
				_actual_entry = find_entry_by_key ("", data.at (pos));
				pos++;

				if (false == _actual_entry->has_param) {
					process_entry (*_actual_entry, "");
				}
				else if (pos < data.size ()) {
					process_entry (*_actual_entry, data.substr (pos));
					break;
				}
			}
		}
		else {
			cThrow ("Unknown option '%s'", data);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ArgTable::ArgTable ()
	{
	}

	ArgTable::ArgTable (const std::string_view argv0) :
		_argv0 (argv0)
	{
	}

	void ArgTable::set_argv0 (const std::string_view str)
	{
		_argv0.assign (str);
	}

	std::string ArgTable::get_usage_string (void) const
	{
		std::string s = "Usage: ";

		if (_argv0.empty () == false) {
			s.append (_argv0);
		}
		else {
			s.append ("executable");
		}

		for (const auto &e : _entries) {
			s.append (" ");
			switch (e.incidence) {
				case INCIDENCE::ANY:
					s.append ("[" + e.get_str () + " ...]");
					break;

				case INCIDENCE::ZERO_OR_ONE:
					s.append ("[" + e.get_str () + "]");
					break;

				case INCIDENCE::ONE:
					s.append (e.get_str ());
					break;

				case INCIDENCE::AT_LEAST_ONE:
					s.append (e.get_str () + " [" + e.get_str () + " ...]");
					break;

			}
		}

		return s;
	}

	COMMON_COMMON_VECTOR ArgTable::get_help_vector (void) const
	{
		COMMON_COMMON_VECTOR v;

		for (const auto &e : _entries) {
			COMMON_VECTOR x;

			x.push_back (e.get_str ());
			x.push_back (e.help);

			v.push_back (x);
		}

		return v;
	}

	std::string ArgTable::get_help_string (void) const
	{
		const COMMON_COMMON_VECTOR v = get_help_vector ();
		std::string s = "Options:";

		size_t cc = 0;
		for (const auto &e : v) {
			if (e[0].size () > cc) {
				cc = e[0].size ();
			}
		}

		for (const auto &e : v) {
			s.append ("\n    ");
			s.append (e[0]);
			s.append (std::string (cc - e[0].size (), ' '));
			s.append (" : ");
			s.append (e[1]);
		}

		return s;
	}

	void ArgTable::print_usage (void) const
	{
		std::cout << get_usage_string () << "\n\n" << get_help_string () << std::endl;
	}

	std::ostream& operator<< (std::ostream& stream, const ArgTable& t)
	{
		stream << t.get_usage_string () << "\n\n" << t.get_help_string ();
		return stream;
	}


	ArgTable & ArgTable::add (const std::string_view key_long, const char key_short, const ArgTable::INCIDENCE incidence, const bool has_param, const std::string_view help, const std::string_view param_type, CheckerCallback checker)
	{
		for (const auto &e : _entries) {
			if (e.key_long.empty () == false && key_long == e.key_long) {
				cThrow ("Option '--%s' is duplicated", key_long);
			}
			if (e.key_short != 0 && key_short == e.key_short) {
				cThrow ("Option '-%c' is duplicated", key_short);
			}
		}

		Entry e;
		e.key_long.assign (key_long);
		e.key_short = key_short;
		e.help.assign (help);
		e.checker = checker;
		e.has_param = has_param;
		e.incidence = incidence;

		if (param_type.empty () == true) {
			e.param_type.assign ("PARAM");
		}
		else {
			e.param_type.assign (param_type);
		}

		_entries.push_back (e);

		return *this;
	}

	ArgTable & ArgTable::add (const std::string_view key_long, const ArgTable::INCIDENCE incidence, const bool has_param, const std::string_view help, const std::string_view param_type, CheckerCallback checker)
	{
		return add (key_long, '\0', incidence, has_param, help, param_type, checker);
	}

	ArgTable & ArgTable::add (const char key_short, const ArgTable::INCIDENCE incidence, const bool has_param, const std::string_view help, const std::string_view param_type, CheckerCallback checker)
	{
		return add ("", key_short, incidence, has_param, help, param_type, checker);
	}


	bool ArgTable::process (const COMMON_VECTOR &v, const bool thr)
	{
		for (auto &e : _entries) {
			e.vars.clear ();
		}
		_next_entry_is_param = false;
		_actual_entry = _entries.end ();

		try {
			/* Process all entries */
			for (auto &e : v) {
				process_entry (e);
			}

			if (true == _next_entry_is_param) {
				cThrow ("Option '%s' is missing parameter", v.back());
			}

			/* Check for correct incidence */
			for (const auto &e : _entries) {
				switch (e.incidence) {
					case INCIDENCE::ANY:
						break;

					case INCIDENCE::ZERO_OR_ONE:
						if (e.vars.size () > 1) {
							cThrow ("Option '%s' must be used at most once", e.get_str ());
						}
						break;

					case INCIDENCE::ONE:
						if (e.vars.size () != 1) {
							cThrow ("Option '%s' must be used exactly once", e.get_str ());
						}
						break;

					case INCIDENCE::AT_LEAST_ONE:
						if (e.vars.size () < 1) {
							cThrow ("Option '%s' must be used at least once", e.get_str ());
						}
						break;
				}
			}

			/* Chechk using special checker functions */
			for (const auto &e : _entries) {
				if (e.checker != nullptr) {
					for (const auto &v : e.vars) {
						if (e.checker (v) == false) {
							cThrow ("Parameter '%s' is not valid for option '%s'", v, e.get_str ());
						}
					}
				}
			}
		}
		catch (...) {
			if (thr == false) {
				return false;
			}
			throw;
		}

		return true;
	}

	bool ArgTable::process (const COMMON_VECTOR &v, std::string &error)
	{
		try {
			process (v, true);
		}
		catch (const std::exception &e) {
			error.assign ("Error: " + std::string (e.what ()));
			return false;
		}
		catch (...) {
			error.assign ("Error: Unable to parse options");
			return false;
		}

		return true;
	}

	INI ArgTable::export_ini (const std::string_view section) const
	{
		std::string s;

		if (section.empty () == false) {
			s.append ("[" + std::string (section) + "]\n");
		}

		for (const auto &e : _entries) {
			for (const auto &v: e.vars) {
				if (e.key_long.empty () == true) {
					s.append (1, e.key_short);
				}
				else {
					s.append (e.key_long);
				}
				s.append ("[]=" + v + "\n");
			}
		}

		INI ini;
		ini.load_buffer (s, false);
		return ini;
	}

	INI ArgTable::export_ini (void) const
	{
		return export_ini ("");
	}
}

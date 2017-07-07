/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#include <cstdarg>
#include <climits>

namespace shaga {

	void STR::sprintf (std::string &str, const char *fmt, va_list &ap)
	{
		if (str.capacity () < 8) {
			str.resize (8);
		}
		else {
			str.resize (str.capacity ());
		}

		va_list cpy;
		::va_copy (cpy, ap);
		/* Note: Since C++11, std::string::data memory must be continuous */
		/* Note: Since C++17, std::string::data memory won't be const */
		const int l = ::vsnprintf (&str[0], str.size (), fmt, cpy);
		::va_end (cpy);

		if (l < static_cast<int> (str.size ())) {
			str.resize (l);
		}
		else {
			str.resize (l + 1);
			::vsnprintf (&str[0], str.size (), fmt, ap);
			str.resize (l);
		}
	}

	std::string STR::sprintf (const char *fmt, va_list &ap)
	{
		std::string str;
		sprintf (str, fmt, ap);
		return str;
	}

	void STR::sprintf (std::string &str, const char *fmt, ...)
	{
		va_list ap;
		::va_start (ap, fmt);
		sprintf (str, fmt, ap);
		::va_end (ap);
	}

	std::string STR::sprintf (const char *fmt, ...)
	{
		std::string str;
		va_list ap;
		::va_start (ap, fmt);
		sprintf (str, fmt, ap);
		::va_end (ap);

		return str;
	}

	void STR::sprintf (COMMON_VECTOR &v, const char *fmt, ...)
	{
		std::string str;
		va_list ap;
		::va_start (ap, fmt);
		sprintf (str, fmt, ap);
		::va_end (ap);

		v.push_back (str);
	}

	void STR::sprintf (COMMON_LIST &v, const char *fmt, ...)
	{
		std::string str;
		va_list ap;
		::va_start (ap, fmt);
		sprintf (str, fmt, ap);
		::va_end (ap);

		v.push_back (str);
	}

	static inline void _to_uint_process_char (uint64_t &result, const uint8_t chr, const uint64_t base)
	{
		uint64_t digit = 0;
		if (chr >= '0' && chr <= '9') {
			digit = chr - '0';
		}
		else if (chr >= 'A' && chr <= 'Z') {
			digit = chr - 'A' + 10;
		}
		else if (chr >= 'a' && chr <= 'z') {
			digit = chr - 'a' + 10;
		}
		else {
			cThrow ("Unrecognized character '%c'", chr);
		}

		if (digit >= base) {
			cThrow ("Unrecognized character '%c'", chr);
		}

		if (result > (UINT64_MAX / base) || (result * base) > (UINT64_MAX - digit)) {
			cThrow ("Out of range");
		}

		result = {(result * base) + digit};
	}

	template <typename T>
	static void _to_uint_process (T &result, const std::string &s, const int base, const char *type)
	{
		if (base < 2) {
			cThrow ("Base must be at least 2");
		}

		uint64_t out {0};
		bool is_negative {false};

		try {
			enum class Stage {
				START_SPACES,
				DIGITS,
				TRAILING_SPACES,
			} stage {Stage::START_SPACES};

			for (std::string::const_iterator iter = s.cbegin (); iter != s.cend (); ++iter) {
				if (::isspace (*iter)) {
					switch (stage) {
						case Stage::START_SPACES:
						case Stage::TRAILING_SPACES:
							break;

						case Stage::DIGITS:
							stage = Stage::TRAILING_SPACES;
							break;
					}
				}
				else if ('\'' == (*iter)) {
					/* Ignore this character */
				}
				else if ('+' == (*iter) || '-' == (*iter)) {
					switch (stage) {
						case Stage::START_SPACES:
							stage = Stage::DIGITS;
							break;

						case Stage::DIGITS:
						case Stage::TRAILING_SPACES:
							cThrow ("Unrecognized character");
					}

					if ('-' == (*iter)) {
						is_negative = true;
					}
				}
				else {
					switch (stage) {
						case Stage::START_SPACES:
						case Stage::DIGITS:
							_to_uint_process_char (out, static_cast <const uint8_t> (*iter), static_cast<const uint64_t> (base));
							stage = Stage::DIGITS;
							break;

						case Stage::TRAILING_SPACES:
							cThrow ("Unrecognized character");
					}
				}
			}


			if (std::numeric_limits<T>::is_signed == false && true == is_negative) {
				/* This is not a signed type, but the parsed integer is negative */
				cThrow ("Out of range");
			}
			else if (false == is_negative) {
				if (out > std::numeric_limits<T>::max ()) {
					cThrow ("Out of range");
				}

				result = static_cast<T> (out);
			}
			else if (std::is_same <T, int64_t>::value) {
				if (out > static_cast<uint64_t> (-std::numeric_limits<T>::min ())) {
					cThrow ("Out of range");
				}

				result = -out;
			}
			else {
				int64_t nout = -static_cast<int64_t> (out);
				if (nout < static_cast<int64_t> (std::numeric_limits<T>::min ())) {
					cThrow ("Out of range");
				}

				result = static_cast<T> (nout);
			}
		}
		catch (const std::exception &e) {
			cThrow ("Could not convert '%s' to %s: %s", s.c_str (), type, e.what ());
		}
	}

	bool STR::to_bool (const std::string &s, const int base)
	{
		(void) base;
		if (icompare (s, "true") == true || icompare (s, "on") == true || icompare (s, "yes") == true || s == "1") {
			return true;
		}
		else if (icompare (s, "false") == true || icompare (s, "off") == true || icompare (s, "no") == true || s == "0") {
			return false;
		}
		cThrow ("Could not convert '%s' to bool: Not recognized", s.c_str ());
	}

	uint8_t STR::to_uint8 (const std::string &s, const int base)
	{
		uint8_t out;
		_to_uint_process (out, s, base, "uint8");
		return out;
	}

	uint16_t STR::to_uint16 (const std::string &s, const int base)
	{
		uint16_t out;
		_to_uint_process (out, s, base, "uint16");
		return out;
	}

	uint32_t STR::to_uint32 (const std::string &s, const int base)
	{
		uint32_t out;
		_to_uint_process (out, s, base, "uint32");
		return out;
	}

	uint64_t STR::to_uint64 (const std::string &s, const int base)
	{
		uint64_t out;
		_to_uint_process (out, s, base, "uint64");
		return out;
	}

	int8_t STR::to_int8 (const std::string &s, const int base)
	{
		int8_t out;
		_to_uint_process (out, s, base, "int8");
		return out;
	}

	int16_t STR::to_int16 (const std::string &s, const int base)
	{
		int16_t out;
		_to_uint_process (out, s, base, "int16");
		return out;
	}

	int32_t STR::to_int32 (const std::string &s, const int base)
	{
		int32_t out;
		_to_uint_process (out, s, base, "int32");
		return out;
	}

	int64_t STR::to_int64 (const std::string &s, const int base)
	{
		int64_t out;
		_to_uint_process (out, s, base, "int64");
		return out;
	}

	void STR::split (const std::string &what, const std::string &delimiter, std::function<void (const std::string &)> f)
	{
		size_t  start = 0, end = 0;
		std::string t;

		while (end != std::string::npos) {
			end = what.find_first_of (delimiter, start);

			// If at end, use length=maxLength.  Else use length=end-start.
			t = what.substr (start, (end == std::string::npos) ? std::string::npos : (end - start));
			if (t.size() > 0) {
				trim (t);
				f (t);
			}

			// If at end, use start=maxSize.  Else use start=end+delimiter.
			start = ( (end > (std::string::npos - 1)) ? std::string::npos : end + 1);
		}
	}


	COMMON_VECTOR STR::split_to_vector (const std::string &what, const std::string &delimiter)
	{
		COMMON_VECTOR out;
		split (out, what, delimiter);
		return out;
	}

	COMMON_DEQUE STR::split_to_deque (const std::string &what, const std::string &delimiter)
	{
		COMMON_DEQUE out;
		split (out, what, delimiter);
		return out;
	}

	COMMON_LIST STR::split_to_list (const std::string &what, const std::string &delimiter)
	{
		COMMON_LIST out;
		split (out, what, delimiter);
		return out;
	}

	void STR::format_time (std::string &out, const time_t theTime, const bool local, const bool for_filename)
	{
		struct tm t;
		::memset (&t, 0, sizeof (t));

		if (local == true) {
			::localtime_r (&theTime, &t);
		}
		else {
			::gmtime_r (&theTime, &t);
		}
#ifndef OS_WIN
		if (true == for_filename) {
			sprintf (out, "%04d-%02d-%02d_%02d%02d%02d_%s", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, t.tm_zone);
		}
		else {
			sprintf (out, "%04d-%02d-%02d %02d:%02d:%02d %s", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, t.tm_zone);
		}
#else
		if (true == for_filename) {
			sprintf (out, "%04d-%02d-%02d_%02d%02d%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
		}
		else {
			sprintf (out, "%04d-%02d-%02d %02d:%02d:%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
		}
#endif // OS_WIN
	}

	std::string STR::format_time (const time_t theTime, const bool local, const bool for_filename)
	{
		std::string out;
		format_time (out, theTime, local, for_filename);
		return out;
	}

	std::string STR::format_time (const bool local, const bool for_filename)
	{
		time_t theTime;
		::time (&theTime);
		return format_time (theTime, local, for_filename);
	}

	void STR::format_date (std::string &out, const time_t theTime, const bool local, const char *separatorstr)
	{
		struct tm t;
		memset (&t, 0, sizeof (t));

		if (local == true) {
			::localtime_r (&theTime, &t);
		}
		else {
			::gmtime_r (&theTime, &t);
		}

		sprintf (out, "%04d%s%02d%s%02d", t.tm_year + 1900, separatorstr, t.tm_mon + 1, separatorstr, t.tm_mday);
	}

	std::string STR::format_date (const time_t theTime, const bool local, const char *separatorstr)
	{
		std::string out;
		format_date (out, theTime, local, separatorstr);
		return out;
	}

	std::string STR::format_date (const bool local, const char *separatorstr)
	{
		time_t theTime;
		::time (&theTime);
		return format_date (theTime, local, separatorstr);
	}

	void STR::replace (std::string &str, const std::string &what, const std::string &with)
	{
		size_t pos = 0;
		while (true) {
			pos = str.find (what, pos);
			if (pos == std::string::npos) {
				break;
			}

			str.replace (pos, what.size (), with);
			pos += with.size ();
		}
	}

	bool STR::has_suffix (const std::string &str, const std::string &suffix)
	{
		return (str.size () >= suffix.size ()) && std::equal (suffix.rbegin (), suffix.rend (), str.rbegin ());
	}

	bool STR::has_isuffix (const std::string &str, const std::string &suffix)
	{
		return (str.size () >= suffix.size ()) && std::equal (suffix.rbegin (), suffix.rend (), str.rbegin (),
			[](const auto a, const auto b) -> bool {
				return std::tolower(a) == std::tolower(b);
			});
	}

	bool STR::has_prefix (const std::string &str, const std::string &prefix)
	{
		return (str.size () >= prefix.size ()) && std::equal (prefix.begin (), prefix.end (), str.begin ());
	}

	bool STR::has_iprefix (const std::string &str, const std::string &prefix)
	{
		return (str.size () >= prefix.size ()) && std::equal (prefix.begin (), prefix.end (), str.begin (),
			[](const auto a, const auto b) -> bool {
				return std::tolower(a) == std::tolower(b);
			});
	}

	void STR::make_printable (std::string &text, const unsigned char replace_with)
	{
		std::transform (text.begin (), text.end (), text.begin (), [replace_with](const unsigned char val) {
			if (val < 0x20 || val > 0x7e) {
				return replace_with;
			}
			else {
				return val;
			}
		});
	}

	std::string STR::get_printable (const std::string &text, const unsigned char replace_with)
	{
		std::string output (text);
		make_printable (output, replace_with);
		return output;
	}

	void STR::rtrim (std::string &str, const std::string &chars)
	{
		const auto found = str.find_last_not_of (chars);
		if (std::string::npos != found) {
			str.resize (found + 1);
		}
		else {
			str.resize (0);
		}
	}

	void STR::ltrim (std::string &str, const std::string &chars)
	{
		const auto found = str.find_first_not_of (chars);
		if (std::string::npos != found) {
			str.erase (0, found);
		}
		else {
			str.resize (0);
		}
	}

	void STR::trim (std::string &str, const std::string &chars)
	{
		rtrim (str, chars);
		ltrim (str, chars);
	}

	bool STR::icompare (const std::string &a, const std::string &b)
	{
		return std::equal (a.begin (), a.end (), b.begin (), b.end (), [](const auto a, const auto b) {
			return std::tolower(a) == std::tolower(b);
		});
	}

	char * STR::dirdup (const char *buf, const bool remove_trailing_slash)
	{
		ssize_t l = static_cast<ssize_t> (strlen (buf));
		char *output = reinterpret_cast<char *> (::malloc (l + 2));
		if (nullptr == output) {
			cThrow ("Alloc failed");
		}
		::memcpy (output, buf, l);
		output [l] = '\0';

		if (true == remove_trailing_slash) {
			while (l >= 2 && (output [l - 1] == '/' || output [l - 1] == '\\')) {
				output [l - 1] = '\0';
				l--;
			}
		}
		else {
			if (l == 0 || (output [l - 1] != '/' && output [l - 1] != '\\')) {
				output [l] = '/';
				output [l + 1] = '\0';
			}
		}

		return output;
	}

	char * STR::strdup (const char *buf)
	{
		char *output = ::strdup (buf);
		if (nullptr == output) {
			cThrow ("strdup failed");
		}
		return output;
	}

	std::string STR::from_argv (const COMMON_VECTOR &v, const char separator)
	{
		std::string out;

		for (COMMON_VECTOR::const_iterator iter = v.begin (); iter != v.end (); ++iter) {
			if (iter != v.begin ()) {
				out.append (1, separator);
			}
			out.append (*iter);
		}

		return out;
	}

	std::string STR::from_argv (const int argc, const char * const * argv, const char separator)
	{
		std::string out;

		for (int i = 1; i < argc; ++i) {
			if (i > 1) {
				out.append (1, separator);
			}
			out.append (argv[i]);
		}

		return out;
	}

	COMMON_VECTOR STR::to_argv (const int argc, const char * const * argv)
	{
		COMMON_VECTOR v;
		for (int i = 1; i < argc; ++i) {
			v.push_back (argv[i]);
		}

		return v;
	}

	COMMON_VECTOR STR::to_argv (const std::string &vars)
	{
		std::string cmd = vars;
		COMMON_VECTOR v;

		replace (cmd, "\\\\", std::string (1, 0x02));
		replace (cmd, "\\\"", std::string (1, 0x03));
		replace (cmd, "\\'",  std::string (1, 0x04));

		std::string what_find = "\"'";
		size_t pos = 0, last_pos;
		while (true) {
			pos = cmd.find_first_of (what_find, pos);
			if (pos == std::string::npos) {
				break;
			}

			if (what_find.size () > 1) {
				what_find = cmd.at (pos);
			}
			else {
				what_find = "\"'";
			}

			cmd.replace (pos, 1, 1, 0x01);
			pos++;
		}

		replace (cmd, std::string (1, 0x02), "\\");
		replace (cmd, std::string (1, 0x03), "\"");
		replace (cmd, std::string (1, 0x04), "'");

		pos = 0;
		bool in_quotes = false;

		while (true) {
			last_pos = pos;
			pos = cmd.find_first_of (0x01, pos);
			if (pos == std::string::npos) {
				if (in_quotes) {
					if (last_pos >= 2 && (cmd.at (last_pos - 2) == ' ' || cmd.at (last_pos - 2) == '\t')) {
						v.push_back (cmd.substr (last_pos));
					}
					else {
						v.back ().append (cmd.substr (last_pos));
					}
				}
				else {
					if (last_pos < cmd.size () && cmd.at (last_pos) != ' ' && cmd.at (last_pos) != '\t' && v.empty () == false) {
						v.back ().append (1, 0x01);
					}
					split (v, cmd.substr (last_pos), " \t");
				}

				break;
			}

			if (pos > 0) {
				if (in_quotes) {
					if (last_pos >= 2 && (cmd.at (last_pos - 2) == ' ' || cmd.at (last_pos - 2) == '\t')) {
						v.push_back (cmd.substr (last_pos, pos - last_pos));
					}
					else {
						v.back ().append (cmd.substr (last_pos, pos - last_pos));
					}
				}
				else {
					if (last_pos < cmd.size () && cmd.at (last_pos) != ' ' && cmd.at (last_pos) != '\t' && v.empty () == false) {
						v.back ().append (1, 0x01);
					}
					split (v, cmd.substr (last_pos, pos - last_pos), " \t");
				}
			}
			else {
				v.push_back ("");
			}

			pos++;
			in_quotes = !in_quotes;
		}

		std::string to_prefix;
		to_prefix.clear ();

		for (COMMON_VECTOR::iterator iter = v.begin (); iter != v.end (); ) {
			if (to_prefix.empty () == false) {
				*iter = to_prefix + *iter;
				to_prefix.clear ();
			}

			if ( (*iter).empty ()) {
				iter = v.erase (iter);
				continue;
			}
			if (iter->back () == 0x01) {
				iter->pop_back ();
				to_prefix.assign (*iter);

				iter = v.erase (iter);
				continue;
			}

			++iter;
		}

		if (to_prefix.empty () == false) {
			v.push_back (to_prefix);
		}

		return v;
	}

}

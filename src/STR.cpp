/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {

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
			cThrow ("Unrecognized character '{:c}'"sv, static_cast<char> (chr));
		}

		if (digit >= base) {
			cThrow ("Unrecognized character '{:c}'"sv, static_cast<char> (chr));
		}

		if (result > (UINT64_MAX / base) || (result * base) > (UINT64_MAX - digit)) {
			cThrow ("Out of range"sv);
		}

		result = {(result * base) + digit};
	}

	template <typename T>
	static void _to_uint_process (T &result, const std::string_view src, const int base, const std::string_view type)
	{
		if (base < 2) {
			cThrow ("Base must be at least 2"sv);
		}

		uint64_t out {0};
		bool is_negative {false};

		try {
			enum class Stage {
				START_SPACES,
				DIGITS,
				TRAILING_SPACES,
			} stage {Stage::START_SPACES};

			for (auto iter = src.cbegin (); iter != src.cend (); ++iter) {
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
							cThrow ("Unrecognized character"sv);
					}

					if ('-' == (*iter)) {
						is_negative = true;
					}
				}
				else {
					switch (stage) {
						case Stage::START_SPACES:
						case Stage::DIGITS:
							_to_uint_process_char (out, static_cast <uint8_t> (*iter), static_cast<uint64_t> (base));
							stage = Stage::DIGITS;
							break;

						case Stage::TRAILING_SPACES:
							cThrow ("Unrecognized character"sv);
					}
				}
			}

			if (std::numeric_limits<T>::is_signed == false && true == is_negative) {
				/* This is not a signed type, but the parsed integer is negative */
				cThrow ("Out of range"sv);
			}
			else if (false == is_negative) {
				if (out > std::numeric_limits<T>::max ()) {
					cThrow ("Out of range"sv);
				}

				result = static_cast<T> (out);
			}
			else if (std::is_same <T, int64_t>::value) {
				// uint64_t can be converted to int64_t when out is larger than 1<<63 on most platforms
				// !!! IMPORTANT !!! Fix this code to be platform independent
				if (out > (1ULL << 63)) {
					cThrow ("Out of range"sv);
				}

				result = -out;
			}
			else {
				const int64_t nout = -static_cast<int64_t> (out);
				if (nout < static_cast<int64_t> (std::numeric_limits<T>::min ())) {
					cThrow ("Out of range"sv);
				}

				result = static_cast<T> (nout);
			}
		}
		catch (const std::exception &e) {
			cThrow ("Could not convert '{}' to {}: {}"sv, src, type, e.what ());
		}
	}

	bool STR::to_bool (const std::string_view s, [[maybe_unused]] const int base)
	{
		if (icompare (s, "true"sv) == true || icompare (s, "on"sv) == true || icompare (s, "yes"sv) == true || s == "1"sv) {
			return true;
		}
		else if (icompare (s, "false"sv) == true || icompare (s, "off"sv) == true || icompare (s, "no"sv) == true || s == "0"sv) {
			return false;
		}
		cThrow ("Could not convert '{}' to bool: Not recognized"sv);
	}

	uint8_t STR::to_uint8 (const std::string_view s, const int base)
	{
		uint8_t out;
		_to_uint_process (out, s, base, "uint8"sv);
		return out;
	}

	uint16_t STR::to_uint16 (const std::string_view s, const int base)
	{
		uint16_t out;
		_to_uint_process (out, s, base, "uint16"sv);
		return out;
	}

	uint32_t STR::to_uint32 (const std::string_view s, const int base)
	{
		uint32_t out;
		_to_uint_process (out, s, base, "uint32"sv);
		return out;
	}

	uint64_t STR::to_uint64 (const std::string_view s, const int base)
	{
		uint64_t out;
		_to_uint_process (out, s, base, "uint64"sv);
		return out;
	}

	int8_t STR::to_int8 (const std::string_view s, const int base)
	{
		int8_t out;
		_to_uint_process (out, s, base, "int8"sv);
		return out;
	}

	int16_t STR::to_int16 (const std::string_view s, const int base)
	{
		int16_t out;
		_to_uint_process (out, s, base, "int16"sv);
		return out;
	}

	int32_t STR::to_int32 (const std::string_view s, const int base)
	{
		int32_t out;
		_to_uint_process (out, s, base, "int32"sv);
		return out;
	}

	int64_t STR::to_int64 (const std::string_view s, const int base)
	{
		int64_t out;
		_to_uint_process (out, s, base, "int64"sv);
		return out;
	}

	void STR::split (const std::string_view what, const std::string_view delimiter, std::function<void (std::string_view)> callback)
	{
		size_t p_start = 0;
		size_t p_end = 0;

		while (p_end != std::string_view::npos) {
			p_end = what.find_first_of (delimiter, p_start);

			// If at end, use length=maxLength.  Else use length=end-start.
			std::string_view token = what.substr (p_start, (p_end == std::string_view::npos) ? std::string_view::npos : (p_end - p_start));
			if (token.size() > 0) {
				trim (token);
				callback (token);
			}

			// If at end, use start=maxSize.  Else use start=end+delimiter.
			p_start = (p_end > (std::string_view::npos - 1)) ? std::string_view::npos : (p_end + 1);
		}
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
			STR::sprint (out, "{:04}-{:02}-{:02}_{:02}{:02}{:02d}_{}"sv,
				t.tm_year + 1900,
				t.tm_mon + 1,
				t.tm_mday,
				t.tm_hour,
				t.tm_min,
				t.tm_sec,
				t.tm_zone);
		}
		else {
			STR::sprint (out, "{:04}-{:02}-{:02} {:02}:{:02}:{:02d} {}"sv,
				t.tm_year + 1900,
				t.tm_mon + 1,
				t.tm_mday,
				t.tm_hour,
				t.tm_min,
				t.tm_sec,
				t.tm_zone);
		}
#else
		if (true == for_filename) {
			STR::sprint (out, "{:04}-{:02}-{:02}_{:02}{:02}{:02d}"sv,
				t.tm_year + 1900,
				t.tm_mon + 1,
				t.tm_mday,
				t.tm_hour,
				t.tm_min,
				t.tm_sec);
		}
		else {
			STR::sprint (out, "{:04}-{:02}-{:02} {:02}:{:02}:{:02d}"sv,
				t.tm_year + 1900,
				t.tm_mon + 1,
				t.tm_mday,
				t.tm_hour,
				t.tm_min,
				t.tm_sec);
		}
#endif // OS_WIN
	}

	void STR::format_time (std::string &out, const bool local, const bool for_filename)
	{
		time_t theTime;
		::time (&theTime);
		return format_time (out, theTime, local, for_filename);
	}

	std::string STR::format_time (const time_t theTime, const bool local, const bool for_filename)
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
			return fmt::format ("{:04}-{:02}-{:02}_{:02}{:02}{:02d}_{}"sv,
				t.tm_year + 1900,
				t.tm_mon + 1,
				t.tm_mday,
				t.tm_hour,
				t.tm_min,
				t.tm_sec,
				t.tm_zone);
		}
		else {
			return fmt::format ("{:04}-{:02}-{:02} {:02}:{:02}:{:02d} {}"sv,
				t.tm_year + 1900,
				t.tm_mon + 1,
				t.tm_mday,
				t.tm_hour,
				t.tm_min,
				t.tm_sec,
				t.tm_zone);
		}
#else
		if (true == for_filename) {
			return fmt::format ("{:04}-{:02}-{:02}_{:02}{:02}{:02d}"sv,
				t.tm_year + 1900,
				t.tm_mon + 1,
				t.tm_mday,
				t.tm_hour,
				t.tm_min,
				t.tm_sec);
		}
		else {
			return fmt::format ("{:04}-{:02}-{:02} {:02}:{:02}:{:02d}"sv,
				t.tm_year + 1900,
				t.tm_mon + 1,
				t.tm_mday,
				t.tm_hour,
				t.tm_min,
				t.tm_sec);
		}
#endif // OS_WIN
	}

	std::string STR::format_time (const bool local, const bool for_filename)
	{
		time_t theTime;
		::time (&theTime);
		return format_time (theTime, local, for_filename);
	}

	void STR::format_date (std::string &out, const time_t theTime, const bool local, const std::string_view separatorstr)
	{
		struct tm t;
		::memset (&t, 0, sizeof (t));

		if (local == true) {
			::localtime_r (&theTime, &t);
		}
		else {
			::gmtime_r (&theTime, &t);
		}

		STR::sprint (out, "{:04}{}{:02}{}{:02}"sv,
			t.tm_year + 1900,
			separatorstr,
			t.tm_mon + 1,
			separatorstr,
			t.tm_mday);
	}

	void STR::format_date (std::string &out, const bool local, const std::string_view separatorstr)
	{
		time_t theTime;
		::time (&theTime);
		return format_date (out, theTime, local, separatorstr);
	}

	std::string STR::format_date (const time_t theTime, const bool local, const std::string_view separatorstr)
	{
		struct tm t;
		::memset (&t, 0, sizeof (t));

		if (local == true) {
			::localtime_r (&theTime, &t);
		}
		else {
			::gmtime_r (&theTime, &t);
		}

		return fmt::format ("{:04}{}{:02}{}{:02}"sv,
			t.tm_year + 1900,
			separatorstr,
			t.tm_mon + 1,
			separatorstr,
			t.tm_mday);
	}

	std::string STR::format_date (const bool local, const std::string_view separatorstr)
	{
		time_t theTime;
		::time (&theTime);
		return format_date (theTime, local, separatorstr);
	}

	void STR::replace (std::string &str, const std::string_view what, const std::string_view with)
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

	bool STR::has_suffix (const std::string_view str, const std::string_view suffix)
	{
		return (str.size () >= suffix.size ()) && std::equal (suffix.rbegin (), suffix.rend (), str.rbegin ());
	}

	bool STR::has_isuffix (const std::string_view str, const std::string_view suffix)
	{
		return (str.size () >= suffix.size ()) && std::equal (suffix.rbegin (), suffix.rend (), str.rbegin (),
			[](const auto a, const auto b) -> bool {
				return std::tolower(a) == std::tolower(b);
			});
	}

	bool STR::has_prefix (const std::string_view str, const std::string_view prefix)
	{
		return (str.size () >= prefix.size ()) && std::equal (prefix.begin (), prefix.end (), str.begin ());
	}

	bool STR::has_iprefix (const std::string_view str, const std::string_view prefix)
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

	std::string STR::get_printable (const std::string_view text, const unsigned char replace_with)
	{
		std::string output (text);
		make_printable (output, replace_with);
		return output;
	}

	void STR::rtrim (std::string &str, const std::string_view chars)
	{
		const auto found = str.find_last_not_of (chars);
		if (std::string::npos != found) {
			str.resize (found + 1);
		}
		else {
			str.resize (0);
		}
	}

	void STR::rtrim (std::string_view &str, const std::string_view chars)
	{
		const auto found = str.find_last_not_of (chars);
		if (std::string::npos != found) {
			str.remove_suffix (str.size () - found - 1);
		}
		else {
			str.remove_suffix (str.size ());
		}
	}

	void STR::ltrim (std::string &str, const std::string_view chars)
	{
		const auto found = str.find_first_not_of (chars);
		if (std::string::npos != found) {
			str.erase (0, found);
		}
		else {
			str.resize (0);
		}
	}

	void STR::ltrim (std::string_view &str, const std::string_view chars)
	{
		const auto found = std::min (str.find_first_not_of (chars), str.size ());
		str.remove_prefix (found);
	}

	void STR::trim (std::string &str, const std::string_view chars)
	{
		rtrim (str, chars);
		ltrim (str, chars);
	}

	void STR::trim (std::string_view &str, const std::string_view chars)
	{
		rtrim (str, chars);
		ltrim (str, chars);
	}

	bool STR::icompare (const std::string_view a, const std::string_view b)
	{
		return std::equal (a.cbegin (), a.cend (), b.cbegin (), b.cend (), [](const auto _a, const auto _b) -> bool {
			return std::tolower(_a) == std::tolower(_b);
		});
	}

	std::string STR::multiply (std::string_view what, const uint_fast32_t cnt)
	{
		std::string out;
		out.reserve (what.size () * cnt);
		for (uint_fast32_t i = 0; i < cnt; ++i) {
			out.append (what);
		}
		return out;
	}

	std::string STR::multiply (std::string_view what, const uint_fast32_t cnt, const std::string_view postfix)
	{
		std::string out;
		out.reserve (postfix.size () + (what.size () * cnt));
		for (uint_fast32_t i = 0; i < cnt; ++i) {
			out.append (what);
		}
		out.append (postfix);
		return out;
	}

	char * STR::dirdup (const char *buf, const bool remove_trailing_slash)
	{
		ssize_t l = static_cast<ssize_t> (strlen (buf));
		char *output = reinterpret_cast<char *> (::malloc (l + 2));
		if (nullptr == output) {
			cThrow ("Alloc failed"sv);
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
			cThrow ("strdup failed"sv);
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

	COMMON_VECTOR STR::to_argv (std::string cmd)
	{
		COMMON_VECTOR vout;

		replace (cmd, "\\\\"s, std::string (1, 0x02));
		replace (cmd, "\\\""s, std::string (1, 0x03));
		replace (cmd, "\\'"s,  std::string (1, 0x04));

		std::string what_find = "\"'";
		size_t pos = 0, last_pos;
		while (true) {
			pos = cmd.find_first_of (what_find, pos);
			if (std::string::npos == pos) {
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

		replace (cmd, std::string (1, 0x02), "\\"s);
		replace (cmd, std::string (1, 0x03), "\""s);
		replace (cmd, std::string (1, 0x04), "'"s);

		pos = 0;
		bool in_quotes = false;

		while (true) {
			last_pos = pos;
			pos = cmd.find_first_of (0x01, pos);
			if (std::string::npos == pos) {
				if (true == in_quotes) {
					if (last_pos >= 2 && (cmd.at (last_pos - 2) == ' ' || cmd.at (last_pos - 2) == '\t')) {
						vout.push_back (cmd.substr (last_pos));
					}
					else {
						vout.back ().append (cmd.substr (last_pos));
					}
				}
				else {
					if (last_pos < cmd.size () && cmd.at (last_pos) != ' ' && cmd.at (last_pos) != '\t' && vout.empty () == false) {
						vout.back ().append (1, 0x01);
					}
					split (vout, cmd.substr (last_pos), " \t"sv);
				}

				break;
			}

			if (pos > 0) {
				if (true == in_quotes) {
					if (last_pos >= 2 && (cmd.at (last_pos - 2) == ' ' || cmd.at (last_pos - 2) == '\t')) {
						vout.push_back (cmd.substr (last_pos, pos - last_pos));
					}
					else {
						vout.back ().append (cmd.substr (last_pos, pos - last_pos));
					}
				}
				else {
					if (last_pos < cmd.size () && cmd.at (last_pos) != ' ' && cmd.at (last_pos) != '\t' && vout.empty () == false) {
						vout.back ().append (1, 0x01);
					}
					split (vout, cmd.substr (last_pos, pos - last_pos), " \t"sv);
				}
			}
			else {
				vout.push_back ("");
			}

			pos++;
			in_quotes = !in_quotes;
		}

		std::string to_prefix;
		to_prefix.clear ();

		for (COMMON_VECTOR::iterator iter = vout.begin (); iter != vout.end (); ) {
			if (to_prefix.empty () == false) {
				*iter = to_prefix + *iter;
				to_prefix.clear ();
			}

			if ( (*iter).empty ()) {
				iter = vout.erase (iter);
				continue;
			}
			if (iter->back () == 0x01) {
				iter->pop_back ();
				to_prefix.assign (*iter);

				iter = vout.erase (iter);
				continue;
			}

			++iter;
		}

		if (to_prefix.empty () == false) {
			vout.push_back (to_prefix);
		}

		return vout;
	}
}

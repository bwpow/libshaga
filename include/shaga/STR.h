/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_STR
#define HEAD_shaga_STR

#include "common.h"

namespace shaga::STR {
	#define DASH std::string_view ("-", 1)
	#define SPACES std::string_view ("\n\r\t ", 4)
	#define SPACESZERO std::string_view ("\n\r\t\0 ", 5)
	#define NEWLINES std::string_view ("\n\r", 2)
	#define NEWLINESZERO std::string_view ("\n\r\0", 3)

	template <typename... Args>
	std::string sprintf (const char *format, const Args & ... args)
	{
		return fmt::sprintf (format, args...);
	}

	template <typename... Args>
	void sprintf (std::string &vout, const char *format, const Args & ... args)
	{
		vout.assign (fmt::sprintf (format, args...));
	}

	template <typename... Args>
	void sprintf (COMMON_VECTOR &vout, const char *format, const Args & ... args)
	{
		vout.push_back (fmt::sprintf (format, args...));
	}

	template <typename... Args>
	void sprintf (COMMON_LIST &vout, const char *format, const Args & ... args)
	{
		vout.push_back (fmt::sprintf (format, args...));
	}

	template <typename... Args>
	void sprintf (COMMON_DEQUE &vout, const char *format, const Args & ... args)
	{
		vout.push_back (fmt::sprintf (format, args...));
	}

	template <typename... Args>
	std::string format (const char *format, const Args & ... args)
	{
		return fmt::format (format, args...);
	}

	template <typename... Args>
	void format (std::string &vout, const char *format, const Args & ... args)
	{
		vout.assign (fmt::format (format, args...));
	}

	template <typename... Args>
	void format (COMMON_VECTOR &vout, const char *format, const Args & ... args)
	{
		vout.push_back (fmt::format (format, args...));
	}

	template <typename... Args>
	void format (COMMON_LIST &vout, const char *format, const Args & ... args)
	{
		vout.push_back (fmt::format (format, args...));
	}

	template <typename... Args>
	void format (COMMON_DEQUE &vout, const char *format, const Args & ... args)
	{
		vout.push_back (fmt::format (format, args...));
	}

	bool to_bool (const std::string_view s, const int base = 10);
	uint8_t to_uint8 (const std::string_view s, const int base = 10);
	uint16_t to_uint16 (const std::string_view s, const int base = 10);
	uint32_t to_uint32 (const std::string_view s, const int base = 10);
	uint64_t to_uint64 (const std::string_view s, const int base = 10);
	int8_t to_int8 (const std::string_view s, const int base = 10);
	int16_t to_int16 (const std::string_view s, const int base = 10);
	int32_t to_int32 (const std::string_view s, const int base = 10);
	int64_t to_int64 (const std::string_view s, const int base = 10);

	template <typename T>
	auto to_int (const std::string_view s, const int base = 10) -> T
	{
		if (std::is_same<T, bool>::value) {
			return to_bool (s, base);
		}
		else if (std::is_same<T, uint8_t>::value) {
			return to_uint8 (s, base);
		}
		else if (std::is_same<T, uint16_t>::value) {
			return to_uint16 (s, base);
		}
		else if (std::is_same<T, uint32_t>::value) {
			return to_uint32 (s, base);
		}
		else if (std::is_same<T, uint64_t>::value) {
			return to_uint64 (s, base);
		}
		else if (std::is_same<T, int8_t>::value) {
			return to_int8 (s, base);
		}
		else if (std::is_same<T, int16_t>::value) {
			return to_int16 (s, base);
		}
		else if (std::is_same<T, int32_t>::value) {
			return to_int32 (s, base);
		}
		else if (std::is_same<T, int64_t>::value) {
			return to_int64 (s, base);
		}
		else {
			cThrow ("Unknown type");
		}
	}

	template <typename T>
	std::string from_int (const T t, const int base = 10)
	{
		std::ostringstream ss;

		if (std::is_same <T, bool>::value) {
			if (t == true) {
				ss << "true";
			}
			else {
				ss << "false";
			}
		}
		else if (std::is_same <T, int8_t>::value || std::is_same <T, uint8_t>::value || std::is_same <T, int16_t>::value || std::is_same <T, uint16_t>::value) {
			ss << std::setbase (base) << static_cast<int32_t> (t);
		}
		else {
			ss << std::setbase (base) << t;
		}
		return ss.str ();
	}

	void split (const std::string_view what, const std::string_view delimiter, std::function<void (std::string_view)> callback);

	template <typename T>
	void split (T &out, const std::string_view what, const std::string_view delimiter)
	{
		split (what, delimiter, [&out](std::string_view tok) -> void { out.emplace_back (tok); });
	}

	template <typename T>
	T split (const std::string_view what, const std::string_view delimiter)
	{
		T out;
		split (what, delimiter, [&out](std::string_view tok) -> void { out.emplace_back (tok); });
		return out;
	}

	template <typename T>
	void join (std::string &out, const T &input, const std::string_view delimiter)
	{
		bool first = true;
		for (const auto &entry : input) {
			if (false == first) {
				out.append (delimiter);
			}
			else {
				first = false;
			}
			out.append (entry);
		}
	}

	template <typename T>
	std::string join (const T &input, const std::string_view delimiter)
	{
		std::string out;
		join (out, input, delimiter);
		return out;
	}

	template <typename T>
	void join (std::string &out, const T &input, const std::string_view prefix, const std::string_view suffix)
	{
		for (const auto &entry : input) {
			out.append (prefix);
			out.append (entry);
			out.append (suffix);
		}
	}

	template <typename T>
	std::string join (const T &input, const std::string_view prefix, const std::string_view suffix)
	{
		std::string out;
		join (out, input, prefix, suffix);
		return out;
	}

	void format_time (std::string &out, const time_t theTime, const bool local, const bool for_filename = false);
	std::string format_time (const time_t theTime, const bool local, const bool for_filename = false);
	std::string format_time (const bool local, const bool for_filename = false);

	void format_date (std::string &out, const time_t theTime, const bool local, const std::string_view separatorstr = DASH);
	std::string format_date (const time_t theTime, const bool local, const std::string_view separatorstr = DASH);
	std::string format_date (const bool local, const std::string_view separatorstr = DASH);

	void replace (std::string &str, const std::string_view what, const std::string_view with);

	bool has_suffix (const std::string_view str, const std::string_view suffix);
	bool has_isuffix (const std::string_view str, const std::string_view suffix);

	bool has_prefix (const std::string_view str, const std::string_view prefix);
	bool has_iprefix (const std::string_view str, const std::string_view prefix);

	void make_printable (std::string &text, const unsigned char replace_with = '.');
	std::string get_printable (const std::string_view text, const unsigned char replace_with = '.');

	void rtrim (std::string &str, const std::string_view chars = SPACESZERO);
	void rtrim (std::string_view &str, const std::string_view chars = SPACESZERO);

	template <typename T>
	void rtrim (T &input, const std::string_view chars = SPACESZERO)
	{
		for (auto &instance : input) {
			ltrim (instance, chars);
		}
	}

	void ltrim (std::string &str, const std::string_view chars = SPACESZERO);
	void ltrim (std::string_view &str, const std::string_view chars = SPACESZERO);

	template <typename T>
	void ltrim (T &input, const std::string_view chars = SPACESZERO)
	{
		for (auto &instance : input) {
			ltrim (instance, chars);
		}
	}

	void trim (std::string &str, const std::string_view chars = SPACESZERO);
	void trim (std::string_view &str, const std::string_view chars = SPACESZERO);

	template <typename T>
	void trim (T &input, const std::string_view chars = SPACESZERO)
	{
		for (auto &instance : input) {
			trim (instance, chars);
		}
	}

	bool icompare (const std::string_view a, const std::string_view b);

	std::string multiply (std::string_view what, const uint_fast32_t cnt);
	std::string multiply (std::string_view what, const uint_fast32_t cnt, const std::string_view postfix);

	char * dirdup (const char *buf, const bool remove_trailing_slash = true);
	char * strdup (const char *buf);

	std::string from_argv (const COMMON_VECTOR &v, const char separator = ' ');
	std::string from_argv (const int argc, const char * const * argv, const char separator = ' ');
	COMMON_VECTOR to_argv (const int argc, const char * const * argv);
	COMMON_VECTOR to_argv (std::string cmd);
}

#endif // HEAD_shaga_STR

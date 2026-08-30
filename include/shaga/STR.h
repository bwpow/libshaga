/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#pragma once

#include "common.h"

namespace shaga::STR {
	#define DASH std::string_view ("-", 1)
	#define SPACES std::string_view ("\n\r\t ", 4)
	#define SPACESZERO std::string_view ("\n\r\t\0 ", 5)
	#define NEWLINES std::string_view ("\n\r", 2)
	#define NEWLINESZERO std::string_view ("\n\r\0", 3)

	template <typename... Args, typename = std::enable_if_t<all_convertible_to_string_view<Args...>::value>>
	std::string concat (const Args & ... args)
	{
		const size_t len = (std::string_view (args).size () + ... + 0);

		std::string out;
		out.reserve (len);
		(out.append (std::string_view (args)), ...);

		return out;
	}

	template <typename... Args>
	std::string format_concat (const Args & ... args)
	{
		const std::array<std::string_view,sizeof...(Args)> vin {args...};
		return fmt::to_string (fmt::join (vin, ""sv));
	}

	template <typename... Args>
	std::string format_concat_delimiter (const std::string_view delimiter, const Args & ... args)
	{
		const std::array<std::string_view,sizeof...(Args)> vin {args...};
		return fmt::to_string (fmt::join (vin, delimiter));
	}

	template <typename... Args>
	void sprint (std::string &append_to, const std::string_view format, const Args & ... args)
	{
		const size_t sze = fmt::formatted_size (format, args...);
		append_to.reserve (append_to.size () + sze);
		fmt::format_to (std::back_inserter(append_to), format, args...);
	}

	template <typename... Args>
	std::string sprint (const std::string_view format, const Args & ... args)
	{
		return fmt::format (format, args...);
	}

	template <typename... Args>
	void sprint (COMMON_VECTOR &vout, const std::string_view format, const Args & ... args)
	{
		vout.push_back (fmt::format (format, args...));
	}

	template <typename... Args>
	void sprint (COMMON_LIST &vout, const std::string_view format, const Args & ... args)
	{
		vout.push_back (fmt::format (format, args...));
	}

	template <typename... Args>
	void sprint (COMMON_DEQUE &vout, const std::string_view format, const Args & ... args)
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

	float to_float (const std::string_view s);
	double to_double (const std::string_view s);
	long double to_long_double (const std::string_view s);

	std::optional<bool> to_bool_optional (const std::string_view s, const int base = 10) noexcept;
	std::optional<uint8_t> to_uint8_optional (const std::string_view s, const int base = 10) noexcept;
	std::optional<uint16_t> to_uint16_optional (const std::string_view s, const int base = 10) noexcept;
	std::optional<uint32_t> to_uint32_optional (const std::string_view s, const int base = 10) noexcept;
	std::optional<uint64_t> to_uint64_optional (const std::string_view s, const int base = 10) noexcept;
	std::optional<int8_t> to_int8_optional (const std::string_view s, const int base = 10) noexcept;
	std::optional<int16_t> to_int16_optional (const std::string_view s, const int base = 10) noexcept;
	std::optional<int32_t> to_int32_optional (const std::string_view s, const int base = 10) noexcept;
	std::optional<int64_t> to_int64_optional (const std::string_view s, const int base = 10) noexcept;

	std::optional<float> to_float_optional (const std::string_view s) noexcept;
	std::optional<double> to_double_optional (const std::string_view s) noexcept;
	std::optional<long double> to_long_double_optional (const std::string_view s) noexcept;

	template <typename T, SHAGA_TYPE_IS_INTEGRAL(T)>
	auto to_int (const std::string_view s, const int base = 10) -> T
	{
		if constexpr (std::is_same<T, bool>::value) {
			return to_bool (s, base);
		}
		else if constexpr (std::is_same<T, uint8_t>::value) {
			return to_uint8 (s, base);
		}
		else if constexpr (std::is_same<T, uint16_t>::value) {
			return to_uint16 (s, base);
		}
		else if constexpr (std::is_same<T, uint32_t>::value) {
			return to_uint32 (s, base);
		}
		else if constexpr (std::is_same<T, int8_t>::value) {
			return to_int8 (s, base);
		}
		else if constexpr (std::is_same<T, int16_t>::value) {
			return to_int16 (s, base);
		}
		else if constexpr (std::is_same<T, int32_t>::value) {
			return to_int32 (s, base);
		}
		else if constexpr (std::is_same<T, int64_t>::value) {
			return to_int64 (s, base);
		}
		else {
			return to_uint64 (s, base);
		}
	}

	template <typename T, SHAGA_TYPE_IS_INTEGRAL(T)>
	auto to_int_optional (const std::string_view s, const int base = 10) noexcept -> std::optional<T>
	{
		try {
			return to_int<T> (s, base);
		}
		catch (...) {
			return std::nullopt;
		}
	}

	template <typename T, SHAGA_TYPE_IS_INTEGRAL(T)>
	std::string from_int (const T t, [[maybe_unused]]const int base = 10)
	{
		if constexpr (std::is_same_v<T, bool>) {
			return t ? "true"s : "false"s;
		}
		else {
			char buf[std::numeric_limits<T>::digits + 3];
			const int supported_base = (base == 8 || base == 16) ? base : 10;

			if constexpr (sizeof (T) < sizeof (int)) {
				const auto res = std::to_chars (buf, buf + sizeof (buf), static_cast<int> (t), supported_base);
				if (res.ec != std::errc ()) {
					throw std::runtime_error ("Could not convert integer to string");
				}
				return std::string (buf, res.ptr);
			}
			else {
				const auto res = std::to_chars (buf, buf + sizeof (buf), t, supported_base);
				if (res.ec != std::errc ()) {
					throw std::runtime_error ("Could not convert integer to string");
				}
				return std::string (buf, res.ptr);
			}
		}
	}

	void split (const std::string_view what, const std::string_view delimiter, std::function<void (std::string_view)> callback);

	template <typename T, SHAGA_TYPE_IS_ITERABLE(T)>
	void split (T &out, const std::string_view what, const std::string_view delimiter)
	{
		split (what, delimiter, [&out](std::string_view tok) -> void { out.emplace_back (tok); });
	}

	template <typename T, SHAGA_TYPE_IS_ITERABLE(T)>
	auto split (const std::string_view what, const std::string_view delimiter) -> T
	{
		T out;
		split (what, delimiter, [&out](std::string_view tok) -> void { out.emplace_back (tok); });
		return out;
	}

	template <typename T, SHAGA_TYPE_IS_ITERABLE(T)>
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
			out.append (fmt::to_string (entry));
		}
	}

	template <typename T, SHAGA_TYPE_IS_ITERABLE(T)>
	std::string join (const T &input, const std::string_view delimiter)
	{
		std::string out;
		join (out, input, delimiter);
		return out;
	}

	template <typename T, SHAGA_TYPE_IS_ITERABLE(T)>
	void join (std::string &out, const T &input, const std::string_view prefix, const std::string_view suffix)
	{
		for (const auto &entry : input) {
			out.append (prefix);
			out.append (entry);
			out.append (suffix);
		}
	}

	template <typename T, SHAGA_TYPE_IS_ITERABLE(T)>
	std::string join (const T &input, const std::string_view prefix, const std::string_view suffix)
	{
		std::string out;
		join (out, input, prefix, suffix);
		return out;
	}

	/* Note about WIN32 version of format_time: timezone is never added */
	void format_time (std::string &out, const time_t theTime, const bool local, const bool for_filename = false, const bool add_tz = true);
	void format_time (std::string &out, const bool local, const bool for_filename = false, const bool add_tz = true);
	std::string format_time (const time_t theTime, const bool local, const bool for_filename = false, const bool add_tz = true);
	std::string format_time (const bool local, const bool for_filename = false, const bool add_tz = true);

	void format_date (std::string &out, const time_t theTime, const bool local, const std::string_view separatorstr = DASH);
	void format_date (std::string &out, const bool local, const std::string_view separatorstr = DASH);
	std::string format_date (const time_t theTime, const bool local, const std::string_view separatorstr = DASH);
	std::string format_date (const bool local, const std::string_view separatorstr = DASH);

	void replace (std::string &str, const std::string_view what, const std::string_view with);

	bool has_suffix (const std::string_view str, const std::string_view suffix) noexcept;
	bool has_isuffix (const std::string_view str, const std::string_view suffix) noexcept;

	bool has_prefix (const std::string_view str, const std::string_view prefix) noexcept;
	bool has_iprefix (const std::string_view str, const std::string_view prefix) noexcept;

	void make_printable (std::string &text, const unsigned char replace_with = '.');
	std::string get_printable (const std::string_view text, const unsigned char replace_with = '.');

	void rtrim (std::string &str, const std::string_view chars = SPACESZERO);
	void rtrim (std::string_view &str, const std::string_view chars = SPACESZERO) noexcept;

	template <typename T, SHAGA_TYPE_IS_ITERABLE(T)>
	void rtrim (T &input, const std::string_view chars = SPACESZERO)
	{
		for (auto &instance : input) {
			ltrim (instance, chars);
		}
	}

	void ltrim (std::string &str, const std::string_view chars = SPACESZERO);
	void ltrim (std::string_view &str, const std::string_view chars = SPACESZERO) noexcept;

	template <typename T, SHAGA_TYPE_IS_ITERABLE(T)>
	void ltrim (T &input, const std::string_view chars = SPACESZERO)
	{
		for (auto &instance : input) {
			ltrim (instance, chars);
		}
	}

	void trim (std::string &str, const std::string_view chars = SPACESZERO);
	void trim (std::string_view &str, const std::string_view chars = SPACESZERO) noexcept;

	template <typename T, SHAGA_TYPE_IS_ITERABLE(T)>
	void trim (T &input, const std::string_view chars = SPACESZERO)
	{
		for (auto &instance : input) {
			trim (instance, chars);
		}
	}

	bool icompare (const std::string_view a, const std::string_view b) noexcept;

	std::string multiply (const std::string_view what, const uint_fast32_t cnt);
	std::string multiply (const std::string_view what, const uint_fast32_t cnt, const std::string_view suffix);

	char * dirdup (const char *const buf, const bool remove_trailing_slash = true);
	char * strdup (const char *const buf);

	std::string from_argv (const COMMON_VECTOR &v, const char separator = ' ');
	std::string from_argv (const int argc, const char * const * argv, const char separator = ' ');
	COMMON_VECTOR to_argv (const int argc, const char * const * argv);
	COMMON_VECTOR to_argv (std::string cmd);
}

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_STR
#define HEAD_shaga_STR

#include "common.h"

namespace shaga {
	namespace STR {

		#define SPACES std::string ("\n\r\t ", 4)
		#define SPACESZERO std::string ("\n\r\t\0 ", 5)
		#define NEWLINES std::string ("\n\r", 2)
		#define NEWLINESZERO std::string ("\n\r\0", 3)

		void sprintf (std::string &str, const char *fmt, va_list &ap);
		std::string sprintf (const char *fmt, va_list &ap);
		void sprintf (std::string &str, const char *fmt, ...) ;
		std::string sprintf (const char *fmt, ...) ;
		void sprintf (COMMON_VECTOR &v, const char *fmt, ...) ;
		void sprintf (COMMON_LIST &v, const char *fmt, ...) ;

		bool to_bool (const std::string &s, const int base = 10);
		uint8_t to_uint8 (const std::string &s, const int base = 10);
		uint16_t to_uint16 (const std::string &s, const int base = 10);
		uint32_t to_uint32 (const std::string &s, const int base = 10);
		uint64_t to_uint64 (const std::string &s, const int base = 10);
		int8_t to_int8 (const std::string &s, const int base = 10);
		int16_t to_int16 (const std::string &s, const int base = 10);
		int32_t to_int32 (const std::string &s, const int base = 10);
		int64_t to_int64 (const std::string &s, const int base = 10);

		template <typename T>
		std::string from_int (const T &t, const int base = 10)
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

		void split (const std::string &what, const std::string &delimiter, std::function<void (const std::string &)> f);

		template <typename T>
		void split (T &out, const std::string &what, const std::string &delimiter)
		{
			split (what, delimiter, [&out](const std::string &tok) { out.push_back (tok); });
		}

		COMMON_VECTOR split_to_vector (const std::string &what, const std::string &delimiter);
		COMMON_DEQUE split_to_deque (const std::string &what, const std::string &delimiter);
		COMMON_LIST split_to_list (const std::string &what, const std::string &delimiter);

		template <typename T>
		void join (std::string &out, const T &input, const std::string &delimiter)
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
		std::string join (const T &input, const std::string &delimiter)
		{
			std::string out;
			join (out, input, delimiter);
			return out;
		}

		template <typename T>
		void join (std::string &out, const T &input, const std::string &prefix, const std::string &suffix)
		{
			for (const auto &entry : input) {
				out.append (prefix);
				out.append (entry);
				out.append (suffix);
			}
		}

		template <typename T>
		std::string join (const T &input, const std::string &prefix, const std::string &suffix)
		{
			std::string out;
			join (out, input, prefix, suffix);
			return out;
		}

		void format_time (std::string &out, const time_t theTime, const bool local, const bool for_filename = false);
		std::string format_time (const time_t theTime, const bool local, const bool for_filename = false);
		std::string format_time (const bool local, const bool for_filename = false);

		void format_date (std::string &out, const time_t theTime, const bool local, const char *separatorstr = "-");
		std::string format_date (const time_t theTime, const bool local, const char *separatorstr = "-");
		std::string format_date (const bool local, const char *separatorstr = "-");

		void replace (std::string &str, const std::string &what, const std::string &with);

		bool has_suffix (const std::string &str, const std::string &suffix);
		bool has_isuffix (const std::string &str, const std::string &suffix);

		bool has_prefix (const std::string &str, const std::string &prefix);
		bool has_iprefix (const std::string &str, const std::string &prefix);

		void make_printable (std::string &text, const unsigned char replace_with = '.');
		std::string get_printable (const std::string &text, const unsigned char replace_with = '.');

		void rtrim (std::string &str, const std::string &chars = SPACESZERO);

		template <typename T>
		void rtrim (T &input, const std::string &chars = SPACESZERO)
		{
			for (auto &instance : input) {
				ltrim (instance, chars);
			}
		}

		void ltrim (std::string &str, const std::string &chars = SPACESZERO);

		template <typename T>
		void ltrim (T &input, const std::string &chars = SPACESZERO)
		{
			for (auto &instance : input) {
				ltrim (instance, chars);
			}
		}

		void trim (std::string &str, const std::string &chars = SPACESZERO);

		template <typename T>
		void trim (T &input, const std::string &chars = SPACESZERO)
		{
			for (auto &instance : input) {
				trim (instance, chars);
			}
		}

		bool icompare (const std::string &a, const std::string &b);

		char * dirdup (const char *buf, const bool remove_trailing_slash = true);
		char * strdup (const char *buf);

		std::string from_argv (const COMMON_VECTOR &v, const char separator = ' ');
		std::string from_argv (const int argc, const char * const * argv, const char separator = ' ');
		COMMON_VECTOR to_argv (const int argc, const char * const * argv);
		COMMON_VECTOR to_argv (const std::string &vars);
	}
}

#endif // HEAD_shaga_STR

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2024, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_INI
#define HEAD_shaga_INI

#include "common.h"

namespace shaga {
	class INI {
		private:
			typedef struct {
				std::string section;
				std::string line;
			} INI_KEY;

			friend bool operator< (const INI_KEY &a, const INI_KEY &b);
			friend bool operator== (const INI_KEY &a, const INI_KEY &b);

			typedef std::map<INI_KEY, COMMON_LIST> INI_MAP;

			INI_MAP _map;
			COMMON_DEQUE _nested_parse_file;
			const size_t _max_nested_files {8};

			std::string get_last_nested_realpath (void) const;

			INI_KEY get_key (const std::string_view section, const std::string_view line) const;
			COMMON_LIST & get_list_ref (INI_MAP &ini_map, const INI_KEY &key, const bool create) const;
			const COMMON_LIST & get_list_ref (const INI_MAP &ini_map, const INI_KEY &key) const;
			COMMON_LIST get_list_copy (const INI_MAP &ini_map, const INI_KEY &key) const;
			INI_MAP::iterator begin_of_section (INI_MAP &ini_map, const std::string_view section) const;
			bool is_same_section (const INI_KEY &key, const std::string_view section) const;

			void parse_line (INI_MAP &ini_map, const std::string_view line, std::string &active_section, const bool allow_include);

			void parse_file (INI_MAP &ini_map, const std::string_view fname, const bool allow_include, const bool allow_missing);
			void parse_file (const std::string_view fname, const bool allow_include, const bool allow_missing);

			void parse_buffer (INI_MAP &ini_map, const std::string_view buf, const bool allow_include);
			void parse_buffer (const std::string_view buf, const bool allow_include);

			SHAGA_STRV std::optional<std::string_view> get_last_value (const INI_MAP &m, const INI_KEY &key) const;

		public:
			INI ();
			INI (const INI &other);
			INI (INI &&other);

			INI& operator= (const INI &other);
			INI& operator= (INI &&other);

			INI (const std::string_view fname, const bool allow_include = false);

			void reset (void);

			void load_file (const std::string_view fname, const bool append = false, const bool allow_include = false);
			void save_to_file (ShFile &file) const;
			void save_to_file (const std::string_view fname) const;

			void load_buffer (const std::string_view buf, const bool append = false, const bool allow_include = false);
			void save_to_buffer (std::string &out) const;
			std::string save_to_buffer (void) const;

			COMMON_LIST get_section_list (void) const;

			const COMMON_VECTOR get_vector (const std::string_view section, const std::string_view key) const;
			size_t get_vector_size (const std::string_view section, const std::string_view key, const bool thr = false) const;

			const COMMON_LIST & get_list (const std::string_view section, const std::string_view key, const COMMON_LIST &defvalue, const bool thr = false) const;
			const COMMON_LIST get_list (const std::string_view section, const std::string_view key) const;
			size_t get_list_size (const std::string_view section, const std::string_view key, const bool thr = false) const;

			template <typename T, SHAGA_TYPE_IS_INTEGRAL(T)>
			auto get_int (const std::string_view section, const std::string_view key, const T defvalue, const bool thr = false, const int base = 10) const -> T
			{
				if (auto val = get_last_value (_map, get_key (section, key))) {
					return STR::to_int<T> (*val, base);
				}

				if (true == thr) {
					cThrow ("Nonexistent entry '[{}]:{}' requested"sv, section, key);
				}

				return defvalue;
			}

			/* Please note: Use string_view only when you don't use any set_* methods. */
			template <typename T = std::string_view, SHAGA_TYPE_IS_CLASS(T)>
			SHAGA_STRV auto get_string (const std::string_view section, const std::string_view key, const std::string_view defvalue, const bool thr = false) const -> T
			{
				if (auto val = get_last_value (_map, get_key (section, key))) {
					return T (*val);
				}

				if (true == thr) {
					cThrow ("Nonexistent entry '[{}]:{}' requested"sv, section, key);
				}

				return T (defvalue);
			}

			template <typename T = std::string_view, SHAGA_TYPE_IS_CLASS(T)>
			SHAGA_STRV auto get_string (const std::string_view section, const std::string_view key) const -> T
			{
				if (auto val = get_last_value (_map, get_key (section, key))) {
					return T (*val);
				}
				return T(""sv);
			}

			float get_float (const std::string_view section, const std::string_view key, const float defvalue, const bool thr = false) const;
			double get_double (const std::string_view section, const std::string_view key, const double defvalue, const bool thr = false) const;
			long double get_long_double (const std::string_view section, const std::string_view key, const long double defvalue, const bool thr = false) const;

			bool get_bool (const std::string_view section, const std::string_view key, const bool defvalue, const bool thr = false) const;
			uint8_t get_uint8 (const std::string_view section, const std::string_view key, const uint8_t defvalue, const bool thr = false, const int base = 10) const;
			uint16_t get_uint16 (const std::string_view section, const std::string_view key, const uint16_t defvalue, const bool thr = false, const int base = 10) const;
			uint32_t get_uint32 (const std::string_view section, const std::string_view key, const uint32_t defvalue, const bool thr = false, const int base = 10) const;
			uint64_t get_uint64 (const std::string_view section, const std::string_view key, const uint64_t defvalue, const bool thr = false, const int base = 10) const;
			int8_t get_int8 (const std::string_view section, const std::string_view key, const int8_t defvalue, const bool thr = false, const int base = 10) const;
			int16_t get_int16 (const std::string_view section, const std::string_view key, const int16_t defvalue, const bool thr = false, const int base = 10) const;
			int32_t get_int32 (const std::string_view section, const std::string_view key, const int32_t defvalue, const bool thr = false, const int base = 10) const;
			int64_t get_int64 (const std::string_view section, const std::string_view key, const int64_t defvalue, const bool thr = false, const int base = 10) const;

			template <typename T, SHAGA_TYPE_IS_INTEGRAL(T)>
			void set_int (const std::string_view section, const std::string_view key, const T val, const bool append = false, const int base = 10)
			{
				set_string (section, key, STR::from_int (val, base), append);
			}

			void set_string (const std::string_view section, const std::string_view key, const std::string_view val, const bool append = false);
			void set_vector (const std::string_view section, const std::string_view key, const COMMON_VECTOR &val, const bool append);
			void set_list (const std::string_view section, const std::string_view key, const COMMON_LIST &val, const bool append);

			template <typename... Args>
			void set_format (const std::string_view section, const std::string_view key, const std::string_view format, const Args & ... args)
			{
				set_string (section, key, fmt::format (format, args...), false);
			}

			template <typename... Args>
			void set_format_append (const std::string_view section, const std::string_view key, const std::string_view format, const Args & ... args)
			{
				set_string (section, key, fmt::format (format, args...), true);
			}
	};
}

#endif // HEAD_shaga_INI

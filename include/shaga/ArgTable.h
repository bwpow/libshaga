/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_ArgTable
#define HEAD_shaga_ArtTable

#include "common.h"

namespace shaga {

	class ArgTable {
		public:
			enum class INCIDENCE { ANY, ZERO_OR_ONE, ONE, AT_LEAST_ONE };

		private:

			struct Entry {
				std::string key_long;
				char key_short;
				INCIDENCE incidence;
				std::string help;
				std::function <bool (const std::string &)> checker;
				bool has_param;
				std::string param_type;

				COMMON_VECTOR vars;

				std::string get_str (void) const;
			};

			typedef std::vector <Entry> Entries;

			Entries _entries;
			Entries::iterator _actual_entry;
			bool _next_entry_is_param;
			std::string _argv0;

			Entries::iterator find_entry_by_key (const std::string &key_long, const char key_short);
			void process_entry (Entry &e, const std::string &var);
			void process_entry (const std::string &data);

		public:
			ArgTable ();
			ArgTable (const std::string &argv0);

			void set_argv0 (const std::string &str);
			std::string get_usage_string (void) const;
			COMMON_COMMON_VECTOR get_help_vector (void) const;
			std::string get_help_string (void) const;
			void print_usage (void) const;
			friend std::ostream& operator<< (std::ostream& stream, const ArgTable& t);

			ArgTable & add (const std::string &key_long, const char key_short, const ArgTable::INCIDENCE incidence, const bool has_param, const std::string &help, const std::string &param_type = "", std::function <bool (const std::string &)> checker = nullptr);
			ArgTable & add (const std::string &key_long, const ArgTable::INCIDENCE incidence, const bool has_param, const std::string &help, const std::string &param_type = "", std::function <bool (const std::string &)> checker = nullptr);
			ArgTable & add (const char key_short, const ArgTable::INCIDENCE incidence, const bool has_param, const std::string &help, const std::string &param_type = "", std::function <bool (const std::string &)> checker = nullptr);

			bool process (const COMMON_VECTOR &v, const bool thr = false);
			bool process (const COMMON_VECTOR &v, std::string &error);

			INI export_ini (const std::string &section) const;
			INI export_ini (void) const;
	};

}

#endif // HEAD_shaga_ArgTable

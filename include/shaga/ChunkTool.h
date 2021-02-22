/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2021, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_ChunkTool
#define HEAD_shaga_ChunkTool

#include "common.h"

namespace shaga
{
	typedef std::list<Chunk> CHUNKLIST;
	typedef std::multiset<Chunk> CHUNKSET;

	class ChunkTool
	{
		private:
			const Chunk::SPECIAL_TYPES *const _special_types {nullptr};

			bool _enable_thr {true};
			bool _store_binary {false};

			std::string _temp_str;
			std::string _out_str;

		public:
			ChunkTool (const bool enable_thr = true, const Chunk::SPECIAL_TYPES *const special_types = nullptr);
			ChunkTool (const Chunk::SPECIAL_TYPES *const special_types);

			void set_enable_thr (const bool enable_thr);
			void set_store_binary (const bool store_binary);

			/*** From binary string ***/
			void from_bin (const std::string_view buf, size_t &offset, CHUNKLIST &out_append) const;
			void from_bin (const std::string_view buf, size_t &offset, CHUNKSET &out_append) const;

			template <class T>
			void from_bin (const std::string_view buf, T &out_append) const
			{
				size_t offset = 0;
				from_bin (buf, offset, out_append);
			}

			template <class T>
			T from_bin (const std::string_view buf, size_t &offset) const
			{
				T out;
				from_bin (buf, offset, out);
				return out;
			}

			template <class T>
			T from_bin (const std::string_view buf) const
			{
				T out;
				size_t offset = 0;
				from_bin (buf, offset, out);
				return out;
			}

			/*** To binary string ***/
			/* All entries that are converted to binary are erased from the list/set */
			void to_bin (CHUNKLIST &lst_erase, std::string &out_append, const size_t max_size = 0, const Chunk::Priority max_priority = Chunk::Priority::pDEBUG, const bool erase_skipped = false);
			std::string to_bin (CHUNKLIST &lst_erase, const size_t max_size = 0, const Chunk::Priority max_priority = Chunk::Priority::pDEBUG, const bool erase_skipped = false);

			void to_bin (CHUNKSET &cs_erase, std::string &out_append, const size_t max_size = 0, const Chunk::Priority max_priority = Chunk::Priority::pDEBUG);
			std::string to_bin (CHUNKSET &cs_erase, const size_t max_size = 0, const Chunk::Priority max_priority = Chunk::Priority::pDEBUG);

			/*** Tools ***/
			static void change_source_hwid (CHUNKLIST &lst, const HWID new_source_hwid, const bool replace_only_zero);
			static void trim (CHUNKSET &cset, const size_t treshold_size, const Chunk::Priority treshold_prio);

			/* Purge entry if callback returns false */
			template <class T>
			static void purge (T &lst, std::function<bool(const Chunk &)> callback)
			{
				if (nullptr == callback) {
					cThrow ("Callback function is not defined"sv);
				}

				for (auto iter = lst.begin (); iter != lst.end ();) {
					if (callback (*iter) == false) {
						iter = lst.erase (iter);
					}
					else {
						++iter;
					}
				}
			}
	};

	/*** TTL ***/
	static const Chunk::TTL TTL_local {Chunk::TTL::TTL1};
	static const Chunk::TTL TTL_wide {Chunk::_TTL_last};

	Chunk::TTL uint8_to_ttl (const uint8_t v);
	uint8_t ttl_to_uint8 (const Chunk::TTL v);

	/*** TrustLevel ***/
	Chunk::TrustLevel operator++ (Chunk::TrustLevel &x);
	Chunk::TrustLevel operator++ (Chunk::TrustLevel &x, int r);
	Chunk::TrustLevel operator* (Chunk::TrustLevel c);
	Chunk::TrustLevel begin (Chunk::TrustLevel r);
	Chunk::TrustLevel end (Chunk::TrustLevel r);
	Chunk::TrustLevel uint8_to_trustlevel (const uint8_t v);
	uint8_t trustlevel_to_uint8 (const Chunk::TrustLevel v);
	SHAGA_STRV std::string_view trustlevel_to_string (const Chunk::TrustLevel level);
	Chunk::TrustLevel string_to_trustlevel (const std::string_view str);

	/*** Priority ***/
	Chunk::Priority uint8_to_priority (const uint8_t v);
	uint8_t priority_to_uint8 (const Chunk::Priority v);
	SHAGA_STRV std::string_view priority_to_string (const Chunk::Priority prio);

	/*** Channel ***/
	Chunk::Channel bool_to_channel (const bool val);
	bool channel_to_bool (const Chunk::Channel channel);
	SHAGA_STRV std::string_view channel_to_string (const Chunk::Channel channel);
}

#endif // HEAD_shaga_ChunkTool

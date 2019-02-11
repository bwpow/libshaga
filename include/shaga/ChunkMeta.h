/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_ChunkMeta
#define HEAD_shaga_ChunkMeta

#include "common.h"

namespace shaga {
	/* ChunkMeta is key-value structure with repeating key allowed.	Every key has to be three capital letters ('A' to 'Z'). */

	class Chunk;

	typedef std::unordered_multimap <uint_fast16_t, std::string> ChunkMetaData;
	typedef std::set <std::pair<uint_fast16_t, std::string>> ChunkMetaDataSet;
	typedef std::pair <ChunkMetaData::const_iterator, ChunkMetaData::const_iterator> ChunkMetaDataRange;

	static constexpr uint16_t _chunkmeta_key_to_bin_helper (const char str[4], const size_t pos)
	{
		return (pos < 3) ?
				((_chunkmeta_key_to_bin_helper (str, pos + 1) * 27) + (str[pos] == '_' ? (26) : (str[pos] - 'A')))
			:
				0;
	}

	static constexpr uint16_t _chunkmeta_key_to_bin (const char str[4])
	{
		return ((str[0] >= 'A' && str[0] <= 'Z') || str[0] == '_') &&
			((str[1] >= 'A' && str[1] <= 'Z') || str[1] == '_') &&
			((str[2] >= 'A' && str[2] <= 'Z') || str[2] == '_') &&
			str[3] == '\0' ?
				(_chunkmeta_key_to_bin_helper (str, 0) + 1)
			:
				0;
	}

	#define ChMetaKEY(a) shaga::_chunkmeta_key_to_bin(a)

	class ChunkMeta {
		public:
			static const constexpr uint16_t key_type_min = ChMetaKEY ("AAA");
			static const constexpr uint16_t key_type_max = ChMetaKEY ("___");

			static const uint16_t key_type_mask   {0b0111'1111'1111'1111};
			static const uint16_t key_repeat_mask {0b0111'1111'0000'0000};
			static const uint8_t key_repeat_byte  {0b0111'1111};

			static uint16_t key_to_bin (const std::string &key);
			static std::string bin_to_key (const uint16_t bkey);

		private:
			ChunkMetaData _data;

			bool should_continue (const std::string &s, const size_t offset) const;
		public:
			ChunkMeta ();
			ChunkMeta (const uint16_t key);
			ChunkMeta (const uint16_t key, const std::string &value);
			ChunkMeta (const uint16_t key, std::string &&value);
			ChunkMeta (const std::string &key);
			ChunkMeta (const std::string &key, const std::string &value);
			ChunkMeta (const std::string &key, std::string &&value);

			ChunkMetaData::const_iterator begin () const;
			ChunkMetaData::const_iterator end () const;

			ChunkMetaData::const_iterator cbegin () const;
			ChunkMetaData::const_iterator cend () const;

			ChunkMetaData::const_iterator find (const std::string &key) const;
			ChunkMetaData::const_iterator find (const uint16_t key) const;

			void clear (void);
			void reset (void);

			void add_value (const uint16_t key);
			void add_value (const std::string &key);

			void add_value (const uint16_t key, const std::string &value);
			void add_value (const std::string &key, const std::string &value);

			void add_value (const uint16_t key, std::string &&value);
			void add_value (const std::string &key, std::string &&value);

			size_t size (void) const;
			size_t count (const std::string &key) const;
			size_t count (const uint16_t key) const;
			bool empty (void) const;

			size_t erase (const uint16_t key);
			size_t erase (const std::string &key);

			void unique (void);

			void merge (const ChunkMeta &other);
			void merge (ChunkMeta &&other);

			ChunkMetaDataRange equal_range (const std::string &key) const;
			ChunkMetaDataRange equal_range (const uint16_t key) const;

			COMMON_LIST get_values (const std::string &key) const;
			COMMON_LIST get_values (const uint16_t key) const;

			/* Callback returning true will continue to the next entry, false will end */
			void modify_values (const std::string &key, std::function<bool(std::string&)> callback);
			void modify_values (const uint16_t key, std::function<bool(std::string&)> callback);

			std::string get_value (const std::string &key) const;
			std::string get_value (const uint16_t key) const;

			void modify_value (const std::string &key, std::function<void(std::string&)> callback);
			void modify_value (const uint16_t key, std::function<void(std::string&)> callback);

			COMMON_LIST get_keys (void) const;

			void to_bin (std::string &s) const;
			std::string to_bin (void) const;

			void from_bin (const std::string &s, size_t &offset);
	};

}

#endif // HEAD_shaga_ChunkMeta

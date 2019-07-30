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
			typedef std::function<bool(std::string&)> ValuesCallback;
			typedef std::function<void(std::string&)> ValueCallback;

			static const constexpr uint16_t key_type_min = ChMetaKEY ("AAA");
			static const constexpr uint16_t key_type_max = ChMetaKEY ("___");

			static const uint16_t key_type_mask   {0b0111'1111'1111'1111};
			static const uint16_t key_repeat_mask {0b0111'1111'0000'0000};
			static const uint8_t key_repeat_byte  {0b0111'1111};

			static uint16_t key_to_bin (const std::string_view key);
			static std::string bin_to_key (const uint16_t bkey);

		private:
			ChunkMetaData _data;

			bool should_continue (const std::string_view s, const size_t offset) const;
		public:
			ChunkMeta ();
			ChunkMeta (const uint16_t key);
			ChunkMeta (const uint16_t key, const std::string_view value);
			ChunkMeta (const uint16_t key, std::string &&value);
			ChunkMeta (const std::string_view key);
			ChunkMeta (const std::string_view key, const std::string_view value);
			ChunkMeta (const std::string_view key, std::string &&value);

			ChunkMetaData::const_iterator begin () const;
			ChunkMetaData::const_iterator end () const;

			ChunkMetaData::const_iterator cbegin () const;
			ChunkMetaData::const_iterator cend () const;

			ChunkMetaData::const_iterator find (const std::string_view key) const;
			ChunkMetaData::const_iterator find (const uint16_t key) const;

			void clear (void);
			void reset (void);

			void add_value (const uint16_t key);
			void add_value (const std::string_view key);

			void add_value (const uint16_t key, const std::string_view value);
			void add_value (const std::string_view key, const std::string_view value);

			void add_value (const uint16_t key, std::string &&value);
			void add_value (const std::string_view key, std::string &&value);

			size_t size (void) const;
			size_t count (const std::string_view key) const;
			size_t count (const uint16_t key) const;
			bool empty (void) const;

			size_t erase (const uint16_t key);
			size_t erase (const std::string_view key);

			void unique (void);

			void merge (const ChunkMeta &other);
			void merge (ChunkMeta &&other);

			ChunkMetaDataRange equal_range (const std::string_view key) const;
			ChunkMetaDataRange equal_range (const uint16_t key) const;

			template<typename T = COMMON_LIST>
			T get_values (const std::string_view key) const
			{
				return get_values<T> (key_to_bin (key));
			}

			template<typename T = COMMON_LIST>
			T get_values (const uint16_t key) const
			{
				T vout;
				auto [i_begin, i_end] = _data.equal_range (key);

				for (auto iter = i_begin; iter != i_end; ++iter) {
					vout.push_back (iter->second);
				}

				return vout;
			}

			/* Callback returning true will continue to the next entry, false will end */
			void modify_values (const std::string_view key, ValuesCallback callback);
			void modify_values (const uint16_t key, ValuesCallback callback);

			template<typename T = std::string_view>
			SHAGA_STRV T get_value (const std::string_view key) const
			{
				return get_value<T> (key_to_bin (key));
			}

			template<typename T = std::string_view>
			SHAGA_STRV T get_value (const uint16_t key) const
			{
				ChunkMetaData::const_iterator iter = _data.find (key);
				if (iter != _data.cend ()) {
					return iter->second;
				}

				return T("");
			}

			void modify_value (const std::string_view key, ValueCallback callback);
			void modify_value (const uint16_t key, ValueCallback callback);

			template<typename T = COMMON_LIST>
			T get_keys (void) const
			{
				T vout;
				uint16_t last_key = UINT16_MAX;

				for (ChunkMetaData::const_iterator iter = _data.cbegin (); iter != _data.cend (); ++iter) {
					if (iter->first != last_key) {
						last_key = iter->first;
						vout.push_back (bin_to_key (last_key));
					}
				}

				return vout;
			}

			void to_bin (std::string &s) const;
			std::string to_bin (void) const;

			void from_bin (const std::string_view s, size_t &offset);
	};
}

#endif // HEAD_shaga_ChunkMeta

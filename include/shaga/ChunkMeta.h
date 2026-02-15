/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_ChunkMeta
#define HEAD_shaga_ChunkMeta

#include "common.h"

namespace shaga {
	/* ChunkMeta is key-value structure with repeating key allowed.	Every key has to be three capital letters ('A' to 'Z'). */
	class Chunk;

	using ChunkMetaData = std::unordered_multimap<uint_fast16_t, std::string>;
	using ChunkMetaDataSet = std::set<std::pair<uint_fast16_t, std::string>>;
	using ChunkMetaDataRange = std::pair<ChunkMetaData::const_iterator, ChunkMetaData::const_iterator>;

	static constexpr uint16_t _chunkmeta_key_to_bin_helper (const char str[4], const size_t pos)
	{
		return (pos < 3) ?
				((_chunkmeta_key_to_bin_helper (str, pos + 1) * 27) + (str[pos] == '_' ? (26) : (str[pos] - 'A')))
			:
				0;
	}

	static constexpr uint16_t _chunkmeta_key_to_bin (const char str[4])
	{
		return
			(((str[0] >= 'A' && str[0] <= 'Z') || str[0] == '_') &&
			((str[1] >= 'A' && str[1] <= 'Z') || str[1] == '_') &&
			((str[2] >= 'A' && str[2] <= 'Z') || str[2] == '_') &&
			str[3] == '\0') ?
				(_chunkmeta_key_to_bin_helper (str, 0) + 1)
			:
				0;
	}

	#define ChMetaKEY(a) ([] () constexpr -> uint16_t {                                   \
		static_assert (((a)[0] >= 'A' && (a)[0] <= 'Z') || (a)[0] == '_', "Invalid character in chunk key"); \
		static_assert (((a)[1] >= 'A' && (a)[1] <= 'Z') || (a)[1] == '_', "Invalid character in chunk key"); \
		static_assert (((a)[2] >= 'A' && (a)[2] <= 'Z') || (a)[2] == '_', "Invalid character in chunk key"); \
		static_assert ((a)[3] == '\0', "Chunk key must be exactly 3 characters long");    \
		return shaga::_chunkmeta_key_to_bin (a);                                          \
	}())


	class ChunkMeta {
		public:
			using ValuesCallback = std::function<bool(std::string&)>;
			using ValueCallback = std::function<void(std::string&)>;

			static const constexpr uint16_t key_type_min = ChMetaKEY ("AAA");
			static const constexpr uint16_t key_type_max = ChMetaKEY ("___");

			static const uint16_t key_type_mask   {0b0111'1111'1111'1111};
			static const uint16_t key_repeat_mask {0b0111'1111'0000'0000};
			static const uint8_t key_repeat_byte  {0b0111'1111};

			static uint16_t key_to_bin (const std::string_view key);
			static std::string bin_to_key (const uint16_t bkey);

		private:
			ChunkMetaData _data;

			bool should_continue (const std::string_view s, const size_t offset) const noexcept;
		public:
			ChunkMeta ();
			ChunkMeta (const uint16_t key);
			ChunkMeta (const uint16_t key, const std::string_view value);
			ChunkMeta (const uint16_t key, std::string &&value);
			ChunkMeta (const std::string_view key);
			ChunkMeta (const std::string_view key, const std::string_view value);
			ChunkMeta (const std::string_view key, std::string &&value);

			template<typename T, SHAGA_TYPE_IS_SUPPORTED_xINT(T)>
			ChunkMeta (const uint16_t key, const T value)
			{
				add_value (key, value);
			}

			template<typename T, SHAGA_TYPE_IS_SUPPORTED_xINT(T)>
			ChunkMeta (const std::string_view key, const T value)
			{
				add_value (key, value);
			}

			ChunkMetaData::const_iterator begin () const noexcept;
			ChunkMetaData::const_iterator end () const noexcept;

			ChunkMetaData::const_iterator cbegin () const noexcept;
			ChunkMetaData::const_iterator cend () const noexcept;

			ChunkMetaData::const_iterator find (const std::string_view key) const;
			ChunkMetaData::const_iterator find (const uint16_t key) const noexcept;

			void clear (void) noexcept;
			void reset (void) noexcept;

			void add_value (const uint16_t key);
			void add_value (const std::string_view key);

			void add_value (const uint16_t key, const std::string_view value);
			void add_value (const std::string_view key, const std::string_view value);

			void add_value (const uint16_t key, std::string &&value);
			void add_value (const std::string_view key, std::string &&value);

			template<typename T, SHAGA_TYPE_IS_SUPPORTED_xINTBOOL(T)>
			void add_value (const uint16_t key, const T value)
			{
				using CleanT = std::remove_cv_t<std::remove_reference_t<T>>;
				if constexpr (std::is_same_v<CleanT, bool>) {
					add_bool (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, uint8_t>) {
					add_uint8 (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, int8_t>) {
					add_int8 (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, uint16_t>) {
					add_uint16 (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, int16_t>) {
					add_int16 (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, uint32_t>) {
					add_uint32 (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, int32_t>) {
					add_int32 (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, uint64_t>) {
					add_uint64 (key, value);
				}
				else {
					add_int64 (key, value);
				}
			}

			template<typename T, SHAGA_TYPE_IS_SUPPORTED_xINTBOOL(T)>
			void add_value (const std::string_view key, const T value)
			{
				using CleanT = std::remove_cv_t<std::remove_reference_t<T>>;
				if constexpr (std::is_same_v<CleanT, bool>) {
					add_bool (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, uint8_t>) {
					add_uint8 (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, int8_t>) {
					add_int8 (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, uint16_t>) {
					add_uint16 (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, int16_t>) {
					add_int16 (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, uint32_t>) {
					add_uint32 (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, int32_t>) {
					add_int32 (key, value);
				}
				else if constexpr (std::is_same_v<CleanT, uint64_t>) {
					add_uint64 (key, value);
				}
				else {
					add_int64 (key, value);
				}
			}

			void add_bool (const uint16_t key, const bool value);
			void add_bool (const std::string_view key, const bool value);

			void add_uint8 (const uint16_t key, const uint8_t value);
			void add_uint8 (const std::string_view key, const uint8_t value);

			void add_int8 (const uint16_t key, const int8_t value);
			void add_int8 (const std::string_view key, const int8_t value);

			void add_uint16 (const uint16_t key, const uint16_t value);
			void add_uint16 (const std::string_view key, const uint16_t value);

			void add_int16 (const uint16_t key, const int16_t value);
			void add_int16 (const std::string_view key, const int16_t value);

			void add_uint24 (const uint16_t key, const uint32_t value);
			void add_uint24 (const std::string_view key, const uint32_t value);

			void add_uint32 (const uint16_t key, const uint32_t value);
			void add_uint32 (const std::string_view key, const uint32_t value);

			void add_int32 (const uint16_t key, const int32_t value);
			void add_int32 (const std::string_view key, const int32_t value);

			void add_uint64 (const uint16_t key, const uint64_t value);
			void add_uint64 (const std::string_view key, const uint64_t value);

			void add_int64 (const uint16_t key, const int64_t value);
			void add_int64 (const std::string_view key, const int64_t value);

			size_t get_max_bytes (void) const noexcept;

			size_t size (void) const noexcept;
			size_t count (const std::string_view key) const;
			size_t count (const uint16_t key) const noexcept;
			bool empty (void) const noexcept;

			size_t erase (const uint16_t key);
			size_t erase (const std::string_view key);

			void unique (void);

			void merge (const ChunkMeta &other);
			void merge (ChunkMeta &&other);

			ChunkMetaDataRange equal_range (const std::string_view key) const;
			ChunkMetaDataRange equal_range (const uint16_t key) const noexcept;

			template<typename T = COMMON_LIST, SHAGA_TYPE_IS_ITERABLE(T)>
			auto get_values (const std::string_view key) const -> T
			{
				return get_values<T> (key_to_bin (key));
			}

			template<typename T = COMMON_LIST, SHAGA_TYPE_IS_ITERABLE(T)>
			auto get_values (const uint16_t key) const -> T
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

				return T(""sv);
			}

			template <typename T = std::string_view>
			SHAGA_STRV std::optional<T> get_value_optional (const std::string_view key) const
			{
				return get_value_optional<T> (key_to_bin (key));
			}

			template <typename T = std::string_view>
			SHAGA_STRV std::optional<T> get_value_optional (const uint16_t key) const
			{
				ChunkMetaData::const_iterator iter = _data.find (key);
				if (iter != _data.cend ()) {
					return iter->second;
				}

				return std::nullopt;
			}

			bool get_bool (const std::string_view key, const bool default_value) const;
			bool get_bool (const uint16_t key, const bool default_value) const noexcept;

			uint8_t get_uint8 (const std::string_view key, const uint8_t default_value) const;
			uint8_t get_uint8 (const uint16_t key, const uint8_t default_value) const noexcept;

			int8_t get_int8 (const std::string_view key, const int8_t default_value) const;
			int8_t get_int8 (const uint16_t key, const int8_t default_value) const noexcept;

			uint16_t get_uint16 (const std::string_view key, const uint16_t default_value) const;
			uint16_t get_uint16 (const uint16_t key, const uint16_t default_value) const noexcept;

			int16_t get_int16 (const std::string_view key, const int16_t default_value) const;
			int16_t get_int16 (const uint16_t key, const int16_t default_value) const noexcept;

			uint32_t get_uint24 (const std::string_view key, const uint32_t default_value) const;
			uint32_t get_uint24 (const uint16_t key, const uint32_t default_value) const noexcept;

			uint32_t get_uint32 (const std::string_view key, const uint32_t default_value) const;
			uint32_t get_uint32 (const uint16_t key, const uint32_t default_value) const noexcept;

			int32_t get_int32 (const std::string_view key, const int32_t default_value) const;
			int32_t get_int32 (const uint16_t key, const int32_t default_value) const noexcept;

			uint64_t get_uint64 (const std::string_view key, const uint64_t default_value) const;
			uint64_t get_uint64 (const uint16_t key, const uint64_t default_value) const noexcept;

			int64_t get_int64 (const std::string_view key, const int64_t default_value) const;
			int64_t get_int64 (const uint16_t key, const int64_t default_value) const noexcept;

			std::optional<bool> get_bool_optional (const std::string_view key) const;
			std::optional<bool> get_bool_optional (const uint16_t key) const noexcept;

			std::optional<uint8_t> get_uint8_optional (const std::string_view key) const;
			std::optional<uint8_t> get_uint8_optional (const uint16_t key) const noexcept;

			std::optional<int8_t> get_int8_optional (const std::string_view key) const;
			std::optional<int8_t> get_int8_optional (const uint16_t key) const noexcept;

			std::optional<uint16_t> get_uint16_optional (const std::string_view key) const;
			std::optional<uint16_t> get_uint16_optional (const uint16_t key) const noexcept;

			std::optional<int16_t> get_int16_optional (const std::string_view key) const;
			std::optional<int16_t> get_int16_optional (const uint16_t key) const noexcept;

			std::optional<uint32_t> get_uint24_optional (const std::string_view key) const;
			std::optional<uint32_t> get_uint24_optional (const uint16_t key) const noexcept;

			std::optional<uint32_t> get_uint32_optional (const std::string_view key) const;
			std::optional<uint32_t> get_uint32_optional (const uint16_t key) const noexcept;

			std::optional<int32_t> get_int32_optional (const std::string_view key) const;
			std::optional<int32_t> get_int32_optional (const uint16_t key) const noexcept;

			std::optional<uint64_t> get_uint64_optional (const std::string_view key) const;
			std::optional<uint64_t> get_uint64_optional (const uint16_t key) const noexcept;

			std::optional<int64_t> get_int64_optional (const std::string_view key) const;
			std::optional<int64_t> get_int64_optional (const uint16_t key) const noexcept;

			void modify_value (const std::string_view key, ValueCallback callback);
			void modify_value (const uint16_t key, ValueCallback callback);

			template<typename T = COMMON_LIST, SHAGA_TYPE_IS_ITERABLE(T)>
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

			void to_bin (std::string &out_append) const;
			std::string to_bin (void) const;

			void from_bin (const std::string_view s, size_t &offset);
	};
}

#endif // HEAD_shaga_ChunkMeta

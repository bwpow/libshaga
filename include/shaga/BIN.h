/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_BIN
#define HEAD_shaga_BIN

#include "common.h"

namespace shaga {
	namespace BIN {

		template <typename T>
		constexpr size_t _count_trailing_zeros_helper (const T x, const size_t pos, const size_t sze) {
			return (pos >= sze || (x & (static_cast<T>(1) << pos)) != 0) ? pos : _count_trailing_zeros_helper (x, pos + 1, sze);
		}

		template <typename T>
		constexpr size_t count_trailing_zeros (const T x) {
			return _count_trailing_zeros_helper (x, 0, sizeof (x) * CHAR_BIT);
		}

		template <typename T>
		constexpr size_t _count_zeros_helper (const T x, const size_t pos, const size_t sze) {
			return (pos >= sze) ? 0 : (_count_zeros_helper (x, pos + 1, sze) + (((x & (static_cast<T>(1) << pos)) == 0) ? 1 : 0));
		}

		template <typename T>
		constexpr size_t count_zeros (const T x) {
			return _count_zeros_helper (x, 0, sizeof (x) * CHAR_BIT);
		}

		template <typename T>
		constexpr size_t _count_trailing_ones_helper (const T x, const size_t pos, const size_t sze) {
			return (pos >= sze || (x & (static_cast<T>(1) << pos)) == 0) ? pos : _count_trailing_ones_helper (x, pos + 1, sze);
		}

		template <typename T>
		constexpr size_t count_trailing_ones (const T x) {
			return _count_trailing_ones_helper (x, 0, sizeof (x) * CHAR_BIT);
		}

		template <typename T>
		constexpr size_t _count_ones_helper (const T x, const size_t pos, const size_t sze) {
			return (pos >= sze) ? 0 : (_count_ones_helper (x, pos + 1, sze) + (((x & (static_cast<T>(1) << pos)) != 0) ? 1 : 0));
		}

		template <typename T>
		constexpr size_t count_ones (const T x) {
			return _count_ones_helper (x, 0, sizeof (x) * CHAR_BIT);
		}

		template <typename T>
		constexpr bool check_bit_coverage (const T x) {
			return (x == 0);
		}

		template <typename T, typename S, typename ... Types>
		constexpr bool check_bit_coverage (const T x, const S y, Types&& ... rest) {
			return ((x | y) == x) && check_bit_coverage ((x & (~y)), rest...);
		}

		enum class Endian { UNKNOWN, LITTLE, BIG };
		extern Endian _endian;

		void endian_detect (void);
		bool is_little_endian (void);
		bool is_big_endian (void);
		std::string endian_to_string (void);

		void OR (std::string &lhs, const std::string &rhs);
		void XOR (std::string &lhs, const std::string &rhs);
		void AND (std::string &lhs, const std::string &rhs);

		/* hex */
		uint8_t byte_from_hex (const uint8_t b_high, const uint8_t b_low);
		uint8_t hex_from_byte (const uint8_t b, const bool high);
		size_t to_hex (const std::string &from, std::string &to);
		std::string to_hex (const std::string &from);
		size_t from_hex (const std::string &from, std::string &to);
		std::string from_hex (const std::string &from);

		/* base64 */
		uint8_t byte_from_b64 (const uint8_t c, const bool use_alt = false);
		uint8_t b64_from_byte (const uint8_t b, const bool use_alt = false);
		size_t to_base64 (const std::string &from, std::string &to, const bool use_alt = false);
		std::string to_base64 (const std::string &from, const bool use_alt = false);
		size_t from_base64 (const std::string &from, std::string &to, const bool use_alt = false);
		std::string from_base64 (const std::string &from, const bool use_alt = false);

		/* big endian */
		void be_from_uint8 (const uint8_t v, std::string &s);
		void be_from_uint16 (const uint16_t v, std::string &s);
		void be_from_uint24 (const uint32_t v, std::string &s);
		void be_from_uint32 (const uint32_t v, std::string &s);
		void be_from_uint64 (const uint64_t v, std::string &s);
		std::string be_from_uint8 (const uint8_t v);
		std::string be_from_uint16 (const uint16_t v);
		std::string be_from_uint24 (const uint32_t v);
		std::string be_from_uint32 (const uint32_t v);
		std::string be_from_uint64 (const uint64_t v);

		void be_from_int8 (const int8_t v, std::string &s);
		void be_from_int16 (const int16_t v, std::string &s);
		void be_from_int32 (const int32_t v, std::string &s);
		void be_from_int64 (const int64_t v, std::string &s);
		std::string be_from_int8 (const int8_t v);
		std::string be_from_int16 (const int16_t v);
		std::string be_from_int32 (const int32_t v);
		std::string be_from_int64 (const int64_t v);

		uint8_t be_to_uint8 (const std::string &s, size_t &offset);
		uint16_t be_to_uint16 (const std::string &s, size_t &offset);
		uint32_t be_to_uint24 (const std::string &s, size_t &offset);
		uint32_t be_to_uint32 (const std::string &s, size_t &offset);
		uint64_t be_to_uint64 (const std::string &s, size_t &offset);
		uint8_t be_to_uint8 (const std::string &s);
		uint16_t be_to_uint16 (const std::string &s);
		uint32_t be_to_uint24 (const std::string &s);
		uint32_t be_to_uint32 (const std::string &s);
		uint64_t be_to_uint64 (const std::string &s);

		int8_t be_to_int8 (const std::string &s, size_t &offset);
		int16_t be_to_int16 (const std::string &s, size_t &offset);
		int32_t be_to_int32 (const std::string &s, size_t &offset);
		int64_t be_to_int64 (const std::string &s, size_t &offset);
		int8_t be_to_int8 (const std::string &s);
		int16_t be_to_int16 (const std::string &s);
		int32_t be_to_int32 (const std::string &s);
		int64_t be_to_int64 (const std::string &s);

		/* little endian */
		void from_uint8 (const uint8_t v, std::string &s);
		void from_uint16 (const uint16_t v, std::string &s);
		void from_uint24 (const uint32_t v, std::string &s);
		void from_uint32 (const uint32_t v, std::string &s);
		void from_uint64 (const uint64_t v, std::string &s);
		std::string from_uint8 (const uint8_t v);
		std::string from_uint16 (const uint16_t v);
		std::string from_uint24 (const uint32_t v);
		std::string from_uint32 (const uint32_t v);
		std::string from_uint64 (const uint64_t v);

		void from_int8 (const int8_t v, std::string &s);
		void from_int16 (const int16_t v, std::string &s);
		void from_int32 (const int32_t v, std::string &s);
		void from_int64 (const int64_t v, std::string &s);
		std::string from_int8 (const int8_t v);
		std::string from_int16 (const int16_t v);
		std::string from_int32 (const int32_t v);
		std::string from_int64 (const int64_t v);

		uint8_t to_uint8 (const std::string &s, size_t &offset);
		uint16_t to_uint16 (const std::string &s, size_t &offset);
		uint32_t to_uint24 (const std::string &s, size_t &offset);
		uint32_t to_uint32 (const std::string &s, size_t &offset);
		uint64_t to_uint64 (const std::string &s, size_t &offset);
		uint8_t to_uint8 (const std::string &s);
		uint16_t to_uint16 (const std::string &s);
		uint32_t to_uint24 (const std::string &s);
		uint32_t to_uint32 (const std::string &s);
		uint64_t to_uint64 (const std::string &s);

		int8_t to_int8 (const std::string &s, size_t &offset);
		int16_t to_int16 (const std::string &s, size_t &offset);
		int32_t to_int32 (const std::string &s, size_t &offset);
		int64_t to_int64 (const std::string &s, size_t &offset);
		int8_t to_int8 (const std::string &s);
		int16_t to_int16 (const std::string &s);
		int32_t to_int32 (const std::string &s);
		int64_t to_int64 (const std::string &s);

		/* size */
		size_t to_size (const std::string &s, size_t &offset);

		void from_size (const size_t sze, std::string &s);
		std::string from_size (const size_t sze);
	}
}

#endif // HEAD_shaga_BIN

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2022, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {
	namespace BIN {

		static const uint8_t _b_hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

		static const uint8_t _b_64_table[64] = {
				'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
				'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
				'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };
		static const uint8_t _b_64_pad = '=';

		static const uint8_t _b_alt64_table[64] = {
				'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
				'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
				'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_', '.' };
		static const uint8_t _b_alt64_pad = '-';
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Endian detection  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool BIN::is_little_endian (void)
	{
		#if BYTE_ORDER == LITTLE_ENDIAN
		return true;
		#else
		return false;
		#endif // BYTE_ORDER
	}

	bool BIN::is_big_endian (void)
	{
		#if BYTE_ORDER == BIG_ENDIAN
		return true;
		#else
		return false;
		#endif // BYTE_ORDER
	}

	SHAGA_STRV std::string_view BIN::endian_to_string (void)
	{
		if (is_little_endian ()) {
			return "little endian"sv;
		}
		else {
			return "big endian"sv;
		}
	}

	std::string BIN::machine_type_to_string (void)
	{
		return fmt::format ("{}bit {} machine with {}bit time"sv, sizeof (void*) * 8, endian_to_string (), sizeof (time_t) * 8);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Bitwise operations  /////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void BIN::OR (std::string &lhs, const std::string_view rhs)
	{
		if (lhs.size () != rhs.size ()) {
			cThrow ("Both inputs must have same length"sv);
		}
		std::string::iterator lhs_it = lhs.begin ();
		std::string_view::const_iterator rhs_it = rhs.cbegin ();

		while (lhs_it != lhs.end () && rhs_it != rhs.cend ()) {
			*lhs_it = static_cast<uint8_t> (*lhs_it) | static_cast<uint8_t> (*rhs_it);
			++lhs_it; ++rhs_it;
		}
	}

	void BIN::XOR (std::string &lhs, const std::string_view rhs)
	{
		if (lhs.size () != rhs.size ()) {
			cThrow ("Both inputs must have same length"sv);
		}
		std::string::iterator lhs_it = lhs.begin ();
		std::string_view::const_iterator rhs_it = rhs.cbegin ();

		while (lhs_it != lhs.end () && rhs_it != rhs.cend ()) {
			*lhs_it = static_cast<uint8_t> (*lhs_it) ^ static_cast<uint8_t> (*rhs_it);
			++lhs_it; ++rhs_it;
		}
	}

	void BIN::AND (std::string &lhs, const std::string_view rhs)
	{
		if (lhs.size () != rhs.size ()) {
			cThrow ("Both inputs must have same length"sv);
		}
		std::string::iterator lhs_it = lhs.begin ();
		std::string_view::const_iterator rhs_it = rhs.cbegin ();

		while (lhs_it != lhs.end () && rhs_it != rhs.cend ()) {
			*lhs_it = static_cast<uint8_t> (*lhs_it) & static_cast<uint8_t> (*rhs_it);
			++lhs_it; ++rhs_it;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Hex operations  /////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	namespace BIN {
		inline static uint8_t _byte_from_hex (const uint8_t b_high, const uint8_t b_low)
		{
			uint8_t c;

			if (b_high >= '0' && b_high <= '9') {
				c = b_high - '0';
			}
			else if (b_high >= 'a' && b_high <= 'f') {
				c = (b_high - 'a') + 10;
			}
			else if (b_high >= 'A' && b_high <= 'F') {
				c = (b_high - 'A') + 10;
			}
			else {
				cThrow ("character out of range"sv);
			}

			c <<= 4;

			if (b_low >= '0' && b_low <= '9') {
				c |= b_low - '0';
			}
			else if (b_low >= 'a' && b_low <= 'f') {
				c |= (b_low - 'a') + 10;
			}
			else if (b_low >= 'A' && b_low <= 'F') {
				c |= (b_low - 'A') + 10;
			}
			else {
				cThrow ("character out of range"sv);
			}

			return c;
		}

		inline static uint8_t _hex_from_byte (const uint8_t b, const bool high)
		{
			if (high) {
				return _b_hex[b >> 4];
			}
			else {
				return _b_hex[b & 15];
			}
		}
	}

	uint8_t BIN::byte_from_hex (const uint8_t b_high, const uint8_t b_low)
	{
		return _byte_from_hex (b_high, b_low);
	}

	uint8_t BIN::hex_from_byte (const uint8_t b, const bool high)
	{
		return _hex_from_byte (b, high);
	}

	size_t BIN::to_hex (const std::string_view from, std::string &append_to)
	{
		const size_t start_size = append_to.size ();
		append_to.reserve (append_to.size () + (from.size () << 1));

		for (std::string_view::const_iterator iter = from.cbegin (); iter != from.cend (); ++iter) {
			append_to.push_back (_hex_from_byte (*iter, true));
			append_to.push_back (_hex_from_byte (*iter, false));
		}

		return append_to.size () - start_size;
	}

	std::string BIN::to_hex (const std::string_view from)
	{
		std::string out;
		to_hex (from, out);
		return out;
	}

	size_t BIN::from_hex (const std::string_view from, std::string &append_to)
	{
		const size_t start_size = append_to.size ();
		append_to.reserve (append_to.size () + (from.size () >> 1));

		for (std::string_view::const_iterator iter = from.cbegin (); iter != from.cend (); ++iter) {
			const uint8_t b_high = *iter;
			++iter;
			if (iter == from.cend ()) {
				cThrow ("Malformed data"sv);
			}
			const uint8_t b_low = *iter;

			append_to.push_back (_byte_from_hex (b_high, b_low));
		}

		return append_to.size () - start_size;
	}

	std::string BIN::from_hex (const std::string_view from)
	{
		std::string out;
		from_hex (from, out);
		return out;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Base64 functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* Base64 adapted from episec.com/people/edelkind/arc/c/misc/base64.c‎ */

	namespace BIN {
		static inline uint8_t _byte_from_b64 (const uint8_t c, const bool use_alt)
		{
			if (use_alt) {
				for (uint_fast8_t i = 0; i < 64; i++) {
					if (_b_alt64_table[i] == c) {
						return i;
					}
				}
			}
			else {
				for (uint_fast8_t i = 0; i < 64; i++) {
					if (_b_64_table[i] == c) {
						return i;
					}
				}
			}
			cThrow ("Value out of range"sv);
		}

		static inline uint8_t _b64_from_byte (const uint8_t b, const bool use_alt)
		{
			if (b >= 64) {
				cThrow ("Value out of range"sv);
			}
			if (use_alt) {
				return _b_alt64_table[b];
			}
			return _b_64_table[b];
		}
	}

	uint8_t BIN::byte_from_b64 (const uint8_t c, const bool use_alt)
	{
		return _byte_from_b64 (c, use_alt);
	}

	uint8_t BIN::b64_from_byte (const uint8_t b, const bool use_alt)
	{
		return _b64_from_byte (b, use_alt);
	}

	size_t BIN::to_base64 (const std::string_view from, std::string &append_to, const bool use_alt)
	{
		append_to.reserve (append_to.size () + (from.size () << 1));

		const size_t start_size = append_to.size ();
		std::string_view::const_iterator fromp = from.cbegin ();
		uint8_t cbyte;
		uint8_t obyte;
		uint8_t end[3];
		size_t len = from.size ();
		const uint8_t pad = use_alt ? _b_alt64_pad : _b_64_pad;

		for (; len >= 3; len -= 3) {
			cbyte = (uint8_t) *(fromp++);
			append_to.push_back (_b64_from_byte (cbyte >> 2, use_alt));
			obyte = (cbyte << 4) & 0x30; /* 0011 0000 */

			cbyte = (uint8_t) *(fromp++);
			obyte |= (cbyte >> 4); /* 0000 1111 */
			append_to.push_back (_b64_from_byte (obyte, use_alt));
			obyte = (cbyte << 2) & 0x3C; /* 0011 1100 */

			cbyte = (uint8_t) *(fromp++);
			obyte |= (cbyte >> 6); /* 0000 0011 */
			append_to.push_back (_b64_from_byte (obyte, use_alt));
			append_to.push_back (_b64_from_byte (cbyte & 0x3F, use_alt)); /* 0011 1111 */
		}

		if (len) {
			end[0] = (uint8_t) * (fromp++);
			if (--len) {
				end[1] = (uint8_t) * (fromp++);
			} else {
				end[1] = 0;
			}
			end[2] = 0;

			cbyte = end[0];
			append_to.push_back (_b64_from_byte (cbyte >> 2, use_alt));
			obyte = (cbyte << 4) & 0x30; /* 0011 0000 */

			cbyte = end[1];
			obyte |= (cbyte >> 4);
			append_to.push_back (_b64_from_byte (obyte, use_alt));
			obyte = (cbyte << 2) & 0x3C; /* 0011 1100 */

			if (len) {
				append_to.push_back (_b64_from_byte (obyte, use_alt));
			} else {
				append_to.push_back (pad);
			}

			append_to.push_back (pad);
		}

		return append_to.size () - start_size;
	}

	std::string BIN::to_base64 (const std::string_view from, const bool use_alt)
	{
		std::string out;
		to_base64 (from, out, use_alt);
		return out;
	}

	size_t BIN::from_base64 (const std::string_view from, std::string &append_to, const bool use_alt)
	{
		append_to.reserve (append_to.size () + (from.size () >> 1));
		const size_t start_size = append_to.size ();

		try {
			std::string_view::const_iterator fromp = from.cbegin ();
			uint8_t cbyte;
			uint8_t obyte;
			int padding = 0;
			size_t len = from.size ();
			const uint8_t pad = use_alt ? _b_alt64_pad : _b_64_pad;

			for (; len >= 4; len -= 4) {
				if ( (cbyte = (uint8_t) * (fromp++)) == pad) {
					cbyte = 0;
				} else {
					cbyte = _byte_from_b64 (cbyte, use_alt);
				}
				obyte = cbyte << 2; /* 1111 1100 */

				if ( (cbyte = (uint8_t) * (fromp++)) == pad) {
					cbyte = 0;
				} else {
					cbyte = _byte_from_b64 (cbyte, use_alt);
				}
				obyte |= cbyte >> 4; /* 0000 0011 */

				append_to.push_back (obyte);

				obyte = cbyte << 4; /* 1111 0000 */
				if ( (cbyte = (uint8_t) * (fromp++)) == pad) {
					cbyte = 0;
					padding++;
				} else {
					padding = 0;
					cbyte = _byte_from_b64 (cbyte, use_alt);
				}
				obyte |= cbyte >> 2; /* 0000 1111 */

				append_to.push_back (obyte);

				obyte = cbyte << 6; /* 1100 0000 */
				if ( (cbyte = (uint8_t) * (fromp++)) == pad) {
					cbyte = 0;
					padding++;
				} else {
					padding = 0;
					cbyte = _byte_from_b64 (cbyte, use_alt);
				}
				obyte |= cbyte; /* 0011 1111 */

				append_to.push_back (obyte);
			}

			if (len || fromp != from.cend ()) {
				cThrow ("Malformed data"sv);
			}

			append_to.erase (append_to.size () - padding, std::string::npos);
		}
		catch (...) {
			append_to.resize (start_size);
			throw;
		}
		return append_to.size () - start_size;
	}

	std::string BIN::from_base64 (const std::string_view from, const bool use_alt)
	{
		std::string out;
		from_base64 (from, out, use_alt);
		return out;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Big endian operations  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	namespace BIN {
		#ifdef SHAGA_THREADING
			static thread_local char _be_le_buf[8];
			static thread_local const char *_be_le_data;
		#else
			static char _be_le_buf[8];
			static const char *_be_le_data;
		#endif // SHAGA_THREADING

		template<typename T, typename S>
		static inline auto _be_to_int (const S &s, size_t &offset) -> T
		{
			if ((offset + sizeof (T)) > s.size ()) {
				cThrow ("Not enough bytes"sv);
			}
			else if (std::is_same<T, uint8_t>::value || std::is_same<T, int8_t>::value) {
				return static_cast<T> (s[offset++]);
			}
			else if (std::is_same<T, uint16_t>::value || std::is_same<T, int16_t>::value) {
				return static_cast<T> (_be_to_uint16 (s.data (), offset));
			}
			else if (std::is_same<T, uint32_t>::value || std::is_same<T, int32_t>::value) {
				return static_cast<T> (_be_to_uint32 (s.data (), offset));
			}
			else if (std::is_same<T, uint64_t>::value || std::is_same<T, int64_t>::value) {
				return static_cast<T> (_be_to_uint64 (s.data (), offset));
			}
			else {
				cThrow ("Unknown data type"sv);
			}
		}

		template<typename T, typename S>
		static inline auto _to_int (const S &s, size_t &offset) -> T
		{
			if ((offset + sizeof (T)) > s.size ()) {
				cThrow ("Not enough bytes"sv);
			}
			else if (std::is_same<T, uint8_t>::value || std::is_same<T, int8_t>::value) {
				return static_cast<T> (s[offset++]);
			}
			else if (std::is_same<T, uint16_t>::value || std::is_same<T, int16_t>::value) {
				return static_cast<T> (_to_uint16 (s.data (), offset));
			}
			else if (std::is_same<T, uint32_t>::value || std::is_same<T, int32_t>::value) {
				return static_cast<T> (_to_uint32 (s.data (), offset));
			}
			else if (std::is_same<T, uint64_t>::value || std::is_same<T, int64_t>::value) {
				return static_cast<T> (_to_uint64 (s.data (), offset));
			}
			else {
				cThrow ("Unknown data type"sv);
			}
		}

		template<typename S>
		static inline uint32_t _be_to_uint24 (const S &s, size_t &offset)
		{
			if ((offset + 3) > s.size ()) {
				cThrow ("Not enough bytes"sv);
			}
			_be_le_data = s.data () + offset;
			offset += 3;

			#define SAT(x) static_cast<uint8_t> (_be_le_data[x])
			const uint32_t v = (SAT(0) << 16) | (SAT(1) << 8) | SAT(2);
			#undef SAT

			return v;
		}

		template<typename S>
		static inline uint32_t _to_uint24 (const S &s, size_t &offset)
		{
			if ((offset + 3) > s.size ()) {
				cThrow ("Not enough bytes"sv);
			}
			_be_le_data = s.data () + offset;
			offset += 3;

			#define SAT(x) static_cast<uint8_t> (_be_le_data[x])
			const uint32_t v = SAT(0) | (SAT(1) << 8) | (SAT(2) << 16);
			#undef SAT

			return v;
		}
	}

	void BIN::be_from_uint8 (const uint8_t v, std::string &s)
	{
		s.push_back (v);
	}

	void BIN::be_from_uint16 (const uint16_t v, std::string &s)
	{
		_be_from_uint16 (v, _be_le_buf);
		s.append (_be_le_buf, sizeof (uint16_t));
	}

	void BIN::be_from_uint24 (const uint32_t v, std::string &s)
	{
		#define TAS(p,x) _be_le_buf[p] = static_cast<char> ((v >> x) & 0xFF)
		TAS (0, 16);
		TAS (1, 8);
		TAS (2, 0);
		#undef TAS

		s.append (_be_le_buf, 3);
	}

	void BIN::be_from_uint32 (const uint32_t v, std::string &s)
	{
		_be_from_uint32 (v, _be_le_buf);
		s.append (_be_le_buf, sizeof (uint32_t));
	}

	void BIN::be_from_uint64 (const uint64_t v, std::string &s)
	{
		_be_from_uint64 (v, _be_le_buf);
		s.append (_be_le_buf, sizeof (uint64_t));
	}

	std::string BIN::be_from_uint8 (const uint8_t v)
	{
		std::string s;
		s.reserve (1);
		be_from_uint8 (v, s);
		return s;
	}

	std::string BIN::be_from_uint16 (const uint16_t v)
	{
		std::string s;
		s.reserve (2);
		be_from_uint16 (v, s);
		return s;
	}

	std::string BIN::be_from_uint24 (const uint32_t v)
	{
		std::string s;
		s.reserve (3);
		be_from_uint24 (v, s);
		return s;
	}

	std::string BIN::be_from_uint32 (const uint32_t v)
	{
		std::string s;
		s.reserve (4);
		be_from_uint32 (v, s);
		return s;
	}

	std::string BIN::be_from_uint64 (const uint64_t v)
	{
		std::string s;
		s.reserve (8);
		be_from_uint64 (v, s);
		return s;
	}

	void BIN::be_from_int8 (const int8_t v, std::string &s)
	{
		be_from_uint8 (static_cast<uint8_t> (v), s);
	}

	void BIN::be_from_int16 (const int16_t v, std::string &s)
	{
		be_from_uint16 (static_cast<uint16_t> (v), s);
	}

	void BIN::be_from_int32 (const int32_t v, std::string &s)
	{
		be_from_uint32 (static_cast<uint32_t> (v), s);
	}

	void BIN::be_from_int64 (const int64_t v, std::string &s)
	{
		be_from_uint64 (static_cast<uint64_t> (v), s);
	}

	std::string BIN::be_from_int8 (const int8_t v)
	{
		return be_from_uint8 (static_cast<uint8_t> (v));
	}

	std::string BIN::be_from_int16 (const int16_t v)
	{
		return be_from_uint16 (static_cast<uint16_t> (v));
	}

	std::string BIN::be_from_int32 (const int32_t v)
	{
		return be_from_uint32 (static_cast<uint32_t> (v));
	}

	std::string BIN::be_from_int64 (const int64_t v)
	{
		return be_from_uint64 (static_cast<uint64_t> (v));
	}

	uint8_t BIN::be_to_uint8 (const std::string_view s, size_t &offset)
	{
		return _be_to_int<uint8_t> (s, offset);
	}

	uint16_t BIN::be_to_uint16 (const std::string_view s, size_t &offset)
	{
		return _be_to_int<uint16_t> (s, offset);
	}

	uint32_t BIN::be_to_uint24 (const std::string_view s, size_t &offset)
	{
		return _be_to_uint24 (s, offset);
	}

	uint32_t BIN::be_to_uint32 (const std::string_view s, size_t &offset)
	{
		return _be_to_int<uint32_t> (s, offset);
	}

	uint64_t BIN::be_to_uint64 (const std::string_view s, size_t &offset)
	{
		return _be_to_int<uint64_t> (s, offset);
	}

	uint8_t BIN::be_to_uint8 (const std::string_view s)
	{
		size_t offset = 0;
		return _be_to_int<uint8_t> (s, offset);
	}

	uint16_t BIN::be_to_uint16 (const std::string_view s)
	{
		size_t offset = 0;
		return _be_to_int<uint16_t> (s, offset);
	}

	uint32_t BIN::be_to_uint24 (const std::string_view s)
	{
		size_t offset = 0;
		return _be_to_uint24 (s, offset);
	}

	uint32_t BIN::be_to_uint32 (const std::string_view s)
	{
		size_t offset = 0;
		return _be_to_int<uint32_t> (s, offset);
	}

	uint64_t BIN::be_to_uint64 (const std::string_view s)
	{
		size_t offset = 0;
		return _be_to_int<uint64_t> (s, offset);
	}

	int8_t BIN::be_to_int8 (const std::string_view s, size_t &offset)
	{
		return _be_to_int<int8_t> (s, offset);
	}

	int16_t BIN::be_to_int16 (const std::string_view s, size_t &offset)
	{
		return _be_to_int<int16_t> (s, offset);
	}

	int32_t BIN::be_to_int32 (const std::string_view s, size_t &offset)
	{
		return _be_to_int<int32_t> (s, offset);
	}

	int64_t BIN::be_to_int64 (const std::string_view s, size_t &offset)
	{
		return _be_to_int<int64_t> (s, offset);
	}

	int8_t BIN::be_to_int8 (const std::string_view s)
	{
		size_t offset = 0;
		return _be_to_int<int8_t> (s, offset);
	}

	int16_t BIN::be_to_int16 (const std::string_view s)
	{
		size_t offset = 0;
		return _be_to_int<int16_t> (s, offset);
	}

	int32_t BIN::be_to_int32 (const std::string_view s)
	{
		size_t offset = 0;
		return _be_to_int<int32_t> (s, offset);
	}

	int64_t BIN::be_to_int64 (const std::string_view s)
	{
		size_t offset = 0;
		return _be_to_int<int64_t> (s, offset);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Little endian operations  ///////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void BIN::from_uint8 (const uint8_t v, std::string &s)
	{
		s.push_back (v);
	}

	void BIN::from_uint16 (const uint16_t v, std::string &s)
	{
		_from_uint16 (v, _be_le_buf);
		s.append (_be_le_buf, sizeof (uint16_t));
	}

	void BIN::from_uint24 (const uint32_t v, std::string &s)
	{
		#define TAS(p,x) _be_le_buf[p] = static_cast<unsigned char> ((v >> x) & 0xFF)
		TAS (0, 0);
		TAS (1, 8);
		TAS (2, 16);
		#undef TAS

		s.append (_be_le_buf, 3);
	}

	void BIN::from_uint32 (const uint32_t v, std::string &s)
	{
		_from_uint32 (v, _be_le_buf);
		s.append (_be_le_buf, sizeof (uint32_t));
	}

	void BIN::from_uint64 (const uint64_t v, std::string &s)
	{
		_from_uint64 (v, _be_le_buf);
		s.append (_be_le_buf, sizeof (uint64_t));
	}

	std::string BIN::from_uint8 (const uint8_t v)
	{
		std::string s;
		s.reserve (1);
		from_uint8 (v, s);
		return s;
	}

	std::string BIN::from_uint16 (const uint16_t v)
	{
		std::string s;
		s.reserve (2);
		from_uint16 (v, s);
		return s;
	}

	std::string BIN::from_uint24 (const uint32_t v)
	{
		std::string s;
		s.reserve (3);
		from_uint24 (v, s);
		return s;
	}

	std::string BIN::from_uint32 (const uint32_t v)
	{
		std::string s;
		s.reserve (4);
		from_uint32 (v, s);
		return s;
	}

	std::string BIN::from_uint64 (const uint64_t v)
	{
		std::string s;
		s.reserve (8);
		from_uint64 (v, s);
		return s;
	}

	void BIN::from_int8 (const int8_t v, std::string &s)
	{
		from_uint8 (static_cast<uint8_t> (v), s);
	}

	void BIN::from_int16 (const int16_t v, std::string &s)
	{
		from_uint16 (static_cast<uint16_t> (v), s);
	}

	void BIN::from_int32 (const int32_t v, std::string &s)
	{
		from_uint32 (static_cast<uint32_t> (v), s);
	}

	void BIN::from_int64 (const int64_t v, std::string &s)
	{
		from_uint64 (static_cast<uint64_t> (v), s);
	}

	std::string BIN::from_int8 (const int8_t v)
	{
		return from_uint8 (static_cast<uint8_t> (v));
	}

	std::string BIN::from_int16 (const int16_t v)
	{
		return from_uint16 (static_cast<uint16_t> (v));
	}

	std::string BIN::from_int32 (const int32_t v)
	{
		return from_uint32 (static_cast<uint32_t> (v));
	}

	std::string BIN::from_int64 (const int64_t v)
	{
		return from_uint64 (static_cast<uint64_t> (v));
	}

	uint8_t BIN::to_uint8 (const std::string_view s, size_t &offset)
	{
		return _to_int<uint8_t> (s, offset);
	}

	uint16_t BIN::to_uint16 (const std::string_view s, size_t &offset)
	{
		return _to_int<uint16_t> (s, offset);
	}

	uint32_t BIN::to_uint24 (const std::string_view s, size_t &offset)
	{
		return _to_uint24 (s, offset);
	}

	uint32_t BIN::to_uint32 (const std::string_view s, size_t &offset)
	{
		return _to_int<uint32_t> (s, offset);
	}

	uint64_t BIN::to_uint64 (const std::string_view s, size_t &offset)
	{
		return _to_int<uint64_t> (s, offset);
	}

	uint8_t BIN::to_uint8 (const std::string_view s)
	{
		size_t offset = 0;
		return _to_int<uint8_t> (s, offset);
	}

	uint16_t BIN::to_uint16 (const std::string_view s)
	{
		size_t offset = 0;
		return _to_int<uint16_t> (s, offset);
	}

	uint32_t BIN::to_uint24 (const std::string_view s)
	{
		size_t offset = 0;
		return _to_uint24 (s, offset);
	}

	uint32_t BIN::to_uint32 (const std::string_view s)
	{
		size_t offset = 0;
		return _to_int<uint32_t> (s, offset);
	}

	uint64_t BIN::to_uint64 (const std::string_view s)
	{
		size_t offset = 0;
		return _to_int<uint64_t> (s, offset);
	}

	int8_t BIN::to_int8 (const std::string_view s, size_t &offset)
	{
		return _to_int<int8_t> (s, offset);
	}

	int16_t BIN::to_int16 (const std::string_view s, size_t &offset)
	{
		return _to_int<int16_t> (s, offset);
	}

	int32_t BIN::to_int32 (const std::string_view s, size_t &offset)
	{
		return _to_int<int32_t> (s, offset);
	}

	int64_t BIN::to_int64 (const std::string_view s, size_t &offset)
	{
		return _to_int<int64_t> (s, offset);
	}

	int8_t BIN::to_int8 (const std::string_view s)
	{
		size_t offset = 0;
		return _to_int<int8_t> (s, offset);
	}

	int16_t BIN::to_int16 (const std::string_view s)
	{
		size_t offset = 0;
		return _to_int<int16_t> (s, offset);
	}

	int32_t BIN::to_int32 (const std::string_view s)
	{
		size_t offset = 0;
		return _to_int<int32_t> (s, offset);
	}

	int64_t BIN::to_int64 (const std::string_view s)
	{
		size_t offset = 0;
		return _to_int<int64_t> (s, offset);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Size functions  /////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	size_t BIN::to_size (const std::string_view s, size_t &offset)
	{
		size_t v {0};

		#define SAT static_cast<uint8_t> (s.at (offset)); ++offset
		const size_t d1 = SAT;
		if (d1 & 0x80) {
			const size_t d2 = SAT;
			if (d1 & 0x40) {
				const size_t d3 = SAT;
				if (d1 & 0x20) {
					const size_t d4 = SAT;
					v = ( (d1 & 0x1F) << 24) | (d2 << 16) | (d3 << 8) | d4;
				}
				else {
					v = ( (d1 & 0x3F) << 16) | (d2 << 8) | d3;
				}
			}
			else {
				v = ( (d1 & 0x7F) << 8) | d2;
			}
		}
		else {
			v = d1;
		}
		#undef SAT

		return v;
	}

	void BIN::from_size (const size_t sze, std::string &s)
	{
		if (sze <= 0x7F) {
			s.push_back (static_cast<uint8_t> (sze));
		}
		else if (sze <= 0x3FFF) {
			s.push_back (static_cast<uint8_t> ( (0x01 << 7) | ( (sze >> 8) & 0x3F) ));
			s.push_back (static_cast<uint8_t> ( sze & 0xFF ));
		}
		else if (sze <= 0x1FFFFF) {
			s.push_back (static_cast<uint8_t> ( (0x03 << 6) | ( (sze >> 16) & 0x1F) ));
			s.push_back (static_cast<uint8_t> ( (sze >> 8) & 0xFF ));
			s.push_back (static_cast<uint8_t> ( sze & 0xFF ));
		}
		else if (sze <= 0xFFFFFFF) {
			s.push_back (static_cast<uint8_t> ( (0x07 << 5) | ( (sze >> 24) & 0xF) ));
			s.push_back (static_cast<uint8_t> ( (sze >> 16) & 0xFF ));
			s.push_back (static_cast<uint8_t> ( (sze >> 8) & 0xFF ));
			s.push_back (static_cast<uint8_t> ( sze & 0xFF ));
		}
		else {
			cThrow ("Unable to encode size larger than 0xFFFFFFF"sv);
		}
	}

	std::string BIN::from_size (const size_t sze)
	{
		std::string out;
		out.reserve (4);
		from_size (sze, out);
		return out;
	}
}

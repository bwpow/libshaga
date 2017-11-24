/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

namespace shaga {
	namespace BIN {
		Endian _endian = Endian::UNKNOWN;

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

		#ifdef SHAGA_THREADING
			static std::once_flag _endian_once_flag;
		#else
			static bool _endian_once_flag {false};
		#endif // SHAGA_THREADING

		static void _endian_detect_once (void)
		{
			const uint32_t t = htonl (0x12345678);
			if (t == 0x12345678) {
				_endian = Endian::BIG;
			}
			else if (t == 0x78563412) {
				_endian = Endian::LITTLE;
			}
			else {
				_endian = Endian::UNKNOWN;
			}

			ENDIAN_IS_BIG
				if (Endian::BIG != _endian) {
					exit ("Unable to continue. Library is build for big endian and little endian was detected.");
				}
			ENDIAN_END

			ENDIAN_IS_LITTLE
				if (Endian::LITTLE != _endian) {
					exit ("Unable to continue. Library is build for little endian and big endian was detected.");
				}
			ENDIAN_END
		}
	}

	void BIN::endian_detect (void)
	{
		#ifdef SHAGA_THREADING
			std::call_once (_endian_once_flag, _endian_detect_once);
		#else
			if (std::exchange (_endian_once_flag, true) == false) {
				_endian_detect_once ();
			}
		#endif // SHAGA_THREADING
	}

	bool BIN::is_little_endian (void)
	{
		endian_detect ();
		return _endian == Endian::LITTLE;
	}

	bool BIN::is_big_endian (void)
	{
		endian_detect ();
		return _endian == Endian::BIG;
	}

	std::string BIN::endian_to_string (void)
	{
		endian_detect ();
		switch (_endian) {
			case Endian::UNKNOWN: return "unknown endian";
			case Endian::LITTLE: return "little endian";
			case Endian::BIG: return "big endian";
		}

		cThrow ("Undefined endian");
	}

	void BIN::OR (std::string &lhs, const std::string &rhs)
	{
		if (lhs.size () != rhs.size ()) {
			cThrow ("String must have same size");
		}
		std::string::iterator lhs_it = lhs.begin ();
		std::string::const_iterator rhs_it = rhs.cbegin ();

		while (lhs_it != lhs.end () && rhs_it != rhs.cend ()) {
			*lhs_it = static_cast<uint8_t> (*lhs_it) | static_cast<uint8_t> (*rhs_it);
			++lhs_it; ++rhs_it;
		}

	}

	void BIN::XOR (std::string &lhs, const std::string &rhs)
	{
		if (lhs.size () != rhs.size ()) {
			cThrow ("String must have same size");
		}
		std::string::iterator lhs_it = lhs.begin ();
		std::string::const_iterator rhs_it = rhs.cbegin ();

		while (lhs_it != lhs.end () && rhs_it != rhs.cend ()) {
			*lhs_it = static_cast<uint8_t> (*lhs_it) ^ static_cast<uint8_t> (*rhs_it);
			++lhs_it; ++rhs_it;
		}

	}

	void BIN::AND (std::string &lhs, const std::string &rhs)
	{
		if (lhs.size () != rhs.size ()) {
			cThrow ("String must have same size");
		}
		std::string::iterator lhs_it = lhs.begin ();
		std::string::const_iterator rhs_it = rhs.cbegin ();

		while (lhs_it != lhs.end () && rhs_it != rhs.cend ()) {
			*lhs_it = static_cast<uint8_t> (*lhs_it) & static_cast<uint8_t> (*rhs_it);
			++lhs_it; ++rhs_it;
		}

	}

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
				cThrow ("character out of range");
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
				cThrow ("character out of range");
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

	size_t BIN::to_hex (const std::string &from, std::string &to)
	{
		const size_t start_size = to.size ();
		to.reserve (to.size () + (from.size () << 1));

		for (std::string::const_iterator iter = from.begin (); iter != from.end (); ++iter) {
			to.push_back (_hex_from_byte (*iter, true));
			to.push_back (_hex_from_byte (*iter, false));
		}

		return to.size () - start_size;
	}

	std::string BIN::to_hex (const std::string &from)
	{
		std::string out;
		to_hex (from, out);
		return out;
	}

	size_t BIN::from_hex (const std::string &from, std::string &to)
	{
		const size_t start_size = to.size ();
		to.reserve (to.size () + (from.size () >> 1));

		for (std::string::const_iterator iter = from.cbegin (); iter != from.cend (); ++iter) {
			const uint8_t b_high = *iter;
			++iter;
			if (iter == from.cend ()) {
				cThrow ("Malformed data");
			}
			const uint8_t b_low = *iter;

			to.push_back (_byte_from_hex (b_high, b_low));
		}

		return to.size () - start_size;
	}

	std::string BIN::from_hex (const std::string &from)
	{
		std::string out;
		from_hex (from, out);
		return out;
	}

	/* Base64 adapted from episec.com/people/edelkind/arc/c/misc/base64.c‎ */

	namespace BIN {
		static inline uint8_t _byte_from_b64 (const uint8_t c, const bool use_alt)
		{
			if (use_alt) {
				for (uint8_t i = 0; i < 64; i++) {
					if (_b_alt64_table[i] == c) {
						return i;
					}
				}
			}
			else {
				for (uint8_t i = 0; i < 64; i++) {
					if (_b_64_table[i] == c) {
						return i;
					}
				}
			}
			cThrow ("Value out of range");
		}

		static inline uint8_t _b64_from_byte (const uint8_t b, const bool use_alt)
		{
			if (b >= 64) {
				cThrow ("Value out of range");
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

	size_t BIN::to_base64 (const std::string &from, std::string &to, const bool use_alt)
	{
		to.reserve (to.size () + (from.size () << 1));

		const size_t start_size = to.size ();
		std::string::const_iterator fromp = from.cbegin ();
		uint8_t cbyte;
		uint8_t obyte;
		uint8_t end[3];
		size_t len = from.size ();
		const uint8_t pad = use_alt ? _b_alt64_pad : _b_64_pad;

		for (; len >= 3; len -= 3) {
			cbyte = (uint8_t) * (fromp++);
			to.push_back (_b64_from_byte (cbyte >> 2, use_alt));
			obyte = (cbyte << 4) & 0x30; /* 0011 0000 */

			cbyte = (uint8_t) * (fromp++);
			obyte |= (cbyte >> 4); /* 0000 1111 */
			to.push_back (_b64_from_byte (obyte, use_alt));
			obyte = (cbyte << 2) & 0x3C; /* 0011 1100 */

			cbyte = (uint8_t) * (fromp++);
			obyte |= (cbyte >> 6); /* 0000 0011 */
			to.push_back (_b64_from_byte (obyte, use_alt));
			to.push_back (_b64_from_byte (cbyte & 0x3F, use_alt)); /* 0011 1111 */
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
			to.push_back (_b64_from_byte (cbyte >> 2, use_alt));
			obyte = (cbyte << 4) & 0x30; /* 0011 0000 */

			cbyte = end[1];
			obyte |= (cbyte >> 4);
			to.push_back (_b64_from_byte (obyte, use_alt));
			obyte = (cbyte << 2) & 0x3C; /* 0011 1100 */

			if (len) {
				to.push_back (_b64_from_byte (obyte, use_alt));
			} else {
				to.push_back (pad);
			}

			to.push_back (pad);
		}

		return to.size () - start_size;
	}

	std::string BIN::to_base64 (const std::string &from, const bool use_alt)
	{
		std::string out;
		to_base64 (from, out, use_alt);
		return out;
	}

	size_t BIN::from_base64 (const std::string &from, std::string &to, const bool use_alt)
	{
		to.reserve (to.size () + (from.size () >> 1));

		const size_t start_size = to.size ();
		std::string::const_iterator fromp = from.cbegin ();
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

			to.push_back (obyte);

			obyte = cbyte << 4; /* 1111 0000 */
			if ( (cbyte = (uint8_t) * (fromp++)) == pad) {
				cbyte = 0;
				padding++;
			} else {
				padding = 0;
				cbyte = _byte_from_b64 (cbyte, use_alt);
			}
			obyte |= cbyte >> 2; /* 0000 1111 */

			to.push_back (obyte);

			obyte = cbyte << 6; /* 1100 0000 */
			if ( (cbyte = (uint8_t) * (fromp++)) == pad) {
				cbyte = 0;
				padding++;
			} else {
				padding = 0;
				cbyte = _byte_from_b64 (cbyte, use_alt);
			}
			obyte |= cbyte; /* 0011 1111 */

			to.push_back (obyte);
		}

		if (len || fromp != from.end ()) {
			cThrow ("Malformed data");
		}

		to.erase (to.size () - padding, std::string::npos);
		return to.size () - start_size;
	}

	std::string BIN::from_base64 (const std::string &from, const bool use_alt)
	{
		std::string out;
		from_base64 (from, out, use_alt);
		return out;
	}

	/*** big endian operations ***/

	namespace BIN {
		static thread_local uint8_t _be_le_buf[8];
		static thread_local const uint8_t *_be_le_data;
	}

	void BIN::be_from_uint8 (const uint8_t v, std::string &s)
	{
		s.push_back (v);
	}

	void BIN::be_from_uint16 (const uint16_t v, std::string &s)
	{
		#define BYTES 2

		ENDIAN_IS_BIG
			::memcpy (_be_le_buf, &v, BYTES);
		ENDIAN_ELSE
			#define TAS(p,x) _be_le_buf[p] = ((v >> x) & 0xFF)
			TAS (0, 8);
			TAS (1, 0);
			#undef TAS
		ENDIAN_END

		s.append (reinterpret_cast<const char *> (_be_le_buf), BYTES);

		#undef BYTES
	}

	void BIN::be_from_uint24 (const uint32_t v, std::string &s)
	{
		#define BYTES 3

		#define TAS(p,x) _be_le_buf[p] = ((v >> x) & 0xFF)
		TAS (0, 16);
		TAS (1, 8);
		TAS (2, 0);
		#undef TAS

		s.append (reinterpret_cast<const char *> (_be_le_buf), BYTES);

		#undef BYTES
	}

	void BIN::be_from_uint32 (const uint32_t v, std::string &s)
	{
		#define BYTES 4

		ENDIAN_IS_BIG
			::memcpy (_be_le_buf, &v, BYTES);
		ENDIAN_ELSE
			#define TAS(p,x) _be_le_buf[p] = ((v >> x) & 0xFF)
			TAS (0, 24);
			TAS (1, 16);
			TAS (2, 8);
			TAS (3, 0);
			#undef TAS
		ENDIAN_END

		s.append (reinterpret_cast<const char *> (_be_le_buf), BYTES);

		#undef BYTES
	}

	void BIN::be_from_uint64 (const uint64_t v, std::string &s)
	{
		#define BYTES 8

		ENDIAN_IS_BIG
			::memcpy (_be_le_buf, &v, BYTES);
		ENDIAN_ELSE
			#define TAS(p,x) _be_le_buf[p] = ((v >> x) & 0xFF)
			TAS (0, 56);
			TAS (1, 48);
			TAS (2, 40);
			TAS (3, 32);

			TAS (4, 24);
			TAS (5, 16);
			TAS (6, 8);
			TAS (7, 0);
			#undef TAS
		ENDIAN_END

		s.append (reinterpret_cast<const char *> (_be_le_buf), BYTES);

		#undef BYTES
	}

	std::string BIN::be_from_uint8 (const uint8_t v)
	{
		std::string s;
		be_from_uint8 (v, s);
		return s;
	}

	std::string BIN::be_from_uint16 (const uint16_t v)
	{
		std::string s;
		be_from_uint16 (v, s);
		return s;
	}

	std::string BIN::be_from_uint24 (const uint32_t v)
	{
		std::string s;
		be_from_uint24 (v, s);
		return s;
	}

	std::string BIN::be_from_uint32 (const uint32_t v)
	{
		std::string s;
		be_from_uint32 (v, s);
		return s;
	}

	std::string BIN::be_from_uint64 (const uint64_t v)
	{
		std::string s;
		be_from_uint64 (v, s);
		return s;
	}

	void BIN::be_from_int8 (const int8_t v, std::string &s)
	{
		uint8_t uv;
		memcpy (&uv, &v, sizeof (uint8_t));
		be_from_uint8 (uv, s);
	}

	void BIN::be_from_int16 (const int16_t v, std::string &s)
	{
		uint16_t uv;
		memcpy (&uv, &v, sizeof (uint16_t));
		be_from_uint16 (uv, s);
	}

	void BIN::be_from_int32 (const int32_t v, std::string &s)
	{
		uint32_t uv;
		memcpy (&uv, &v, sizeof (uint32_t));
		be_from_uint32 (uv, s);
	}

	void BIN::be_from_int64 (const int64_t v, std::string &s)
	{
		uint64_t uv;
		memcpy (&uv, &v, sizeof (uint64_t));
		be_from_uint64 (uv, s);
	}

	std::string BIN::be_from_int8 (const int8_t v)
	{
		std::string s;
		be_from_int8 (v, s);
		return s;
	}

	std::string BIN::be_from_int16 (const int16_t v)
	{
		std::string s;
		be_from_int16 (v, s);
		return s;
	}

	std::string BIN::be_from_int32 (const int32_t v)
	{
		std::string s;
		be_from_int32 (v, s);
		return s;
	}

	std::string BIN::be_from_int64 (const int64_t v)
	{
		std::string s;
		be_from_int64 (v, s);
		return s;
	}

	uint8_t BIN::be_to_uint8 (const std::string &s, size_t &offset)
	{
		const uint8_t v = static_cast<uint8_t> (s.at (offset));
		++offset;
		return v;
	}

	uint16_t BIN::be_to_uint16 (const std::string &s, size_t &offset)
	{
		#define BYTES 2

		if (offset + BYTES > s.size ()) {
			cThrow ("Not enough bytes");
		}
		_be_le_data = reinterpret_cast<const uint8_t*> (s.data ()) + offset;

		uint16_t v;

		ENDIAN_IS_BIG
			::memcpy (&v, _be_le_data, BYTES);
		ENDIAN_ELSE
			#define SAT(x) static_cast<uint16_t> (_be_le_data[x])
			v =	SAT(0) << 8 |
			SAT(1);
			#undef SAT
		ENDIAN_END

		offset += BYTES;

		#undef BYTES
		return v;
	}

	uint32_t BIN::be_to_uint24 (const std::string &s, size_t &offset)
	{
		#define BYTES 3

		if (offset + BYTES > s.size ()) {
			cThrow ("Not enough bytes");
		}
		_be_le_data = reinterpret_cast<const uint8_t*> (s.data ()) + offset;

		uint32_t v;

		#define SAT(x) static_cast<uint32_t> (_be_le_data[x])
		v = SAT(0) << 16 |
		SAT(1) << 8 |
		SAT(2);
		#undef SAT

		offset += BYTES;

		#undef BYTES
		return v;
	}

	uint32_t BIN::be_to_uint32 (const std::string &s, size_t &offset)
	{
		#define BYTES 4

		if (offset + BYTES > s.size ()) {
			cThrow ("Not enough bytes");
		}
		_be_le_data = reinterpret_cast<const uint8_t*> (s.data ()) + offset;

		uint32_t v;

		ENDIAN_IS_BIG
			::memcpy (&v, _be_le_data, BYTES);
		ENDIAN_ELSE
			#define SAT(x) static_cast<uint32_t> (_be_le_data[x])
			v = SAT(0) << 24 |
			SAT(1) << 16 |
			SAT(2) << 8 |
			SAT(3);
			#undef SAT
		ENDIAN_END

		offset += BYTES;

		#undef BYTES
		return v;
	}

	uint64_t BIN::be_to_uint64 (const std::string &s, size_t &offset)
	{
		#define BYTES 8

		if (offset + BYTES > s.size ()) {
			cThrow ("Not enough bytes");
		}
		_be_le_data = reinterpret_cast<const uint8_t*> (s.data ()) + offset;

		uint64_t v;

		ENDIAN_IS_BIG
			::memcpy (&v, _be_le_data, BYTES);
		ENDIAN_ELSE
			#define SAT(x) static_cast<uint64_t> (_be_le_data[x])
			v = SAT(0) << 56 |
			SAT(1) << 48 |
			SAT(2) << 40 |
			SAT(3) << 32 |

			SAT(4) << 24 |
			SAT(5) << 16 |
			SAT(6) << 8 |
			SAT(7);
			#undef SAT
		ENDIAN_END

		offset += BYTES;

		#undef BYTES
		return v;
	}

	uint8_t BIN::be_to_uint8 (const std::string &s)
	{
		size_t offset = 0;
		return be_to_uint8 (s, offset);
	}

	uint16_t BIN::be_to_uint16 (const std::string &s)
	{
		size_t offset = 0;
		return be_to_uint16 (s, offset);
	}

	uint32_t BIN::be_to_uint24 (const std::string &s)
	{
		size_t offset = 0;
		return be_to_uint24 (s, offset);
	}

	uint32_t BIN::be_to_uint32 (const std::string &s)
	{
		size_t offset = 0;
		return be_to_uint32 (s, offset);
	}

	uint64_t BIN::be_to_uint64 (const std::string &s)
	{
		size_t offset = 0;
		return be_to_uint64 (s, offset);
	}

	int8_t BIN::be_to_int8 (const std::string &s, size_t &offset)
	{
		uint8_t uv = be_to_uint8 (s, offset);
		int8_t v;
		memcpy (&v, &uv, sizeof (int8_t));
		return v;
	}

	int16_t BIN::be_to_int16 (const std::string &s, size_t &offset)
	{
		uint16_t uv = be_to_uint16 (s, offset);
		int16_t v;
		memcpy (&v, &uv, sizeof (int16_t));
		return v;
	}

	int32_t BIN::be_to_int32 (const std::string &s, size_t &offset)
	{
		uint32_t uv = be_to_uint32 (s, offset);
		int32_t v;
		memcpy (&v, &uv, sizeof (int32_t));
		return v;
	}

	int64_t BIN::be_to_int64 (const std::string &s, size_t &offset)
	{
		uint64_t uv = be_to_uint64 (s, offset);
		int64_t v;
		memcpy (&v, &uv, sizeof (int64_t));
		return v;
	}

	int8_t BIN::be_to_int8 (const std::string &s)
	{
		size_t offset = 0;
		return be_to_int8 (s, offset);
	}

	int16_t BIN::be_to_int16 (const std::string &s)
	{
		size_t offset = 0;
		return be_to_int16 (s, offset);
	}

	int32_t BIN::be_to_int32 (const std::string &s)
	{
		size_t offset = 0;
		return be_to_int32 (s, offset);
	}

	int64_t BIN::be_to_int64 (const std::string &s)
	{
		size_t offset = 0;
		return be_to_int64 (s, offset);
	}

	/*** little endian operations ***/

	void BIN::from_uint8 (const uint8_t v, std::string &s)
	{
		s.push_back (v);
	}

	void BIN::from_uint16 (const uint16_t v, std::string &s)
	{
		#define BYTES 2

		ENDIAN_IS_LITTLE
			::memcpy (_be_le_buf, &v, BYTES);
		ENDIAN_ELSE
			#define TAS(p,x) _be_le_buf[p] = ((v >> x) & 0xFF)
			TAS (0, 0);
			TAS (1, 8);
			#undef TAS
		ENDIAN_END

		s.append (reinterpret_cast<const char *> (_be_le_buf), BYTES);

		#undef BYTES
	}

	void BIN::from_uint24 (const uint32_t v, std::string &s)
	{
		#define BYTES 3

		#define TAS(p,x) _be_le_buf[p] = ((v >> x) & 0xFF)
		TAS (0, 0);
		TAS (1, 8);
		TAS (2, 16);
		#undef TAS

		s.append (reinterpret_cast<const char *> (_be_le_buf), BYTES);

		#undef BYTES
	}

	void BIN::from_uint32 (const uint32_t v, std::string &s)
	{
		#define BYTES 4

		ENDIAN_IS_LITTLE
			::memcpy (_be_le_buf, &v, BYTES);
		ENDIAN_ELSE
			#define TAS(p,x) _be_le_buf[p] = ((v >> x) & 0xFF)
			TAS (0, 0);
			TAS (1, 8);
			TAS (2, 16);
			TAS (3, 24);
			#undef TAS
		ENDIAN_END

		s.append (reinterpret_cast<const char *> (_be_le_buf), BYTES);

		#undef BYTES
	}

	void BIN::from_uint64 (const uint64_t v, std::string &s)
	{
		#define BYTES 8

		ENDIAN_IS_LITTLE
			::memcpy (_be_le_buf, &v, BYTES);
		ENDIAN_ELSE
			#define TAS(p,x) _be_le_buf[p] = ((v >> x) & 0xFF)
			TAS (0, 0);
			TAS (1, 8);
			TAS (2, 16);
			TAS (3, 24);

			TAS (4, 32);
			TAS (5, 40);
			TAS (6, 48);
			TAS (7, 56);
			#undef TAS
		ENDIAN_END

		s.append (reinterpret_cast<const char *> (_be_le_buf), BYTES);

		#undef BYTES
	}

	std::string BIN::from_uint8 (const uint8_t v)
	{
		std::string s;
		from_uint8 (v, s);
		return s;
	}

	std::string BIN::from_uint16 (const uint16_t v)
	{
		std::string s;
		from_uint16 (v, s);
		return s;
	}

	std::string BIN::from_uint24 (const uint32_t v)
	{
		std::string s;
		from_uint24 (v, s);
		return s;
	}

	std::string BIN::from_uint32 (const uint32_t v)
	{
		std::string s;
		from_uint32 (v, s);
		return s;
	}

	std::string BIN::from_uint64 (const uint64_t v)
	{
		std::string s;
		from_uint64 (v, s);
		return s;
	}

	void BIN::from_int8 (const int8_t v, std::string &s)
	{
		uint8_t uv;
		::memcpy (&uv, &v, sizeof (uint8_t));
		from_uint8 (uv, s);
	}

	void BIN::from_int16 (const int16_t v, std::string &s)
	{
		uint16_t uv;
		::memcpy (&uv, &v, sizeof (uint16_t));
		from_uint16 (uv, s);
	}

	void BIN::from_int32 (const int32_t v, std::string &s)
	{
		uint32_t uv;
		::memcpy (&uv, &v, sizeof (uint32_t));
		from_uint32 (uv, s);
	}

	void BIN::from_int64 (const int64_t v, std::string &s)
	{
		uint64_t uv;
		::memcpy (&uv, &v, sizeof (uint64_t));
		from_uint64 (uv, s);
	}

	std::string BIN::from_int8 (const int8_t v)
	{
		std::string s;
		from_int8 (v, s);
		return s;
	}

	std::string BIN::from_int16 (const int16_t v)
	{
		std::string s;
		from_int16 (v, s);
		return s;
	}

	std::string BIN::from_int32 (const int32_t v)
	{
		std::string s;
		from_int32 (v, s);
		return s;
	}

	std::string BIN::from_int64 (const int64_t v)
	{
		std::string s;
		from_int64 (v, s);
		return s;
	}

	uint8_t BIN::to_uint8 (const std::string &s, size_t &offset)
	{
		const uint8_t v = static_cast<uint8_t> (s.at (offset));
		++offset;
		return v;
	}

	uint16_t BIN::to_uint16 (const std::string &s, size_t &offset)
	{
		#define BYTES 2

		if (offset + BYTES > s.size ()) {
			cThrow ("Not enough bytes");
		}
		_be_le_data = reinterpret_cast<const uint8_t*> (s.data ()) + offset;

		uint16_t v;

		ENDIAN_IS_LITTLE
			::memcpy (&v, _be_le_data, BYTES);
		ENDIAN_ELSE
			#define SAT(x) static_cast<uint16_t> (_be_le_data[x])
			v = SAT(0) |
			SAT(1) << 8;
			#undef SAT
		ENDIAN_END

		offset += BYTES;

		#undef BYTES
		return v;
	}

	uint32_t BIN::to_uint24 (const std::string &s, size_t &offset)
	{
		#define BYTES 3

		if (offset + BYTES > s.size ()) {
			cThrow ("Not enough bytes");
		}
		_be_le_data = reinterpret_cast<const uint8_t*> (s.data ()) + offset;

		uint32_t v;

		#define SAT(x) static_cast<uint32_t> (_be_le_data[x])
		v = SAT(0) |
		SAT(1) << 8 |
		SAT(2) << 16;
		#undef SAT

		offset += BYTES;

		#undef BYTES
		return v;
	}

	uint32_t BIN::to_uint32 (const std::string &s, size_t &offset)
	{
		#define BYTES 4

		if (offset + BYTES > s.size ()) {
			cThrow ("Not enough bytes");
		}
		_be_le_data = reinterpret_cast<const uint8_t*> (s.data ()) + offset;

		uint32_t v;

		ENDIAN_IS_LITTLE
			::memcpy (&v, _be_le_data, BYTES);
		ENDIAN_ELSE
			#define SAT(x) static_cast<uint32_t> (_be_le_data[x])
			v = SAT(0) |
			SAT(1) << 8 |
			SAT(2) << 16 |
			SAT(3) << 24;
			#undef SAT
		ENDIAN_END

		offset += BYTES;

		#undef BYTES
		return v;
	}

	uint64_t BIN::to_uint64 (const std::string &s, size_t &offset)
	{
		#define BYTES 8

		if (offset + BYTES > s.size ()) {
			cThrow ("Not enough bytes");
		}
		_be_le_data = reinterpret_cast<const uint8_t*> (s.data ()) + offset;

		uint64_t v;

		ENDIAN_IS_LITTLE
			::memcpy (&v, _be_le_data, BYTES);
		ENDIAN_ELSE
			#define SAT(x) static_cast<uint64_t> (_be_le_data[x])
			v = SAT(0) |
			SAT(1) << 8 |
			SAT(2) << 16 |
			SAT(3) << 24 |

			SAT(4) << 32 |
			SAT(5) << 40 |
			SAT(6) << 48 |
			SAT(7) << 56;
			#undef SAT
		ENDIAN_END

		offset += BYTES;

		#undef BYTES
		return v;
	}

	uint8_t BIN::to_uint8 (const std::string &s)
	{
		size_t offset = 0;
		return to_uint8 (s, offset);
	}

	uint16_t BIN::to_uint16 (const std::string &s)
	{
		size_t offset = 0;
		return to_uint16 (s, offset);
	}

	uint32_t BIN::to_uint24 (const std::string &s)
	{
		size_t offset = 0;
		return to_uint24 (s, offset);
	}

	uint32_t BIN::to_uint32 (const std::string &s)
	{
		size_t offset = 0;
		return to_uint32 (s, offset);
	}

	uint64_t BIN::to_uint64 (const std::string &s)
	{
		size_t offset = 0;
		return to_uint64 (s, offset);
	}

	int8_t BIN::to_int8 (const std::string &s, size_t &offset)
	{
		uint8_t uv = to_uint8 (s, offset);
		int8_t v;
		::memcpy (&v, &uv, sizeof (int8_t));
		return v;
	}

	int16_t BIN::to_int16 (const std::string &s, size_t &offset)
	{
		uint16_t uv = to_uint16 (s, offset);
		int16_t v;
		::memcpy (&v, &uv, sizeof (int16_t));
		return v;
	}

	int32_t BIN::to_int32 (const std::string &s, size_t &offset)
	{
		uint32_t uv = to_uint32 (s, offset);
		int32_t v;
		::memcpy (&v, &uv, sizeof (int32_t));
		return v;
	}

	int64_t BIN::to_int64 (const std::string &s, size_t &offset)
	{
		uint64_t uv = to_uint64 (s, offset);
		int64_t v;
		::memcpy (&v, &uv, sizeof (int64_t));
		return v;
	}

	int8_t BIN::to_int8 (const std::string &s)
	{
		size_t offset = 0;
		return to_int8 (s, offset);
	}

	int16_t BIN::to_int16 (const std::string &s)
	{
		size_t offset = 0;
		return to_int16 (s, offset);
	}

	int32_t BIN::to_int32 (const std::string &s)
	{
		size_t offset = 0;
		return to_int32 (s, offset);
	}

	int64_t BIN::to_int64 (const std::string &s)
	{
		size_t offset = 0;
		return to_int64 (s, offset);
	}

	/*** size ***/

	size_t BIN::to_size (const std::string &s, size_t &offset)
	{
		size_t v = 0;

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
			cThrow ("Unable to encode size larger than 0xFFFFFFF");
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

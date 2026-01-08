/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_BINstatic
#define HEAD_shaga_BINstatic

#include "common.h"

namespace shaga::BIN {
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Big endian static inline functions  /////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static inline void _be_from_uint8 (const uint8_t val, void *const buf)
	{
		::memcpy (reinterpret_cast<char *>(buf), &val, sizeof (uint8_t));
	}

	static inline void _be_from_uint8 (const uint8_t val, void *const buf, size_t &pos)
	{
		::memcpy (reinterpret_cast<char *>(buf) + pos, &val, sizeof (uint8_t));
		pos += sizeof (uint8_t);
	}

	static inline void _be_from_uint16 (uint16_t val, void *const buf)
	{
		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP16 (val);
		ENDIAN_END

		::memcpy (reinterpret_cast<char *>(buf), &val, sizeof (uint16_t));
	}

	static inline void _be_from_uint16 (uint16_t val, void *const buf, size_t &pos)
	{
		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP16 (val);
		ENDIAN_END

		::memcpy (reinterpret_cast<char *>(buf) + pos, &val, sizeof (uint16_t));
		pos += sizeof (uint16_t);
	}

	static inline void _be_from_uint32 (uint32_t val, void *const buf)
	{
		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP32 (val);
		ENDIAN_END

		::memcpy (reinterpret_cast<char *>(buf), &val, sizeof (uint32_t));
	}

	static inline void _be_from_uint32 (uint32_t val, void *const buf, size_t &pos)
	{
		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP32 (val);
		ENDIAN_END

		::memcpy (reinterpret_cast<char *>(buf) + pos, &val, sizeof (uint32_t));
		pos += sizeof (uint32_t);
	}

	static inline void _be_from_uint64 (uint64_t val, void *const buf)
	{
		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP64 (val);
		ENDIAN_END

		::memcpy (reinterpret_cast<char *>(buf), &val, sizeof (uint64_t));
	}

	static inline void _be_from_uint64 (uint64_t val, void *const buf, size_t &pos)
	{
		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP64 (val);
		ENDIAN_END

		::memcpy (reinterpret_cast<char *>(buf) + pos, &val, sizeof (uint64_t));
		pos += sizeof (uint64_t);
	}

	static inline void _be_to_uint8 (uint8_t &val, const void *const buf, size_t &pos)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint8_t));
		pos += sizeof (uint8_t);
	}

	static inline uint8_t _be_to_uint8 (const void *const buf, size_t &pos)
	{
		uint8_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint8_t));
		pos += sizeof (uint8_t);

		return val;
	}

	static inline void _be_to_uint8 (uint8_t &val, const void *const buf)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint8_t));
	}

	static inline uint8_t _be_to_uint8 (const void *const buf)
	{
		uint8_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint8_t));

		return val;
	}

	static inline void _be_to_uint16 (uint16_t &val, const void *const buf, size_t &pos)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint16_t));
		pos += sizeof (uint16_t);

		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP16 (val);
		ENDIAN_END
	}

	static inline uint16_t _be_to_uint16 (const void *const buf, size_t &pos)
	{
		uint16_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint16_t));
		pos += sizeof (uint16_t);

		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP16 (val);
		ENDIAN_END

		return val;
	}

	static inline void _be_to_uint16 (uint16_t &val, const void *const buf)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint16_t));

		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP16 (val);
		ENDIAN_END
	}

	static inline uint16_t _be_to_uint16 (const void *const buf)
	{
		uint16_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint16_t));

		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP16 (val);
		ENDIAN_END

		return val;
	}

	static inline void _be_to_uint32 (uint32_t &val, const void *const buf, size_t &pos)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint32_t));
		pos += sizeof (uint32_t);

		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP32 (val);
		ENDIAN_END
	}

	static inline uint32_t _be_to_uint32 (const void *const buf, size_t &pos)
	{
		uint32_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint32_t));
		pos += sizeof (uint32_t);

		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP32 (val);
		ENDIAN_END

		return val;
	}

	static inline void _be_to_uint32 (uint32_t &val, const void *const buf)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint32_t));

		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP32 (val);
		ENDIAN_END
	}

	static inline uint32_t _be_to_uint32 (const void *const buf)
	{
		uint32_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint32_t));

		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP32 (val);
		ENDIAN_END

		return val;
	}

	static inline void _be_to_uint64 (uint64_t &val, const void *const buf, size_t &pos)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint64_t));
		pos += sizeof (uint64_t);

		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP64 (val);
		ENDIAN_END
	}

	static inline uint64_t _be_to_uint64 (const void *const buf, size_t &pos)
	{
		uint64_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint64_t));
		pos += sizeof (uint64_t);

		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP64 (val);
		ENDIAN_END

		return val;
	}

	static inline void _be_to_uint64 (uint64_t &val, const void *const buf)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint64_t));

		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP64 (val);
		ENDIAN_END
	}

	static inline uint64_t _be_to_uint64 (const void *const buf)
	{
		uint64_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint64_t));

		ENDIAN_IS_LITTLE
		ENDIAN_BSWAP64 (val);
		ENDIAN_END

		return val;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Little endian static inline functions  //////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static inline void _from_uint8 (const uint8_t val, void *const buf)
	{
		::memcpy (reinterpret_cast<char *>(buf), &val, sizeof (uint8_t));
	}

	static inline void _from_uint8 (const uint8_t val, void *const buf, size_t &pos)
	{
		::memcpy (reinterpret_cast<char *>(buf) + pos, &val, sizeof (uint8_t));
		pos += sizeof (uint8_t);
	}

	static inline void _from_uint16 (uint16_t val, void *const buf)
	{
		ENDIAN_IS_BIG
		ENDIAN_BSWAP16 (val);
		ENDIAN_END

		::memcpy (reinterpret_cast<char *>(buf), &val, sizeof (uint16_t));
	}

	static inline void _from_uint16 (uint16_t val, void *const buf, size_t &pos)
	{
		ENDIAN_IS_BIG
		ENDIAN_BSWAP16 (val);
		ENDIAN_END

		::memcpy (reinterpret_cast<char *>(buf) + pos, &val, sizeof (uint16_t));
		pos += sizeof (uint16_t);
	}

	static inline void _from_uint32 (uint32_t val, void *const buf)
	{
		ENDIAN_IS_BIG
		ENDIAN_BSWAP32 (val);
		ENDIAN_END

		::memcpy (reinterpret_cast<char *>(buf), &val, sizeof (uint32_t));
	}

	static inline void _from_uint32 (uint32_t val, void *const buf, size_t &pos)
	{
		ENDIAN_IS_BIG
		ENDIAN_BSWAP32 (val);
		ENDIAN_END

		::memcpy (reinterpret_cast<char *>(buf) + pos, &val, sizeof (uint32_t));
		pos += sizeof (uint32_t);
	}

	static inline void _from_uint64 (uint64_t val, void *const buf)
	{
		ENDIAN_IS_BIG
		ENDIAN_BSWAP64 (val);
		ENDIAN_END

		::memcpy (reinterpret_cast<char *>(buf), &val, sizeof (uint64_t));
	}

	static inline void _from_uint64 (uint64_t val, void *const buf, size_t &pos)
	{
		ENDIAN_IS_BIG
		ENDIAN_BSWAP64 (val);
		ENDIAN_END

		::memcpy (reinterpret_cast<char *>(buf) + pos, &val, sizeof (uint64_t));
		pos += sizeof (uint64_t);
	}

	static inline void _to_uint8 (uint8_t &val, const void *const buf, size_t &pos)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint8_t));
		pos += sizeof (uint8_t);
	}

	static inline uint8_t _to_uint8 (const void *const buf, size_t &pos)
	{
		uint8_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint8_t));
		pos += sizeof (uint8_t);

		return val;
	}

	static inline void _to_uint8 (uint8_t &val, const void *const buf)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint8_t));
	}

	static inline uint8_t _to_uint8 (const void *const buf)
	{
		uint8_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint8_t));

		return val;
	}

	static inline void _to_uint16 (uint16_t &val, const void *const buf, size_t &pos)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint16_t));
		pos += sizeof (uint16_t);

		ENDIAN_IS_BIG
		ENDIAN_BSWAP16 (val);
		ENDIAN_END
	}

	static inline uint16_t _to_uint16 (const void *const buf, size_t &pos)
	{
		uint16_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint16_t));
		pos += sizeof (uint16_t);

		ENDIAN_IS_BIG
		ENDIAN_BSWAP16 (val);
		ENDIAN_END

		return val;
	}

	static inline void _to_uint16 (uint16_t &val, const void *const buf)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint16_t));

		ENDIAN_IS_BIG
		ENDIAN_BSWAP16 (val);
		ENDIAN_END
	}

	static inline uint16_t _to_uint16 (const void *const buf)
	{
		uint16_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint16_t));

		ENDIAN_IS_BIG
		ENDIAN_BSWAP16 (val);
		ENDIAN_END

		return val;
	}

	static inline void _to_uint32 (uint32_t &val, const void *const buf, size_t &pos)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint32_t));
		pos += sizeof (uint32_t);

		ENDIAN_IS_BIG
		ENDIAN_BSWAP32 (val);
		ENDIAN_END
	}

	static inline uint32_t _to_uint32 (const void *const buf, size_t &pos)
	{
		uint32_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint32_t));
		pos += sizeof (uint32_t);

		ENDIAN_IS_BIG
		ENDIAN_BSWAP32 (val);
		ENDIAN_END

		return val;
	}

	static inline void _to_uint32 (uint32_t &val, const void *const buf)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint32_t));

		ENDIAN_IS_BIG
		ENDIAN_BSWAP32 (val);
		ENDIAN_END
	}

	static inline uint32_t _to_uint32 (const void *const buf)
	{
		uint32_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint32_t));

		ENDIAN_IS_BIG
		ENDIAN_BSWAP32 (val);
		ENDIAN_END

		return val;
	}

	static inline void _to_uint64 (uint64_t &val, const void *const buf, size_t &pos)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint64_t));
		pos += sizeof (uint64_t);

		ENDIAN_IS_BIG
		ENDIAN_BSWAP64 (val);
		ENDIAN_END
	}

	static inline uint64_t _to_uint64 (const void *const buf, size_t &pos)
	{
		uint64_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf) + pos, sizeof (uint64_t));
		pos += sizeof (uint64_t);

		ENDIAN_IS_BIG
		ENDIAN_BSWAP64 (val);
		ENDIAN_END

		return val;
	}

	static inline void _to_uint64 (uint64_t &val, const void *const buf)
	{
		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint64_t));

		ENDIAN_IS_BIG
		ENDIAN_BSWAP64 (val);
		ENDIAN_END
	}

	static inline uint64_t _to_uint64 (const void *const buf)
	{
		uint64_t val;

		::memcpy (&val, reinterpret_cast<const char *>(buf), sizeof (uint64_t));

		ENDIAN_IS_BIG
		ENDIAN_BSWAP64 (val);
		ENDIAN_END

		return val;
	}
}

#endif // HEAD_shaga_BINstatic

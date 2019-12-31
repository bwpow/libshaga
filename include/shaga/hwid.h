/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_HWID
#define HEAD_shaga_HWID

#include "common.h"

namespace shaga {
	typedef uint32_t HWID;
	#define HWID_MAX UINT32_MAX

	#define bin_be_from_hwid BIN::be_from_uint32
	#define bin_be_to_hwid BIN::be_to_uint32
	#define bin_from_hwid BIN::from_uint32
	#define bin_to_hwid BIN::to_uint32

	#define str_to_hwid STR::to_uint32
	#define str_from_hwid STR::from_int

	struct HWIDMASK {
		HWID hwid;
		HWID mask;

		constexpr HWIDMASK (const HWID _hwid, const HWID _mask) : hwid (_hwid), mask (_mask) {}
		constexpr HWIDMASK (const HWID _hwid) : HWIDMASK (_hwid, HWID_MAX) {}
		constexpr HWIDMASK () : HWIDMASK (0, 0) {}

		constexpr bool operator== (const HWIDMASK &other) const
		{
			return (mask != other.mask) ? false : (hwid & mask) == (other.hwid & other.mask);
		}

		constexpr bool operator< (const HWIDMASK &other) const
		{
			return (mask < other.mask) ? true :	((mask > other.mask) ? false : (hwid & mask) < (other.hwid & other.mask));
		}

		constexpr bool operator> (const HWIDMASK &other) const
		{
			return (mask > other.mask) ? true :	((mask < other.mask) ? false : (hwid & mask) > (other.hwid & other.mask));
		}

		constexpr bool check (const HWID other_hwid) const
		{
			return (other_hwid & mask) == (hwid & mask);
		}

		constexpr bool empty (void) const
		{
			return (hwid == 0 && mask == 0);
		}
	};

	typedef std::list<HWID> HWID_LIST;
	typedef std::set<HWID> HWID_SET;
	typedef std::vector<HWID> HWID_VECTOR;
	typedef std::deque<HWID> HWID_DEQUE;

	inline static std::string hwid_describe (const HWIDMASK &hwidmask)
	{
		if (0 == hwidmask.mask) {
			if (0 == hwidmask.hwid) {
				return "broadcast"s;
			}
			else {
				return fmt::format ("{:08X}"sv, hwidmask.hwid);
			}
		}
		else {
			return fmt::format ("{:08X}/{:08X}"sv, hwidmask.hwid, hwidmask.mask);
		}
	}

	inline static std::string hwid_describe (const HWID hwid)
	{
		return fmt::format ("{:08X}"sv, hwid);
	}
}

#endif // HEAD_shaga_HWID

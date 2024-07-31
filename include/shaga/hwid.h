/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2024, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_HWID
#define HEAD_shaga_HWID

#include "common.h"

namespace shaga {
	typedef uint32_t HWID;
	#define HWID_MAX UINT32_MAX

	#define HWID_UNKNOWN 0
	#define HWID_PLACEHOLDER HWID_MAX

	#define bin_be_from_hwid BIN::be_from_uint32
	#define bin_be_to_hwid BIN::be_to_uint32
	#define bin_from_hwid BIN::from_uint32
	#define bin_to_hwid BIN::to_uint32

	#define _bin_be_from_hwid BIN::_be_from_uint32
	#define _bin_be_to_hwid BIN::_be_to_uint32
	#define _bin_from_hwid BIN::_from_uint32
	#define _bin_to_hwid BIN::_to_uint32

	#define str_to_hwid STR::to_uint32
	#define str_from_hwid STR::from_int

	struct HWIDMASK
	{
		HWID hwid {0};
		HWID mask {0};

		constexpr HWIDMASK (const HWID _hwid, const HWID _mask) :
			hwid (_hwid & _mask),
			mask (_mask)
		{}

		constexpr HWIDMASK (const HWID _hwid) : HWIDMASK (_hwid, HWID_MAX) {}
		constexpr HWIDMASK () {}

		constexpr bool operator== (const HWIDMASK &other) const
		{
			return ((mask == other.mask) && ((hwid & mask) == (other.hwid & other.mask)));
		}

		constexpr bool operator< (const HWIDMASK &other) const
		{
			return (mask < other.mask) ?
				true
				:
				((mask > other.mask) ?
					false
					:
					((hwid & mask) < (other.hwid & other.mask))
				);
		}

		constexpr bool operator> (const HWIDMASK &other) const
		{
			return (mask > other.mask) ?
				true
				:
				((mask < other.mask) ?
					false
					:
					((hwid & mask) > (other.hwid & other.mask))
				);
		}

		constexpr bool check (const HWID other_hwid) const
		{
			return ((other_hwid & mask) == (hwid & mask));
		}

		constexpr bool check_exact (const HWID other_hwid) const
		{
			return (true == is_unicast () && (other_hwid == hwid));
		}

		constexpr bool empty (void) const
		{
			return ((0 == hwid) && (0 == mask));
		}

		constexpr bool is_broadcast (void) const
		{
			return (0 == mask);
		}

		constexpr bool is_unicast (void) const
		{
			return (HWID_MAX == mask);
		}

		static constexpr bool is_placeholder (const HWID _hwid)
		{
			return (_hwid == HWID_PLACEHOLDER);
		}

		static constexpr bool is_unknown (const HWID _hwid)
		{
			return (_hwid == HWID_UNKNOWN);
		}
	};

	typedef std::list<HWID> HWID_LIST;
	typedef std::set<HWID> HWID_SET;
	typedef std::vector<HWID> HWID_VECTOR;
	typedef std::deque<HWID> HWID_DEQUE;

	inline static std::string hwid_describe (const HWIDMASK &hwidmask)
	{
		if (hwidmask.is_broadcast ()) {
			return "broadcast"s;
		}
		else if (hwidmask.is_unicast ()) {
			return fmt::format ("{:08X}"sv, hwidmask.hwid);
		}
		else {
			return fmt::format ("{:08X}/{:08X}"sv, (hwidmask.hwid & hwidmask.mask), hwidmask.mask);
		}
	}

	inline static std::string hwid_describe (const HWID hwid)
	{
		if (HWIDMASK::is_placeholder (hwid)) {
			return "placeholder"s;
		}
		else if (HWIDMASK::is_unknown (hwid)) {
			return "unknown/unset"s;
		}
		else {
			return fmt::format ("{:08X}"sv, hwid);
		}
	}
}

#endif // HEAD_shaga_HWID

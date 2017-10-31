/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_common
#define HEAD_shaga_common

#if (!defined (__cplusplus)) || (__cplusplus < 201402L)
	#error At least C++14 compiler required.
#endif

#ifndef SHAGA
	#error Must be included from shaga.h or shagalite.h
#endif // SHAGA

#ifndef _POSIX_C_SOURCE
	#define _POSIX_C_SOURCE
#endif // _POSIX_C_SOURCE

#ifndef __USE_POSIX
	#define __USE_POSIX
#endif // __USE_POSIX

#ifndef __STDC_FORMAT_MACROS
	#define __STDC_FORMAT_MACROS
#endif // __STDC_FORMAT_MACROS

#define _FILE_OFFSET_BITS 64

/* Detection based upon: http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system */
#undef OS_WIN
#undef OS_LINUX
#undef OS_MAC

#if defined (_WIN32) || defined (__MINGW32__) || defined (__CYGWIN__)

	#define OS_WIN

	#ifdef WINVER
		#undef WINVER
	#endif // WINVER
	#define WINVER 0x0600

	#ifdef _WIN32_WINNT
		#undef _WIN32_WINNT
	#endif // _WIN32_WINNT
	#define _WIN32_WINNT 0x0600

	#include <winsock2.h>
	#include <windows.h>

#elif defined (__linux__)

	#define OS_LINUX

#elif defined (__APPLE__) && defined (__MACH__)

	#include <TargetConditionals.h>
	#if TARGET_OS_MAC == 1
		#define OS_MAC
	#else
		#error Mobile Apple OS are not supported
	#endif // TARGET_OS_MAC

#else
	#error Unsupported OS
#endif // OS detection

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cinttypes>
#include <climits>

#include <new>
#include <vector>
#include <string>
#include <deque>
#include <queue>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <functional>

#include <random>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <condition_variable>

#ifdef OS_LINUX
	#include <sys/eventfd.h>
	#include <unistd.h>
#endif // OS_LINUX

#ifndef INT64_C
	#define INT64_C(c) (c ## LL)
#endif

#ifndef UINT64_C
	#define UINT64_C(c) (c ## ULL)
#endif

namespace shaga
{
	typedef std::vector< std::string > COMMON_VECTOR;
	typedef std::map< std::string, std::string > COMMON_MAP;
	typedef std::multimap< std::string, std::string > COMMON_MULTIMAP;
	typedef std::set< std::string > COMMON_SET;
	typedef std::list< std::string > COMMON_LIST;
	typedef std::deque< std::string > COMMON_DEQUE;

	typedef std::vector <COMMON_VECTOR> COMMON_COMMON_VECTOR;

	class CommonException: public std::exception
	{
		private:
			char _text[1024];
			size_t _info_pos;
		public:
			CommonException (const bool log, const char *str_file, const char *str_function, int str_line, const char *fmt, ...) throw ();
			const char *what () const throw ();
			const char *debugwhat () const throw ();
	};

	#define cLogThrow(format, ...) throw shaga::CommonException(true, __FILE__, __PRETTY_FUNCTION__, __LINE__, format, ##__VA_ARGS__)
	#define cThrow(format, ...) throw shaga::CommonException(false, __FILE__, __PRETTY_FUNCTION__, __LINE__, format, ##__VA_ARGS__)

	//#define cEnter P::debug_printf ("FUNC ENTER %s line %d", __PRETTY_FUNCTION__, __LINE__)
	//#define cLeave P::debug_printf ("FUNC LEAVE %s line %d", __PRETTY_FUNCTION__, __LINE__)

	void add_at_exit_callback (std::function<void (void)> func);
	[[noreturn]] void exit (const int rcode, const char *fmt, ...);
	[[noreturn]] void exit (const char *fmt, ...);
	[[noreturn]] void exit (const int rcode);
	[[noreturn]] void exit_failure (void);
	[[noreturn]] void exit (void);

	typedef std::function<void (const char *, const int)> FINAL_CALL;
	void set_final_call (FINAL_CALL func);

	#define TRY_TO_SHUTDOWN() _try_to_shutdown(__FILE__, __PRETTY_FUNCTION__, __LINE__)
	void add_at_shutdown_callback (std::function<void (void)> func);
	void _try_to_shutdown (const char *file, const char *funct, const int line);
	bool is_shutting_down (void);

	#define RELOG_REGISTER

	#define SHAGA_MAIN(a) int main (int argc, char **argv) try { \
		(void) argc; \
		(void) argv; \
		BIN::endian_detect (); \
		RELOG_REGISTER; \
		a \
		shaga::exit (); \
		cThrow ("Fell over the edge of the world"); \
		return 127; \
	} \
	catch (const std::exception &e) { \
		shaga::exit ("FATAL ERROR: %s", e.what ()); \
	} \
	catch (...) { \
		shaga::exit ("FATAL ERROR: Unknown failure"); \
	} \


	int64_t timeval_diff_msec (const struct timeval &starttime, const struct timeval &finishtime);
	int64_t timespec_diff_msec (const struct timespec &starttime, const struct timespec &finishtime);

	uint64_t get_monotime_sec (void);
	uint64_t get_monotime_msec (void);
	uint64_t get_monotime_usec (void);

	uint64_t get_realtime_sec (void);
	uint64_t get_realtime_msec (void);
	uint64_t get_realtime_usec (void);
}

#include "hwid.h"
#include "CRC.h"
#include "STR.h"
#include "P.h"
#include "ShFile.h"
#include "FS.h"
#include "BIN.h"
#include "ChunkMeta.h"
#include "Chunk.h"
#include "ReData.h"
#include "INI.h"
#include "UNIX.h"
#include "LINUX.h"
#include "WIN.h"
#include "ArgTable.h"
#include "Semaphore.h"
#include "IPHelper.h"
#include "SPSC.h"
#include "SPSCData.h"
#include "EncodeSPSC.h"
#include "DecodeSPSC.h"
#include "DiSig.h"
#include "aho_corasick.h"

namespace shaga
{
	template <typename T>
	void container_from_bin (T &out, const std::string &buf, size_t &offset)
	{
		out.clear ();

		if (buf.size () == offset) {
			return;
		}

		const size_t num = shaga::BIN::to_size (buf, offset);

		out.reserve (num + 128);

		for (size_t i = 0; i < num; ++i) {
			out.emplace_back (buf, offset);
		}
	}

	template <typename T>
	void container_from_bin (T &out, const std::string &buf)
	{
		size_t offset = 0;
		container_from_bin (out, buf, offset);

		if (offset != buf.size ()) {
			cThrow ("Extra data at the end of buffer");
		}
	}

	template <typename T>
	void container_from_ini (T &out, const shaga::INI &ini, const std::string &section, const std::string &key)
	{
		out.clear ();

		const std::string buf = shaga::BIN::from_hex (ini.get_string (section, key));
		container_from_bin (out, buf);
	}

	template <typename T>
	void container_to_bin (const T &input, std::string &out)
	{
		shaga::BIN::from_size (input.size (), out);
		for (const auto &val : input) {
			val.to_bin (out);
		}
	}

	template <typename T>
	std::string container_to_bin (const T &input)
	{
		std::string out;
		container_to_bin (input, out);
		return out;
	}

	template <typename T>
	void container_to_ini (const T &input, shaga::INI &ini, const std::string &section, const std::string &key)
	{
		if (input.empty () == true) {
			return;
		}

		std::string buf = container_to_bin (input);
		ini.set_string (section, key, shaga::BIN::to_hex (buf), false);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static asserts  /////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static_assert (('Z' - 'A') == 25, "Expected 26 letters between 'A' and 'Z'");
	static_assert (8 == CHAR_BIT, "Expected char to be 8 bits");
	static_assert (1 == sizeof (char), "Expected char to be 8 bits");

	/*** Chunk ***/
	static_assert (std::underlying_type<Chunk::Priority>::type (Chunk::_Priority_first) < std::underlying_type<Chunk::Priority>::type (Chunk::_Priority_last),
		"Chunk::Priority must begin before end");

	static_assert (std::underlying_type<Chunk::Priority>::type (Chunk::_Priority_last) <= (Chunk::key_prio_mask >> Chunk::key_prio_shift),
		"Chunk::Priority must fit into bitmask");

	static_assert (std::underlying_type<Chunk::TrustLevel>::type (Chunk::_TrustLevel_first) < std::underlying_type<Chunk::TrustLevel>::type (Chunk::_TrustLevel_last),
		"Chunk::TrustLevel must begin before end");

	static_assert (std::underlying_type<Chunk::TrustLevel>::type (Chunk::_TrustLevel_last) <= (Chunk::key_trust_mask >> Chunk::key_trust_shift),
		"Chunk::TrustLevel must fit into bitmask");

	static_assert (BIN::check_bit_coverage (UINT32_MAX, Chunk::key_type_mask, Chunk::key_is_tracert_mask, Chunk::key_trust_mask,
					Chunk::key_prio_mask, Chunk::key_ttl_mask, Chunk::key_has_payload_mask, Chunk::key_has_dest_mask,
					Chunk::key_channel_mask, Chunk::key_reserved_mask, Chunk::key_highbit_mask), "Chunk key don't add up or overlap");

	static_assert ((Chunk::key_type_min < Chunk::key_type_max) && (Chunk::key_type_max <= Chunk::key_type_mask), "Chunk type key doesn't fit mask");

	static_assert ((Chunk::key_tracert_hop_shift >= 16) && ((Chunk::key_is_tracert_mask & 0xffff) == 0), "Tracert hop shift and mask must be defined in high 16-bits");

	/*** ChunkMeta ***/
	static_assert ((ChunkMeta::key_type_min < ChunkMeta::key_type_max) && (ChunkMeta::key_type_max <= ChunkMeta::key_type_mask), "Chunkmeta type key doesn't fit mask");

	static_assert (ChunkMeta::key_repeat_mask == (ChunkMeta::key_repeat_byte << 8), "ChunkMeta key repeat byte and mask does not match");

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Most used templates  ////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	extern template class SPSC<std::string>;

	extern template class UartEncodeSPSC<SPSCDataPreAlloc>;
	extern template class UartDecodeSPSC<SPSCDataPreAlloc>;

	extern template class PacketEncodeSPSC<SPSCDataDynAlloc>;
	extern template class PacketDecodeSPSC<SPSCDataDynAlloc>;

	extern template class SeqPacketEncodeSPSC<SPSCDataDynAlloc>;
	extern template class SeqPacketDecodeSPSC<SPSCDataDynAlloc>;
}

#endif // HEAD_shaga

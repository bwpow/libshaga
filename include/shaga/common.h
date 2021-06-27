/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2021, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_common
#define HEAD_shaga_common

#if (!defined (__cplusplus)) || (__cplusplus < 201703L)
	#error At least C++17 compiler required.
#endif

#if (defined(Q_CC_GNU) && Q_CC_GNU >= 408) || (defined(Q_CC_CLANG) && Q_CC_CLANG >= 302)
	#error Only GCC and Clang compilers are supported in this version.
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

/* If you want to use single thread optimalizations during compile time, define SHAGA_SINGLE_THREAD before including shaga*.h */
#if defined(SHAGA_MULTI_THREAD) && defined(SHAGA_SINGLE_THREAD)
	#error Both SHAGA_MULTI_THREAD and SHAGA_SINGLE_THREAD defined
#endif // defined

#ifdef SHAGA_THREADING
	#undef SHAGA_THREADING
#endif // SHAGA_THREADING

#ifdef SHAGA_MULTI_THREAD
	#define SHAGA_THREADING
#endif // SHAGA_MULTI_THREAD

/* Detection based upon: http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system */
#undef OS_WIN
#undef OS_LINUX
#undef OS_MAC

#if defined (_WIN32) || defined (_WIN64) || defined (__MINGW32__) || defined (__MINGW64__) || defined (__CYGWIN__)

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

#include "3rdparty/hedley.h"

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
#include <memory>
#include <optional>
#include <any>

#include <random>
#include <chrono>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef SHAGA_THREADING
	#include <thread>
	#include <mutex>
	#include <atomic>
	#include <condition_variable>
#endif // SHAGA_THREADING

#ifdef OS_LINUX
	#include <sys/eventfd.h>
	#include <unistd.h>
	#include <sched.h>
#endif // OS_LINUX

#ifdef SHAGA_FULL
	/* Used in ReData and CRC */
	#include <mbedtls/cipher.h>
	#include <mbedtls/aes.h>
	#include <mbedtls/chacha20.h>
	#include <mbedtls/chachapoly.h>

	#include <mbedtls/sha256.h>
	#include <mbedtls/sha512.h>

	/* Used in DiSig */
	#include <mbedtls/pk.h>
	#include <mbedtls/md_internal.h>

	#include <mbedtls/error.h>

	#if !defined(MBEDTLS_CIPHER_MODE_CBC) || \
		!defined(MBEDTLS_AES_C) || \
		!defined(MBEDTLS_CHACHA20_C) || \
		!defined(MBEDTLS_CHACHAPOLY_C) || \
		!defined(MBEDTLS_SHA256_C) || \
		!defined(MBEDTLS_SHA512_C) || \
		!defined(MBEDTLS_MD_C) || \
		!defined(MBEDTLS_ECP_C) || \
		!defined(MBEDTLS_PK_C) || \
		!defined(MBEDTLS_ERROR_C) || \
		!defined(MBEDTLS_VERSION_C)
		#error Missing required component in mbed TLS
	#endif
#endif // SHAGA_FULL

/* FMT is used for string formatting */
/* https://github.com/fmtlib/fmt */
#define FMT_HEADER_ONLY
#include "3rdparty/fmt/format.h"

/* Use literals for ""s and ""sv */
using namespace std::literals;

#ifndef INT64_C
	#define INT64_C(c) (c ## LL)
#endif

#ifndef UINT64_C
	#define UINT64_C(c) (c ## ULL)
#endif

/* Detect endian */
#if __has_include(<endian.h>)
	#include <endian.h>
#elif defined(_WIN32) || defined(_WIN64)
	#define BYTE_ORDER __LITTLE_ENDIAN
#endif

#ifndef LITTLE_ENDIAN
	#define LITTLE_ENDIAN __LITTLE_ENDIAN
#endif

#ifndef BIG_ENDIAN
	#define BIG_ENDIAN __BIG_ENDIAN
#endif

#if BYTE_ORDER == LITTLE_ENDIAN
	#define ENDIAN_IS_BIG if (false) {
	#define ENDIAN_IS_LITTLE if (true) {
	#define ENDIAN_END }
#elif BYTE_ORDER == BIG_ENDIAN
	#define ENDIAN_IS_BIG if (true) {
	#define ENDIAN_IS_LITTLE if (false) {
	#define ENDIAN_END }
#else
	#error Unable to detect endian
#endif

#define ENDIAN_BSWAP16(val) (val) = __builtin_bswap16 (val)
#define ENDIAN_BSWAP32(val) (val) = __builtin_bswap32 (val)
#define ENDIAN_BSWAP64(val) (val) = __builtin_bswap64 (val)

#define _SHAGA_SPSC_RING(x,y) const uint_fast32_t x = ((y) + 1) % _size
#define _SHAGA_SPSC_D_RING(x,y) const uint_fast32_t x = ((y) + 1) % this->_num_packets
#define _SHAGA_SPSC_I_RING(x) x = (x + 1) % this->_num_packets

/* This macro is used to identify methods and functions that return std::string view */
/* Useful for static analysis to point out potentially dangerous methods. */
#ifndef SHAGA_STRV
	#define SHAGA_STRV
#endif // SHAGA_STRV

namespace shaga
{
	typedef std::map <std::string, std::string> COMMON_MAP;
	typedef std::multimap <std::string, std::string> COMMON_MULTIMAP;

	typedef std::vector <std::string> COMMON_VECTOR;
	typedef std::set <std::string> COMMON_SET;
	typedef std::list <std::string> COMMON_LIST;
	typedef std::deque <std::string> COMMON_DEQUE;

	typedef std::vector <COMMON_VECTOR> COMMON_COMMON_VECTOR;

	typedef std::vector <std::string_view> VIEW_VECTOR;
	typedef std::set <std::string_view> VIEW_SET;
	typedef std::list <std::string_view> VIEW_LIST;
	typedef std::deque <std::string_view> VIEW_DEQUE;
}

/* Needed first for P::_printf used in CommonException */
#include "STR.h"
#include "P.h"

namespace shaga
{
	/* Shutdown and exit handling */
	void add_at_exit_callback (std::function<void (void)> func);
	HEDLEY_NO_RETURN void _exit (const std::string_view text = ""sv, const int rcode = EXIT_FAILURE, const std::string_view prefix = ""sv) noexcept;
	HEDLEY_NO_RETURN void exit (const int rcode) noexcept;
	HEDLEY_NO_RETURN void exit_failure (void) noexcept;
	HEDLEY_NO_RETURN void exit (void) noexcept;

	template <typename... Args>
	HEDLEY_NO_RETURN void exit (const int rcode, const std::string_view format, const Args & ... args) noexcept
	{
		if (sizeof...(Args) == 0) {
			shaga::_exit (format, rcode);
		}
		try {
			shaga::_exit (fmt::format (format, args...), rcode);
		}
		catch (...) {
			shaga::_exit (format, rcode);
		}
	}

	template <typename... Args>
	HEDLEY_NO_RETURN void exit (const std::string_view format, const Args & ... args) noexcept
	{
		if (sizeof...(Args) == 0) {
			shaga::_exit (format, EXIT_FAILURE);
		}
		try {
			shaga::_exit (fmt::format (format, args...), EXIT_FAILURE);
		}
		catch (...) {
			shaga::_exit (format, EXIT_FAILURE);
		}
}

	typedef std::function<void (const std::string_view, const int)> FINAL_CALL;
	void set_final_call (FINAL_CALL func);

	#define TRY_TO_SHUTDOWN() _try_to_shutdown(__FILE__, __PRETTY_FUNCTION__, __LINE__)
	void add_at_shutdown_callback (std::function<void (void)> func);
	void _try_to_shutdown (const char *file, const char *funct, const int line);
	HEDLEY_WARN_UNUSED_RESULT bool is_shutting_down (void);

	/* Set in common.cpp to constant value depending on compile-time settings */
	extern const bool _shaga_compiled_little_endian;
	extern const bool _shaga_compiled_with_threading;
	extern const bool _shaga_compiled_full;

	/* Call from main to check if compile time settings match runtime settings */
	inline static void shaga_check (void)
	{
		#if defined SHAGA_MULTI_THREAD
			if (false == _shaga_compiled_with_threading) {
				cThrow ("Threading mismatch, compiled without threading and used with threading"sv);
			}
		#elif defined SHAGA_SINGLE_THREAD
			if (true == _shaga_compiled_with_threading) {
				cThrow ("Threading mismatch, compiled with threading and used without threading"sv);
			}
		#else
			#error Unable to detect version of the library
		#endif

		#if defined SHAGA_LITE
			if (true == _shaga_compiled_full) {
				cThrow ("Library version mismatch, compiled as full and used as lite"sv);
			}
		#elif defined SHAGA_FULL
			if (false == _shaga_compiled_full) {
				cThrow ("Library version mismatch, compiled as lite and used as full"sv);
			}
		#else
			#error Unable to detect version of the library
		#endif

		union {
			uint32_t i;
			char c[4];
		} testvar = {0x12345678};

		#if BYTE_ORDER == LITTLE_ENDIAN
			if (false == _shaga_compiled_little_endian || (testvar.c[0] != 0x78)) {
				cThrow ("Library version mismatch, compiled with little endian and used on big endian system"sv);
			}
		#elif BYTE_ORDER == BIG_ENDIAN
			if (true == _shaga_compiled_little_endian || (testvar.c[0] != 0x12)) {
				cThrow ("Library version mismatch, compiled with big endian and used on little endian system"sv);
			}
		#else
			#error Unable to detect version of the library
		#endif
	}

	#define LOG_REGISTER

	#define SHAGA_MAIN(body) int main ([[maybe_unused]] int argc, [[maybe_unused]] char **argv) try \
	{ \
		{ shaga_check (); LOG_REGISTER; } \
		{ body } \
		shaga::exit (); \
	} \
	catch (const std::exception &e) { \
		shaga::_exit (e.what (), EXIT_FAILURE, "FATAL ERROR: "sv); \
	} \
	catch (...) { \
		shaga::_exit ("Unknown failure"sv, EXIT_FAILURE, "FATAL ERROR: "sv); \
	}

	HEDLEY_WARN_UNUSED_RESULT int64_t timeval_diff_msec (const struct timeval &starttime, const struct timeval &finishtime);
	HEDLEY_WARN_UNUSED_RESULT int64_t timespec_diff_msec (const struct timespec &starttime, const struct timespec &finishtime);

	HEDLEY_WARN_UNUSED_RESULT uint64_t get_monotime_sec (void);
	HEDLEY_WARN_UNUSED_RESULT uint64_t get_monotime_msec (void);
	HEDLEY_WARN_UNUSED_RESULT uint64_t get_monotime_usec (void);

	HEDLEY_WARN_UNUSED_RESULT uint64_t get_realtime_sec (void);
	HEDLEY_WARN_UNUSED_RESULT uint64_t get_realtime_msec (void);
	HEDLEY_WARN_UNUSED_RESULT uint64_t get_realtime_usec (void);

	typedef struct {
		uint_fast16_t year;
		uint_fast8_t month;
		uint_fast8_t day;
		uint_fast8_t hour;
		uint_fast8_t minute;
		uint_fast8_t second;
	} SHAGA_PARSED_REALTIME;

	HEDLEY_WARN_UNUSED_RESULT SHAGA_PARSED_REALTIME get_realtime_parsed (const time_t theTime, const bool local);
	HEDLEY_WARN_UNUSED_RESULT SHAGA_PARSED_REALTIME get_realtime_parsed (const bool local);

	HEDLEY_WARN_UNUSED_RESULT time_t get_realtime_sec_shifted (const time_t shift_start);
}

#include "3rdparty/aho_corasick.h"
#include "3rdparty/randutils.h"
#include "3rdparty/json.hpp"

#include "BINstatic.h"
#include "BIN.h"
#include "hwid.h"
#include "CRC.h"
#include "Digest.h"
#include "DiSig.h"
#include "ShSocket.h"
#include "ShFile.h"
#include "FS.h"
#include "ChunkMeta.h"
#include "Chunk.h"
#include "ChunkTool.h"
#include "ReData.h"
#include "INI.h"
#include "UNIX.h"
#include "LINUX.h"
#include "WIN.h"
#include "ArgTable.h"
#include "Semaphore.h"
#include "IPHelper.h"

#include "SPSC.h"
namespace shaga
{
	typedef SPSC<std::string> StringSPSC;
}
#include "PreAllocSPSC.h"
#include "SPSCData.h"
#include "EncodeSPSC.h"
#include "DecodeSPSC.h"

namespace shaga
{
	template <typename T>
	void container_from_bin (T &out, const std::string_view buf, size_t &offset)
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
	void container_from_bin (T &out, const std::string_view buf)
	{
		size_t offset = 0;
		container_from_bin (out, buf, offset);

		if (offset != buf.size ()) {
			cThrow ("Extra data at the end of buffer"sv);
		}
	}

	template <typename T>
	void container_from_ini (T &out, const shaga::INI &ini, const std::string_view section, const std::string_view key)
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
	void container_to_ini (const T &input, shaga::INI &ini, const std::string_view section, const std::string_view key)
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

	static_assert (('Z' - 'A' + 1) == 26, "Expected 26 letters between (and including) 'A' and 'Z'");

	static_assert (8 == CHAR_BIT, "Expected char to be 8 bits");
	static_assert (1 == sizeof (char), "Expected char to be 8 bits");

	static_assert (1 == sizeof (uint8_t), "Expected uint8_t to be 8 bits");
	static_assert (2 == sizeof (uint16_t), "Expected uint16_t to be 16 bits");
	static_assert (4 == sizeof (uint32_t), "Expected uint32_t to be 32 bits");
	static_assert (8 == sizeof (uint64_t), "Expected uint64_t to be 64 bits");

	static_assert (1 <= sizeof (uint_fast8_t), "Expected uint_fast8_t to be at least 8 bits");
	static_assert (2 <= sizeof (uint_fast16_t), "Expected uint_fast16_t to be at least 16 bits");
	static_assert (4 <= sizeof (uint_fast32_t), "Expected uint_fast32_t to be at least 32 bits");
	static_assert (8 <= sizeof (uint_fast64_t), "Expected uint_fast64_t to be at least 64 bits");

	static_assert (8 == sizeof (off64_t), "Expected off64_t to be 64 bits");
	static_assert (sizeof (off_t) == sizeof (off64_t), "Expected off_t to be same size as off64_t");

	static_assert (4 == sizeof (time_t) || 8 == sizeof (time_t), "Expected time_t to be 32 or 64 bits");

	/*** Chunk ***/
	static_assert (std::underlying_type<Chunk::Priority>::type (Chunk::_Priority_first) < std::underlying_type<Chunk::Priority>::type (Chunk::_Priority_last),
		"Chunk::Priority must begin before end");

	static_assert (std::underlying_type<Chunk::Priority>::type (Chunk::_Priority_last) <= (Chunk::key_prio_mask >> Chunk::key_prio_shift),
		"Chunk::Priority must fit into bitmask");

	static_assert (std::underlying_type<Chunk::TrustLevel>::type (Chunk::_TrustLevel_first) < std::underlying_type<Chunk::TrustLevel>::type (Chunk::_TrustLevel_last),
		"Chunk::TrustLevel must begin before end");

	static_assert (std::underlying_type<Chunk::TrustLevel>::type (Chunk::_TrustLevel_last) <= (Chunk::key_trust_mask >> Chunk::key_trust_shift),
		"Chunk::TrustLevel must fit into bitmask");

	static_assert (BIN::check_bit_coverage (UINT32_MAX,
		Chunk::key_type_mask,
		Chunk::key_special_type_mask,
		Chunk::key_trust_mask,
		Chunk::key_prio_mask,
		Chunk::key_ttl_mask,
		Chunk::key_has_payload_mask,
		Chunk::key_has_dest_mask,
		Chunk::key_has_cbor_mask,
		Chunk::key_channel_mask,
		Chunk::key_highbit_mask),
		"Chunk key don't add up or overlap");

	static_assert ((Chunk::key_type_min < Chunk::key_type_max) && (Chunk::key_type_max <= Chunk::key_type_mask),
		"Chunk type key doesn't fit mask");

	/*** ChunkMeta ***/
	static_assert ((ChunkMeta::key_type_min < ChunkMeta::key_type_max) && (ChunkMeta::key_type_max <= ChunkMeta::key_type_mask),
		"Chunkmeta type key doesn't fit mask");

	static_assert (ChunkMeta::key_repeat_mask == (ChunkMeta::key_repeat_byte << 8),
		"ChunkMeta key repeat byte and mask does not match");

	static_assert (BIN::check_bit_coverage (UINT8_MAX,
		ReDataConfig::key_digest_mask,
		ReDataConfig::key_crypto_mask,
		ReDataConfig::key_highbit_mask),
		"ReDataConfig key don't add up or overlap");

	static_assert (std::underlying_type<ReDataConfig::DIGEST>::type (ReDataConfig::DIGEST::_MAX) <= (1 + (ReDataConfig::key_digest_mask >> ReDataConfig::key_digest_shift)),
		"ReDataConfig::DIGEST contains more entries than allowed");

	static_assert (std::underlying_type<ReDataConfig::CRYPTO>::type (ReDataConfig::CRYPTO::_MAX) <= (1 + (ReDataConfig::key_crypto_mask >> ReDataConfig::key_crypto_shift)),
		"ReDataConfig::CRYPTO contains more entries than allowed");

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Most used templates  ////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	extern template class SPSC<std::string>;

	extern template class Simple8EncodeSPSC<SPSCDataPreAlloc>;
	extern template class Simple8DecodeSPSC<SPSCDataPreAlloc>;

	extern template class Simple16EncodeSPSC<SPSCDataPreAlloc>;
	extern template class Simple16DecodeSPSC<SPSCDataPreAlloc>;

	extern template class PacketEncodeSPSC<SPSCDataDynAlloc>;
	extern template class PacketDecodeSPSC<SPSCDataDynAlloc>;

	extern template class SeqPacketEncodeSPSC<SPSCDataDynAlloc>;
	extern template class SeqPacketDecodeSPSC<SPSCDataDynAlloc>;
}

#endif // HEAD_shaga

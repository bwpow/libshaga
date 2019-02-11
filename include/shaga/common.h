/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_common
#define HEAD_shaga_common

#if (!defined (__cplusplus)) || (__cplusplus < 201402L)
	#error At least C++14 compiler required.
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
#ifdef SHAGA_THREADING
	#undef SHAGA_THREADING
#endif // SHAGA_THREADING

#ifndef SHAGA_SINGLE_THREAD
	#define SHAGA_THREADING
#endif // SHAGA_SINGLE_THREAD

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
#include <memory>

#include <random>
#include <chrono>

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
#endif // OS_LINUX

#ifdef SHAGA_FULL
	/* Used in ReData and CRC */
	#include <mbedtls/cipher.h>
	#include <mbedtls/aes.h>

	#include <mbedtls/ripemd160.h>
	#include <mbedtls/sha1.h>
	#include <mbedtls/sha256.h>
	#include <mbedtls/sha512.h>

	/* Used in DiSig */
	#include <mbedtls/pk.h>
	#include <mbedtls/md_internal.h>

	#include <mbedtls/error.h>

	#if !defined(MBEDTLS_CIPHER_MODE_CBC) || \
		!defined(MBEDTLS_AES_C) || \
		!defined(MBEDTLS_RIPEMD160_C) || \
		!defined(MBEDTLS_SHA1_C) || \
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

#ifndef INT64_C
	#define INT64_C(c) (c ## LL)
#endif

#ifndef UINT64_C
	#define UINT64_C(c) (c ## ULL)
#endif

/* Detect endian */
#ifdef _WIN32
	#define BYTE_ORDER __LITTLE_ENDIAN
#elif !defined BYTE_ORDER
	#include <endian.h>
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
	#define ENDIAN_IS_BIG if (BIN::Endian::BIG == BIN::_endian) {
	#define ENDIAN_IS_LITTLE if (BIN::Endian::LITTLE == BIN::_endian) {
	#define ENDIAN_END }
#endif

#define ENDIAN_BSWAP16(val) val = __builtin_bswap16 (val)
#define ENDIAN_BSWAP32(val) val = __builtin_bswap32 (val)
#define ENDIAN_BSWAP64(val) val = __builtin_bswap64 (val)

#if __has_cpp_attribute(fallthrough)
	#define SHAGA_FALLTHROUGH [[fallthrough]]
#elif __has_cpp_attribute(gcc::fallthrough)
	#define SHAGA_FALLTHROUGH [[gcc::fallthrough]]
#elif __has_cpp_attribute(clang::fallthrough)
	#define SHAGA_FALLTHROUGH [[clang::fallthrough]]
#else
	#define SHAGA_FALLTHROUGH
#endif

#if __has_cpp_attribute(nodiscard)
	#define SHAGA_NODISCARD [[nodiscard]]
#elif __has_cpp_attribute(gnu::warn_unused_result)
	#define SHAGA_NODISCARD [[gnu::warn_unused_result]]
#else
	#define SHAGA_NODISCARD
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
	SHAGA_NODISCARD bool is_shutting_down (void);

	/* Set in shaga.cpp to constant value depending on compile-time settings */
	extern const bool _shaga_compiled_with_threading;
	extern const bool _shaga_compiled_full;

	/* Call from main to check if compile time settings match runtime settings */
	inline void shaga_check_threading (void)
	{
		#ifdef SHAGA_THREADING
			if (false == _shaga_compiled_with_threading) {
				cThrow ("Shaga threading mismatch, compiled without threading and used with threading");
			}
		#else
			if (true == _shaga_compiled_with_threading) {
				cThrow ("Shaga threading mismatch, compiled with threading and used without threading");
			}
		#endif // SHAGA_THREADING
	}

	inline void shaga_check_version (void)
	{
		#if defined SHAGA_LITE
			if (true == _shaga_compiled_full) {
				cThrow ("Shaga library version mismatch, compiled as full and used as lite");
			}
		#elif defined SHAGA_FULL
			if (false == _shaga_compiled_full) {
				cThrow ("Shaga library version mismatch, compiled as lite and used as full");
			}
		#else
			#error Unable to detect version of the library
		#endif
	}

	#define LOG_REGISTER

	#define SHAGA_MAIN(a) int main (int argc, char **argv) try { \
		(void) argc; \
		(void) argv; \
		shaga_check_threading (); \
		shaga_check_version (); \
		BIN::endian_detect (); \
		LOG_REGISTER; \
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
#include "ShSocket.h"
#include "ShFile.h"
#include "FS.h"
#include "BINstatic.h"
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
		Chunk::key_is_tracert_mask,
		Chunk::key_trust_mask,
		Chunk::key_prio_mask,
		Chunk::key_ttl_mask,
		Chunk::key_has_payload_mask,
		Chunk::key_has_dest_mask,
		Chunk::key_channel_mask,
		Chunk::key_reserved_mask,
		Chunk::key_highbit_mask),
		"Chunk key don't add up or overlap");

	static_assert ((Chunk::key_type_min < Chunk::key_type_max) && (Chunk::key_type_max <= Chunk::key_type_mask),
		"Chunk type key doesn't fit mask");

	static_assert ((Chunk::key_tracert_hop_shift >= 16) && ((Chunk::key_is_tracert_mask & 0xffff) == 0),
		"Tracert hop shift and mask must be defined in high 16-bits");

	/*** ChunkMeta ***/
	static_assert ((ChunkMeta::key_type_min < ChunkMeta::key_type_max) && (ChunkMeta::key_type_max <= ChunkMeta::key_type_mask),
		"Chunkmeta type key doesn't fit mask");

	static_assert (ChunkMeta::key_repeat_mask == (ChunkMeta::key_repeat_byte << 8),
		"ChunkMeta key repeat byte and mask does not match");

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Most used templates  ////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	extern template class SPSC<std::string>;

	extern template class Uart8EncodeSPSC<SPSCDataPreAlloc>;
	extern template class Uart8DecodeSPSC<SPSCDataPreAlloc>;

	extern template class Uart16EncodeSPSC<SPSCDataPreAlloc>;
	extern template class Uart16DecodeSPSC<SPSCDataPreAlloc>;

	extern template class PacketEncodeSPSC<SPSCDataDynAlloc>;
	extern template class PacketDecodeSPSC<SPSCDataDynAlloc>;

	extern template class SeqPacketEncodeSPSC<SPSCDataDynAlloc>;
	extern template class SeqPacketDecodeSPSC<SPSCDataDynAlloc>;
}

#endif // HEAD_shaga

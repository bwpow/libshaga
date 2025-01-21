/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_P
#define HEAD_shaga_P

#include "common.h"

/* Concat function taking string and string_view and returning new string */
#define _CC(x,...) shaga::STR::concat(x,##__VA_ARGS__)

/* Helper to convert string_vieew to C-style string */
#define s_c_str(x) std::string(x).c_str()

/* Throwing CommonException helpers */
#define cLogThrow(format, ...) throw shaga::CommonException(true, __FILE__, __PRETTY_FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define cThrow(format, ...) throw shaga::CommonException(false, __FILE__, __PRETTY_FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#define MACRO_CONCAT2(x, y) x ## y
#define MACRO_CONCAT(x, y) MACRO_CONCAT2(x, y)

/* Create scoped variable that will keep CommonException muted */
#define cMute shaga::CommonExceptionMute MACRO_CONCAT(_common_exception_mute_, __LINE__)

namespace shaga::P {
	/* Cache to avoid allocation for every P::print */
	typedef std::array<char, 256> P_CACHE_TYPE;

	void set_dir_log (const std::string_view var);

	void set_dir_log (const COMMON_DEQUE &lst);
	void set_dir_log (const COMMON_LIST &lst);
	void set_dir_log (const COMMON_VECTOR &lst);

	void set_name_log (const std::string_view var);
	void set_app_name (const std::string_view var);

	void check_size (const bool enabled) noexcept;
	void set_max_size_mb (const int soft, const int hard) noexcept;
	bool soft_limit_reached (void) noexcept;

	void rescan_available_directories (void);
	void set_enabled (const bool enabled);
	void show_ms (const bool enabled) noexcept;
	void use_gmt (const bool enabled) noexcept;
	bool is_enabled (void) noexcept;

	P_CACHE_TYPE& _p_cache_lock (void) noexcept;
	void _p_cache_release (void) noexcept;

	void _print (const std::string_view message, const std::string_view prefix = ""sv, const bool also_to_stderr = false) noexcept;

	template <typename... Args>
	void print (const std::string_view format, const Args & ... args) noexcept
	{
		if (false == is_enabled ()) {
			return;
		}

		if (sizeof...(Args) == 0) {
			_print (format);
		}
		else {
			P_CACHE_TYPE &cache = _p_cache_lock ();
			try {
				const auto result = fmt::format_to_n (cache.begin (), cache.size (), format, args...);
				_print (std::string_view (cache.data (), std::min (result.size, cache.size ())));
			}
			catch (...) {
				_print (format, "(!FORMAT ERROR!) "sv);
			}
			_p_cache_release ();
		}
	}

	void debug_set_enabled (const bool enabled) noexcept;
	bool debug_is_enabled (void) noexcept;

	template <typename... Args>
	void debug_print (const std::string_view format, const Args & ... args) noexcept
	{
		if (false == debug_is_enabled ()) {
			return;
		}

		if (sizeof...(Args) == 0) {
			_print (format, "[DEBUG] "sv);
		}
		else {
			P_CACHE_TYPE &cache = _p_cache_lock ();
			try {
				const auto result = fmt::format_to_n (cache.begin (), cache.size (), format, args...);
				_print (std::string_view (cache.data (), std::min (result.size, cache.size ())), "[DEBUG] "sv);
			}
			catch (...) {
				_print (format, "[DEBUG] (!FORMAT ERROR!) "sv);
			}
			_p_cache_release ();
		}
	}
}

namespace shaga
{
	bool commonexception_is_muted (void) noexcept;
	void commonexception_set_muted (const bool state) noexcept;

	class CommonExceptionMute
	{
		private:
			const bool _previous_state {false};
			bool _active {false};

		public:
			explicit CommonExceptionMute () : _previous_state (commonexception_is_muted ())
			{
				if (false == P::debug_is_enabled ()) {
					_active = true;
					commonexception_set_muted (true);
				}
			}

			~CommonExceptionMute ()
			{
				reset ();
			}

			void reset (void)
			{
				if (true == _active) {
					commonexception_set_muted (_previous_state);
					_active = false;
				}
			}
	};

	class CommonException : public std::exception
	{
		private:
			std::string _text;
			size_t _info_pos {0};
			static const constexpr char *_muted_str = "CommonException muted";

		public:
			template <typename... Args>
			CommonException (const bool log, const std::string_view str_file, const std::string_view str_function, const int str_line, const std::string_view format, const Args & ... args) noexcept
			{
				if (false == log && true == commonexception_is_muted ()) {
					/* If logging is not enabled and exceptions are muted, don't do anything */
					return;
				}

				_text.reserve (str_file.size () + str_function.size () + 16 + (format.size () * 3));

				_text.assign (str_file);
				_text.append ("("sv);
				_text.append (STR::from_int (str_line));
				_text.append ("): "sv);
				_text.append (str_function);
				_text.append (": "sv);

				_info_pos = _text.size ();

				if (sizeof...(Args) == 0) {
					_text.append (format);
				}
				else {
					try {
						STR::sprint (_text, format, args...);
					}
					catch (...) {
						_text.append (format);
						_text.append (" (!format error!)"sv);
					}
				}

				if (true == log || true == P::debug_is_enabled ()) {
					P::_print (_text, "{Exception} "sv);
				}
			}

			const char *what () const noexcept
			{
				if (_text.empty ()) {
					return _muted_str;
				}
				else {
					return (_text.c_str ()) + _info_pos;
				}
			}

			const char *debugwhat () const noexcept
			{
				if (_text.empty ()) {
					return _muted_str;
				}
				else {
					return _text.c_str ();
				}
			}
	};
}

#endif // HEAD_shaga_P

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2021, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_ShFile
#define HEAD_shaga_ShFile

#include "common.h"

namespace shaga {
	class ShFile {
		public:
			enum class CallbackAction {
				OPEN,		/* Right after file is opened */
				CLOSE,		/* Right before file is closed */
				OVERWRITE,	/* */
			};
			/* Reference of calling ShFile and action */
			typedef std::function<bool (ShFile &file, const ShFile::CallbackAction action)> ShFileCallback;

			static const uint8_t mREAD    { 0b00'00'00'01 };
			static const uint8_t mWRITE   { 0b00'00'00'10 };
			static const uint8_t mAPPEND  { 0b00'00'01'00 };
			static const uint8_t mNOCREAT { 0b00'00'10'00 };
			static const uint8_t mSYNC    { 0b00'01'00'00 };
			static const uint8_t mTRUNC   { 0b00'10'00'00 };
			static const uint8_t mEXCL    { 0b01'00'00'00 };

			static const uint8_t mR {mREAD};
			static const uint8_t mW {mWRITE | mTRUNC};
			static const uint8_t mA {mWRITE | mAPPEND};
			static const uint8_t mRW {mREAD | mWRITE};
			static const uint8_t mBRANDNEW {mWRITE | mEXCL};

			enum class SEEK { SET, CUR, END	};

			static const mode_t mask777 { S_IRWXU | S_IRWXG | S_IRWXO };
			static const mode_t mask666 { S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH };
			static const mode_t mask644 { S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH };
			static const mode_t mask600 { S_IRUSR | S_IWUSR };

		private:
			std::string _filename;
			uint8_t _mode {mREAD};
			int _fd {-1};
			mode_t _mask {mask644};

			ShFileCallback _callback {nullptr};

		public:
			ShFile (const std::string_view filename, const uint8_t mode = mREAD, const mode_t mask = mask644, const bool do_open = true);
			ShFile ();
			~ShFile ();

			/* Disable copy and assignment */
			ShFile (const ShFile &) = delete;
			ShFile& operator= (const ShFile &) = delete;

			void set_callback (ShFileCallback callback);
			void unset_callback (void);

			void open (void);
			void close (void) noexcept;
			void sync (const bool also_metadata = false);

			/* Write substr of string_view */
			void write (const std::string_view data, const size_t len);

			/* Write string_view */
			void write (const std::string_view data);

			/* Write memory buffer of particular length */
			void write (const void *const buf, const size_t len);

			/* Write C-style string */
			void write (const void *const buf);

			/* Write single character */
			void write (const uint8_t val);
			void write (const char val);

			template <typename... Args>
			void print (const std::string_view format, const Args & ... args)
			{
				write (fmt::format (format, args...));
			}

			bool read (std::string &data, const size_t len, const bool thr_eof = true);
			std::string read (const size_t len, const bool thr_eof = true);
			bool read (void *const buf, const size_t len, const bool thr_eof = true);
			uint8_t read (void);

			void read_whole_file (std::string &data, const size_t max_len = SIZE_MAX);
			std::string read_whole_file (const size_t max_len = SIZE_MAX);
			void read_whole_file (void *const data, const size_t max_len = SIZE_MAX);

			void dump_in_c_format (const std::string_view varname, const size_t len, std::function<std::string(const size_t pos)> callback, const size_t per_line = 16, const bool add_len = true);
			void dump_in_c_format (const std::string_view varname, const void *const data, const size_t len, const size_t per_line = 16, const bool add_len = true);
			void dump_in_c_format (const std::string_view varname, const std::string_view data, const size_t per_line = 16, const bool add_len = true);

			void set_file_name (const std::string_view filename, const uint8_t mode);
			void set_file_name (const std::string_view filename);

			template <typename T = std::string_view>
			SHAGA_STRV T get_file_name (void) const
			{
				return T (_filename);
			}

			void set_mode (const uint8_t mode);
			void set_mode_read (void);
			void set_mode_write (void);
			uint8_t get_mode (void) const;

			void set_mask (const mode_t mask);
			mode_t get_mask (void) const;

			off64_t seek (const off64_t offset, const SEEK whence = SEEK::SET);
			off64_t tell (void);
			void rewind (void);

			struct stat get_stat (void) const;
			off64_t get_file_size (void) const;
			time_t get_file_mtime (void) const;
			int get_file_descriptor (void) const;

			bool is_opened (void) const;
	};
}

#endif // HEAD_shaga_ShFile

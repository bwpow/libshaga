/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2018, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_ShFile
#define HEAD_shaga_ShFile

#include "common.h"

namespace shaga {

	class ShFile {
		public:
			enum class CallbackAction {
				OPEN, /* Right after file is opened */
				CLOSE /* Right before file is closed */
			};
			/* Reference of calling ShFile and action */
			typedef std::function<void (ShFile &file, const ShFile::CallbackAction action)> ShFileCallback;

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
			static const size_t printf_buf_size_min {256};
			static const size_t printf_buf_size_def {4096};
			static const size_t printf_buf_size_max {SSIZE_MAX};

			std::string _filename;
			uint8_t _mode {mREAD};
			int _fd {-1};
			mode_t _mask {mask644};

			char *_printf_buf {nullptr};
			ssize_t _printf_buf_req_size {printf_buf_size_def};
			ssize_t _printf_buf_cur_size {0};

			ShFileCallback _callback {nullptr};

		public:
			ShFile (const std::string &filename, const uint8_t mode = mREAD, const mode_t mask = mask644, const bool do_open = true);
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

			void write (const std::string &data, const size_t len);
			void write (const std::string &data);
			void write (const char *buf, const size_t len);
			void write (const char *buf);
			void write (const uint8_t val);

			bool read (std::string &data, const size_t len, const bool thr_eof = true);
			std::string read (const size_t len, const bool thr_eof = true);
			bool read (char *buf, const size_t len, const bool thr_eof = true);
			uint8_t read (void);

			void printf (const char *fmt, ...);
			void set_printf_buffer_size (const size_t sz);
			void realloc_printf_buffer (void);
			void free_printf_buffer (void);

			void set_file_name (const std::string &filename, const uint8_t mode);
			void set_file_name (const std::string &filename);
			std::string get_file_name (void) const;

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

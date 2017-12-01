/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdarg>

namespace shaga {

	static_assert (sizeof (off_t) == sizeof (off64_t), "Expected off_t to be same size as off64_t");

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Static functions  ///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Private class methods  //////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Public class methods  ///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ShFile::ShFile ()
	{ }

	ShFile::ShFile (const std::string &filename, const uint8_t mode, const mode_t mask) try
	{
		_filename.assign (filename);
		_mode = mode;
		_mask = mask;

		open ();
	}
	catch (...) {
		close ();
	}

	ShFile::~ShFile ()
	{
		close ();
	}

	void ShFile::open (void)
	{
		if (_fd >= 0) {
			cThrow ("File is already opened");
		}

		if (_filename.empty () == true) {
			cThrow ("Filename is not set");
		}

#ifdef O_BINARY
		int flags = O_BINARY;
#else
		int flags = 0;
#endif // O_BINARY

		if ((_mode & mREAD) && (_mode & mWRITE)) {
			flags |= O_RDWR | O_CREAT;
		}
		else if (_mode & mREAD) {
#ifndef OS_WIN
			flags |= O_RDONLY | O_NOATIME;
#else
			flags |= O_RDONLY;
#endif // OS_WIN
		}
		else if (_mode & mWRITE) {
			flags |= O_WRONLY | O_CREAT;
		}
		else {
			cThrow ("Mode must contain at least mREAD or mWRITE flags");
		}

		bool req_write = false;

		if (_mode & mAPPEND) {
			req_write = true;
		}

#ifndef OS_WIN
		if (_mode & mSYNC) {
			flags |= O_SYNC;
		}
#endif // OS_WIN

		if (_mode & mTRUNC) {
			flags |= O_TRUNC;
			req_write = true;
		}

		if (_mode & mEXCL) {
			flags |= O_EXCL | O_CREAT;
			req_write = true;
		}

		if (_mode & mNOCREAT) {
			flags &= ~O_CREAT;
		}

		if (true == req_write && (_mode & mWRITE) == 0) {
			cThrow ("Flag mWRITE is required for this mode");
		}

		if ((_mode & mNOCREAT) && (_mode & mEXCL)) {
			cThrow ("Flags mNOCREAT and mEXCL cannot be used together");
		}

		_fd = ::open (_filename.c_str (), flags, _mask);
		if (_fd < 0) {
			cThrow ("Error opening file '%s': %s", _filename.c_str (), strerror (errno));
		}

		if (_mode & mAPPEND) {
			seek (0, SEEK::END);
		}
	}

	void ShFile::close (void) noexcept
	{
		if (nullptr != _printf_buf) {
			::free (_printf_buf);
			_printf_buf = nullptr;
			_printf_buf_cur_size = 0;
		}

		if (_fd >= 0) {
			::close (_fd);
			_fd = -1;
		}
	}

	void ShFile::sync (const bool also_metadata)
	{
#ifndef OS_WIN
		if (true == also_metadata) {
			if (::fsync (_fd) == -1) {
				cThrow ("Sync error in file '%s': %s", _filename.c_str (), strerror (errno));
			}
		}
		else {
			if (::fdatasync (_fd) == -1) {
				cThrow ("Sync error in file '%s': %s", _filename.c_str (), strerror (errno));
			}
		}
#else
		(void) also_metadata;
#endif // OS_WIN
	}

	void ShFile::write (const std::string &data, const size_t len)
	{
		const ssize_t ret = ::write (_fd, data.data (), len);
		if (ret < 0) {
			cThrow ("Error writing to file '%s': %s", _filename.c_str (), strerror (errno));
		}
		if (static_cast<size_t> (ret) != len) {
			cThrow ("Error writing to file '%s': Not all bytes written", _filename.c_str ());
		}
	}

	void ShFile::write (const std::string &data)
	{
		write (data, data.size ());
	}

	void ShFile::write (const char *buf, const size_t len)
	{
		const ssize_t ret = ::write (_fd, buf, len);
		if (ret < 0) {
			cThrow ("Error writing to file '%s': %s", _filename.c_str (), strerror (errno));
		}
		if (static_cast<size_t> (ret) != len) {
			cThrow ("Error writing to file '%s': Not all bytes were written", _filename.c_str ());
		}
	}

	void ShFile::write (const char *buf)
	{
		write (buf, ::strlen (buf));
	}

	void ShFile::write (const uint8_t val)
	{
		const char buf[1] = { static_cast<const char> (val) };
		write (buf, 1);
	}

	bool ShFile::read (std::string &data, const size_t len, const bool thr_eof)
	{
		if (len > SSIZE_MAX) {
			cThrow ("Error reading from file '%s': Too many bytes requested", _filename.c_str ());
		}

		data.resize (len);
		if (data.capacity () < len) {
			cThrow ("Error reading from file '%s': Unable to allocate buffer", _filename.c_str ());
		}

		/* Note: Since C++11, std::string::data memory must be continuous */
		/* Note: Since C++17, std::string::data memory won't be const */
		#ifdef OS_WIN
			const int ret = ::read (_fd, &data[0], static_cast<unsigned int> (len));
		#else
			const ssize_t ret = ::read (_fd, &data[0], len);
		#endif // OS_WIN
		if (ret < 0) {
			cThrow ("Error reading from file '%s': %s", _filename.c_str (), strerror (errno));
		}
		else if (0 == ret) {
			/* EOF */
			if (true == thr_eof) {
				cThrow ("Error reading from file '%s': EOF", _filename.c_str ());
			}
			data.resize (0);
			return false;
		}
		else if (static_cast<size_t>(ret) != len) {
			cThrow ("Error reading from file '%s': Not enough bytes, requested = %zu, read = %zu", _filename.c_str (), len, static_cast<size_t> (ret));
		}

		return true;
	}

	std::string ShFile::read (const size_t len, const bool thr_eof)
	{
		std::string data;
		read (data, len, thr_eof);
		return data;
	}

	bool ShFile::read (char *buf, const size_t len, const bool thr_eof)
	{
		if (len > SSIZE_MAX) {
			cThrow ("Error reading from file '%s': Too many bytes requested", _filename.c_str ());
		}

		#ifdef OS_WIN
			const int ret = ::read (_fd, buf, static_cast<unsigned int> (len));
		#else
			const ssize_t ret = ::read (_fd, buf, len);
		#endif // OS_WIN
		if (ret < 0) {
			cThrow ("Error reading from file '%s': %s", _filename.c_str (), strerror (errno));
		}
		else if (0 == ret) {
			/* EOF */
			if (true == thr_eof) {
				cThrow ("Error reading from file '%s': EOF", _filename.c_str ());
			}
			return false;
		}
		else if (static_cast<size_t>(ret) != len) {
			cThrow ("Error reading from file '%s': Not enough bytes, requested = %zu, read = %zu", _filename.c_str (), len, static_cast<size_t> (ret));
		}

		return true;
	}

	uint8_t ShFile::read (void)
	{
		char buf[1];
		if (read (buf, 1) == false) {
			cThrow ("Error reading from file '%s': EOF", _filename.c_str ());
		}
		return static_cast<uint8_t> (buf[0]);
	}

	void ShFile::printf (const char *fmt, ...)
	{
		realloc_printf_buffer ();

		va_list ap;
		::va_start (ap, fmt);
		const int len = ::vsnprintf (_printf_buf, _printf_buf_cur_size - 1, fmt, ap);
		::va_end (ap);
		if (len < 0 || len >= (_printf_buf_cur_size - 1)) {
			cThrow ("Error printing to file '%s': Buffer too small", _filename.c_str ());
		}

		write (_printf_buf, len);
	}

	void ShFile::set_printf_buffer_size (const size_t sz)
	{
		if (sz < printf_buf_size_min) {
			cThrow ("Minimal allowed printf buffer size is %zu", printf_buf_size_min);
		}
		if (sz > printf_buf_size_max) {
			cThrow ("Minimal allowed printf buffer size is %zu", printf_buf_size_max);
		}
		_printf_buf_req_size = static_cast<ssize_t> (sz);
	}

	void ShFile::realloc_printf_buffer (void)
	{
		if (_printf_buf_cur_size == _printf_buf_req_size && nullptr != _printf_buf) {
			return;
		}

		_printf_buf = reinterpret_cast<char *> (::realloc (_printf_buf, _printf_buf_req_size));
		if (nullptr == _printf_buf) {
			cThrow ("Error printing to file '%s': Realloc error", _filename.c_str ());
		}
		_printf_buf_cur_size = _printf_buf_req_size;
	}

	void ShFile::free_printf_buffer (void)
	{
		if (nullptr != _printf_buf) {
			::free (_printf_buf);
			_printf_buf = nullptr;
			_printf_buf_cur_size = 0;
		}
	}

	void ShFile::set_file_name (const std::string &filename, const uint8_t mode)
	{
		set_file_name (filename);
		_mode = mode;
	}

	void ShFile::set_file_name (const std::string &filename)
	{
		if (is_opened () == true) {
			cThrow ("File is already opened");
		}
		_filename.assign (filename);
	}

	std::string ShFile::get_file_name (void) const
	{
		return _filename;
	}

	void ShFile::set_mode (const uint8_t mode)
	{
		if (is_opened () == true) {
			cThrow ("File is already opened");
		}

		_mode = mode;
	}

	void ShFile::set_mode_read (void)
	{
		set_mode (mREAD);
	}

	void ShFile::set_mode_write (void)
	{
		set_mode (mWRITE);
	}

	uint8_t ShFile::get_mode (void) const
	{
		return _mode;
	}

	void ShFile::set_mask (const mode_t mask)
	{
		if (is_opened () == true) {
			cThrow ("File is already opened");
		}

		_mask = mask;
	}

	mode_t ShFile::get_mask (void) const
	{
		return _mask;
	}

	off64_t ShFile::seek (const off64_t offset, const SEEK whence)
	{
		off64_t ret = 0;
		switch (whence) {
			case SEEK::SET:
				ret = ::lseek64 (_fd, offset, SEEK_SET);
				break;

			case SEEK::CUR:
				ret = ::lseek64 (_fd, offset, SEEK_CUR);
				break;

			case SEEK::END:
				ret = ::lseek64 (_fd, offset, SEEK_END);
				break;
		}


		if (ret == static_cast<off64_t>(-1)) {
			cThrow ("Seek error in file '%s': %s", _filename.c_str (), strerror (errno));
		}

		return ret;
	}

	off64_t ShFile::tell (void)
	{
		return seek (0, SEEK::CUR);
	}

	void ShFile::rewind (void)
	{
		seek (0, SEEK::SET);
	}

	struct stat ShFile::get_stat (void) const
	{
		struct stat ret;

		if (is_opened () == true) {
			if (::fstat (_fd, &ret) == -1) {
				cThrow ("Stat error in file '%s': %s", _filename.c_str (), strerror (errno));
			}
		}
		else {
			if (_filename.empty () == true) {
				cThrow ("Filename is not set");
			}
			if (::stat (_filename.c_str (), &ret) == -1) {
				cThrow ("Stat error in file '%s': %s", _filename.c_str (), strerror (errno));
			}
		}

		return ret;
	}

	off64_t ShFile::get_file_size (void) const
	{
		struct stat st = get_stat ();
		return st.st_size;
	}

	time_t ShFile::get_file_mtime (void) const
	{
		struct stat st = get_stat ();
		return st.st_mtime;
	}

	int ShFile::get_file_descriptor (void) const
	{
		return _fd;
	}

	bool ShFile::is_opened (void) const
	{
		return (_fd >= 0);
	}

}

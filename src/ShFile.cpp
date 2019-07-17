/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

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

	ShFile::ShFile (const std::string_view filename, const uint8_t mode, const mode_t mask, const bool do_open) try
	{
		_filename.assign (filename);
		_mode = mode;
		_mask = mask;

		if (true == do_open) {
			open ();
		}
	}
	catch (...) {
		close ();
	}

	ShFile::~ShFile ()
	{
		close ();
	}

	void ShFile::set_callback (ShFileCallback callback)
	{
		_callback = callback;
	}

	void ShFile::unset_callback (void)
	{
		_callback = nullptr;
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
			cThrow ("Error opening file '{}': {}", _filename, strerror (errno));
		}

		if (_mode & mAPPEND) {
			seek (0, SEEK::END);
		}

		if (nullptr != _callback) {
			_callback (*this, CallbackAction::OPEN);
		}
	}

	void ShFile::close (void) noexcept
	{
		if (_fd >= 0) {
			if (nullptr != _callback) {
				_callback (*this, CallbackAction::CLOSE);
			}
			::close (_fd);
			_fd = -1;
		}
	}

	void ShFile::sync ([[maybe_unused]] const bool also_metadata)
	{
#ifndef OS_WIN
		if (true == also_metadata) {
			if (::fsync (_fd) == -1) {
				cThrow ("Sync error in file '{}': {}", _filename, strerror (errno));
			}
		}
		else {
			if (::fdatasync (_fd) == -1) {
				cThrow ("Sync error in file '{}': {}", _filename, strerror (errno));
			}
		}
#endif // OS_WIN
	}

	void ShFile::write (const std::string_view data, const size_t len)
	{
		const ssize_t ret = ::write (_fd, data.data (), std::min (len, data.size()));
		if (ret < 0) {
			cThrow ("Error writing to file '{}': {}", _filename, strerror (errno));
		}
		if (static_cast<size_t> (ret) != len) {
			cThrow ("Error writing to file '{}': Not all bytes written", _filename);
		}
	}

	void ShFile::write (const std::string_view data)
	{
		write (data, data.size ());
	}

	void ShFile::write (const char *buf, const size_t len)
	{
		const ssize_t ret = ::write (_fd, buf, len);
		if (ret < 0) {
			cThrow ("Error writing to file '{}': {}", _filename, strerror (errno));
		}
		if (static_cast<size_t> (ret) != len) {
			cThrow ("Error writing to file '{}': Not all bytes were written", _filename);
		}
	}

	void ShFile::write (const char *buf)
	{
		write (buf, ::strlen (buf));
	}

	void ShFile::write (const uint8_t val)
	{
		const char buf[1] = { static_cast<char> (val) };
		write (buf, 1);
	}

	bool ShFile::read (std::string &data, const size_t len, const bool thr_eof)
	{
		if (len > SSIZE_MAX) {
			cThrow ("Error reading from file '{}': Too many bytes requested", _filename);
		}

		data.resize (len);
		if (data.capacity () < len) {
			cThrow ("Error reading from file '{}': Unable to allocate buffer", _filename);
		}

		/* Note: Since C++11, std::string::data memory must be continuous */
		/* Note: Since C++17, std::string::data memory isn't const */
		#ifdef OS_WIN
			const int ret = ::read (_fd, data.data (), static_cast<unsigned int> (len));
		#else
			const ssize_t ret = ::read (_fd, data.data (), len);
		#endif // OS_WIN
		if (ret < 0) {
			cThrow ("Error reading from file '{}': {}", _filename, strerror (errno));
		}
		else if (0 == ret) {
			/* EOF */
			if (true == thr_eof) {
				cThrow ("Error reading from file '{}': EOF", _filename);
			}
			data.resize (0);
			return false;
		}
		else if (static_cast<size_t>(ret) != len) {
			cThrow ("Error reading from file '{}': Not enough bytes, requested = {}, read = {}", _filename, len, ret);
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
			cThrow ("Error reading from file '{}': Too many bytes requested", _filename);
		}

		#ifdef OS_WIN
			const int ret = ::read (_fd, buf, static_cast<unsigned int> (len));
		#else
			const ssize_t ret = ::read (_fd, buf, len);
		#endif // OS_WIN
		if (ret < 0) {
			cThrow ("Error reading from file '{}': {}", _filename, strerror (errno));
		}
		else if (0 == ret) {
			/* EOF */
			if (true == thr_eof) {
				cThrow ("Error reading from file '{}': EOF", _filename);
			}
			return false;
		}
		else if (static_cast<size_t>(ret) != len) {
			cThrow ("Error reading from file '{}': Not enough bytes, requested = {}, read = {}", _filename, len, ret);
		}

		return true;
	}

	uint8_t ShFile::read (void)
	{
		char buf[1];
		if (read (buf, 1) == false) {
			cThrow ("Error reading from file '{}': EOF", _filename);
		}
		return static_cast<uint8_t> (buf[0]);
	}

	void ShFile::set_file_name (const std::string_view filename, const uint8_t mode)
	{
		set_file_name (filename);
		_mode = mode;
	}

	void ShFile::set_file_name (const std::string_view filename)
	{
		if (is_opened () == true) {
			cThrow ("File is already opened");
		}
		_filename.assign (filename);
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
			cThrow ("Seek error in file '{}': {}", _filename, strerror (errno));
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
				cThrow ("Stat error in file '{}': {}", _filename, strerror (errno));
			}
		}
		else {
			if (_filename.empty () == true) {
				cThrow ("Filename is not set");
			}
			if (::stat (_filename.c_str (), &ret) == -1) {
				cThrow ("Stat error in file '{}': {}", _filename, strerror (errno));
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

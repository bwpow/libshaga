/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include "shaga/common.h"

#include <fcntl.h>

namespace shaga {

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

	ShFile::ShFile (const std::string_view filename, const uint8_t mode, const mode_t mask, const bool do_open)
	{
		_filename.assign (filename);
		_mode = mode;
		_mask = mask;

		try {
			if (true == do_open) {
				open ();
			}
		}
		catch (...) {
			close ();
			throw;
		}
	}

	ShFile::~ShFile ()
	{
		if (true == _unlink_on_destruct) {
			unlink ();
		}
		else {
			close ();
		}
	}

	void ShFile::set_unlink_on_destruct (const bool enable)
	{
		_unlink_on_destruct = enable;
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
			cThrow ("File is already opened"sv);
		}

		if (_filename.empty () == true) {
			cThrow ("Filename is not set"sv);
		}

#ifdef O_BINARY
		int flags {O_BINARY};
#else
		int flags {0};
#endif // O_BINARY

#ifdef O_NOCTTY
		flags |= O_NOCTTY;
#endif // O_NOCTTY

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
			cThrow ("Mode must contain at least mREAD or mWRITE flags"sv);
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
			cThrow ("Flag mWRITE is required for this mode"sv);
		}

		if ((_mode & mNOCREAT) && (_mode & mEXCL)) {
			cThrow ("Flags mNOCREAT and mEXCL cannot be used together"sv);
		}

		if ((nullptr != _callback) && (_mode & mWRITE) != 0 && (_mode & mEXCL) == 0 && (_mode & mAPPEND) == 0) {
			if (FS::is_file (_filename) && _callback (*this, CallbackAction::OVERWRITE) == false) {
				cThrow ("Error opening file '{}': File already exists and overwriting is not allowed"sv, _filename);
			}
		}

		_fd = ::open (_filename.c_str (), flags, _mask);
		if (_fd < 0) {
			cThrow ("Error opening file '{}': {}"sv, _filename, strerror (errno));
		}

		if (_mode & mAPPEND) {
			seek (0, SEEK::END);
			P::debug_print ("ShFile {}: APPEND"sv, _filename);
		}
		else {
			P::debug_print ("ShFile {}: OPEN {}"sv, _filename, (_mode & mWRITE) ? "READ-WRITE"sv : "READ-ONLY"sv);
		}

		if (nullptr != _callback) {
			_callback (*this, CallbackAction::OPEN);
		}
	}

	void ShFile::close (const bool ignore_rename_on_close) noexcept
	{
		/* Disarm destruct */
		_unlink_on_destruct = false;

		if (_fd >= 0) {
			P::debug_print ("ShFile {}: CLOSE"sv, _filename);
			if (nullptr != _callback) {
				_callback (*this, CallbackAction::CLOSE);
			}
			::close (_fd);
			_fd = -1;

			if (false == ignore_rename_on_close && has_rename_on_close () == true) {
				const int ret = ::rename (_filename.c_str (), _rename_on_close_filename.c_str ());
				if (ret < 0) {
					if (nullptr != _callback) {
						_callback (*this, CallbackAction::RENAME_FAIL);
					}
				}
				else {
					if (nullptr != _callback) {
						_callback (*this, CallbackAction::RENAME);
					}
				}
			}
		}
	}

	void ShFile::sync ([[maybe_unused]] const bool also_metadata)
	{
#ifndef OS_WIN
		if (true == also_metadata) {
			if (::fsync (_fd) == -1) {
				cThrow ("Sync error in file '{}': {}"sv, _filename, strerror (errno));
			}
		}
		else {
			if (::fdatasync (_fd) == -1) {
				cThrow ("Sync error in file '{}': {}"sv, _filename, strerror (errno));
			}
		}
#endif // OS_WIN
	}

	void ShFile::write (const std::string_view data, const size_t len)
	{
		const ssize_t ret = ::write (_fd, data.data (), std::min (len, data.size()));
		if (ret < 0) {
			cThrow ("Error writing to file '{}': {}"sv, _filename, strerror (errno));
		}
		if (static_cast<size_t> (ret) != len) {
			cThrow ("Error writing to file '{}': Not all bytes written"sv, _filename);
		}
	}

	void ShFile::write (const std::string_view data)
	{
		write (data, data.size ());
	}

	void ShFile::write (const void *const buf, const size_t len)
	{
		const ssize_t ret = ::write (_fd, buf, len);
		if (ret < 0) {
			cThrow ("Error writing to file '{}': {}"sv, _filename, strerror (errno));
		}
		if (static_cast<size_t> (ret) != len) {
			cThrow ("Error writing to file '{}': Not all bytes were written"sv, _filename);
		}
	}

	void ShFile::write (const void *const buf)
	{
		write (buf, ::strlen (reinterpret_cast<const char *>(buf)));
	}

	void ShFile::write (const uint8_t val)
	{
		write (&val, 1);
	}

	void ShFile::write (const char val)
	{
		write (&val, 1);
	}

	bool ShFile::read (std::string &data, const size_t len, const bool thr_eof)
	{
		if (HEDLEY_UNLIKELY (len > SSIZE_MAX)) {
			cThrow ("Error reading from file '{}': Too many bytes requested"sv, _filename);
		}

		data.resize (0);
		data.resize (len);
		if (HEDLEY_UNLIKELY (data.capacity () < len)) {
			cThrow ("Error reading from file '{}': Unable to allocate buffer"sv, _filename);
		}

		/* Note: Since C++11, std::string::data memory must be continuous */
		/* Note: Since C++17, std::string::data memory isn't const */
		#ifdef OS_WIN
			const int ret = ::read (_fd, data.data (), static_cast<unsigned int> (len));
		#else
			const ssize_t ret = ::read (_fd, data.data (), len);
		#endif // OS_WIN
		if (ret < 0) {
			cThrow ("Error reading from file '{}': {}"sv, _filename, strerror (errno));
		}
		else if (0 == ret) {
			/* EOF */
			if (true == thr_eof) {
				cThrow ("Error reading from file '{}': EOF"sv, _filename);
			}
			data.resize (0);
			return false;
		}
		else if (static_cast<size_t>(ret) != len) {
			cThrow ("Error reading from file '{}': Not enough bytes, requested = {}, read = {}"sv, _filename, len, ret);
		}

		return true;
	}

	std::string ShFile::read (const size_t len, const bool thr_eof)
	{
		std::string data;
		read (data, len, thr_eof);
		return data;
	}

	bool ShFile::read (void *const buf, const size_t len, const bool thr_eof)
	{
		if (HEDLEY_UNLIKELY (len > SSIZE_MAX)) {
			cThrow ("Error reading from file '{}': Too many bytes requested"sv, _filename);
		}

		#ifdef OS_WIN
			const int ret = ::read (_fd, buf, static_cast<unsigned int> (len));
		#else
			const ssize_t ret = ::read (_fd, buf, len);
		#endif // OS_WIN
		if (ret < 0) {
			cThrow ("Error reading from file '{}': {}"sv, _filename, strerror (errno));
		}
		else if (0 == ret) {
			/* EOF */
			if (true == thr_eof) {
				cThrow ("Error reading from file '{}': EOF"sv, _filename);
			}
			return false;
		}
		else if (static_cast<size_t>(ret) != len) {
			cThrow ("Error reading from file '{}': Not enough bytes, requested = {}, read = {}"sv, _filename, len, ret);
		}

		return true;
	}

	uint8_t ShFile::read (void)
	{
		uint8_t buf[1];
		if (read (buf, 1) == false) {
			cThrow ("Error reading from file '{}': EOF"sv, _filename);
		}
		return buf[0];
	}

	void ShFile::read_whole_file (std::string &data, const size_t max_len)
	{
		seek (0, SEEK::SET);
		const uint64_t file_sze = static_cast<uint64_t> (get_file_size ());
		read (data, std::min (static_cast<uint64_t> (max_len), file_sze));
	}

	std::string ShFile::read_whole_file (const size_t max_len)
	{
		std::string out;
		read_whole_file (out, max_len);
		return out;
	}

	void ShFile::read_whole_file (void *const data, const size_t max_len)
	{
		seek (0, SEEK::SET);
		const uint64_t file_sze = static_cast<uint64_t> (get_file_size ());
		read (data, std::min (static_cast<uint64_t> (max_len), file_sze));
	}

	void ShFile::dump_in_c_format (const std::string_view varname, const size_t len, std::function<std::string(const size_t pos)> callback, const size_t per_line, const bool add_len)
	{
		if (true == add_len) {
			print ("{}[{}] = {{"sv, varname, len);
		}
		else {
			print ("{} = {{"sv, varname);
		}

		for (size_t pos = 0; pos < len; ++pos) {
			if ((pos % per_line) == 0) {
				write ("\n\t"sv);
			}
			else {
				write (' ');
			}
			write (callback (pos));
			write (',');
		}
		write ("\n};\n"sv);
	}

	void ShFile::dump_in_c_format (const std::string_view varname, const void *const data, const size_t len, const size_t per_line, const bool add_len)
	{
		dump_in_c_format (varname, len, [&data](const size_t pos) -> std::string {
			return fmt::format ("{:#04x}"sv, reinterpret_cast<const uint8_t *>(data)[pos]);
		}, per_line, add_len);
	}

	void ShFile::dump_in_c_format (const std::string_view varname, const std::string_view data, const size_t per_line, const bool add_len)
	{
		dump_in_c_format (varname, data.size (), [&data](const size_t pos) -> std::string {
			return fmt::format ("{:#04x}"sv, static_cast<uint8_t> (data.at (pos)));
		}, per_line, add_len);
	}

	void ShFile::dump_json (const nlohmann::json &data, const bool pretty)
	{
		if (true == pretty) {
			write (data.dump (1, '\t'));
		}
		else {
			write (data.dump ());
		}
	}

	bool ShFile::has_file_name (void) const
	{
		return (_filename.empty () == false);
	}

	void ShFile::set_file_name (const std::string_view filename, const uint8_t mode)
	{
		set_file_name (filename);
		_mode = mode;
	}

	void ShFile::set_file_name (const std::string_view filename)
	{
		if (is_opened () == true) {
			cThrow ("File is already opened"sv);
		}
		_filename.assign (filename);
	}

	bool ShFile::has_rename_on_close (void) const
	{
		return (_rename_on_close_filename.empty () == false);
	}

	void ShFile::set_rename_on_close_file_name (const std::string_view filename)
	{
		_rename_on_close_filename.assign (filename);
	}

	void ShFile::disable_rename_on_close (void)
	{
		_rename_on_close_filename.clear ();
	}

	void ShFile::set_mode (const uint8_t mode)
	{
		if (is_opened () == true) {
			cThrow ("File is already opened"sv);
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
			cThrow ("File is already opened"sv);
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
			cThrow ("Seek error in file '{}': {}"sv, _filename, strerror (errno));
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

	void ShFile::unlink (const bool also_rename_on_close_file)
	{
		close (true);

		if (has_file_name () == true) {
			if (nullptr != _callback) {
				_callback (*this, CallbackAction::UNLINK);
			}

			const int ret = ::unlink (_filename.c_str ());
			if (ret != 0) {
				if (ENOENT != errno) {
					cThrow ("Unable to unlink '{}': {}"sv, _filename, strerror (errno));
				}
			}
		}

		if (true == also_rename_on_close_file && has_rename_on_close () == true) {
			if (nullptr != _callback) {
				_callback (*this, CallbackAction::UNLINK_RENAME);
			}

			const int ret = ::unlink (_rename_on_close_filename.c_str ());
			if (ret != 0) {
				if (ENOENT != errno) {
					cThrow ("Unable to unlink '{}': {}"sv, _rename_on_close_filename, strerror (errno));
				}
			}
		}
	}

	void ShFile::touch (void)
	{
		if (has_file_name () == false) {
			cThrow ("Filename is not set"sv);
		}

		int flags {O_WRONLY | O_CREAT};
#ifdef O_NONBLOCK
		flags |= O_NONBLOCK;
#endif // O_NONBLOCK
#ifdef O_NOCTTY
		flags |= O_NOCTTY;
#endif // O_NOCTTY

		int fd = ::open (_filename.c_str (), flags, _mask);
		if (fd < 0) {
			cThrow ("Touch failed: {}"sv, strerror (errno));
		}

#ifndef OS_WIN
		const int ret = ::futimens (fd, nullptr);
		if (ret < 0) {
			cThrow ("Touch failed: {}"sv, strerror (errno));
		}
#endif // OS_WIN

		::close (fd);
	}

	struct stat ShFile::get_stat (void) const
	{
		struct stat ret;

		if (is_opened () == true) {
			if (::fstat (_fd, &ret) == -1) {
				cThrow ("Stat error in file '{}': {}"sv, _filename, strerror (errno));
			}
		}
		else {
			if (_filename.empty () == true) {
				cThrow ("Filename is not set"sv);
			}
			if (::stat (_filename.c_str (), &ret) == -1) {
				cThrow ("Stat error in file '{}': {}"sv, _filename, strerror (errno));
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

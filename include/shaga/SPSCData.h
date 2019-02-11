/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2019, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_SPSCData
#define HEAD_shaga_SPSCData

#include "common.h"

namespace shaga {

	class SPSCDataInterface
	{
		public:
			explicit SPSCDataInterface () {}
			virtual ~SPSCDataInterface () {}

			/* Disable copy and assignment */
			SPSCDataInterface (const SPSCDataInterface &) = delete;
			SPSCDataInterface& operator= (const SPSCDataInterface &) = delete;

			virtual uint_fast32_t size (void) const = 0;
			virtual uint_fast32_t inc_size (void) = 0;
			virtual uint_fast32_t set_size (const uint_fast32_t new_size) = 0;
			virtual void dec_size (void) = 0;
			virtual void dec2_size (void) = 0;
			virtual void zero_size (void) = 0;
			virtual void alloc (void) = 0;
			virtual void alloc (const uint_fast32_t alloc_size) = 0;
			virtual void free (void) = 0;
	};

	class SPSCDataPreAlloc : private SPSCDataInterface
	{
		private:
			const uint_fast32_t _bufsize;
			uint_fast32_t _size {0};

		public:
			uint8_t *buffer {nullptr};

			SPSCDataPreAlloc (const uint_fast32_t bufsize) :
				_bufsize (bufsize)
			{
				buffer = reinterpret_cast<uint8_t *> (::operator new (_bufsize, std::nothrow));
				if (nullptr == buffer) {
					cThrow ("Unable to allocate buffer");
				}
			}

			virtual ~SPSCDataPreAlloc ()
			{
				if (nullptr != buffer) {
					::operator delete (buffer, _bufsize);
				}
			}

			virtual uint_fast32_t size (void) const override
			{
				return _size;
			}

			virtual uint_fast32_t inc_size (void) override
			{
				if (_size >= _bufsize) {
					cThrow ("Unable to increase size");
				}
				return _size++;
			}

			virtual uint_fast32_t set_size (const uint_fast32_t new_size) override
			{
				if (new_size > _bufsize) {
					cThrow ("Unable to set size");
				}
				return (_size = new_size);
			}

			virtual void dec_size (void) override
			{
				if (0 == _size) {
					cThrow ("Unable to decrease size");
				}
				_size--;
			}

			virtual void dec2_size (void) override
			{
				if (_size < 2) {
					cThrow ("Unable to decrease size");
				}
				_size -= 2;
			}

			virtual void zero_size (void) override
			{
				_size = 0;
			}

			virtual void alloc (void) override {}

			virtual void alloc (const uint_fast32_t alloc_size) override
			{
				if (alloc_size > _bufsize) {
					cThrow ("Requested larger alloc size in constant container");
				}
			}

			virtual void free (void) override {}
	};

	class SPSCDataDynAlloc : private SPSCDataInterface
	{
		private:
			const uint_fast32_t _bufsize;
			uint_fast32_t _real_bufsize {0};
			uint_fast32_t _size {0};

		public:
			uint8_t *buffer {nullptr};

			SPSCDataDynAlloc (const uint_fast32_t bufsize) :
				_bufsize (bufsize)
			{ }

			virtual ~SPSCDataDynAlloc ()
			{
				free ();
			}

			virtual uint_fast32_t size (void) const override
			{
				return _size;
			}

			virtual uint_fast32_t inc_size (void) override
			{
				if (_size >= _real_bufsize) {
					cThrow ("Unable to increase size");
				}
				return _size++;
			}

			virtual uint_fast32_t set_size (const uint_fast32_t new_size) override
			{
				if (new_size > _real_bufsize) {
					cThrow ("Unable to set size");
				}
				return (_size = new_size);
			}

			virtual void dec_size (void) override
			{
				if (0 == _size) {
					cThrow ("Unable to decrease size");
				}
				_size--;
			}

			virtual void dec2_size (void) override
			{
				if (_size < 2) {
					cThrow ("Unable to decrease size");
				}
				_size -= 2;
			}

			virtual void zero_size (void) override
			{
				_size = 0;
			}

			virtual void alloc (void) override
			{
				alloc (_bufsize);
			}

			virtual void alloc (const uint_fast32_t alloc_size) override
			{
				if (alloc_size != _real_bufsize) {
					free ();

					if (alloc_size > 0) {
						buffer = reinterpret_cast<uint8_t *> (::operator new (alloc_size, std::nothrow));
						if (nullptr == buffer) {
							cThrow ("Unable to allocate buffer");
						}
					}

					_real_bufsize = alloc_size;
				}
			}

			virtual void free (void) override
			{
				if (nullptr != buffer) {
					::operator delete (buffer, _real_bufsize);
					buffer = nullptr;
				}

				_real_bufsize = 0;
			}
	};

}

#endif // HEAD_shaga_SPSCData

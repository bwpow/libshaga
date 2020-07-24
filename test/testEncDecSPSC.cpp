/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2020, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

static const uint8_t STX {0x45};
static const uint8_t ETX {0x4A};
static const uint8_t NTX {0x10};

static const std::array<uint8_t, 2> STX16 = {0x45, 0x55};

template<class T> static void _uart8spsc_sizes_test (void)
{
	const size_t datasize = 1024;
	const size_t testsize = datasize * 2;

	Simple8EncodeSPSC<T> encodering (datasize, testsize + 1, STX, ETX, NTX);
	Simple8DecodeSPSC<T> decodering (datasize, testsize + 1, STX, ETX, NTX);

	uint8_t buffer[testsize];
	for (size_t pos = 0; pos < testsize; ++pos) {
		buffer[pos] = pos & 0xff;
	}

	/* Fill encodering with testsize of messages, sized one byte to testsize bytes */
	for (size_t cursize = 1; cursize <= datasize; ++cursize) {
		ASSERT_NO_THROW (encodering.push_buffer (buffer, 0, cursize));
		ASSERT_NO_THROW (encodering.push_buffer (buffer, 1, cursize + 1));
	}

	/* Now push everything from encodering to decodering */
	char tempbuffer[testsize];
	size_t available = 0;
	while (true) {
		/* Test different available sizes */
		++available;
		if (available > testsize) {
			available = 1;
		}

		available = encodering.fill_front_buffer (tempbuffer, testsize);
		if (0 == available) {
			/* No more data */
			break;
		}
		decodering.push_buffer (tempbuffer, available);
		ASSERT_NO_THROW (encodering.move_front_buffer (available));
	}

	/* Read back all messages and compare them */
	std::string str;

	for (size_t cursize = 1; cursize <= datasize; ++cursize) {
		ASSERT_TRUE (decodering.pop_buffer (str));
		ASSERT_TRUE (str.size () == cursize);
		ASSERT_TRUE (::memcmp (buffer, str.data (), str.size ()) == 0);

		auto ret = decodering.peek_buffer ();
		ASSERT_TRUE (ret);
		ASSERT_TRUE (ret->size () == cursize);
		ASSERT_TRUE (::memcmp (buffer + 1, ret->data (), ret->size ()) == 0);
		ASSERT_TRUE (decodering.pop_buffer ());
	}

	/* Nothing more to read, should return false */
	ASSERT_FALSE (decodering.pop_buffer (str));
	ASSERT_FALSE (decodering.pop_buffer ());
	ASSERT_FALSE (decodering.peek_buffer ());
}

template<class T> static void _uart16spsc_sizes_test (void)
{
	const size_t datasize = 1024;
	const size_t testsize = datasize * 2;

	Simple16EncodeSPSC<T> encodering (datasize, testsize + 1, STX16);
	Simple16DecodeSPSC<T> decodering (datasize, testsize + 1, STX16);

	uint8_t buffer[testsize];
	for (size_t pos = 0; pos < testsize; ++pos) {
		buffer[pos] = pos & 0xff;
	}

	/* Fill encodering with testsize of messages, sized one byte to testsize bytes */
	for (size_t cursize = 1; cursize <= datasize; ++cursize) {
		ASSERT_NO_THROW (encodering.push_buffer (buffer, 0, cursize));
		ASSERT_NO_THROW (encodering.push_buffer (buffer, 1, cursize + 1));
	}

	/* Now push everything from encodering to decodering */
	char tempbuffer[testsize];
	size_t available = 0;
	while (true) {
		/* Test different available sizes */
		++available;
		if (available > testsize) {
			available = 1;
		}

		available = encodering.fill_front_buffer (tempbuffer, testsize);
		if (0 == available) {
			/* No more data */
			break;
		}
		decodering.push_buffer (tempbuffer, available);
		ASSERT_NO_THROW (encodering.move_front_buffer (available));
	}

	/* Read back all messages and compare them */
	std::string str;

	for (size_t cursize = 1; cursize <= datasize; ++cursize) {
		ASSERT_TRUE (decodering.pop_buffer (str));
		ASSERT_TRUE (str.size () == cursize);
		ASSERT_TRUE (::memcmp (buffer, str.data (), str.size ()) == 0);

		auto ret = decodering.peek_buffer ();
		ASSERT_TRUE (ret);
		ASSERT_TRUE (ret->size () == cursize);
		ASSERT_TRUE (::memcmp (buffer + 1, ret->data (), ret->size ()) == 0);
		ASSERT_TRUE (decodering.pop_buffer ());
	}

	/* Nothing more to read, should return false */
	ASSERT_FALSE (decodering.pop_buffer (str));
	ASSERT_FALSE (decodering.pop_buffer ());
	ASSERT_FALSE (decodering.peek_buffer ());
}

template<class T> static void _packet_sizes_test (void)
{
	const size_t datasize = 1024;
	const size_t testsize = datasize * 2;

	PacketEncodeSPSC<T> encodering (datasize, testsize + 1);
	PacketDecodeSPSC<T> decodering (datasize, testsize + 1);

	uint8_t buffer[testsize];
	for (size_t pos = 0; pos < testsize; ++pos) {
		buffer[pos] = pos & 0xff;
	}

	/* Fill encodering with testsize of messages, sized one byte to testsize bytes */
	for (size_t cursize = 1; cursize <= testsize; ++cursize) {
		if (cursize <= datasize) {
			/* This should fit */
			ASSERT_NO_THROW (encodering.push_buffer (buffer, 0, cursize));
			ASSERT_NO_THROW (encodering.push_buffer (buffer, 1, cursize + 1));
		}
		else {
			/* This shouldn't fit */
			ASSERT_THROW (encodering.push_buffer (buffer, 0, cursize), CommonException);
		}
	}

	/* Now push everything from encodering to decodering */
	char tempbuffer[testsize];
	size_t available = 0;
	while (true) {
		/* Test different available sizes */
		++available;
		if (available > testsize) {
			available = 1;
		}

		available = encodering.fill_front_buffer (tempbuffer, testsize);
		if (0 == available) {
			/* No more data */
			break;
		}
		decodering.push_buffer (tempbuffer, available);
		ASSERT_NO_THROW (encodering.move_front_buffer (available));
	}

	/* Read back all messages and compare them */
	std::string str;

	for (size_t cursize = 1; cursize <= datasize; ++cursize) {
		ASSERT_TRUE (decodering.pop_buffer (str));
		ASSERT_TRUE (str.size () == cursize);
		ASSERT_TRUE (::memcmp (buffer, str.data (), str.size ()) == 0);

		auto ret = decodering.peek_buffer ();
		ASSERT_TRUE (ret);
		ASSERT_TRUE (ret->size () == cursize);
		ASSERT_TRUE (::memcmp (buffer + 1, ret->data (), ret->size ()) == 0);
		ASSERT_TRUE (decodering.pop_buffer ());
	}

	/* Nothing more to read, should return false */
	ASSERT_FALSE (decodering.pop_buffer (str));
	ASSERT_FALSE (decodering.pop_buffer ());
	ASSERT_FALSE (decodering.peek_buffer ());
}

template<class T> static void _seqpacket_sizes_test (void)
{
	const size_t datasize = 1024;
	const size_t testsize = datasize * 2;

	SeqPacketEncodeSPSC<T> encodering (datasize, testsize + 1);
	SeqPacketDecodeSPSC<T> decodering (datasize, testsize + 1);

	uint8_t buffer[testsize];
	for (size_t pos = 0; pos < testsize; ++pos) {
		buffer[pos] = pos & 0xff;
	}

	/* Fill encodering with testsize of messages, sized one byte to testsize bytes */
	for (size_t cursize = 1; cursize <= testsize; ++cursize) {
		if (cursize <= datasize) {
			/* This should fit */
			ASSERT_NO_THROW (encodering.push_buffer (buffer, 0, cursize));
		}
		else {
			/* This shouldn't fit */
			ASSERT_THROW (encodering.push_buffer (buffer, 0, cursize), CommonException);
		}
	}

	/* Now push everything from encodering to decodering */
	char tempbuffer[testsize];
	size_t available = 0;
	while (true) {
		if (available > 0 && encodering.empty () == false) {
			/* We filled the encodering with messages increasing in size, so when we request the same size as previous message, fill_front_buffer should fail */
			ASSERT_THROW (encodering.fill_front_buffer (tempbuffer, available), CommonException);
		}

		available = encodering.fill_front_buffer (tempbuffer, testsize);
		if (0 == available) {
			/* No more data */
			break;
		}
		decodering.push_buffer (tempbuffer, available);

		/* If we try to move front buffer using different value, it should fail */
		ASSERT_TRUE (available > 1);
		ASSERT_THROW (encodering.move_front_buffer (available - 1), CommonException);
		ASSERT_THROW (encodering.move_front_buffer (available + 1), CommonException);

		ASSERT_NO_THROW (encodering.move_front_buffer (available));
	}

	/* Read back all messages and compare them */
	std::string str;

	for (size_t cursize = 1; cursize <= datasize; ++cursize) {
		ASSERT_TRUE (decodering.pop_buffer (str));
		ASSERT_TRUE (str.size () == cursize);
		ASSERT_TRUE (::memcmp (buffer, str.data (), str.size ()) == 0);
	}

	/* Nothing more to read, should return false */
	ASSERT_FALSE (decodering.pop_buffer (str));
}

template<class T> static void _uart8spsc_test (void)
{
	const size_t datasize = 256;
	const size_t sze = 256;
	const size_t loops = 64;
	const size_t totalsize = datasize * sze;

	Simple8EncodeSPSC<T> encodering (datasize, sze + 1, STX, ETX, NTX);
	Simple8DecodeSPSC<T> decodering (datasize, sze + 1, STX, ETX, NTX);

	char tempbuffer[600];
	size_t pos;

	uint8_t buffer[totalsize];

	for (pos = 0; pos < totalsize; ++pos) {
		buffer[pos] = pos & 0xff;
	}

	for (size_t loop = 0; loop < loops; ++loop) {
		decodering.has_crc8 ((loop % 2) == 0);
		encodering.has_crc8 ((loop % 2) == 0);

		for (pos = 0; pos < totalsize; pos += datasize) {
			ASSERT_NO_THROW (encodering.push_buffer (buffer, pos, pos + datasize));
		}

		/* Ring should be full at this point */
		ASSERT_THROW (encodering.push_buffer (buffer, 0, 1), CommonException);

		/* Insert bad message, this will generate 3 errors */
		tempbuffer[0] = STX;
		tempbuffer[1] = STX; /* Duplicated STX */
		tempbuffer[2] = NTX;
		tempbuffer[3] = NTX; /* Duplicated NTX */
		tempbuffer[4] = STX;
		tempbuffer[5] = NTX;
		tempbuffer[6] = ETX; /* ETX right after NTX */
		decodering.push_buffer (tempbuffer, 7);

		ASSERT_TRUE (decodering.get_err_count_reset () == 3);

		/* Now lets fill tempbuffer with encoded data and push it to decodering */
		pos = sze;
		while (true) {
			pos = (pos + 1) % sizeof (tempbuffer);
			const size_t available = encodering.fill_front_buffer (tempbuffer, pos);
			if (0 == available) {
				if (0 == pos) {
					/* We actually requested 0 bytes */
					continue;
				}
				/* No more data */
				break;
			}

			/* Push to decodering */
			ASSERT_NO_THROW (decodering.push_buffer (tempbuffer, available));

			/* Move read pointer */
			ASSERT_NO_THROW (encodering.move_front_buffer (available));
		}

		ASSERT_TRUE (decodering.get_err_count_reset () == 0);

		pos = 0;
		while (true) {
			std::string str;
			if (decodering.pop_buffer (str) == false) {
				break;
			}

			ASSERT_TRUE (::memcmp (buffer + pos, str.data (), str.size ()) == 0);

			pos += str.size ();
		}

		ASSERT_TRUE (pos == totalsize);
	}
}

template<class T> static void _uart16spsc_test (void)
{
	const size_t datasize = 4000;
	const size_t sze = 128;
	const size_t loops = 64;
	const size_t totalsize = datasize * sze;

	Simple16EncodeSPSC<T> encodering (datasize, sze + 1, STX16);
	Simple16DecodeSPSC<T> decodering (datasize, sze + 1, STX16);

	uint8_t tempbuffer[600];
	size_t pos;

	uint8_t buffer[totalsize];

	for (pos = 0; pos < totalsize; ++pos) {
		buffer[pos] = pos & 0xff;
	}

	for (size_t loop = 0; loop < loops; ++loop) {
		decodering.has_crc16 ((loop % 2) == 0, (loop % 4) == 0);
		encodering.has_crc16 ((loop % 2) == 0, (loop % 4) == 0);

		for (pos = 0; pos < totalsize; pos += datasize) {
			ASSERT_NO_THROW (encodering.push_buffer (buffer, pos, pos + datasize));
		}

		/* Ring should be full at this point */
		ASSERT_THROW (encodering.push_buffer (buffer, 0, 1), CommonException);

		if ((loop % 2) == 0) {
			/* Insert bad message, this will generate error if CRC is used */
			tempbuffer[0] = STX16[0];
			tempbuffer[1] = STX16[1];
			tempbuffer[2] = 0;
			tempbuffer[3] = 0;
			tempbuffer[4] = 0;
			tempbuffer[5] = 0;
			decodering.push_buffer (tempbuffer, 0, 6);
		}
		else {
			/* Insert message larger than allocated */
			tempbuffer[0] = STX16[0];
			tempbuffer[1] = STX16[1];
			tempbuffer[2] = UINT8_MAX;
			tempbuffer[3] = UINT8_MAX;
			decodering.push_buffer (tempbuffer, 0, 4);
		}

		ASSERT_TRUE (decodering.get_err_count_reset () == 1);

		/* Now lets fill tempbuffer with encoded data and push it to decodering */
		pos = sze;
		while (true) {
			pos = (pos + 1) % sizeof (tempbuffer);
			const size_t available = encodering.fill_front_buffer (reinterpret_cast<char *> (tempbuffer), pos);
			if (0 == available) {
				if (0 == pos) {
					/* We actually requested 0 bytes */
					continue;
				}
				/* No more data */
				break;
			}

			/* Push to decodering */
			ASSERT_NO_THROW (decodering.push_buffer (tempbuffer, available));

			/* Move read pointer */
			ASSERT_NO_THROW (encodering.move_front_buffer (available));
		}

		ASSERT_TRUE (decodering.get_err_count_reset () == 0);

		pos = 0;
		while (true) {
			std::string str;
			if (decodering.pop_buffer (str) == false) {
				break;
			}

			ASSERT_TRUE (::memcmp (buffer + pos, str.data (), str.size ()) == 0);

			pos += str.size ();
		}

		ASSERT_TRUE (pos == totalsize);
	}
}

template<class T> static void _packetspsc_test (void)
{
	const size_t datasize = 256;
	const size_t sze = 256;
	const size_t loops = 64;
	const size_t totalsize = datasize * sze;

	PacketEncodeSPSC<T> encodering (datasize, sze + 1);
	PacketDecodeSPSC<T> decodering (datasize, sze + 1);

	char tempbuffer[600];
	size_t pos;

	uint8_t buffer[totalsize];

	for (pos = 0; pos < totalsize; ++pos) {
		buffer[pos] = pos & 0xff;
	}

	for (size_t loop = 0; loop < loops; ++loop) {
		for (pos = 0; pos < totalsize; pos += datasize) {
			ASSERT_NO_THROW (encodering.push_buffer (buffer, pos, pos + datasize));
		}

		/* Ring should be full at this point */
		ASSERT_THROW (encodering.push_buffer (buffer, 0, 1), CommonException);

		/* Now lets fill tempbuffer with encoded data and push it to decodering */
		pos = sze;
		while (true) {
			pos = (pos + 1) % sizeof (tempbuffer);
			const size_t available = encodering.fill_front_buffer (tempbuffer, pos);
			if (0 == available) {
				if (0 == pos) {
					/* We actually requested 0 bytes */
					continue;
				}
				/* No more data */
				break;
			}

			/* Push to decodering */
			ASSERT_NO_THROW (decodering.push_buffer (tempbuffer, available));

			/* Move read pointer */
			ASSERT_NO_THROW (encodering.move_front_buffer (available));
		}

		ASSERT_TRUE (decodering.get_err_count_reset () == 0);

		pos = 0;
		while (true) {
			std::string str;
			if (decodering.pop_buffer (str) == false) {
				break;
			}

			ASSERT_TRUE (::memcmp (buffer + pos, str.data (), str.size ()) == 0);

			pos += str.size ();
		}

		ASSERT_TRUE (pos == totalsize);
	}
}

template<class T>
static void _seqpacketspsc_test (void)
{
	const size_t datasize = 256;
	const size_t sze = 256;
	const size_t loops = 64;
	const size_t totalsize = datasize * sze;

	SeqPacketEncodeSPSC<T> encodering (datasize, sze + 1);
	SeqPacketDecodeSPSC<T> decodering (datasize, sze + 1);

	char tempbuffer[600];
	size_t pos;

	uint8_t buffer[totalsize];

	for (pos = 0; pos < totalsize; ++pos) {
		buffer[pos] = pos & 0xff;
	}

	for (size_t loop = 0; loop < loops; ++loop) {

		for (pos = 0; pos < totalsize; pos += datasize) {
			ASSERT_NO_THROW (encodering.push_buffer (buffer, pos, pos + datasize));
		}

		/* Ring should be full at this point */
		ASSERT_THROW (encodering.push_buffer (buffer, 0, 1), CommonException);

		for (pos = 0; pos < totalsize; pos += datasize) {
			ASSERT_THROW (encodering.fill_front_buffer (tempbuffer, datasize - 1), CommonException);

			const size_t available = encodering.fill_front_buffer (tempbuffer, sizeof (tempbuffer));

			/* Push to decodering */
			ASSERT_NO_THROW (decodering.push_buffer (reinterpret_cast<const uint8_t *> (tempbuffer), 0, available));

			/* Move read pointer */
			ASSERT_THROW (encodering.move_front_buffer (available - 1), CommonException);
			ASSERT_NO_THROW (encodering.move_front_buffer (available));
		}

		ASSERT_TRUE (decodering.get_err_count_reset () == 0);

		pos = 0;
		while (true) {
			std::string str;
			if (decodering.pop_buffer (str) == false) {
				break;
			}

			ASSERT_TRUE (::memcmp (buffer + pos, str.data (), str.size ()) == 0);

			pos += str.size ();
		}

		ASSERT_TRUE (pos == totalsize);
		ASSERT_FALSE (decodering.pop_buffer ());
	}
}

TEST (EncDecSPSC, uart8_push_pop_prealloc)
{
	_uart8spsc_test<SPSCDataPreAlloc> ();
}

TEST (EncDecSPSC, uart8_push_pop)
{
	_uart8spsc_test<SPSCDataDynAlloc> ();
}

TEST (EncDecSPSC, uart8_different_sizes)
{
	_uart8spsc_sizes_test<SPSCDataPreAlloc> ();
	_uart8spsc_sizes_test<SPSCDataDynAlloc> ();
}

TEST (EncDecSPSC, uart16_push_pop_prealloc)
{
	_uart16spsc_test<SPSCDataPreAlloc> ();
}

TEST (EncDecSPSC, uart16_push_pop)
{
	_uart16spsc_test<SPSCDataDynAlloc> ();
}

TEST (EncDecSPSC, uart16_different_sizes)
{
	_uart16spsc_sizes_test<SPSCDataPreAlloc> ();
	_uart16spsc_sizes_test<SPSCDataDynAlloc> ();
}

TEST (EncDecSPSC, packet_push_pop_prealloc)
{
	_packetspsc_test<SPSCDataPreAlloc> ();
}

TEST (EncDecSPSC, packet_push_pop)
{
	_packetspsc_test<SPSCDataDynAlloc> ();
}

TEST (EncDecSPSC, packet_different_sizes)
{
	_packet_sizes_test<SPSCDataDynAlloc> ();
}

TEST (EncDecSPSC, seqpacket_push_pop_prealloc)
{
	_seqpacketspsc_test<SPSCDataPreAlloc> ();
}

TEST (EncDecSPSC, seqpacket_push_pop)
{
	_seqpacketspsc_test<SPSCDataDynAlloc> ();
}

TEST (EncDecSPSC, seqpacket_different_sizes)
{
	_seqpacket_sizes_test<SPSCDataDynAlloc> ();
}

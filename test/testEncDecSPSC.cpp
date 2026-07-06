/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

static const std::array<uint8_t, 3> ControlBytes_8 {0x45, 0x4A, 0x10};
static const std::array<uint8_t, 2> StartSequence_16 = {0x45, 0x55};

template<class T> static void _simple8spsc_sizes_test (void)
{
	const size_t datasize = 1024;
	const size_t testsize = datasize * 2;

	Simple8EncodeSPSC<T> encodering (datasize, testsize + 1, ControlBytes_8);
	Simple8DecodeSPSC<T> decodering (datasize, testsize + 1, ControlBytes_8);

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

template<class T> static void _simple16spsc_sizes_test (void)
{
	const size_t datasize = 1024;
	const size_t testsize = datasize * 2;

	Simple16EncodeSPSC<T> encodering (datasize, testsize + 1, StartSequence_16);
	Simple16DecodeSPSC<T> decodering (datasize, testsize + 1, StartSequence_16);

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

template<class T> static void _simplenewlinespsc_test (void)
{
	const size_t datasize {256};
	const size_t sze {6};
	const size_t loops {64};

	SimpleNewLineDecodeSPSC<T> decodering1 (datasize, sze, true);
	SimpleNewLineDecodeSPSC<T> decodering2 (datasize, sze, false);

	std::string str;

	for (size_t loop = 0; loop < loops; ++loop) {
		decodering1.push_buffer ("\n\ntest1\r\n\ntest2\ntest3"sv);
		decodering2.push_buffer ("\n\ntest1\r\n\ntest2\ntest3"sv);

		ASSERT_TRUE (decodering1.get_err_count_reset () == 0);
		ASSERT_TRUE (decodering2.get_err_count_reset () == 0);

		if (loop > 0) {
			ASSERT_TRUE (decodering1.pop_buffer (str));
			EXPECT_TRUE (str == "test3"s);

			ASSERT_TRUE (decodering2.pop_buffer (str));
			EXPECT_TRUE (str == "test3"s);
		}
		else {
			ASSERT_TRUE (decodering2.pop_buffer (str));
			EXPECT_TRUE (str == ""s);
		}

		ASSERT_TRUE (decodering1.pop_buffer (str));
		EXPECT_TRUE (str == "test1"s);
		ASSERT_TRUE (decodering1.pop_buffer (str));
		EXPECT_TRUE (str == "test2"s);
		ASSERT_FALSE (decodering1.pop_buffer ());

		ASSERT_TRUE (decodering2.pop_buffer (str));
		EXPECT_TRUE (str == ""s);
		ASSERT_TRUE (decodering2.pop_buffer (str));
		EXPECT_TRUE (str == "test1"s);
		ASSERT_TRUE (decodering2.pop_buffer (str));
		EXPECT_TRUE (str == ""s);
		ASSERT_TRUE (decodering2.pop_buffer (str));
		EXPECT_TRUE (str == "test2"s);
		ASSERT_FALSE (decodering1.pop_buffer ());
	}
}

template<class T> static void _simple8spsc_test (void)
{
	const size_t datasize = 256;
	const size_t sze = 256;
	const size_t loops = 64;
	const size_t totalsize = datasize * sze;

	Simple8EncodeSPSC<T> encodering (datasize, sze + 1, ControlBytes_8);
	Simple8DecodeSPSC<T> decodering (datasize, sze + 1, ControlBytes_8);

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
		tempbuffer[0] = ControlBytes_8[0];
		tempbuffer[1] = ControlBytes_8[0]; /* Duplicated STX */
		tempbuffer[2] = ControlBytes_8[2];
		tempbuffer[3] = ControlBytes_8[2]; /* Duplicated NTX */
		tempbuffer[4] = ControlBytes_8[0];
		tempbuffer[5] = ControlBytes_8[2];
		tempbuffer[6] = ControlBytes_8[1]; /* ETX right after NTX */
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

template<class T> static void _simple16spsc_test (void)
{
	const size_t datasize = 4000;
	const size_t sze = 128;
	const size_t loops = 64;
	const size_t totalsize = datasize * sze;

	Simple16EncodeSPSC<T> encodering (datasize, sze + 1, StartSequence_16);
	Simple16DecodeSPSC<T> decodering (datasize, sze + 1, StartSequence_16);

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
			tempbuffer[0] = StartSequence_16[0];
			tempbuffer[1] = StartSequence_16[1];
			tempbuffer[2] = 0;
			tempbuffer[3] = 0;
			tempbuffer[4] = 0;
			tempbuffer[5] = 0;
			decodering.push_buffer (tempbuffer, 0, 6);
		}
		else {
			/* Insert message larger than allocated */
			tempbuffer[0] = StartSequence_16[0];
			tempbuffer[1] = StartSequence_16[1];
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

TEST (EncDecSPSC, simplenewline_push_pop_prealloc)
{
	_simplenewlinespsc_test<SPSCDataPreAlloc> ();
}

TEST (EncDecSPSC, simplenewline_push_pop)
{
	_simplenewlinespsc_test<SPSCDataDynAlloc> ();
}

TEST (EncDecSPSC, simple8_push_pop_prealloc)
{
	_simple8spsc_test<SPSCDataPreAlloc> ();
}

TEST (EncDecSPSC, simple8_push_pop)
{
	_simple8spsc_test<SPSCDataDynAlloc> ();
}

TEST (EncDecSPSC, simple8_different_sizes)
{
	_simple8spsc_sizes_test<SPSCDataPreAlloc> ();
	_simple8spsc_sizes_test<SPSCDataDynAlloc> ();
}

TEST (EncDecSPSC, simple16_push_pop_prealloc)
{
	_simple16spsc_test<SPSCDataPreAlloc> ();
}

TEST (EncDecSPSC, simple16_push_pop)
{
	_simple16spsc_test<SPSCDataDynAlloc> ();
}

TEST (EncDecSPSC, simple16_different_sizes)
{
	_simple16spsc_sizes_test<SPSCDataPreAlloc> ();
	_simple16spsc_sizes_test<SPSCDataDynAlloc> ();
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

/*** Regression tests for bugs fixed in this session ***/

template<class T> static void _packet_decode_oversized_length_nonfatal (void)
{
	/* Regression: PacketDecodeSPSC passed the untrusted 24-bit length field straight to
	alloc() without validating it against _max_packet_size. With SPSCDataDynAlloc this
	allocated up to 16 MiB per forged header (DoS); with SPSCDataPreAlloc it threw a fatal
	exception out of push_buffer. It must now report a non-fatal error and keep working. */
	const size_t datasize = 256;
	PacketDecodeSPSC<T> decodering (datasize, 8);

	/* Forge: magic (5, 23), CRC byte, then 24-bit length 0xFFFFFF (little endian) */
	const uint8_t forged[6] = {5, 23, 0x00, 0xFF, 0xFF, 0xFF};
	ASSERT_NO_THROW (decodering.push_buffer (forged, 0, sizeof (forged)));
	ASSERT_EQ (decodering.get_err_count_reset (), 1u);

	/* Nothing should have been queued */
	ASSERT_FALSE (decodering.pop_buffer ());

	/* The decoder must still accept a subsequent valid packet */
	PacketEncodeSPSC<T> encodering (datasize, 8);
	const std::string payload = "recovered_after_forged_header"s;
	ASSERT_NO_THROW (encodering.push_buffer (payload));

	char tempbuffer[512];
	const size_t available = encodering.fill_front_buffer (tempbuffer, sizeof (tempbuffer));
	ASSERT_GT (available, 0u);
	ASSERT_NO_THROW (decodering.push_buffer (tempbuffer, available));
	ASSERT_NO_THROW (encodering.move_front_buffer (available));

	std::string str;
	ASSERT_TRUE (decodering.pop_buffer (str));
	ASSERT_EQ (str, payload);
	ASSERT_EQ (decodering.get_err_count_reset (), 0u);
}

TEST (EncDecSPSC, packet_decode_oversized_length_nonfatal_prealloc)
{
	_packet_decode_oversized_length_nonfatal<SPSCDataPreAlloc> ();
}

TEST (EncDecSPSC, packet_decode_oversized_length_nonfatal)
{
	_packet_decode_oversized_length_nonfatal<SPSCDataDynAlloc> ();
}

template<class T> static void _seqpacket_decode_zero_length (void)
{
	/* Regression: SeqPacketDecodeSPSC called ::memcpy with a null destination for
	zero-length packets (SPSCDataDynAlloc::alloc(0) leaves buffer == nullptr), which is
	undefined behavior. A zero-length packet must decode to an empty buffer. */
	const size_t datasize = 256;
	SeqPacketDecodeSPSC<T> decodering (datasize, 8);

	/* A valid message with 24-bit length 0 is just the 3-byte header 00 00 00 */
	std::string header;
	BIN::from_uint24 (0, header);
	ASSERT_EQ (header.size (), 3u);

	ASSERT_NO_THROW (decodering.push_buffer (header));
	ASSERT_EQ (decodering.get_err_count_reset (), 0u);

	auto peeked = decodering.peek_buffer ();
	ASSERT_TRUE (peeked);
	ASSERT_TRUE (peeked->empty ());
	ASSERT_TRUE (decodering.pop_buffer ());
	ASSERT_FALSE (decodering.pop_buffer ());
}

TEST (EncDecSPSC, seqpacket_decode_zero_length_prealloc)
{
	_seqpacket_decode_zero_length<SPSCDataPreAlloc> ();
}

TEST (EncDecSPSC, seqpacket_decode_zero_length)
{
	_seqpacket_decode_zero_length<SPSCDataDynAlloc> ();
}

template<class T> static void _simple16_encode_rejects_oversized (void)
{
	/* Regression: Simple16EncodeSPSC did not validate the payload length. A payload larger
	than the buffer would silently truncate the 16-bit length field (corrupt framing).
	It must throw instead. */
	const size_t datasize = 32;
	Simple16EncodeSPSC<T> encodering (datasize, 8, StartSequence_16);

	std::vector<uint8_t> buffer (datasize * 4, 0xAB);

	/* A payload up to datasize must succeed */
	ASSERT_NO_THROW (encodering.push_buffer (buffer.data (), 0, datasize));

	/* A payload larger than the configured maximum must be rejected, not truncated */
	ASSERT_THROW (encodering.push_buffer (buffer.data (), 0, datasize * 4), CommonException);
}

TEST (EncDecSPSC, simple16_encode_rejects_oversized_prealloc)
{
	_simple16_encode_rejects_oversized<SPSCDataPreAlloc> ();
}

TEST (EncDecSPSC, simple16_encode_rejects_oversized)
{
	_simple16_encode_rejects_oversized<SPSCDataDynAlloc> ();
}

template<class T> static void _simple8_empty_frame_with_crc_nonfatal (void)
{
	/* Regression: Simple8DecodeSPSC::check() called dec_size() on an empty packet when CRC
	was enabled, throwing a fatal exception out of push_buffer for a trivially malformed
	STX ETX frame. It must now report a non-fatal error and keep decoding. */
	const size_t datasize = 256;
	Simple8DecodeSPSC<T> decodering (datasize, 8, ControlBytes_8);
	decodering.has_crc8 (true);

	/* STX immediately followed by ETX = empty frame, no room for the CRC byte */
	const uint8_t frame[2] = {ControlBytes_8[0], ControlBytes_8[1]};
	ASSERT_NO_THROW (decodering.push_buffer (frame, 0, sizeof (frame)));
	ASSERT_GE (decodering.get_err_count_reset (), 1u);
	ASSERT_FALSE (decodering.pop_buffer ());

	/* A subsequent valid frame must still decode */
	Simple8EncodeSPSC<T> encodering (datasize, 8, ControlBytes_8);
	encodering.has_crc8 (true);
	const std::string payload = "still_works"s;
	ASSERT_NO_THROW (encodering.push_buffer (payload));

	char tempbuffer[512];
	const size_t available = encodering.fill_front_buffer (tempbuffer, sizeof (tempbuffer));
	ASSERT_GT (available, 0u);
	ASSERT_NO_THROW (decodering.push_buffer (tempbuffer, available));
	ASSERT_NO_THROW (encodering.move_front_buffer (available));

	std::string str;
	ASSERT_TRUE (decodering.pop_buffer (str));
	ASSERT_EQ (str, payload);
	ASSERT_EQ (decodering.get_err_count_reset (), 0u);
}

TEST (EncDecSPSC, simple8_empty_frame_with_crc_nonfatal_prealloc)
{
	_simple8_empty_frame_with_crc_nonfatal<SPSCDataPreAlloc> ();
}

TEST (EncDecSPSC, simple8_empty_frame_with_crc_nonfatal)
{
	_simple8_empty_frame_with_crc_nonfatal<SPSCDataDynAlloc> ();
}

template<class T> static void _simplenewline_oversized_line_resync (void)
{
	/* Regression: an input line longer than max_packet_size hit inc_size()'s throw
	mid-loop, raising a fatal exception, discarding the rest of the input chunk and leaving
	the parser inconsistent. Oversized lines must now be dropped with a non-fatal error and
	the decoder must resync on the next newline. */
	const size_t datasize = 8;
	SimpleNewLineDecodeSPSC<T> decodering (datasize, 8);

	std::string input;
	/* A line far longer than datasize, terminated by newline */
	input.append (std::string (datasize * 4, 'A'));
	input.push_back ('\n');
	/* Followed by a short valid line */
	input.append ("hi"sv);
	input.push_back ('\n');

	ASSERT_NO_THROW (decodering.push_buffer (input));
	ASSERT_GE (decodering.get_err_count_reset (), 1u);

	/* The oversized line must have been dropped, but the following line must decode */
	std::string str;
	ASSERT_TRUE (decodering.pop_buffer (str));
	ASSERT_EQ (str, "hi"s);
	ASSERT_FALSE (decodering.pop_buffer ());
}

TEST (EncDecSPSC, simplenewline_oversized_line_resync_prealloc)
{
	_simplenewline_oversized_line_resync<SPSCDataPreAlloc> ();
}

TEST (EncDecSPSC, simplenewline_oversized_line_resync)
{
	_simplenewline_oversized_line_resync<SPSCDataDynAlloc> ();
}

template<class T> static void _simple16_decode_stx_resync (void)
{
	/* Regression: when the byte following the first STX byte was not the second STX byte,
	Simple16DecodeSPSC reset its state without re-testing that byte against the first STX
	byte. A "stx0 stx0 stx1 ..." sequence would therefore never lock on. */
	const size_t datasize = 256;
	Simple16EncodeSPSC<T> encodering (datasize, 8, StartSequence_16);
	Simple16DecodeSPSC<T> decodering (datasize, 8, StartSequence_16);

	const std::string payload = "resync_payload"s;
	ASSERT_NO_THROW (encodering.push_buffer (payload));

	char tempbuffer[512];
	const size_t available = encodering.fill_front_buffer (tempbuffer, sizeof (tempbuffer));
	ASSERT_GT (available, 0u);

	/* Prepend a duplicated first STX byte so the stream is: stx0 stx0 stx1 <len> <data...> */
	std::string stream;
	stream.push_back (static_cast<char> (StartSequence_16[0]));
	stream.append (tempbuffer, available);

	ASSERT_NO_THROW (decodering.push_buffer (stream));
	ASSERT_NO_THROW (encodering.move_front_buffer (available));

	std::string str;
	ASSERT_TRUE (decodering.pop_buffer (str));
	ASSERT_EQ (str, payload);
}

TEST (EncDecSPSC, simple16_decode_stx_resync_prealloc)
{
	_simple16_decode_stx_resync<SPSCDataPreAlloc> ();
}

TEST (EncDecSPSC, simple16_decode_stx_resync)
{
	_simple16_decode_stx_resync<SPSCDataDynAlloc> ();
}

/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.txt):

Copyright (c) 2012-2017, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <shaga.h>
#include <gtest/gtest.h>

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	shaga::BIN::endian_detect ();
	return RUN_ALL_TESTS();
}

/*
using namespace shaga;

template <typename T>
static void dump(const T &c)
{
	printf ("Dumping %" PRIu64 " items:\n", static_cast<uint64_t> (c.size ()));

	for (typename T::const_iterator iter = c.begin (); iter != c.end (); ++iter) {
		printf ("Item: '%s'\n", iter->c_str ());
	}
}

static void dump(const std::string &c)
{
	printf ("String (%" PRIu64 "): '%s'\n", static_cast<uint64_t> (c.size ()), STR::make_printable (c, '.').c_str ());
}

static void manualtest (void)
{

	try {
		//COMMON_VECTOR x = STR::to_argv ("-s\"xyz\" -o ahoj -obhoj -aabc -x3 -x 4 --preco=\"neviem preco\"");
		COMMON_VECTOR x = STR::to_argv ("-s /etc/passwd");
		dump (x);
		//COMMON_VECTOR x = STR::to_argv (argc, argv);

		ArgTable a;
		a.add ("string", 's', ArgTable::INCIDENCE::ZERO_OR_ONE, true, "Some string", "FILE", FS::is_file);
		a.add ("oooo", 'o', ArgTable::INCIDENCE::AT_LEAST_ONE, true, "Some oooo");
		a.add ("xxx", 'x', ArgTable::INCIDENCE::ANY, true, "Some xxx");
		a.add ("preco", ArgTable::INCIDENCE::ZERO_OR_ONE, true, "Preco? PRECO???");

		a.add ("all", 'a', ArgTable::INCIDENCE::AT_LEAST_ONE, false, "a");
		a.add ('b', ArgTable::INCIDENCE::ZERO_OR_ONE, false, "b");
		a.add ('c', ArgTable::INCIDENCE::ZERO_OR_ONE, false, "c");

		std::string err;
		if (a.process (x,err) == false) {
			std::cerr << err << "\n\n" << a << std::endl;
		}
		else {
			INI ini = a.export_ini ("halo");

			COMMON_VECTOR em;
			dump (ini.get_vector ("halo", "string", em));
		}
*/
/*
		ReData d;
		d.set_config ().set_coding (ReDataConfig::CODING_BASE64_ALT).set_digest (ReDataConfig::HASH_SHA512).set_crypto (ReDataConfig::CRYPTO_AES256);
		d.set_hmac_key ("totojekey");
		d.set_crypto_key ("0123456789abcdef0123456789abcdef");
		d.use_config_header (true);

		d.plaintext () = "sjiejijdijed";

		dump (d.get_config ().get_coding_text ());
		dump (d.get_config ().get_digest_text ());
		dump (d.get_config ().get_crypto_text ());
		dump (d.plaintext ());

		std::string x = d.encode ();

		dump (x);

		ReData e;
		e.set_hmac_key ("totojekey");
		e.set_crypto_key ("0123456789abcdef0123456789abcdef");
		e.use_config_header (true);

		e.decode (x, [](const ReDataConfig &c){ return c.has_digest_size_at_least_bits (256) && c.has_crypto_size_at_least_bits (256); });

		dump (e.get_config ().get_coding_text ());
		dump (e.get_config ().get_digest_text ());
		dump (e.get_config ().get_crypto_text ());
		dump (e.plaintext ());
*/
/*
		Chunk c1("AB77","c1");
		c1.meta.add_value ("XXX", "xxx");
		c1.meta.add_value ("XXX", "yyy");
		c1.meta.add_value ("ZZZ", "zzz");
		c1.set_prio (Chunk::Priority::CRITICAL);

		Chunk c2("9900","c2");
		c2.meta.add_value ("XXX", "xxx");
		c2.meta.add_value ("XXX", "yyy");
		c2.meta.add_value ("ZZZ", "zzz");
		c2.set_prio (Chunk::Priority::DEBUG);

		Chunk c3("9900","c3");
		c3.meta.add_value ("XXX", "xxx");
		c3.meta.add_value ("XXX", "yyy");
		c3.meta.add_value ("ZZZ", "zzz");
		c3.set_prio (Chunk::Priority::MANDATORY);

		Chunk c4("AB77","c4");
		c4.meta.add_value ("XXX", "xxx");
		c4.meta.add_value ("XXX", "yyy");
		c4.meta.add_value ("ZZZ", "zzz");
		c4.set_prio (Chunk::Priority::OPTIONAL);

		CHUNKSET cs;
		cs.insert (c1);
		cs.insert (c2);
		cs.insert (c3);
		cs.insert (c4);

		size_t left;
		std::string x = chunkset_to_string (cs, left);
		printf ("left %d of %d\n", (int)left, (int)cs.size ());
		dump (x);

		CHUNKSET cs2 = string_to_chunkset (x);

		cs = chunkset_get_type (cs2, "AB77");
		for (CHUNKSET::const_iterator iter = cs.begin (); iter != cs.end (); ++iter) {
			printf ("%s -> '%s' : %s\n", iter->get_type ().c_str (), iter->get_payload ().c_str (), iter->get_prio_text ().c_str ());
		}
*/
/*
	}
	catch (const std::exception &e) {
		printf ("Exception: %s\n", e.what ());
	}
	catch (...) {
		printf ("Brutal Exception\n");
	}
}
*/

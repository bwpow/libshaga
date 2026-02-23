/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2026, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>
#include <filesystem>

using namespace shaga;

namespace {
	std::string make_temp_ini_path (void)
	{
		static uint64_t seq = 0;
		++seq;
		return fmt::format ("./test/.tmp_ini_{}_{}"sv, static_cast<uint64_t> (std::time (nullptr)), seq);
	}

	class TempIniFile {
		private:
			std::string _path;

		public:
			explicit TempIniFile (std::string path)
				: _path (std::move (path))
			{ }

			~TempIniFile ()
			{
				std::error_code ec;
				std::filesystem::remove (_path, ec);
			}

			const std::string &path (void) const
			{
				return _path;
			}
	};
}

TEST (INI, load_buffer_basic)
{
	INI ini;

	const std::string buf =
		"[main]\n"
		"name=alpha\n"
		"count=42\n"
		"flag=1\n"
		"\n"
		"[other]\n"
		"list[]=a\n"
		"list[]=b\n"
		"list[]=c\n";

	EXPECT_NO_THROW (ini.load_buffer (buf, false, false));
	EXPECT_EQ (ini.get_string ("main", "name", ""sv), "alpha"sv);
	EXPECT_EQ (ini.get_uint32 ("main", "count", 0), 42u);
	EXPECT_EQ (ini.get_bool ("main", "flag", false), true);

	auto lst = ini.get_list ("other", "list");
	EXPECT_EQ (lst.size (), 3u);
	EXPECT_EQ (lst.front (), "a");
	EXPECT_EQ (lst.back (), "c");
}

TEST (INI, get_optional)
{
	INI ini;
	ini.set_string ("main", "val", "123"sv, false);
	ini.set_string ("main", "bad", "nope"sv, false);

	auto val = ini.get_uint32_optional ("main", "val");
	auto bad = ini.get_uint32_optional ("main", "bad");
	auto missing = ini.get_uint32_optional ("main", "missing");

	EXPECT_TRUE (val.has_value ());
	EXPECT_EQ (val.value (), 123u);
	EXPECT_FALSE (bad.has_value ());
	EXPECT_FALSE (missing.has_value ());
}

TEST (INI, get_string_optional)
{
	INI ini;
	ini.set_string ("main", "name", "beta"sv, false);

	auto val = ini.get_string_optional ("main", "name");
	auto missing = ini.get_string_optional ("main", "missing");

	EXPECT_TRUE (val.has_value ());
	EXPECT_EQ (val.value (), "beta"sv);
	EXPECT_FALSE (missing.has_value ());
}

TEST (INI, vector_list_sizes)
{
	INI ini;
	ini.set_string ("sec", "a", "1"sv, false);
	ini.set_string ("sec", "b", "2"sv, false);
	ini.set_string ("sec", "b", "3"sv, true);

	EXPECT_EQ (ini.get_list_size ("sec", "a"), 1u);
	EXPECT_EQ (ini.get_list_size ("sec", "b"), 2u);
	EXPECT_EQ (ini.get_vector_size ("sec", "b"), 2u);

	auto vec = ini.get_vector ("sec", "b");
	EXPECT_EQ (vec.size (), 2u);
	EXPECT_EQ (vec[0], "2");
	EXPECT_EQ (vec[1], "3");
}

TEST (INI, save_roundtrip)
{
	INI ini;
	ini.set_string ("main", "x", "100"sv, false);
	ini.set_string ("main", "y", "200"sv, false);
	ini.set_string ("list", "v", "a"sv, true);
	ini.set_string ("list", "v", "b"sv, true);

	const std::string out = ini.save_to_buffer ();

	INI restored;
	EXPECT_NO_THROW (restored.load_buffer (out, false, false));
	EXPECT_EQ (restored.get_uint32 ("main", "x", 0), 100u);
	EXPECT_EQ (restored.get_uint32 ("main", "y", 0), 200u);

	auto lst = restored.get_list ("list", "v");
	EXPECT_EQ (lst.size (), 2u);
	EXPECT_EQ (lst.front (), "a");
	EXPECT_EQ (lst.back (), "b");
}

TEST (INI, constructor_ignore_broken_lines)
{
	TempIniFile tmp (make_temp_ini_path ());

	{
		ShFile out (tmp.path (), ShFile::mW);
		out.write ("[main]\n"sv);
		out.write ("valid=ok\n"sv);
		out.write ("broken_line\n"sv);
		out.write ("next=42\n"sv);
	}

	EXPECT_THROW ((INI (tmp.path (), false, false)), CommonException);

	INI ini;
	EXPECT_NO_THROW ((ini = INI (tmp.path (), false, true)));
	EXPECT_EQ (ini.get_string ("main", "valid", ""sv), "ok"sv);
	EXPECT_EQ (ini.get_uint32 ("main", "next", 0), 42u);
}

TEST (INI, load_buffer_ignore_broken_lines)
{
	const std::string buf =
		"[main]\n"
		"ok=one\n"
		"broken_line\n"
		"ok2=two\n";

	INI ini;
	EXPECT_THROW (ini.load_buffer (buf, false, false, false), CommonException);

	EXPECT_NO_THROW (ini.load_buffer (buf, false, false, true));
	EXPECT_EQ (ini.get_string ("main", "ok", ""sv), "one"sv);
	EXPECT_EQ (ini.get_string ("main", "ok2", ""sv), "two"sv);

	EXPECT_THROW (ini.load_buffer (buf, false, false), CommonException);
}

TEST (INI, load_file_ignore_broken_lines)
{
	TempIniFile tmp (make_temp_ini_path ());

	{
		ShFile out (tmp.path (), ShFile::mW);
		out.write ("[main]\n"sv);
		out.write ("a=1\n"sv);
		out.write ("broken_line\n"sv);
		out.write ("b=2\n"sv);
	}

	INI ini;
	EXPECT_THROW (ini.load_file (tmp.path (), false, false, false), CommonException);

	EXPECT_NO_THROW (ini.load_file (tmp.path (), false, false, true));
	EXPECT_EQ (ini.get_uint32 ("main", "a", 0), 1u);
	EXPECT_EQ (ini.get_uint32 ("main", "b", 0), 2u);

	EXPECT_THROW (ini.load_file (tmp.path (), false, false), CommonException);
}

TEST (INI, save_to_json_shape)
{
	INI ini;
	ini.set_string ("main", "name", "alpha"sv, false);
	ini.set_string ("main", "list", "x"sv, false);
	ini.set_string ("main", "list", "y"sv, true);

	nlohmann::json out;
	EXPECT_NO_THROW (ini.save_to_json (out));

	ASSERT_TRUE (out.is_object ());
	ASSERT_TRUE (out.contains ("main"));
	ASSERT_TRUE (out["main"].is_object ());
	ASSERT_TRUE (out["main"].contains ("name"));
	ASSERT_TRUE (out["main"].contains ("list"));

	EXPECT_TRUE (out["main"]["name"].is_string ());
	EXPECT_EQ (out["main"]["name"].get<std::string> (), "alpha");

	EXPECT_TRUE (out["main"]["list"].is_array ());
	ASSERT_EQ (out["main"]["list"].size (), 2u);
	EXPECT_EQ (out["main"]["list"][0].get<std::string> (), "x");
	EXPECT_EQ (out["main"]["list"][1].get<std::string> (), "y");

	const nlohmann::json out2 = ini.save_to_json ();
	EXPECT_EQ (out2.dump (), out.dump ());
}

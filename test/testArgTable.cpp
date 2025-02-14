/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#include <gtest/gtest.h>

using namespace shaga;

TEST (ArgTableTest, NoParamOption)
{
	ArgTable argTable ("prog");
	argTable.add ("version", ArgTable::INCIDENCE::ZERO_OR_ONE, false, "show version");
	COMMON_VECTOR args = {"--version"};
	EXPECT_TRUE (argTable.process (args, true));
	std::string usage = argTable.get_usage_string ();
	EXPECT_NE (usage.find ("--version"), std::string::npos);
	// Verify with export_ini
	auto ini = argTable.export_ini ();
	EXPECT_EQ (ini.get_string ("", "version"), "1");
}

TEST (ArgTableTest, MissingParameter)
{
	ArgTable argTable ("prog");
	argTable.add ("input", ArgTable::INCIDENCE::ONE, true, "input file", "FILE");
	COMMON_VECTOR args = {"--input"};
	EXPECT_THROW (argTable.process (args, true), CommonException);
}

// Additional test to confirm providing parameter works
TEST (ArgTableTest, ParamOptionProvided)
{
	ArgTable argTable ("prog");
	argTable.add ("input", ArgTable::INCIDENCE::ONE, true, "input file", "FILE");
	COMMON_VECTOR args = {"--input=abc.txt"};
	EXPECT_NO_THROW (argTable.process (args, true));
	auto ini = argTable.export_ini ();
	EXPECT_EQ (ini.get_string ("", "input"), "abc.txt");
}

TEST (ArgTableTest, MultipleOccurrenceError)
{
	ArgTable argTable ("prog");
	argTable.add ("verbose", ArgTable::INCIDENCE::ZERO_OR_ONE, false, "be verbose");
	COMMON_VECTOR args = {"--verbose", "--verbose"};
	EXPECT_THROW (argTable.process (args, true), CommonException);
}

// Test using short option with parameter, verify INI
TEST (ArgTableTest, ShortOptionWithParameter)
{
	ArgTable argTable ("prog");
	argTable.add ('o', ArgTable::INCIDENCE::ONE, true, "output file", "FILE");
	// Test short option: attached parameter.
	COMMON_VECTOR args = {"-ooutput.txt"};
	EXPECT_TRUE (argTable.process (args, true));
	// Verify export_ini output contains expected key and value.
	auto ini = argTable.export_ini ();
	EXPECT_EQ (ini.get_string ("", "o"), "output.txt");
}

// Test using long option with '=' parameter, verify INI
TEST (ArgTableTest, LongOptionEqualParameter)
{
	ArgTable argTable ("prog");
	argTable.add ("config", ArgTable::INCIDENCE::ONE, true, "config file", "FILE");
	// Test long option with '=' separator for parameter.
	COMMON_VECTOR args = {"--config=config.yaml"};
	EXPECT_TRUE (argTable.process (args, true));
	auto ini = argTable.export_ini ();
	EXPECT_EQ (ini.get_string ("", "config"), "config.yaml");
}

TEST (ArgTableTest, InvalidOption)
{
	ArgTable argTable ("prog");
	argTable.add ("help", ArgTable::INCIDENCE::ZERO_OR_ONE, false, "display help");
	COMMON_VECTOR args = {"--unknown"};
	EXPECT_THROW (argTable.process (args, true), CommonException);
}

// ANY incidence with multiple options
TEST (ArgTableTest, AnyIncidenceMultipleParams)
{
	ArgTable argTable ("prog");
	argTable.add ("path", ArgTable::INCIDENCE::ANY, true, "Multiple paths", "FILE");
	COMMON_VECTOR args = {"--path=one", "--path=two", "--path=three"};
	EXPECT_TRUE (argTable.process (args, true));
	auto ini = argTable.export_ini ();
	auto list = ini.get_vector ("", "path");
	EXPECT_EQ (list.size (), 3u);
	EXPECT_EQ (list[0], "one");
	EXPECT_EQ (list[1], "two");
	EXPECT_EQ (list[2], "three");
}

// AT_LEAST_ONE incidence is satisfied
TEST (ArgTableTest, AtLeastOneIncidenceOk)
{
	ArgTable argTable ("prog");
	argTable.add ("value", ArgTable::INCIDENCE::AT_LEAST_ONE, true, "At least one required");
	COMMON_VECTOR args = {"--value=foo"};
	EXPECT_TRUE (argTable.process (args, true));
	auto ini = argTable.export_ini ();
	auto list = ini.get_vector ("", "value");
	EXPECT_EQ (list.size (), 1u);
	EXPECT_EQ (list[0], "foo");
}

// AT_LEAST_ONE incidence is missing -> throw
TEST (ArgTableTest, AtLeastOneIncidenceMissing)
{
	ArgTable argTable ("prog");
	argTable.add ("value", ArgTable::INCIDENCE::AT_LEAST_ONE, true, "At least one required");
	COMMON_VECTOR args = {};
	EXPECT_THROW (argTable.process (args, true), CommonException);
}

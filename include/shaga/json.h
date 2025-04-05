/******************************************************************************
Shaga library is released under the New BSD license (see LICENSE.md):

Copyright (c) 2012-2025, SAGE team s.r.o., Samuel Kupka

All rights reserved.
*******************************************************************************/
#ifndef HEAD_shaga_json
#define HEAD_shaga_json

#include "common.h"

namespace shaga {

	template <typename T>
	HEDLEY_WARN_UNUSED_RESULT auto json_getvalue (const nlohmann::json &data, const std::string_view key) -> T
	{
		if (data.contains (key) == false) {
			cThrow ("Parameter '{}' is not present in json"sv, key);
		}

		if constexpr (std::is_same<T, bool>::value) {
			if (true == data.at (key).is_boolean ()) {
				return data.at (key).get<bool> ();
			}
			else {
				return shaga::STR::to_bool (data.at (key).get<std::string_view> ());
			}
		}
		else {
			return data.at (key).get<T> ();
		}
	}

	template <typename T>
	void json_getvalue (const nlohmann::json &data, const std::string_view key, T &val)
	{
		val = json_getvalue<T> (data, key);
	}

	template <typename T>
	HEDLEY_WARN_UNUSED_RESULT auto json_getvalue_def (const nlohmann::json &data, const std::string_view key, const T def) -> T
	{
		if (data.contains (key) == false) {
			return def;
		}

		try {
			if constexpr (std::is_same<T, bool>::value) {
				if (true == data.at (key).is_boolean ()) {
					return data.at (key).get<bool> ();
				}
				else {
					return shaga::STR::to_bool (data.at (key).get<std::string_view> ());
				}
			}
			else {
				return data.at (key).get<T> ();
			}
		}
		catch (...) {
			return def;
		}
	}

	template <typename T>
	void json_getvalue_def (const nlohmann::json &data, const std::string_view key, T &val, const T def)
	{
		val = json_getvalue_def<T> (data, key, def);
	}

}

#endif // HEAD_shaga_json

#pragma once

#include "../StringUtils.h"
#include "../utils/json11/json11.hpp"

#include <string>
#include <sstream>
#include <map>
#include <list>

namespace json11
{
using namespace std;

inline void JsonToMap(const Json::object& m1, std::map<string, string>& m2)
{
	for (auto x : m1)
		m2.insert(std::make_pair(x.first, x.second.string_value()));
}

inline map<string, string> JsonToMap(const Json::object& m1)
{
	map<string, string> m2;
	JsonToMap(m1, m2);
	return m2;
}

inline void CopySelectedObject(Json::object& m1, Json::object& m2, const list<string> select)
{
	for (auto x : select)
		m1[x] = m2[x];
}

inline string VerifyObjectType(Json::object& m1, map<string, Json::Type> select)
{
	string err;
	for (auto x : select)
	{
		if (m1[x.first].is_null())
			err += x.first + " is missing. ";
		else if (m1[x.first].type() != x.second)
			err += x.first + " is wrong type. ";
	}
	return trim(err);
}

inline void JsonCheck(Json::object& o, const string& name, const string& value)
{
	if (!value.empty())
		o[name] = value;
}

inline void JsonCheck(Json::object& o, const string& name, const wstring& value)
{
	if (!value.empty())
		o[name] = ws2s(value);
}

inline void JsonAppend(Json::array& a1, const Json::array& a2)
{
	a1.insert(a1.end(), a2.begin(), a2.end());
}

inline void JsonCleanNull(Json::object& json)
{
	for (auto x = json.begin(); x != json.end(); ++x)
	{
		if (x->second.is_null() || (x->second.is_string() && x->second.string_value() == ""))
			json.erase(x);
	}
}

inline void JsonReplace(Json::object& m2, Json::object m1)
{
	for (auto x : m1)
		m2[x.first] = x.second;
}

inline Json JsonParse(std::string s)
{
	string err;
	return Json::parse(s, err);
}

}

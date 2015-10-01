#pragma once

#include <string>
#include <sstream>
#include <time.h>
#include <chrono>
#include <algorithm>

#ifdef __APPLE__
#include <sstream>
#include "os_spec/StringUtils_osx.h"
#endif

#if __linux__
#include "os_spec/StringUtils_linux.h"
#endif

#ifdef _MSC_VER
#include "os_spec/StringUtils_windows.h"
#endif

inline std::wstring s2ws(const char *cstr);
inline std::string ws2s(const wchar_t *wcstr);
 
inline std::string ws2s(std::wstring ws)
{
	return ws2s(ws.c_str());
}

inline std::wstring s2ws(std::string s)
{
	return s2ws(s.c_str());
}

inline std::string s2lower(const char *cstr)
{
    std::string ret = cstr;
    transform(ret.begin(), ret.end(), ret.begin(), (int(*)(int))tolower);
    return ret;
}

inline std::string s2upper(const char *cstr)
{
    std::string ret = cstr;
    transform(ret.begin(), ret.end(), ret.begin(), (int(*)(int))toupper);
    return ret;
}

inline std::string simple_format(const char* s)
{
	std::stringstream ss;
    while (*s) {
        if (*s == '%') {
            if (*(s + 1) == '%') {
                ++s;
			}
        }
        ss << *s++;
    }
	return ss.str();
}

// simple_format works like printf, but only allows typeless "%" as format specifier relying on stream operators type safe formatting. Example:
//         simple_format("one: % is correct, but two: %d, are %d %s", 1, 2, "ba", "format specifier"); 
// Results in returning: "one: 1 is correct, but two: 2d, are bad format specifiers"
//
template<typename T, typename... Args>
std::string simple_format(const char* s, T& value, Args... args)
{
	std::stringstream ss;
    while (*s) {
        if (*s == '%') {
            if (*(s + 1) == '%') {
                ++s;
            }
            else {
                ss << value;
                ss << simple_format(s + 1, args...);
				break;
            }
        }
        ss << *s++;
    }
	return ss.str();
}

inline std::string trim(const std::string &s)
{
   auto  wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   return std::string(wsfront,std::find_if_not(s.rbegin(),std::string::const_reverse_iterator(wsfront),[](int c){return std::isspace(c);}).base());
}

inline std::string iso_time(std::time_t t = std::time(NULL))
{
	// auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    char time[256] = {0};
	std::strftime(time, sizeof(time), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&t));
	return time;
}

inline std::string d2s(double d, int precision = 2)
{
	std::stringstream ss;
	ss.precision(precision);
	ss << std::fixed << d;
	std::string s = ss.str();
	s.erase(s.find_last_not_of('0') + 1, std::string::npos);	//remove trailing zeros
	return (s[s.size() - 1] == '.') ? s.substr(0, s.size() - 1) : s;	// remove decimal point if present
}

inline std::string humanable_size(uint64_t i)
{
	double d = (double) i;

	int units = 0;
	for (; d > 1024.0 && units <= 5; units++)
		d /= 1024.0;

	switch (units)
	{
	case 0:
		return d2s(d) + "B";
	case 1:
		return d2s(d) + "KB";
	case 2:
		return d2s(d) + "MB";
	case 3:
		return d2s(d) + "GB";
	case 4:
		return d2s(d) + "TB";
	default:
		return d2s(d) + "PB";
	}
}

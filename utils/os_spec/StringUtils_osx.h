#include <codecvt>

inline std::wstring s2ws(const char* s)
{
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(s);
}

inline std::string ws2s(const wchar_t* s)
{
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(s);
}

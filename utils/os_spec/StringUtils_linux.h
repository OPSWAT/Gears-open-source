#pragma once

inline std::string ws2s(const wchar_t *wcstr)
{
    std::wstring wsInput = wcstr;
    std::string ret;
    size_t origsize = wsInput.length() + 1;
    char* converted = new char[origsize];
    wcstombs(converted, wsInput.c_str(), origsize);
    ret = (std::string) converted;
    delete [] converted;
    return ret;
}

inline std::wstring s2ws(const char *cstr)
{
    std::string sInput = cstr;
    std::wstring ret;
    size_t origsize = sInput.length() + 1;
    wchar_t* converted = new wchar_t[origsize];
    mbstowcs(converted, sInput.c_str(), origsize);
    ret = (std::wstring) converted;
    delete [] converted;
    return ret;
}


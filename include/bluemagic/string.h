#pragma once

#include <string>
#include <sstream>

namespace bluemagic
{

template<typename... A>
inline std::string tstrf(const char* f, A... a)
{
    char s[1024];
    sprintf_s(&s[0], 1024, f, a...);
    return std::string(s);
}

template<typename... A>
inline std::wstring twstrf(const wchar_t* f, A... a)
{
    wchar_t ws[1024];
    swprintf_s(&ws[0], 1024, f, a...);
    return std::wstring(ws);
}

}
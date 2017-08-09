#pragma once

#include <algorithm>
#include <string>
#include <sstream>

namespace bluemagic
{

#ifdef UNICODE
#define strf strfW
#define strcmp strcmpW
typedef std::wstring TSTR;
#else
#define strf strfA
#define strcmp strcmpA
typedef std::string TSTR;
#endif

template<typename... A>
inline std::string strfA(const char* f, A... a)
{
    char s[1024];
    sprintf_s(&s[0], 1024, f, a...);
    return std::string(s);
}

template<typename... A>
inline std::wstring strfW(const wchar_t* f, A... a)
{
    wchar_t ws[1024];
    swprintf_s(&ws[0], 1024, f, a...);
    return std::wstring(ws);
}

inline BOOL strcmpA(const char* a, const char* b, bool ignorecase = false)
{
    if (ignorecase)
    {
        std::string sa(a);
        std::string sb(b);
        std::transform(std::begin(sa), std::end(sa), std::begin(sa), ::tolower);
        std::transform(std::begin(sb), std::end(sb), std::begin(sb), ::tolower);
        return !lstrcmpA(&sa[0], &sb[0]);
    }

    return !lstrcmpA(a, b);
}

inline BOOL strcmpW(const wchar_t* a, const wchar_t* b, bool ignorecase = false)
{
    if (ignorecase)
    {
        std::wstring wsa(a);
        std::wstring wsb(b);
        std::transform(std::begin(wsa), std::end(wsa), std::begin(wsa), ::tolower);
        std::transform(std::begin(wsb), std::end(wsb), std::begin(wsb), ::tolower);
        return !lstrcmpW(&wsa[0], &wsb[0]);
    }

    return !lstrcmpW(a, b);
}

}

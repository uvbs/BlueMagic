#pragma once

#include <windows.h>
#include <memory>
#include <type_traits>
#include <vector>

namespace bluemagic
{

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static void* GenericToPointer(T& t)
{
    return reinterpret_cast<void*>(t);
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static T PointerToGeneric(void* p)
{
    return reinterpret_cast<T>(p);
}

static void* BytesToPointer(std::vector<BYTE>& b)
{
    return reinterpret_cast<void*>(&b[0]);
}

static std::vector<BYTE> PointerToBytes(void* p, SIZE_T size)
{
    BYTE* pb = reinterpret_cast<BYTE*>(p);
    return std::vector<BYTE>(pb, pb + size);
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static std::vector<BYTE> GenericToBytes(T t)
{
    return reinterpret_cast<std::vector<BYTE>>(std::addressof(t));
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static T BytesToGeneric(std::vector<BYTE> b)
{
    return reinterpret_cast<T&>(b[0]);
}

}

#pragma once

#include <windows.h>
#include <type_traits>
#include <vector>

#include "protect_operation.h"
#include "pointer.h"
#include "type_converter.h"
#include "winapi_impl.h"

namespace bluemagic
{

static std::vector<BYTE> Read(HANDLE processHandle, UINT_PTR address, SIZE_T size)
{
    return ReadProcessMemoryImpl(processHandle, address, size);;
}

template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static T Read(HANDLE processHandle, UINT_PTR address)
{
    return BytesToGeneric<T>(Read(processHandle, address, sizeof(T)));
}

static bool Write(HANDLE processHandle, UINT_PTR address, std::vector<BYTE> bytes)
{
    ProtectOperation protect = ProtectOperation(processHandle, address, bytes.size());
    return WriteProcessMemoryImpl(processHandle, address, bytes) == bytes.size();
}

template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static bool Write(HANDLE processHandle, UINT_PTR address, T value)
{
    return Write(processHandle, address, GenericToBytes(value));
}

}

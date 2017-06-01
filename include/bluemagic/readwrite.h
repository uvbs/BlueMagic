#pragma once

#include <windows.h>
#include <type_traits>
#include <vector>

#include "protect_operation.h"
#include "pointer.h"
#include "type_converter.h"
#include "winapi.h"

namespace bluemagic
{

static std::vector<BYTE> ReadMemory(HANDLE processHandle, UINT_PTR address, SIZE_T size)
{
    return winapi::ReadProcessMemory(processHandle, address, size);;
}

static std::vector<BYTE> ReadMemory(HANDLE processHandle, Pointer pointer, SIZE_T size)
{
    if (pointer.Offsets.size() == 0)
        return winapi::ReadProcessMemory(processHandle, pointer.BaseAddress, size);

    SIZE_T addressSize = sizeof(UINT_PTR);
    UINT_PTR address = BytesToGeneric<UINT_PTR>(winapi::ReadProcessMemory(processHandle, pointer.BaseAddress, addressSize));
    SIZE_T offsetsCount = pointer.Offsets.size() - 1;

    for (SIZE_T i = 0; i < offsetsCount; ++i)
        address = BytesToGeneric<UINT_PTR>(winapi::ReadProcessMemory(processHandle, address + pointer.Offsets[i], addressSize));

    return winapi::ReadProcessMemory(processHandle, address + pointer.Offsets[offsetsCount], size);
}

template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static const T ReadMemory(HANDLE processHandle, UINT_PTR address, SIZE_T size)
{
    return BytesToGeneric<T>(ReadMemory(processHandle, address, size));
}

template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static const T ReadMemory(HANDLE processHandle, Pointer pointer, SIZE_T size)
{
    return BytesToGeneric<T>(ReadMemory(processHandle, pointer, size));
}

static bool WriteMemory(HANDLE processHandle, UINT_PTR address, std::vector<BYTE> bytes)
{
    ProtectOperation protect = ProtectOperation(processHandle, address, bytes.size());
    return winapi::WriteProcessMemory(processHandle, address, bytes) == bytes.size();
}

static bool WriteMemory(HANDLE processHandle, Pointer pointer, std::vector<BYTE> bytes)
{
    SIZE_T bytesSize = bytes.size();
    if (pointer.Offsets.size() == 0)
    {
        ProtectOperation protect = ProtectOperation(processHandle, pointer.BaseAddress, bytesSize);
        return winapi::WriteProcessMemory(processHandle, pointer.BaseAddress, bytes) == bytesSize;
    }

    SIZE_T addressSize = sizeof(UINT_PTR);
    UINT_PTR address = BytesToGeneric<UINT_PTR>(ReadMemory(processHandle, pointer.BaseAddress, addressSize));
    SIZE_T offsetsCount = pointer.Offsets.size() - 1;

    for (SIZE_T i = 0; i < offsetsCount; ++i)
        address = BytesToGeneric<UINT_PTR>(ReadMemory(processHandle, address + pointer.Offsets[i], addressSize));

    address += pointer.Offsets[offsetsCount];
    ProtectOperation protect = ProtectOperation(processHandle, address, bytesSize);
    return winapi::WriteProcessMemory(processHandle, address, bytes) == bytesSize;
}

template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static bool WriteMemory(HANDLE processHandle, UINT_PTR address, T value)
{
    return Write(processHandle, address, GenericToBytes(value));
}

template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static bool WriteMemory(HANDLE processHandle, Pointer pointer, T value)
{
    return Write(processHandle, pointer, GenericToBytes(value));
}

}
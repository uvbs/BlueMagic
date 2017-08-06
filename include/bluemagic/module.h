#pragma once

#include <windows.h>
#include <tlhelp32.h>

#include "string.h"
#include "type_converter.h"
#include "winapi_impl.h"

namespace bluemagic
{

class Module
{
public:
    Module() { }

    Module(MODULEENTRY32 moduleEntry)
    {
        Name = moduleEntry.szModule;
        Path = moduleEntry.szExePath;
        Handle = moduleEntry.hModule;
        BaseAddress = PointerToGeneric<UINT_PTR>(moduleEntry.modBaseAddr);
        MemorySize = moduleEntry.modBaseSize;
        ProcessId = moduleEntry.th32ProcessID;
    }

    TSTR Name;
    TSTR Path;
    HMODULE Handle;
    UINT_PTR BaseAddress;
    DWORD MemorySize;
    DWORD ProcessId;
};

inline bool operator==(const Module& lhs, const Module& rhs)
{
    return lhs.Name == rhs.Name && lhs.MemorySize == rhs.MemorySize;
}

inline bool operator!=(const Module& lhs, const Module& rhs)
{
    return !(lhs == rhs);
}

}

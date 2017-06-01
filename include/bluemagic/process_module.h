#pragma once

#include <windows.h>
#include <tlhelp32.h>

#include "type_converter.h"
#include "winapi.h"

namespace bluemagic
{

class ProcessModule
{
    public:
        ProcessModule() { }

        ProcessModule(MODULEENTRY32 moduleEntry)
        {
            Name = moduleEntry.szModule;
            Path = moduleEntry.szExePath;
            Id = moduleEntry.th32ModuleID;
            Handle = moduleEntry.hModule;
            BaseAddress = PointerToGeneric<UINT_PTR>(moduleEntry.modBaseAddr);
            MemorySize = moduleEntry.modBaseSize;
            ProcessId = moduleEntry.th32ProcessID;
            ProcessUsageCount = moduleEntry.ProccntUsage;
            GlobalUsageCount = moduleEntry.GlblcntUsage;
        }

        bool operator==(const ProcessModule& rhs) const
        {
            return this->Id == rhs.Id && this->Handle == rhs.Handle;
        }

        TSTR Name;
        TSTR Path;
        DWORD Id;
        HMODULE Handle;
        UINT_PTR BaseAddress;
        DWORD MemorySize;
        DWORD ProcessId;
        DWORD ProcessUsageCount;
        DWORD GlobalUsageCount;
};

}

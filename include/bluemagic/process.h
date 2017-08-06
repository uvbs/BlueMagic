#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <vector>

#include "module.h"
#include "string.h"
#include "winapi_impl.h"

namespace bluemagic
{

class Process
{
public:
    Process() { }

    Process(PROCESSENTRY32 processEntry)
    {
        Id = processEntry.th32ProcessID;
        Handle = OpenProcessImpl(Id);
        Is64 = false;

        std::vector<MODULEENTRY32> moduleEntries = GetModulesFromProcessImpl(Id);
        for (MODULEENTRY32 me32 : moduleEntries)
        {
            Module m = Module(me32);

            if (strcmp(me32.szModule, processEntry.szExeFile, true))
                MainModule = m;

            Modules.push_back(m);

            if (m.BaseAddress >= 0x100000000)
                Is64 = true;
        }

        Name = GetModuleBaseNameImpl(Handle, MainModule.Handle);
    }

    ~Process()
    {
        CloseHandle(Handle);
    }

    TSTR Name;
    DWORD Id;
    HANDLE Handle;
    bool Is64;
    Module MainModule;
    std::vector<Module> Modules;
};

inline bool operator==(const Process& lhs, const Process& rhs)
{
    return lhs.Modules == rhs.Modules;
}

inline bool operator!=(const Process& lhs, const Process& rhs)
{
    return !(lhs == rhs);
}

}

#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <vector>

#include "winapi_impl.h"
#include "process_module.h"

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
            Is64 = IsWow64ProcessImpl(Handle);

            std::vector<MODULEENTRY32> moduleEntries = GetModulesFromProcessImpl(Id);
            for (MODULEENTRY32 me32 : moduleEntries)
            {
                ProcessModule m = ProcessModule(me32);
                if (strcmp(me32.szModule, processEntry.szExeFile, true))
                    MainModule = m;
                Modules.push_back(m);
            }

            Name = GetProcessModuleBaseNameImpl(Handle, MainModule.Handle);
        }

        ~Process()
        {
            CloseHandle(Handle);
        }

        bool operator==(const Process& rhs) const
        {
            return this->Id == rhs.Id && this->Handle == rhs.Handle;
        }

        TSTR Name;
        DWORD Id;
        HANDLE Handle;
        bool Is64;
        ProcessModule MainModule;
        std::vector<ProcessModule> Modules;
};

}

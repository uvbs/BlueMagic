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
    explicit Process() :
        _name{ TSTR() }, _id{ 0 }, _handle{ nullptr },
        _is64{ false }, _main_module{ nullptr }, _modules{ std::vector<Module*>() }
    {
    }

    explicit Process(PROCESSENTRY32 processEntry) :
        _name{ TSTR() }, _id{ processEntry.th32ProcessID }, _handle{ OpenProcessImpl(_id) },
        _is64{ false }, _main_module{ nullptr }, _modules{ std::vector<Module*>() }
    {
        std::vector<MODULEENTRY32> moduleEntries = GetModulesFromProcessImpl(_id);
        for (MODULEENTRY32 me32 : moduleEntries)
        {
            Module* m = new Module(me32);

            if (strcmp(me32.szModule, processEntry.szExeFile, true))
                _main_module = m;

            _modules.push_back(m);

            if (m->GetBaseAddress() >= 0x100000000)
                _is64 = true;
        }

        _name = GetModuleBaseNameImpl(_handle, _main_module->GetHandle());
    }

    Process(Process const& other) = delete;

    Process& operator=(Process const& other) = delete;

    Process(Process&& other)
    {
        _name = other._name;
        _id = other._id;
        _handle = std::move(other._handle);
        _is64 = other._is64;
        _main_module = other._main_module;
        _modules = other._modules;
    }

    Process& operator=(Process&& other)
    {
        _name = other._name;
        _id = other._id;
        _handle = std::move(other._handle);
        _is64 = other._is64;
        _main_module = other._main_module;
        _modules = other._modules;

        return *this;
    }

    ~Process()
    {
        CloseHandle(_handle);
    }

    TSTR GetName() const
    {
        return _name;
    }

    DWORD GetId() const
    {
        return _id;
    }

    HANDLE GetHandle() const
    {
        return _handle;
    }

    bool Is64() const
    {
        return _is64;
    }

    Module* GetMainModule() const
    {
        return _main_module;
    }

    std::vector<Module*> GetModules() const
    {
        return _modules;
    }

private:
    TSTR _name;
    DWORD _id;
    HANDLE _handle;
    bool _is64;
    Module* _main_module;
    std::vector<Module*> _modules;
};

inline bool operator==(const Process& lhs, const Process& rhs)
{
    return lhs.GetModules() == rhs.GetModules();
}

inline bool operator!=(const Process& lhs, const Process& rhs)
{
    return !(lhs == rhs);
}

}

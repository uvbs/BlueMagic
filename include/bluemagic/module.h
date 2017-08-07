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
    explicit Module() :
        _name{ TSTR() }, _path{ TSTR() },
        _handle{ nullptr }, _base_address{ 0 },
        _memory_size{ 0 }, _process_id{ 0 }
    {
    }

    explicit Module(MODULEENTRY32 moduleEntry) :
        _name{moduleEntry.szModule}, _path{moduleEntry.szExePath},
        _handle{moduleEntry.hModule}, _base_address{PointerToGeneric<UINT_PTR>(moduleEntry.modBaseAddr)},
        _memory_size{moduleEntry.modBaseSize}, _process_id{moduleEntry.th32ProcessID}
    {
    }

    Module(Module const& other) = delete;

    Module& operator=(Module const& other) = delete;

    Module(Module&& other)
    {
        _name = other._name;
        _path = other._path;
        _handle = std::move(other._handle);
        _base_address = other._base_address;
        _memory_size = other._memory_size;
        _process_id = other._process_id;
    }

    Module& operator=(Module&& other)
    {
        _name = other._name;
        _path = other._path;
        _handle = std::move(other._handle);
        _base_address = other._base_address;
        _memory_size = other._memory_size;
        _process_id = other._process_id;

        return *this;
    }

    TSTR GetName() const
    {
        return _name;
    }

    TSTR GetPath() const
    {
        return _path;
    }

    HMODULE GetHandle() const
    {
        return _handle;
    }

    UINT_PTR GetBaseAddress() const
    {
        return _base_address;
    }

    DWORD GetMemorySize() const
    {
        return _memory_size;
    }

    DWORD GetParentProcessId() const
    {
        return _process_id;
    }

private:
    TSTR _name;
    TSTR _path;
    HMODULE _handle;
    UINT_PTR _base_address;
    DWORD _memory_size;
    DWORD _process_id;
};

inline bool operator==(const Module& lhs, const Module& rhs)
{
    return lhs.GetName() == rhs.GetName() && lhs.GetMemorySize() == rhs.GetMemorySize();
}

inline bool operator!=(const Module& lhs, const Module& rhs)
{
    return !(lhs == rhs);
}

}

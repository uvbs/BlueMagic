#pragma once

#include <windows.h>

#include "winapi_impl.h"

namespace bluemagic
{

class ProtectGuard
{
public:
    explicit ProtectGuard() :
        _process_handle{ nullptr }, _address{ 0 }, _size{ 0 }, _new_protect{ 0 },
        _old_protect{ 0 }
    {
    }

    explicit ProtectGuard(UINT_PTR address, SIZE_T size, DWORD protection = PAGE_EXECUTE_READWRITE) :
        _process_handle{ nullptr }, _address{ address }, _size{ size }, _new_protect{ protection },
        _old_protect{ VirtualProtectImpl(address, size, protection) }
    {
    }

    explicit ProtectGuard(HANDLE processHandle, UINT_PTR address, SIZE_T size, DWORD protection = PAGE_EXECUTE_READWRITE) :
        _process_handle{ processHandle }, _address{ address }, _size{ size }, _new_protect{ protection },
        _old_protect{ VirtualProtectExImpl(processHandle, address, size, protection) }
    {
    }

    ProtectGuard(const ProtectGuard& other) = delete;

    ProtectGuard& operator=(const ProtectGuard& other) = delete;

    ProtectGuard(ProtectGuard&& other)
    {
        _process_handle = std::move(other._process_handle);
        _address = other._address;
        _size = other._size;
        _new_protect = other._new_protect;
        _old_protect = VirtualProtectExImpl(_process_handle, _address, _size, _new_protect);

        other._old_protect = 0;
    }

    ProtectGuard& operator=(ProtectGuard&& other)
    {
        _process_handle = std::move(other._process_handle);
        _address = other._address;
        _size = other._size;
        _new_protect = other._new_protect;
        _old_protect = VirtualProtectExImpl(_process_handle, _address, _size, _new_protect);

        other._old_protect = 0;

        return *this;
    }

    ~ProtectGuard()
    {
        Restore();
    }

    void Apply()
    {
        if (_process_handle)
            VirtualProtectExImpl(_process_handle, _address, _size, _new_protect);
        else
            VirtualProtectImpl(_address, _size, _new_protect);
    }

    void Restore()
    {
        if (_process_handle)
            VirtualProtectExImpl(_process_handle, _address, _size, _old_protect);
        else
            VirtualProtectImpl(_address, _size, _old_protect);
    }

private:
    HANDLE _process_handle;
    UINT_PTR _address;
    SIZE_T _size;
    DWORD _old_protect;
    DWORD _new_protect;
};

}

#pragma once

#include <windows.h>

#include "winapi_impl.h"

namespace bluemagic
{

class ProtectOperation
{
public:
    ProtectOperation(UINT_PTR address, SIZE_T size, DWORD protection = PAGE_EXECUTE_READWRITE)
    {
        ProcessHandle = nullptr;
        Address = address;
        Size = size;
        NewProtection = protection;
        OldProtection = VirtualProtectExImpl(ProcessHandle, Address, Size, NewProtection);
    }

    ProtectOperation(HANDLE processHandle, UINT_PTR address, SIZE_T size, DWORD protection = PAGE_EXECUTE_READWRITE)
    {
        ProcessHandle = processHandle;
        Address = address;
        Size = size;
        NewProtection = protection;
        OldProtection = VirtualProtectExImpl(ProcessHandle, Address, Size, NewProtection);
    }

    ~ProtectOperation()
    {
        Restore();
    }

    void Restore()
    {
        if (ProcessHandle)
            VirtualProtectExImpl(ProcessHandle, Address, Size, OldProtection);
        else
            VirtualProtectImpl(Address, Size, OldProtection);
    }

private:
    HANDLE ProcessHandle;
    UINT_PTR Address;
    SIZE_T Size;
    DWORD OldProtection;
    DWORD NewProtection;
};

}

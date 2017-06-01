#pragma once

#include <windows.h>

#include "winapi.h"

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
            OldProtection = winapi::VirtualProtectEx(ProcessHandle, Address, Size, NewProtection);
        }

        ProtectOperation(HANDLE processHandle, UINT_PTR address, SIZE_T size, DWORD protection = PAGE_EXECUTE_READWRITE)
        {
            ProcessHandle = processHandle;
            Address = address;
            Size = size;
            NewProtection = protection;
            OldProtection = winapi::VirtualProtectEx(ProcessHandle, Address, Size, NewProtection);
        }

        ~ProtectOperation()
        {
            Restore();
        }

        void Restore()
        {
            if (ProcessHandle)
                winapi::VirtualProtectEx(ProcessHandle, Address, Size, OldProtection);
            else
                winapi::VirtualProtect(Address, Size, OldProtection);
        }

    private:
        HANDLE ProcessHandle;
        UINT_PTR Address;
        SIZE_T Size;
        DWORD OldProtection;
        DWORD NewProtection;
};

}

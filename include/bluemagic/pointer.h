#pragma once

#include <windows.h>
#include <vector>

namespace bluemagic
{

class Pointer
{
    public:
        UINT_PTR BaseAddress;
        std::vector<DWORD> Offsets;

        Pointer() { }

        Pointer(UINT_PTR baseAddress, std::vector<DWORD> offsets)
        {
            BaseAddress = baseAddress;
            for (DWORD offset : offsets)
                Offsets.push_back(offset);
        }

        bool operator==(const Pointer& rhs) const
        {
            return this->BaseAddress == rhs.BaseAddress && this->Offsets == rhs.Offsets;
        }
};

}
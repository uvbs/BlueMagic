#pragma once

#include <windows.h>
#include <vector>

#include "readwrite.h"

namespace bluemagic
{

class Pointer
{
public:
    Pointer(HANDLE processHandle, UINT_PTR baseAddress, UINT_PTR baseOffset, std::vector<UINT_PTR> offsets) :
        _process_handle{ processHandle }, _levels{ std::vector<std::pair<UINT_PTR, UINT_PTR>>(offsets.size() + 1) }
    {
        _levels[0] = std::make_pair(baseAddress, baseOffset);
        for (SIZE_T i = 1; i < _levels.size(); ++i)
            _levels[i] = std::make_pair(0, offsets[i - 1]);
    }

    void Resolve(SIZE_T index = 0)
    {
        for (; index < _levels.size() - 1; ++index)
            _levels[index + 1].first = bluemagic::Read<UINT_PTR>(_process_handle, GetAddress());
    }

    std::vector<BYTE> Read(SIZE_T size) const
    {
        return bluemagic::Read(_process_handle, GetAddress(), size);
    }

    template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>>>
    T Read() const
    {
        return bluemagic::Read<T>(_process_handle, GetAddress());
    }

    bool Write(std::vector<BYTE> bytes) const
    {
        return bluemagic::Write(_process_handle, GetAddress(), bytes);
    }

    template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>>>
    bool Write(T value) const
    {
        return bluemagic::Write(ProcessHandle, GetAddress(), value);
    }

    UINT_PTR GetAddress() const
    {
        return _levels[_levels.size() - 1].first + _levels[_levels.size() - 1].second;
    }

    HANDLE GetProcessHandle() const
    {
        return _process_handle;
    }

    std::vector<std::pair<UINT_PTR, UINT_PTR>> GetLevels() const
    {
        return _levels;
    }

private:
    HANDLE _process_handle;
    std::vector<std::pair<UINT_PTR, UINT_PTR>> _levels;
};

inline bool operator==(const Pointer& lhs, const Pointer& rhs)
{
    std::vector<std::pair<UINT_PTR, UINT_PTR>> lhslevels = lhs.GetLevels(), rhslevels = rhs.GetLevels();

    if (lhs.GetProcessHandle() != rhs.GetProcessHandle() || lhslevels.size() != rhslevels.size())
        return false;

    for (SIZE_T i = 0; i < lhslevels.size(); ++i)
    {
        if (i != 0)
        {
            if (lhslevels[i].second != rhslevels[i].second)
                return false;
        }
        else
        {
            if (lhslevels[i].first != rhslevels[i].first || lhslevels[i].second != rhslevels[i].second)
                return false;
        }
    }

    return true;
}

inline bool operator!=(const Pointer& lhs, const Pointer& rhs)
{
    return !(lhs == rhs);
}

}

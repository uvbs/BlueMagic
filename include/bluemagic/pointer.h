#pragma once

#include <windows.h>
#include <vector>

#include "readwrite.h"

namespace bluemagic
{

    class Pointer
    {
    public:
        HANDLE ProcessHandle;
        std::vector<std::pair<UINT_PTR, UINT_PTR>> Levels;
        SIZE_T LevelsCount;

        Pointer() { }

        Pointer(HANDLE processHandle, UINT_PTR baseAddress, UINT_PTR baseOffset, std::vector<UINT_PTR> offsets)
        {
            ProcessHandle = processHandle;
            LevelsCount = offsets.size() + 1;
            Levels.resize(LevelsCount);
            Levels[0] = std::make_pair(baseAddress, baseOffset);
            for (SIZE_T i = 1; i < LevelsCount; ++i)
                Levels[i] = std::make_pair(0, offsets[i - 1]);
        }

        UINT_PTR GetAddress() const
        {
            const std::pair<UINT_PTR, UINT_PTR>& l = Levels[LevelsCount - 1];
            return l.first + l.second;
        }

        void Resolve(SIZE_T index = 0)
        {
            for (; index < LevelsCount - 1; ++index)
                Levels[index + 1].first = bluemagic::Read<UINT_PTR>(ProcessHandle, GetAddress());
        }

        std::vector<BYTE> Read(SIZE_T size) const
        {
            return bluemagic::Read(ProcessHandle, GetAddress(), size);
        }

        template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
        T Read() const
        {
            return bluemagic::Read<T>(ProcessHandle, GetAddress());
        }

        bool Write(std::vector<BYTE> bytes) const
        {
            return bluemagic::Write(ProcessHandle, GetAddress(), bytes);
        }

        template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
        bool Write(T value) const
        {
            return bluemagic::Write(ProcessHandle, GetAddress(), value);
        }
    };

    inline bool operator==(const Pointer& lhs, const Pointer& rhs)
    {
        if (lhs.ProcessHandle != rhs.ProcessHandle || lhs.LevelsCount != rhs.LevelsCount)
            return false;

        for (SIZE_T i = 0; i < lhs.LevelsCount; ++i)
        {
            if (i != 0)
            {
                if (lhs.Levels[i].second != rhs.Levels[i].second)
                    return false;
            }
            else
            {
                if (lhs.Levels[i].first != rhs.Levels[i].first || lhs.Levels[i].second != rhs.Levels[i].second)
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

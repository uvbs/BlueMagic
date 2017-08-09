#pragma once

#include <windows.h>
#include <type_traits>
#include <vector>

#include "readwrite.h"
#include "regions.h"
#include "process.h"
#include "module.h"
#include "signature.h"

namespace bluemagic
{

static std::vector<UINT_PTR> ScanBuffer(std::vector<BYTE> buffer, std::vector<BYTE> bytes)
{
    std::vector<UINT_PTR> results;
    SIZE_T bs = bytes.size();
    SIZE_T bfs = buffer.size() - bs;
    SIZE_T i, j, k;
    bool f;

    for (i = 0; i <= bfs; ++i)
    {
        if (buffer[i] == bytes[0])
        {
            for (j = i, k = 1, f = true; k < bs; ++i, ++k)
            {
                if (buffer[j + k] != bytes[k])
                {
                    f = false;
                    break;
                }
            }

            if (f)
                results.push_back(j);
        }
    }

    return results;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>>>
static std::vector<UINT_PTR> ScanBuffer(std::vector<BYTE> buffer, T value)
{
    return ScanBuffer(buffer, GenericToBytes(value));
}

static std::vector<UINT_PTR> ScanBuffer(std::vector<BYTE> buffer, Signature signature)
{
    if (!signature.ToBytes().empty())
        return ScanBuffer(buffer, signature.ToBytes());

    std::vector<UINT_PTR> results;
    std::string ss = signature.ToString();
    SIZE_T sss = ss.size();
    SIZE_T sbs = sss / 2;
    SIZE_T bfs = buffer.size() - sbs;
    SIZE_T i, j, k, l, m, x = sss + 1, y, z;
    bool f;
    CHAR b[2];

    for (i = 0; x > sss && i < sss; i += 2)
    {
        if (!strcmpA(ss.substr(i, 2).c_str(), "??"))
            x = i / 2;
    }

    SIZE_T bs = sbs - x;

    for (i = x, y = x * 2, z = y + 1; i <= bfs; ++i)
    {
        sprintf_s(&b[0], 4, "%02x", buffer[i]);

        if ((ss[y] == b[0] && ss[z] == b[1]) || (ss[y] == '?' && ss[z] == '?') || (ss[y] == '?' && ss[z] == b[1]) || (ss[z] == '?' && ss[y] == b[0]))
        {
            for (j = i, k = 1, l = (k * 2) + y, m = l + 1, f = true; k < bs; ++i, ++k, l += 2, m = l + 1)
            {
                sprintf_s(&b[0], 4, "%02x", buffer[j + k]);

                if ((ss[l] != '?' && ss[l] != b[0]) || (ss[m] != '?' && ss[m] != b[1]))
                {
                    f = false;
                    break;
                }
            }

            if (f)
                results.push_back(j - x);
        }
    }

    return results;
}

static std::vector<UINT_PTR> ScanRegion(HANDLE processHandle, MEMORY_BASIC_INFORMATION region, std::vector<BYTE> bytes)
{
    std::vector<UINT_PTR> results;
    std::vector<UINT_PTR> addresses = ScanBuffer(Read(processHandle, PointerToGeneric<UINT_PTR>(region.BaseAddress), region.RegionSize), bytes);
    for (UINT_PTR address : addresses)
        results.push_back(PointerToGeneric<UINT_PTR>(region.BaseAddress) + address);

    return results;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>>>
static std::vector<UINT_PTR> ScanRegion(HANDLE processHandle, MEMORY_BASIC_INFORMATION region, T value)
{
    std::vector<UINT_PTR> results;
    std::vector<UINT_PTR> addresses = ScanBuffer(Read(processHandle, PointerToGeneric<UINT_PTR>(region.BaseAddress), region.RegionSize), value);
    for (UINT_PTR address : addresses)
        results.push_back(PointerToGeneric<UINT_PTR>(region.BaseAddress) + address);

    return results;
}

static std::vector<UINT_PTR> ScanRegion(HANDLE processHandle, MEMORY_BASIC_INFORMATION region, Signature signature)
{
    std::vector<UINT_PTR> results;
    std::vector<UINT_PTR> addresses = ScanBuffer(Read(processHandle, PointerToGeneric<UINT_PTR>(region.BaseAddress), region.RegionSize), signature);
    for (UINT_PTR address : addresses)
        results.push_back(PointerToGeneric<UINT_PTR>(region.BaseAddress) + address);

    return results;
}

static std::vector<UINT_PTR> ScanModule(Process* process, Module* module, std::vector<BYTE> bytes)
{
    std::vector<UINT_PTR> results;
    for (MEMORY_BASIC_INFORMATION region : LoadModuleRegions(process->GetHandle(), module))
        for (UINT_PTR address : ScanRegion(process->GetHandle(), region, bytes))
            results.push_back(address);

    return results;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>>>
static std::vector<UINT_PTR> ScanModule(Process* process, Module* module, T value)
{
    std::vector<UINT_PTR> results;
    for (MEMORY_BASIC_INFORMATION region : LoadModuleRegions(process->GetHandle(), module))
        for (UINT_PTR address : ScanRegion(process->GetHandle(), region, value))
            results.push_back(address);

    return results;
}

static std::vector<UINT_PTR> ScanModule(Process* process, Module* module, Signature signature)
{
    std::vector<UINT_PTR> results;
    for (MEMORY_BASIC_INFORMATION region : LoadModuleRegions(process->GetHandle(), module))
        for (UINT_PTR address : ScanRegion(process->GetHandle(), region, signature))
            results.push_back(address);

    return results;
}

static std::vector<UINT_PTR> ScanProcess(Process* process, std::vector<BYTE> bytes)
{
    std::vector<UINT_PTR> results;
    for (MEMORY_BASIC_INFORMATION region : LoadProcessRegions(process))
        for (UINT_PTR address : ScanRegion(process->GetHandle(), region, bytes))
            results.push_back(address);

    return results;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>>>
static std::vector<UINT_PTR> ScanProcess(Process* process, T value)
{
    std::vector<UINT_PTR> results;
    for (MEMORY_BASIC_INFORMATION region : LoadProcessRegions(process))
        for (UINT_PTR address : ScanRegion(process->GetHandle(), region, value))
            results.push_back(address);

    return results;
}

static std::vector<UINT_PTR> ScanProcess(Process* process, Signature signature)
{
    std::vector<UINT_PTR> results;
    for (MEMORY_BASIC_INFORMATION region : LoadProcessRegions(process))
        for (UINT_PTR address : ScanRegion(process->GetHandle(), region, signature))
            results.push_back(address);

    return results;
}

static UINT_PTR ScanAddress(HANDLE processHandle, UINT_PTR address, std::vector<BYTE> bytes)
{
    if (Read(processHandle, address, bytes.size()) == bytes)
        return address;

    return 0;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>>>
static UINT_PTR ScanAddress(HANDLE processHandle, UINT_PTR address, T value)
{
    return ScanAddress(processHandle, address, GenericToBytes(value));
}

static UINT_PTR ScanAddress(HANDLE processHandle, UINT_PTR address, Signature signature)
{
    if (!signature.ToBytes().empty())
        return ScanAddress(processHandle, address, signature.ToBytes());

    std::string ss = signature.ToString();
    SIZE_T sss = ss.size();
    SIZE_T sbs = sss / 2;
    std::vector<BYTE> buffer = Read(processHandle, address, sbs);
    SIZE_T i, j = sss + 1;
    CHAR b[2];

    for (i = 0; j > sss && i < sss; i += 2)
    {
        if (!strcmpA(ss.substr(i, 2).c_str(), "??"))
            j = i / 2;
    }

    SIZE_T bs = sbs - j;

    for (i = j, j = i + 1; i < bs; ++i, ++j)
    {
        sprintf_s(&b[0], 4, "%02x", buffer[i]);

        if ((ss[i] != '?' && ss[i] != b[0]) || (ss[j] != '?' && ss[j] != b[1]))
            return 0;
    }

    return address;
}

static std::vector<UINT_PTR> ScanAddresses(HANDLE processHandle, std::vector<BYTE> bytes, std::vector<UINT_PTR> addresses)
{
    std::vector<UINT_PTR> results;
    for (UINT_PTR address : addresses)
    {
        UINT_PTR result = ScanAddress(processHandle, address, bytes);
        if (result != 0)
            results.push_back(result);
    }

    return results;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>>>
static std::vector<UINT_PTR> ScanAddresses(HANDLE processHandle, T value, std::vector<UINT_PTR> addresses)
{
    std::vector<UINT_PTR> results;
    for (UINT_PTR address : addresses)
    {
        UINT_PTR result = ScanAddress(processHandle, address, value);
        if (result != 0)
            results.push_back(result);
    }

    return results;
}

static std::vector<UINT_PTR> ScanAddresses(HANDLE processHandle, Signature signature, std::vector<UINT_PTR> addresses)
{
    std::vector<UINT_PTR> results;
    for (UINT_PTR address : addresses)
    {
        UINT_PTR result = ScanAddress(processHandle, address, signature);
        if (result != 0)
            results.push_back(result);
    }

    return results;
}

}

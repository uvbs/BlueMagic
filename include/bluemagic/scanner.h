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

static std::vector<UINT_PTR> ScanBufferForBytes(std::vector<BYTE> buffer, std::vector<BYTE> bytes)
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

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static std::vector<UINT_PTR> ScanBufferForGeneric(std::vector<BYTE> buffer, T value)
{
    return ScanBufferForBytes(GenericToBytes(value), buffer);
}

static std::vector<UINT_PTR> ScanBufferForSignature(std::vector<BYTE> buffer, Signature signature)
{
    if (!signature.ToBytes().empty())
        return ScanBufferForBytes(buffer, signature.ToBytes());

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

static std::vector<UINT_PTR> ScanMemoryRegionForBytes(HANDLE processHandle, MEMORY_BASIC_INFORMATION region, std::vector<BYTE> bytes)
{
    std::vector<UINT_PTR> results;
    std::vector<UINT_PTR> addresses = ScanBufferForBytes(Read(processHandle, PointerToGeneric<UINT_PTR>(region.BaseAddress), region.RegionSize), bytes);
    for (UINT_PTR address : addresses)
        results.push_back(PointerToGeneric<UINT_PTR>(region.BaseAddress) + address);

    return results;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static std::vector<UINT_PTR> ScanMemoryRegionForGeneric(HANDLE processHandle, MEMORY_BASIC_INFORMATION region, T value)
{
    std::vector<UINT_PTR> results;
    std::vector<UINT_PTR> addresses = ScanBufferForGeneric(Read(processHandle, PointerToGeneric<UINT_PTR>(region.BaseAddress), region.RegionSize), value);
    for (UINT_PTR address : addresses)
        results.push_back(PointerToGeneric<UINT_PTR>(region.BaseAddress) + address);

    return results;
}

static std::vector<UINT_PTR> ScanMemoryRegionForSignature(HANDLE processHandle, MEMORY_BASIC_INFORMATION region, Signature signature)
{
    std::vector<UINT_PTR> results;
    std::vector<UINT_PTR> addresses = ScanBufferForSignature(Read(processHandle, PointerToGeneric<UINT_PTR>(region.BaseAddress), region.RegionSize), signature);
    for (UINT_PTR address : addresses)
        results.push_back(PointerToGeneric<UINT_PTR>(region.BaseAddress) + address);

    return results;
}

static std::vector<UINT_PTR> ScanAllMemoryRegionsForBytes(HANDLE processHandle, std::vector<MEMORY_BASIC_INFORMATION> regions, std::vector<BYTE> bytes)
{
    std::vector<UINT_PTR> results;
    for (MEMORY_BASIC_INFORMATION region : regions)
    {
        std::vector<UINT_PTR> r = ScanMemoryRegionForBytes(processHandle, region, bytes);
        results.insert(results.end(), r.begin(), r.end());
    }

    return results;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static std::vector<UINT_PTR> ScanAllMemoryRegionsForGeneric(HANDLE processHandle, std::vector<MEMORY_BASIC_INFORMATION> regions, T value)
{
    std::vector<UINT_PTR> results;
    for (MEMORY_BASIC_INFORMATION region : regions)
    {
        std::vector<UINT_PTR> r = ScanMemoryRegionForGeneric(processHandle, region, value);
        results.insert(results.end(), r.begin(), r.end());
    }

    return results;
}

static std::vector<UINT_PTR> ScanAllMemoryRegionsForSignature(HANDLE processHandle, std::vector<MEMORY_BASIC_INFORMATION> regions, Signature signature)
{
    std::vector<UINT_PTR> results;
    for (MEMORY_BASIC_INFORMATION region : regions)
    {
        std::vector<UINT_PTR> r = ScanMemoryRegionForSignature(processHandle, region, signature);
        results.insert(results.end(), r.begin(), r.end());
    }

    return results;
}

static std::vector<UINT_PTR> ScanModuleForBytes(Process* process, HANDLE processHandle, Module* module, std::vector<BYTE> bytes)
{
    return ScanAllMemoryRegionsForBytes(processHandle, LoadMemoryRegions(process, module), bytes);
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static std::vector<UINT_PTR> ScanModuleForGeneric(Process* process, HANDLE processHandle, Module* module, T value)
{
    return ScanAllMemoryRegionsForGeneric(processHandle, LoadMemoryRegions(process, module), value);
}

static std::vector<UINT_PTR> ScanModuleForSignature(Process* process, HANDLE processHandle, Module* module, Signature signature)
{
    return ScanAllMemoryRegionsForSignature(processHandle, LoadMemoryRegions(process, module), signature);
}

static std::vector<UINT_PTR> ScanAllModulesForBytes(Process* process, HANDLE processHandle, std::vector<BYTE> bytes)
{
    std::vector<UINT_PTR> results;
    for (Module* pm : process->GetModules())
        for (UINT_PTR address : ScanModuleForBytes(process, processHandle, pm, bytes))
            results.push_back(address);

    return results;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static std::vector<UINT_PTR> ScanAllModulesForGeneric(Process* process, HANDLE processHandle, T value)
{
    std::vector<UINT_PTR> results;
    for (Module* pm : process->GetModules())
        for (UINT_PTR address : ScanModuleForGeneric(process, processHandle, pm, value))
            results.push_back(address);

    return results;
}

static std::vector<UINT_PTR> ScanAllModulesForSignature(Process* process, HANDLE processHandle, Signature signature)
{
    std::vector<UINT_PTR> results;
    for (Module* pm : process->GetModules())
        for (UINT_PTR address : ScanModuleForSignature(process, processHandle, pm, signature))
            results.push_back(address);

    return results;
}

static UINT_PTR ScanAddressForBytes(HANDLE processHandle, UINT_PTR address, std::vector<BYTE> bytes)
{
    if (Read(processHandle, address, bytes.size()) == bytes)
        return address;

    return 0;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static UINT_PTR ScanAddressForGeneric(HANDLE processHandle, UINT_PTR address, T value)
{
    return ScanAddressForBytes(processHandle, address, GenericToBytes(value));
}

static UINT_PTR ScanAddressForSignature(HANDLE processHandle, UINT_PTR address, Signature signature)
{
    if (!signature.ToBytes().empty())
        return ScanAddressForBytes(processHandle, address, signature.ToBytes());

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

static std::vector<UINT_PTR> ScanAddressesForBytes(HANDLE processHandle, std::vector<BYTE> bytes, std::vector<UINT_PTR> addresses)
{
    std::vector<UINT_PTR> results;
    for (UINT_PTR address : addresses)
    {
        UINT_PTR result = ScanAddressForBytes(processHandle, address, bytes);
        if (result != 0)
            results.push_back(result);
    }

    return results;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static std::vector<UINT_PTR> ScanAddressesForGeneric(HANDLE processHandle, T value, std::vector<UINT_PTR> addresses)
{
    std::vector<UINT_PTR> results;
    for (UINT_PTR address : addresses)
    {
        UINT_PTR result = ScanAddressForGeneric(processHandle, address, value);
        if (result != 0)
            results.push_back(result);
    }

    return results;
}

static std::vector<UINT_PTR> ScanAddressesForSignature(HANDLE processHandle, Signature signature, std::vector<UINT_PTR> addresses)
{
    std::vector<UINT_PTR> results;
    for (UINT_PTR address : addresses)
    {
        UINT_PTR result = ScanAddressForSignature(processHandle, address, signature);
        if (result != 0)
            results.push_back(result);
    }

    return results;
}

}

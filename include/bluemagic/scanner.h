#pragma once

#include <windows.h>
#include <type_traits>
#include <vector>

#include "readwrite.h"
#include "regions.h"
#include "process.h"
#include "process_module.h"
#include "signature.h"

namespace bluemagic
{

static std::vector<UINT_PTR> ScanBufferForBytes(std::vector<BYTE> buffer, std::vector<BYTE> bytes)
{
    std::vector<UINT_PTR> results;
    SIZE_T bytesSize = bytes.size();
    SIZE_T bufferSize = buffer.size() - bytesSize;
    SIZE_T i, j, k;
    bool f;

    for (i = 0; i <= bufferSize; ++i)
    {
        if (buffer[i] == bytes[0])
        {
            for (j = i, k = 1, f = true; k < bytesSize; ++i, ++k)
            {
                if (buffer[j + k] != bytes[k])
                {
                    f = false;
                    break;
                }
            }

            if (f)
                results.push_back((UINT_PTR)(j));
        }
    }

    return results;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static std::vector<UINT_PTR> ScanBufferForGeneric(std::vector<BYTE> buffer, T value)
{
    return ScanForBytes(GenericToBytes(value), buffer);
}

static std::vector<UINT_PTR> ScanBufferForSignature(std::vector<BYTE> buffer, Signature signature)
{
    if (!signature.Bytes.empty())
        return ScanBufferForBytes(buffer, signature.Bytes);

    std::vector<UINT_PTR> results;
    SIZE_T signatureStringSize = signature.StringSize;
    SIZE_T signatureBytesSize = signatureStringSize / 2;
    SIZE_T bufferSize = buffer.size() - signatureBytesSize;
    std::string s = signature.String;
    SIZE_T i, j, k, l, m, x = signatureStringSize + 1, y, z;
    bool f;
    CHAR b[4];

    for (i = 0; x > signatureStringSize && i < signatureStringSize; i += 2)
    {
        if (strcmpA(s.substr(i, 2).c_str(), "??"))
            x = i / 2;
    }

    SIZE_T bytesSize = signatureBytesSize - x;

    for (i = x, y = x * 2, z = y + 1; i <= bufferSize; ++i)
    {
        _itoa_s(buffer[i], &b[0], 4, 16);

        if ((s[y] == b[0] && s[z] == b[1]) || (s[y] == '?' && s[z] == '?') || (s[y] == '?' && s[z] == b[1]) || (s[z] == '?' && s[y] == b[0]))
        {
            for (j = i, k = 1, l = (k * 2) + y, m = l + 1, f = true; k < bytesSize; ++i, ++k, l += 2, m = l + 1)
            {
                _itoa_s(buffer[j + k], &b[0], 4, 16);

                if ((s[l] != '?' && s[l] != b[0]) || (s[m] != '?' && s[m] != b[1]))
                {
                    f = false;
                    break;
                }
            }

            if (f)
                results.push_back((UINT_PTR)(j - x));
        }
    }

    return results;
}

static std::vector<UINT_PTR> ScanMemoryRegionForBytes(HANDLE processHandle, MEMORY_BASIC_INFORMATION region, std::vector<BYTE> bytes)
{
    std::vector<UINT_PTR> results;
    std::vector<UINT_PTR> addresses = ScanBufferForBytes(ReadMemory(processHandle, PointerToGeneric<UINT_PTR>(region.BaseAddress), region.RegionSize), bytes);
    for (UINT_PTR address : addresses)
        results.push_back(PointerToGeneric<UINT_PTR>(region.BaseAddress) + address);

    return results;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static std::vector<UINT_PTR> ScanMemoryRegionForGeneric(HANDLE processHandle, MEMORY_BASIC_INFORMATION region, T value)
{
    std::vector<UINT_PTR> results;
    std::vector<UINT_PTR> addresses = ScanBufferForGeneric(Read(processHandle, PointerToGeneric<UINT_PTR>(region.BaseAddress), region.RegionSize), value);
    foreach(UINT_PTR address in addresses)
        results.push_back(PointerToGeneric<UINT_PTR>(region.BaseAddress) + address);

    return results;
}

static std::vector<UINT_PTR> ScanMemoryRegionForSignature(HANDLE processHandle, MEMORY_BASIC_INFORMATION region, Signature signature)
{
    std::vector<UINT_PTR> results;
    std::vector<UINT_PTR> addresses = ScanBufferForSignature(ReadMemory(processHandle, PointerToGeneric<UINT_PTR>(region.BaseAddress), region.RegionSize), signature);
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

static std::vector<UINT_PTR> ScanProcessModuleForBytes(Process* process, HANDLE processHandle, ProcessModule module, std::vector<BYTE> bytes)
{
    return ScanAllMemoryRegionsForBytes(processHandle, LoadMemoryRegions(process, module), bytes);
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static std::vector<UINT_PTR> ScanProcessModuleForGeneric(Process process, HANDLE processHandle, ProcessModule module, T value)
{
    return ScanAllMemoryRegionsForGeneric(processHandle, Load(process, module), value);
}

static std::vector<UINT_PTR> ScanProcessModuleForSignature(Process* process, HANDLE processHandle, ProcessModule module, Signature signature)
{
    return ScanAllMemoryRegionsForSignature(processHandle, LoadMemoryRegions(process, module), signature);
}

static std::vector<UINT_PTR> ScanAllProcessModulesForBytes(Process* process, HANDLE processHandle, std::vector<BYTE> bytes)
{
    std::vector<UINT_PTR> results;
    for (ProcessModule pm : process->Modules)
        for (UINT_PTR address : ScanProcessModuleForBytes(process, processHandle, pm, bytes))
            results.push_back(address);

    return results;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static std::vector<UINT_PTR> ScanAllProcessModulesForGeneric(Process* process, HANDLE processHandle, T value)
{
    std::vector<UINT_PTR> results;
    for (ProcessModule pm : process->Modules)
        for (UINT_PTR address : ScanProcessModuleForGeneric(process, processHandle, pm, value))
            results.push_back(address);

    return results;
}

static std::vector<UINT_PTR> ScanAllProcessModulesForSignature(Process* process, HANDLE processHandle, Signature signature)
{
    std::vector<UINT_PTR> results;
    for (ProcessModule pm : process->Modules)
        for (UINT_PTR address : ScanProcessModuleForSignature(process, processHandle, pm, signature))
            results.push_back(address);

    return results;
}

static UINT_PTR ScanAddressForBytes(HANDLE processHandle, UINT_PTR address, std::vector<BYTE> bytes)
{
    if (ReadMemory(processHandle, address, bytes.size()) == bytes)
        return address;

    return 0;
}

template <class T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
static UINT_PTR ScanAddressGeneric(HANDLE processHandle, UINT_PTR address, T value)
{
    return RescanForBytes(processHandle, GenericToBytes(value), address);
}

static UINT_PTR ScanAddressForSignature(HANDLE processHandle, UINT_PTR address, Signature signature)
{
    if (!signature.Bytes.empty())
        return ScanAddressForBytes(processHandle, address, signature.Bytes);

    SIZE_T signatureStringSize = signature.StringSize;
    SIZE_T signatureBytesSize = signatureStringSize / 2;
    std::vector<BYTE> buffer = ReadMemory(processHandle, address, signatureBytesSize);
    std::string s = signature.String;
    SIZE_T i, j = signatureStringSize + 1;
    CHAR b[4];

    for (i = 0; j > signatureStringSize && i < signatureStringSize; i += 2)
    {
        if (strcmpA(s.substr(i, 2).c_str(), "??"))
            j = i / 2;
    }

    SIZE_T bytesSize = signatureBytesSize - j;

    for (i = j, j = i + 1; i < bytesSize; ++i, ++j)
    {
        _itoa_s(buffer[i], &b[0], 4, 16);

        if ((s[i] != '?' && s[i] != b[0]) || (s[j] != '?' && s[j] != b[1]))
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
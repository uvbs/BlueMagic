#pragma once

#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <string>
#include <vector>

#include "type_converter.h"
#include "string.h"

#ifdef UNICODE // r_winnt
typedef std::wstring TSTR;
#else
typedef std::string TSTR;
#endif /* !UNICODE */

namespace bluemagic
{
namespace winapi
{

#pragma region windows.h

static inline HANDLE OpenProcess(DWORD processId, DWORD accessRights = PROCESS_ALL_ACCESS)
{
    HANDLE processHandle = nullptr;
    processHandle = ::OpenProcess(accessRights, FALSE, processId);
    if (!processHandle)
        throw std::exception(tstrf("%s(%u, 0x%0x) failed Win32Error:%u",
            __func__, processId, accessRights, ::GetLastError()).c_str());

    return processHandle;
}

static inline DWORD GetProcessId(HANDLE processHandle)
{
    DWORD processId = 0;
    processId = ::GetProcessId(processHandle);
    if (!processId)
        throw std::exception(tstrf("%s(0x%0x) failed Win32Error:%u",
            __func__, PointerToGeneric<UINT_PTR>(processHandle), ::GetLastError()).c_str());

    return processId;
}

static inline BOOL IsWow64Process(HANDLE processHandle)
{
    BOOL is64 = FALSE;
    BOOL success = ::IsWow64Process(processHandle, &is64);
    if (!success)
        throw std::exception(tstrf("%s(0x%0x) failed Win32Error:%u",
            __func__, PointerToGeneric<UINT_PTR>(processHandle), ::GetLastError()).c_str());

    return is64 != FALSE;
}

static inline TSTR GetWindowClassName(HWND windowHandle)
{
    TCHAR className[MAX_CLASS_NAME];
    DWORD nameSize = ::GetClassName(windowHandle, &className[0], MAX_CLASS_NAME);
    if (!nameSize)
        throw std::exception(tstrf("%s(0x%0x) failed Win32Error:%u",
            __func__, PointerToGeneric<UINT_PTR>(windowHandle), ::GetLastError()).c_str());

    return TSTR(className);
}

static inline BOOL CloseHandle(HANDLE handle)
{
    BOOL success = FALSE;
    success = ::CloseHandle(handle);
    if (!success)
        throw std::exception(tstrf("%s(0x%0x) failed Win32Error:%u",
            __func__, PointerToGeneric<UINT_PTR>(handle), ::GetLastError()).c_str());

    return success;
}

static inline std::vector<BYTE> ReadProcessMemory(HANDLE processHandle, UINT_PTR address, SIZE_T size)
{
    std::vector<BYTE> buffer(size);
    SIZE_T bytesRead = 0;
    BOOL success = ::ReadProcessMemory(processHandle, GenericToPointer(address), BytesToPointer(buffer), size, &bytesRead);
    if (!success)
        throw std::exception(tstrf("%s(0x%0x, 0x%0x, %u) failed Win32Error:%u",
            __func__, PointerToGeneric<UINT_PTR>(processHandle), address, size, ::GetLastError()).c_str());

    return buffer;
}

static inline BOOL WriteProcessMemory(HANDLE processHandle, UINT_PTR address, std::vector<BYTE> bytes)
{
    SIZE_T bytesSize = bytes.size();
    SIZE_T bytesWritten = 0;
    BOOL success = ::WriteProcessMemory(processHandle, GenericToPointer(address), BytesToPointer(bytes), bytesSize, &bytesWritten);
    if (!success)
        throw std::exception(tstrf("%s(0x%0x, 0x%0x) failed Win32Error:%u",
            __func__, PointerToGeneric<UINT_PTR>(processHandle), address, ::GetLastError()).c_str());

    return bytesWritten == bytesSize;
}

static inline UINT_PTR VirtualAlloc(UINT_PTR address, SIZE_T size, DWORD protect = PAGE_EXECUTE_READWRITE, DWORD state = MEM_COMMIT)
{
    UINT_PTR baseAddress = 0;
    baseAddress = PointerToGeneric<UINT_PTR>(::VirtualAlloc(GenericToPointer(address), size, state, protect));
    if (!baseAddress)
        throw std::exception(tstrf("%s(0x%0x, %u, 0x%0x, 0x%0x) failed Win32Error:%u",
            __func__, address, size, protect, state, ::GetLastError()).c_str());

    return baseAddress;
}

static inline UINT_PTR VirtualAllocEx(HANDLE processHandle, UINT_PTR address, SIZE_T size, DWORD protect = PAGE_EXECUTE_READWRITE, DWORD state = MEM_COMMIT)
{
    UINT_PTR baseAddress = 0;
    baseAddress = PointerToGeneric<UINT_PTR>(::VirtualAllocEx(processHandle, GenericToPointer(address), size, state, protect));
    if (!baseAddress)
        throw std::exception(tstrf("%s(0x%0x, 0x%0x, %u, 0x%0x, 0x%0x) failed Win32Error:%u",
            __func__, PointerToGeneric<UINT_PTR>(processHandle), address, size, protect, state, ::GetLastError()).c_str());

    return baseAddress;
}

static inline BOOL VirtualFree(UINT_PTR address, SIZE_T size = 0, DWORD free = MEM_RELEASE)
{
    BOOL success = FALSE;
    success = ::VirtualFree(GenericToPointer(address), size, free);
    if (!success)
        throw std::exception(tstrf("%s(0x%0x, %u, 0x%0x) failed Win32Error:%u",
            __func__, address, size, free, ::GetLastError()).c_str());

    return success;
}

static inline BOOL VirtualFreeEx(HANDLE processHandle, UINT_PTR address, SIZE_T size = 0, DWORD free = MEM_RELEASE)
{
    BOOL success = FALSE;
    success = ::VirtualFreeEx(processHandle, GenericToPointer(address), size, free);
    if (!success)
        throw std::exception(tstrf("%s(0x%0x, 0x%0x, %u, 0x%0x) failed Win32Error:%u",
            __func__, PointerToGeneric<UINT_PTR>(processHandle), address, size, free, ::GetLastError()).c_str());

    return success;
}

static inline DWORD VirtualProtect(UINT_PTR address, SIZE_T size, DWORD newProtect = PAGE_EXECUTE_READWRITE)
{
    DWORD oldProtect = 0;
    BOOL success = ::VirtualProtect(GenericToPointer(address), size, newProtect, &oldProtect);
    if (!success)
        throw std::exception(tstrf("%s(0x%0x, %u, 0x%0x) failed Win32Error:%u",
            __func__, address, size, newProtect, ::GetLastError()).c_str());

    return oldProtect;
}

static inline DWORD VirtualProtectEx(HANDLE processHandle, UINT_PTR address, SIZE_T size, DWORD newProtect = PAGE_EXECUTE_READWRITE)
{
    DWORD oldProtect = 0;
    BOOL success = ::VirtualProtectEx(processHandle, GenericToPointer(address), size, newProtect, &oldProtect);
    if (!success)
        throw std::exception(tstrf("%s(0x%0x, 0x%0x, %u, 0x%0x) failed Win32Error:%u",
            __func__, PointerToGeneric<UINT_PTR>(processHandle), address, size, newProtect, ::GetLastError()).c_str());

    return oldProtect;
}

static inline MEMORY_BASIC_INFORMATION VirtualQuery(UINT_PTR address, SIZE_T size)
{
    MEMORY_BASIC_INFORMATION info;
    SIZE_T infoSize = ::VirtualQuery(GenericToPointer(address), &info, size);
    if (!infoSize)
        throw std::exception(tstrf("%s(0x%0x, %u) failed Win32Error:%u",
            __func__, address, size, ::GetLastError()).c_str());

    return info;
}

static inline MEMORY_BASIC_INFORMATION VirtualQueryEx(HANDLE processHandle, UINT_PTR address, SIZE_T size)
{
    MEMORY_BASIC_INFORMATION info;
    SIZE_T infoSize = ::VirtualQueryEx(processHandle, GenericToPointer(address), &info, size);
    if (!infoSize)
        throw std::exception(tstrf("%s(0x%0x, 0x%0x, %u) failed Win32Error:%u",
            __func__, PointerToGeneric<UINT_PTR>(processHandle), address, size, ::GetLastError()).c_str());

    return info;
}

#pragma endregion

#pragma region psapi.h

static inline TSTR GetProcessModuleBaseName(HANDLE processHandle, HMODULE moduleHandle)
{
    TCHAR baseName[MAX_MODULE_NAME32];
    DWORD baseNameSize = ::GetModuleBaseName(processHandle, moduleHandle, &baseName[0], MAX_MODULE_NAME32);
    if (!baseNameSize)
        throw std::exception(tstrf("%s(0x%0x, 0x%0x) failed Win32Error:%u",
            __func__, PointerToGeneric<UINT_PTR>(moduleHandle), PointerToGeneric<UINT_PTR>(processHandle), ::GetLastError()).c_str());

    return TSTR(baseName);
}

#pragma endregion

#pragma region tlhelp32.h

static inline HANDLE CreateToolhelp32Snapshot(DWORD processId, DWORD flags)
{
    HANDLE snapshot = INVALID_HANDLE_VALUE;
    snapshot = ::CreateToolhelp32Snapshot(flags, processId);
    if (snapshot == INVALID_HANDLE_VALUE)
        throw std::exception(tstrf("%s(%u, 0x%0x) failed Win32Error:%u",
            __func__, processId, flags, ::GetLastError()).c_str());

    return snapshot;
}

static inline PROCESSENTRY32 GetFirstProcess32(HANDLE snapshot)
{
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    if (!::Process32First(snapshot, &processEntry))
        throw std::exception(tstrf("%s(0x%0x) failed Win32Error:%u",
            __func__, PointerToGeneric<UINT_PTR>(snapshot), ::GetLastError()).c_str());

    return processEntry;
}

static inline std::vector<PROCESSENTRY32> GetProcesses()
{
    std::vector<PROCESSENTRY32> processes;
    HANDLE snapshot = CreateToolhelp32Snapshot(0, TH32CS_SNAPPROCESS);
    PROCESSENTRY32 processEntry = GetFirstProcess32(snapshot);

    do
    {
        processes.push_back(processEntry);
    } while (Process32Next(snapshot, &processEntry));

    CloseHandle(snapshot);
    return processes;
}

static inline MODULEENTRY32 GetFirstModule32(HANDLE snapshot)
{
    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);
    if (!::Module32First(snapshot, &moduleEntry))
        throw std::exception(tstrf("%s(0x%0x) failed Win32Error:%u",
            __func__, PointerToGeneric<UINT_PTR>(snapshot), ::GetLastError()).c_str());

    return moduleEntry;
}

static inline std::vector<MODULEENTRY32> GetModulesFromProcess(DWORD processId)
{
    std::vector<MODULEENTRY32> modules;
    HANDLE snapshot = CreateToolhelp32Snapshot(processId, TH32CS_SNAPMODULE);
    MODULEENTRY32 moduleEntry = GetFirstModule32(snapshot);

    do
    {
        modules.push_back(moduleEntry);
    }
    while (Module32Next(snapshot, &moduleEntry));

    CloseHandle(snapshot);
    return modules;
}

#pragma endregion

}
}
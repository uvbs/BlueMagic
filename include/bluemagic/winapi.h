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
        throw std::exception(tstrf("OpenProcess Error[%u] Unable to open process[0x%x] with access 0x%x",
            ::GetLastError(), processId, accessRights).c_str(), __LINE__);

    return processHandle;
}

static inline DWORD GetProcessId(HANDLE processHandle)
{
    DWORD processId = 0;
    processId = ::GetProcessId(processHandle);
    if (!processId)
        throw std::exception(tstrf("GetProcessId Error[%u] Unable to get Id from process handle[0x%x]",
            ::GetLastError(), PointerToGeneric<UINT_PTR>(processHandle)).c_str(), __LINE__);

    return processId;
}

static inline BOOL IsWow64Process(HANDLE processHandle)
{
    BOOL is64 = FALSE;
    BOOL success = ::IsWow64Process(processHandle, &is64);
    if (!success)
        throw std::exception(tstrf("IsWow64Process Error[%u] Unable to determine if process handle[0x%x] is 64bit",
            ::GetLastError(), PointerToGeneric<UINT_PTR>(processHandle)).c_str(), __LINE__);

    return is64 != FALSE;
}

static inline TSTR GetWindowClassName(HWND windowHandle)
{
    TCHAR className[MAX_CLASS_NAME];
    DWORD nameSize = ::GetClassName(windowHandle, &className[0], MAX_CLASS_NAME);
    if (!nameSize)
        throw std::exception(tstrf("GetClassName Error[%u] Unable to get class name from window handle[0x%x]",
            ::GetLastError(), PointerToGeneric<UINT_PTR>(windowHandle)).c_str(), __LINE__);

    return TSTR(className);
}

static inline BOOL CloseHandle(HANDLE handle)
{
    BOOL success = FALSE;
    success = ::CloseHandle(handle);
    if (!success)
        throw std::exception(tstrf("CloseHandle Error[%u] Unable to close handle handle[0x%x]",
            ::GetLastError(), PointerToGeneric<UINT_PTR>(handle)).c_str(), __LINE__);

    return success;
}

static inline std::vector<BYTE> ReadProcessMemory(HANDLE processHandle, UINT_PTR address, SIZE_T size)
{
    std::vector<BYTE> buffer(size);
    SIZE_T bytesRead = 0;
    BOOL success = ::ReadProcessMemory(processHandle, GenericToPointer(address), BytesToPointer(buffer), size, &bytesRead);
    if (!success)
        throw std::exception(tstrf("ReadProcessMemory Error[%u] Unable to read memory from 0x%x,%u from handle[0x%x]",
            ::GetLastError(), address, size, PointerToGeneric<UINT_PTR>(processHandle)).c_str(), __LINE__);

    return buffer;
}

static inline BOOL WriteProcessMemory(HANDLE processHandle, UINT_PTR address, std::vector<BYTE> bytes)
{
    SIZE_T bytesSize = bytes.size();
    SIZE_T bytesWritten = 0;
    BOOL success = ::WriteProcessMemory(processHandle, GenericToPointer(address), BytesToPointer(bytes), bytesSize, &bytesWritten);
    if (!success)
        throw std::exception(tstrf("WriteProcessMemory Error[%u] Unable to write memory to 0x%x,%u to handle[0x%x]",
            ::GetLastError(), address, bytesSize, PointerToGeneric<UINT_PTR>(processHandle)).c_str(), __LINE__);

    return bytesWritten == bytesSize;
}

static inline UINT_PTR VirtualAlloc(UINT_PTR address, SIZE_T size, DWORD protect = PAGE_EXECUTE_READWRITE, DWORD state = MEM_COMMIT)
{
    UINT_PTR baseAddress = 0;
    baseAddress = PointerToGeneric<UINT_PTR>(::VirtualAlloc(GenericToPointer(address), size, state, protect));
    if (!baseAddress)
        throw std::exception(tstrf("VirtualAlloc Error[%u] Unable to allocate memory to 0x%x,%u with 0x%x,0x%x",
            ::GetLastError(), address, size, protect, state).c_str(), __LINE__);

    return baseAddress;
}

static inline UINT_PTR VirtualAllocEx(HANDLE processHandle, UINT_PTR address, SIZE_T size, DWORD protect = PAGE_EXECUTE_READWRITE, DWORD state = MEM_COMMIT)
{
    UINT_PTR baseAddress = 0;
    baseAddress = PointerToGeneric<UINT_PTR>(::VirtualAllocEx(processHandle, GenericToPointer(address), size, state, protect));
    if (!baseAddress)
        throw std::exception(tstrf("VirtualAllocEx Error[%u] Unable to allocate memory to 0x%x,%u with 0x%x,0x%x to handle[0x%x]",
            ::GetLastError(), address, size, protect, state, PointerToGeneric<UINT_PTR>(processHandle)).c_str(), __LINE__);

    return baseAddress;
}

static inline BOOL VirtualFree(UINT_PTR address, SIZE_T size = 0, DWORD free = MEM_RELEASE)
{
    BOOL success = FALSE;
    success = ::VirtualFree(GenericToPointer(address), size, free);
    if (!success)
        throw std::exception(tstrf("VirtualFree Error[%u] Unable to free memory from 0x%x,%u with 0x%x",
            ::GetLastError(), address, size, free).c_str(), __LINE__);

    return success;
}

static inline BOOL VirtualFreeEx(HANDLE processHandle, UINT_PTR address, SIZE_T size = 0, DWORD free = MEM_RELEASE)
{
    BOOL success = FALSE;
    success = ::VirtualFreeEx(processHandle, GenericToPointer(address), size, free);
    if (!success)
        throw std::exception(tstrf("VirtualFreeEx Error[%u] Unable to free memory from 0x%x,%u with 0x%x from handle[0x%x]",
            ::GetLastError(), address, size, free, PointerToGeneric<UINT_PTR>(processHandle)).c_str(), __LINE__);

    return success;
}

static inline DWORD VirtualProtect(UINT_PTR address, SIZE_T size, DWORD newProtect = PAGE_EXECUTE_READWRITE)
{
    DWORD oldProtect = 0;
    BOOL success = ::VirtualProtect(GenericToPointer(address), size, newProtect, &oldProtect);
    if (!success)
        throw std::exception(tstrf("VirtualProtect Error[%u] Unable to protect memory at 0x%x,%u with 0x%x",
            ::GetLastError(), address, size, newProtect).c_str(), __LINE__);

    return oldProtect;
}

static inline DWORD VirtualProtectEx(HANDLE processHandle, UINT_PTR address, SIZE_T size, DWORD newProtect = PAGE_EXECUTE_READWRITE)
{
    DWORD oldProtect = 0;
    BOOL success = ::VirtualProtectEx(processHandle, GenericToPointer(address), size, newProtect, &oldProtect);
    if (!success)
        throw std::exception(tstrf("VirtualProtectEx Error[%u] Unable to protect memory at 0x%x,%u with 0x%x from handle[0x%x]",
            ::GetLastError(), address, size, newProtect, PointerToGeneric<UINT_PTR>(processHandle)).c_str(), __LINE__);

    return oldProtect;
}

static inline MEMORY_BASIC_INFORMATION VirtualQuery(UINT_PTR address, SIZE_T size)
{
    MEMORY_BASIC_INFORMATION info;
    SIZE_T infoSize = ::VirtualQuery(GenericToPointer(address), &info, size);
    if (!infoSize)
        throw std::exception(tstrf("VirtualQuery Error[%u] Unable to query memory at 0x%x,%u",
            ::GetLastError(), address, size).c_str(), __LINE__);

    return info;
}

static inline MEMORY_BASIC_INFORMATION VirtualQueryEx(HANDLE processHandle, UINT_PTR address, SIZE_T size)
{
    MEMORY_BASIC_INFORMATION info;
    SIZE_T infoSize = ::VirtualQueryEx(processHandle, GenericToPointer(address), &info, size);
    if (!infoSize)
        throw std::exception(tstrf("VirtualQueryEx Error[%u] Unable to query memory at 0x%x,%u from handle[0x%x]",
            ::GetLastError(), address, size, PointerToGeneric<UINT_PTR>(processHandle)).c_str(), __LINE__);

    return info;
}

#pragma endregion

#pragma region psapi.h

static inline TSTR GetProcessModuleBaseName(HANDLE processHandle, HMODULE moduleHandle)
{
    TCHAR baseName[MAX_MODULE_NAME32];
    DWORD baseNameSize = ::GetModuleBaseName(processHandle, moduleHandle, &baseName[0], MAX_MODULE_NAME32);
    if (!baseNameSize)
        throw std::exception(tstrf("GetModuleBaseName Error[%u] Unable to get base name from module handle[0x%x] and process handle[0x%x]",
            ::GetLastError(), PointerToGeneric<UINT_PTR>(moduleHandle), PointerToGeneric<UINT_PTR>(processHandle)).c_str(), __LINE__);

    return TSTR(baseName);
}

#pragma endregion

#pragma region tlhelp32.h

static inline HANDLE CreateToolhelp32Snapshot(DWORD processId, DWORD flags)
{
    HANDLE snapshot = INVALID_HANDLE_VALUE;
    snapshot = ::CreateToolhelp32Snapshot(flags, processId);
    if (snapshot == INVALID_HANDLE_VALUE)
        throw std::exception(tstrf("CreateToolhelp32Snapshot Error[%u] Unable to create snapshot with 0x%x from process[0x%x]",
            ::GetLastError(), flags, processId).c_str(), __LINE__);

    return snapshot;
}

static inline PROCESSENTRY32 GetFirstProcess32(HANDLE snapshot)
{
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    if (!::Process32First(snapshot, &processEntry))
        throw std::exception(tstrf("Module32First Error[%u] Unable to get processes from snapshot[0x%x]",
            ::GetLastError(), PointerToGeneric<UINT_PTR>(snapshot)).c_str(), __LINE__);

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
        throw std::exception(tstrf("Module32First Error[%u] Unable to get modules from snapshot[0x%x]",
            ::GetLastError(), PointerToGeneric<UINT_PTR>(snapshot)).c_str(), __LINE__);

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
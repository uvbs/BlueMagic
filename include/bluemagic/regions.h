#pragma once

#include <windows.h>
#include <vector>

#include "process.h"
#include "process_module.h"
#include "winapi_impl.h"

namespace bluemagic
{

static std::vector<MEMORY_BASIC_INFORMATION> ScanMemoryRegions(HANDLE processHandle, std::vector<ProcessModule> processModules, SIZE_T moduleIndex, SIZE_T endModule)
{
    std::vector<MEMORY_BASIC_INFORMATION> regions;

    for (; moduleIndex < endModule; ++moduleIndex)
    {
        UINT_PTR start = processModules[moduleIndex].BaseAddress;
        UINT_PTR end = moduleIndex + 1 > processModules.size() - 1 ? processModules[moduleIndex].MemorySize + 1 : processModules[moduleIndex + 1].BaseAddress;
        UINT_PTR seek = start;

        do
        {
            MEMORY_BASIC_INFORMATION region = VirtualQueryExImpl(processHandle, seek, sizeof(MEMORY_BASIC_INFORMATION));
            if ((region.State & MEM_COMMIT) && !(region.Protect & 0x701))
                regions.push_back(region);

            seek = PointerToGeneric<UINT_PTR>(region.BaseAddress) + region.RegionSize;
        } while (seek < end);
    }

    return regions;
}

static std::vector<MEMORY_BASIC_INFORMATION> ScanAllMemoryRegions(HANDLE processHandle, std::vector<ProcessModule> processModules)
{
    return ScanMemoryRegions(processHandle, processModules, 0, processModules.size());
}

static std::vector<MEMORY_BASIC_INFORMATION> LoadMemoryRegions(Process* process, ProcessModule processModule)
{
    std::vector<ProcessModule> processModules = process->Modules;
    std::vector<ProcessModule>::iterator itr = std::find(processModules.begin(), processModules.end(), processModule);
    if (itr == processModules.end())
        return std::vector<MEMORY_BASIC_INFORMATION>();

    SIZE_T index = std::distance(processModules.begin(), itr);
    return ScanMemoryRegions(process->Handle, processModules, index, index + 1);
}

static std::vector<MEMORY_BASIC_INFORMATION> LoadAllMemoryRegions(Process* process)
{
    return ScanAllMemoryRegions(process->Handle, process->Modules);
}

}

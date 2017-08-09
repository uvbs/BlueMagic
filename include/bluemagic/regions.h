#pragma once

#include <windows.h>
#include <vector>

#include "process.h"
#include "module.h"
#include "winapi_impl.h"

namespace bluemagic
{

static std::vector<MEMORY_BASIC_INFORMATION> LoadModuleRegions(HANDLE processHandle, Module* module)
{
    std::vector<MEMORY_BASIC_INFORMATION> regions;

    UINT_PTR start = module->GetBaseAddress();
    UINT_PTR end = start + module->GetMemorySize();
    UINT_PTR seek = start;

    do
    {
        MEMORY_BASIC_INFORMATION region = VirtualQueryExImpl(processHandle, seek, sizeof(MEMORY_BASIC_INFORMATION));
        if ((region.State & MEM_COMMIT) &&
            !(region.Protect & (PAGE_NOACCESS | PAGE_GUARD | PAGE_NOCACHE | PAGE_WRITECOMBINE | PAGE_TARGETS_INVALID)))
            regions.push_back(region);

        seek = PointerToGeneric<UINT_PTR>(region.BaseAddress) + region.RegionSize;
    }
    while (seek <= end);

    return regions;
}

static std::vector<MEMORY_BASIC_INFORMATION> LoadProcessRegions(Process* process)
{
    std::vector<MEMORY_BASIC_INFORMATION> regions;
    for (Module* module : process->GetModules())
        for (MEMORY_BASIC_INFORMATION region : LoadModuleRegions(process->GetHandle(), module))
            regions.push_back(region);

    return regions;
}

}

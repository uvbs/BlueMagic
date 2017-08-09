#include <Windows.h>
#include <conio.h>

#include <bluemagic\pointer.h>
#include <bluemagic\module.h>
#include <bluemagic\process.h>
#include <bluemagic\protect_guard.h>
#include <bluemagic\readwrite.h>
#include <bluemagic\regions.h>
#include <bluemagic\scanner.h>
#include <bluemagic\signature.h>
#include <bluemagic\string.h>
#include <bluemagic\type_converter.h>
#include <bluemagic\winapi_impl.h>

using namespace bluemagic;

int main()
{
    auto GetProcessesByName = [](TSTR name)->std::vector<Process*>
    {
        std::vector<Process*> processes;
        std::vector<PROCESSENTRY32> ps32 = GetProcessesImpl();
        for (PROCESSENTRY32 p32 : ps32)
            if (strcmp(p32.szExeFile, &name[0], true))
                processes.push_back(new Process(p32));
        return processes;
    };

    try
    {
        Process* p = GetProcessesByName(L"")[0];
    }
    catch (std::exception ex)
    {
        printf("%s\n", ex.what());
    }

    _getch();
    return 0;
}

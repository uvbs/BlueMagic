#include <Windows.h>
#include <conio.h>

#include <bluemagic\pointer.h>
#include <bluemagic\process_module.h>
#include <bluemagic\process.h>
#include <bluemagic\protect_operation.h>
#include <bluemagic\readwrite.h>
#include <bluemagic\regions.h>
#include <bluemagic\scanner.h>
#include <bluemagic\signature.h>
#include <bluemagic\string.h>
#include <bluemagic\type_converter.h>
#include <bluemagic\winapi.h>

using namespace bluemagic;

int main()
{
    Process* p = nullptr;
    std::vector<PROCESSENTRY32> ps32 = winapi::GetProcesses();
    for (PROCESSENTRY32 p32 : ps32)
        if (strcmp(p32.szExeFile, L"", true))
        {
            p = new Process(p32);
            break;
        }

    _getch();
    return 0;
}

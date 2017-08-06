#include <Windows.h>
#include <conio.h>

#include <bluemagic\pointer.h>
#include <bluemagic\module.h>
#include <bluemagic\process.h>
#include <bluemagic\protect_operation.h>
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
    try
    {
        Process* p = nullptr;
        HANDLE ph = nullptr;
        std::vector<PROCESSENTRY32> ps32 = GetProcessesImpl();
        for (PROCESSENTRY32 p32 : ps32)
        {
            if (strcmp(p32.szExeFile, L"", true))
            {
                p = new Process(p32);
                ph = p->Handle;
                break;
            }
        }
    }
    catch (std::exception ex)
    {
        printf("%s\n", ex.what());
    }

    _getch();
    return 0;
}

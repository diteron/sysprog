#include "file_call_interceptor.h"

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    FileCallInterceptor fileCallInterceptor;
    
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            fileCallInterceptor.installHook();
            break;
        case DLL_PROCESS_DETACH:
            fileCallInterceptor.removeHook();
            break;
    }

    return TRUE;
}

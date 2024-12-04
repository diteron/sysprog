#define UNICODE
#define _UNICODE

#include "file_call_interceptor.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    std::locale::global(std::locale(""));

    FileCallInterceptor fileCallInterceptor;
    
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            fileCallInterceptor.installHooks();
            break;
        case DLL_PROCESS_DETACH:
            fileCallInterceptor.removeHooks();
            break;
    }

    return TRUE;
}

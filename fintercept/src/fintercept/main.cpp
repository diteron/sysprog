#include "dll_injector.h"

#include <iostream>

bool startApplication(const char* appPath, STARTUPINFO& si, PROCESS_INFORMATION& pi);

int main(int argc, const char* argv[])
{
    const char* dllPath = "logger.dll";

    #ifdef NDEBUG
        if (argc < 2) {
            std::cerr << "Usage: fintercept <path_to_program>\n";
            return 1;
        }
    #endif
    
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    const char* appPath = argv[1];
    if (!startApplication(appPath, si, pi)) {
        std::cerr << "Failed to launch application '" << appPath << "'. Error code '" << GetLastError() << "'\n";
        return 1;
    }

    DWORD processId = pi.dwProcessId;
    std::cout << "Application '" << appPath << "' started. Process id " << processId << "\n"
            << "Trying to inject dll in process...\n";

    DllInjector dllInjector;
    if (!dllInjector.injectDll(processId, dllPath)) {
        std::cerr << "Failed to inject dll in process with id '" << processId << "'\n"
                << "Logging access to files will not work\n"; 
    }
    else {
        std::cout << "Dll injected successfully\n"
                << "Logging access to files...\n";
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    std::cout << "Application '" << appPath << "' terminated\n";
    
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return 0;
}

bool startApplication(const char* appPath, STARTUPINFO& si, PROCESS_INFORMATION& pi)
{
    if (!CreateProcessA(appPath, nullptr, nullptr,
            nullptr, FALSE, 0, nullptr, nullptr,
            &si, &pi)) {
        return false;
    }

    return true;
}

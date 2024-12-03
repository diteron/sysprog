#pragma once

#include <windows.h>

class DllInjector
{
public:
    DllInjector();

    bool injectDll(DWORD processId, const char* dllPath) const;

private:
    void* copyDllPathToProcessMemory(HANDLE processHandle, const char* dllPath) const;
    bool loadDll(HANDLE processHandle, void* remoteMemory) const;
};

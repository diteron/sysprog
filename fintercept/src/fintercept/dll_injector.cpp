#include "dll_injector.h"
#include <iostream>

DllInjector::DllInjector()
{}

bool DllInjector::injectDll(DWORD processId, const char* dllPath) const
{
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!processHandle) {
        std::cerr << "Failed to open target process. Error code '" << GetLastError() << "'\n";
        return false;
    }

    void* dllPathInRemoteMemory = copyDllPathToProcessMemory(processHandle, dllPath);
    if (!dllPathInRemoteMemory) {
        CloseHandle(processHandle);
        return false;
    }
    
    return loadDll(processHandle, dllPathInRemoteMemory);
}

void* DllInjector::copyDllPathToProcessMemory(HANDLE processHandle, const char* dllPath) const
{
    void* dllPathInRemoteMemory = VirtualAllocEx(processHandle, nullptr, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!dllPathInRemoteMemory) {
        std::cerr << "Failed to allocate memory in target process. Error code '" << GetLastError() << "'\n";
        return nullptr;
    }

    if (!WriteProcessMemory(processHandle, dllPathInRemoteMemory, dllPath, strlen(dllPath) + 1, nullptr)) {
        std::cerr << "Failed to write memory in targer process. Error code '" << GetLastError() << "'\n";
        VirtualFreeEx(processHandle, dllPathInRemoteMemory, 0, MEM_RELEASE);
        return nullptr;
    }

    return dllPathInRemoteMemory;
}

bool DllInjector::loadDll(HANDLE processHandle, void* dllPathInRemoteMemory) const
{
    HANDLE remoteThread = CreateRemoteThread(processHandle, nullptr, 0,
            (LPTHREAD_START_ROUTINE) LoadLibraryA,
            dllPathInRemoteMemory, 0, nullptr);

    if (!remoteThread) {
        std::cerr << "Failed to create remote thread in target process. Error code '" << GetLastError() << "'\n";
        VirtualFreeEx(processHandle, dllPathInRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return false;
    }

    WaitForSingleObject(remoteThread, INFINITE);

    VirtualFreeEx(processHandle, dllPathInRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(remoteThread);
    CloseHandle(processHandle);

    return true;
}

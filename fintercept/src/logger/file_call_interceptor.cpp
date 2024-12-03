#include "file_call_interceptor.h"
#include <iostream>

__declspec(thread) bool inHook = false;

FileCallInterceptor::CreateFileOrig FileCallInterceptor::createFileOrig = nullptr;
const char* FileCallInterceptor::logFileName = "file_calls.log";
BYTE FileCallInterceptor::originalBytes[5] = {};
BYTE* FileCallInterceptor::trampoline = (BYTE*) VirtualAlloc(
        NULL,
        5 + 5,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE);

FileCallInterceptor::FileCallInterceptor()
{}

void FileCallInterceptor::installHook()
{
    HMODULE kernel32Module = GetModuleHandleA("kernel32.dll");
    if (!kernel32Module) {
        return;
    }

    createFileOrig = (CreateFileOrig) GetProcAddress(kernel32Module, "CreateFileW");
    if (!createFileOrig) {
        return;
    }

    BYTE* function = (BYTE*) createFileOrig;
    DWORD oldProtect;

    memcpy(originalBytes, function, sizeof(originalBytes));
    VirtualProtect(trampoline, 10, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(trampoline, function, sizeof(originalBytes));
    trampoline[5] = 0xE9; // JMP instruction
    *(DWORD*) (trampoline + 6) = (DWORD) (function + sizeof(originalBytes) - trampoline - 10);
    VirtualProtect(trampoline, 10, oldProtect, &oldProtect);
    
    VirtualProtect(function, sizeof(originalBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
    function[0] = 0xE9; // JMP instruction
    *(DWORD*) (function + 1) = (DWORD) ((BYTE*) HookedCreateFile - function - 5);
    VirtualProtect(function, sizeof(originalBytes), oldProtect, &oldProtect);
}

void FileCallInterceptor::removeHook()
{
    if (!createFileOrig) {
        return;
    }

    BYTE* function = (BYTE*) createFileOrig;
    DWORD oldProtect;

    VirtualProtect(function, sizeof(originalBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(function, originalBytes, sizeof(originalBytes));
    VirtualProtect(function, sizeof(originalBytes), oldProtect, &oldProtect);
}

HANDLE __stdcall FileCallInterceptor::HookedCreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess,
        DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    std::string operation;

    if (dwDesiredAccess & GENERIC_READ) {
        operation = "Read";
    }
    if (dwDesiredAccess & GENERIC_WRITE) {
        if (!operation.empty()) operation += " & ";
        operation += "Write";
    }
    if (dwCreationDisposition == CREATE_NEW) {
        operation = "Create New File";
    } else if (dwCreationDisposition == OPEN_EXISTING) {
        operation = "Open Existing File";
    }

    logFileCall(operation.c_str(), lpFileName);
    

    return ((CreateFileOrig)trampoline)(lpFileName, dwDesiredAccess, dwShareMode, 
            lpSecurityAttributes, dwCreationDisposition, 
            dwFlagsAndAttributes, hTemplateFile);
}

void FileCallInterceptor::logFileCall(const char* operation, LPCWSTR fileName)
{
    SYSTEMTIME time;
    GetLocalTime(&time);

    std::wofstream logFile{logFileName, std::wios::app};
    if (logFile.is_open()) {
        logFile << "[" << time.wYear << "-" << time.wMonth << "-" << time.wDay << " "
                << time.wHour << ":" << time.wMinute << ":" << time.wSecond << "] "
                << operation << ": " << fileName << std::endl;
        logFile.close();
    }
}

void FileCallInterceptor::changeFunctionToOrig()
{
    BYTE* function = (BYTE*) createFileOrig;
    DWORD oldProtect;

    VirtualProtect(function, sizeof(originalBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(function, originalBytes, sizeof(originalBytes));
    VirtualProtect(function, sizeof(originalBytes), oldProtect, &oldProtect);
}

void FileCallInterceptor::changeFunctionToHook()
{
    BYTE* function = (BYTE*) createFileOrig;
    DWORD oldProtect;

    VirtualProtect(function, sizeof(originalBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
    function[0] = 0xE9; // JMP instruction
    *(DWORD*) (function + 1) = (DWORD) ((BYTE*) HookedCreateFile - function - 5);
    VirtualProtect(function, sizeof(originalBytes), oldProtect, &oldProtect);
}

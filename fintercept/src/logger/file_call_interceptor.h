#pragma once

#include <windows.h>
#include <fstream>

class FileCallInterceptor
{
public:
    using CreateFileOrig = HANDLE (WINAPI*)(LPCWSTR lpFileName, DWORD dwDesiredAccess,
            DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

    // typedef HANDLE(WINAPI* CreateFileOrig)(
    //     LPCWSTR lpFileName,
    //     DWORD dwDesiredAccess,
    //     DWORD dwShareMode,
    //     LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    //     DWORD dwCreationDisposition,
    //     DWORD dwFlagsAndAttributes,
    //     HANDLE hTemplateFile
    // );

    FileCallInterceptor();

    void installHook();
    void removeHook();

private:
    static HANDLE WINAPI HookedCreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess,
            DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

    static void logFileCall(const char* operation, LPCWSTR fileName);
    static void changeFunctionToOrig();
    static void changeFunctionToHook();

    static CreateFileOrig createFileOrig;
    static BYTE originalBytes[5];
    static BYTE* trampoline;

    static const char* logFileName;
};

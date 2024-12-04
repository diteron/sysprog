#pragma once

#include <windows.h>
#include <fstream>

class FileCallInterceptor
{
public:
    using CreateFileOrig = HANDLE (WINAPI*)(LPCWSTR lpFileName, DWORD dwDesiredAccess,
            DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
    using DeleteFileOrig = BOOL (WINAPI*)(LPCWSTR lpFileName);            

    FileCallInterceptor();

    void installHooks();
    void removeHooks();

private:
    bool saveProcessName();

    bool hookCreateFile(HMODULE kernel32Module);
    void unhookCreateFile();
    static HANDLE WINAPI hookedCreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess,
            DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
    static std::wstring getFileOperation(DWORD dwDesiredAccess, DWORD dwCreationDisposition);
    static void logCreateFileCall(const std::wstring&, LPCWSTR fileName);
    static void changeCreateFileToOriginal();
    static void changeCreateFileToHooked();

    bool hookDeleteFile(HMODULE kernel32Module);
    void unhookDeleteFile();
    static BOOL WINAPI hookedDeleteFile(LPCWSTR lpFileName);    
    static void logDeleteFileCall(LPCWSTR fileName);     
    static void changeDeleteFileToOriginal();
    static void changeDeleteFileToHooked();

    static WCHAR processName[];

    static const int JmpOpcodeSize = 14;
    
    static CreateFileOrig createFileOriginal;
    static BYTE createFileOriginalBytes[];
    static BYTE createFileAbsoluteJmpCode[];

    static DeleteFileOrig deleteFileOriginal;
    static BYTE deleteFileOriginalBytes[];
    static BYTE deleteFileAbsoluteJmpCode[];

    static constexpr wchar_t* logFileName = L"file_access.log";
};

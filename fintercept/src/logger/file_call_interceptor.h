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

    inline static WCHAR processName[MAX_PATH] = {};

    static constexpr int JmpOpcodeSize = 14;
    
    inline static CreateFileOrig createFileOriginal = nullptr; 
    inline static BYTE createFileOriginalBytes[JmpOpcodeSize] = {};
    inline static BYTE createFileAbsoluteJmpCode[JmpOpcodeSize] = {
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // JMP [rip + 0x0]
        0, 0, 0, 0, 0, 0, 0, 0              // Placeholder for the address
    };

    inline static DeleteFileOrig deleteFileOriginal = nullptr;
    inline static BYTE deleteFileOriginalBytes[JmpOpcodeSize] = {};
    inline static BYTE deleteFileAbsoluteJmpCode[JmpOpcodeSize] = {
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // JMP [rip + 0x0]
        0, 0, 0, 0, 0, 0, 0, 0              // Placeholder for the address
    };

    static constexpr wchar_t* logFileName = L"file_access.log";
    inline static std::wofstream logFile;
};

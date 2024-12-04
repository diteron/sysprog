#include "file_call_interceptor.h"
#include <iostream>
#include <iomanip>
#include <psapi.h>

WCHAR FileCallInterceptor::processName[MAX_PATH] = {};

FileCallInterceptor::CreateFileOrig FileCallInterceptor::createFileOriginal = nullptr;
BYTE FileCallInterceptor::createFileOriginalBytes[JmpOpcodeSize] = {};
BYTE FileCallInterceptor::createFileAbsoluteJmpCode[JmpOpcodeSize] = {
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // JMP [rip + 0x0]
        0, 0, 0, 0, 0, 0, 0, 0              // Placeholder for the address
};

FileCallInterceptor::DeleteFileOrig FileCallInterceptor::deleteFileOriginal = nullptr;
BYTE FileCallInterceptor::deleteFileOriginalBytes[JmpOpcodeSize] = {};
BYTE FileCallInterceptor::deleteFileAbsoluteJmpCode[JmpOpcodeSize] = {
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // JMP [rip + 0x0]
        0, 0, 0, 0, 0, 0, 0, 0              // Placeholder for the address
};

FileCallInterceptor::FileCallInterceptor()
{}

void FileCallInterceptor::installHooks()
{   
    if (!saveProcessName()) {
        return;
    }

    HMODULE kernel32Module = GetModuleHandleW(L"kernel32.dll");
    if (!kernel32Module) {
        return;
    }

    if (!hookCreateFile(kernel32Module) || !hookDeleteFile(kernel32Module)) {
        return;
    }
}

void FileCallInterceptor::removeHooks()
{
    unhookCreateFile();
    unhookDeleteFile();
}

bool FileCallInterceptor::saveProcessName()
{
    DWORD processId = GetCurrentProcessId();
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!processHandle) {
        return false;
    }

    if (!GetModuleBaseNameW(processHandle, nullptr, processName, MAX_PATH)) {
        CloseHandle(processHandle);
        return false;
    }

    CloseHandle(processHandle);
    return true;
}


bool FileCallInterceptor::hookCreateFile(HMODULE kernel32Module)
{
    createFileOriginal = (CreateFileOrig) GetProcAddress(kernel32Module, "CreateFileW");
    if (!createFileOriginal) {
        return false;
    }

    BYTE* function = (BYTE*) createFileOriginal;
    memcpy(createFileOriginalBytes, function, sizeof(createFileOriginalBytes));
    DWORD oldProtect;

    VirtualProtect(function, sizeof(createFileAbsoluteJmpCode), PAGE_EXECUTE_READWRITE, &oldProtect);
    *(DWORDLONG*) &createFileAbsoluteJmpCode[6] = (DWORDLONG) hookedCreateFile;
    memcpy(function, createFileAbsoluteJmpCode, sizeof(createFileAbsoluteJmpCode));
    VirtualProtect(function, sizeof(createFileAbsoluteJmpCode), oldProtect, &oldProtect);

    return true;
}

void FileCallInterceptor::unhookCreateFile()
{
    if (!createFileOriginal) {
        return;
    }

    BYTE* function = (BYTE*) createFileOriginal;
    DWORD oldProtect;

    VirtualProtect(function, sizeof(createFileOriginalBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(function, createFileOriginalBytes, sizeof(createFileOriginalBytes));
    VirtualProtect(function, sizeof(createFileOriginalBytes), oldProtect, &oldProtect);
}

HANDLE __stdcall FileCallInterceptor::hookedCreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess,
        DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    std::wstring operation = getFileOperation(dwDesiredAccess, dwCreationDisposition);
    logCreateFileCall(operation.c_str(), lpFileName);
    
    changeCreateFileToOriginal();
    HANDLE result = (createFileOriginal)(lpFileName, dwDesiredAccess, dwShareMode, 
            lpSecurityAttributes, dwCreationDisposition, 
            dwFlagsAndAttributes, hTemplateFile);
    changeCreateFileToHooked();

    return result;
}

std::wstring FileCallInterceptor::getFileOperation(DWORD dwDesiredAccess, DWORD dwCreationDisposition)
{
    std::wstring operation;
    if (dwDesiredAccess & GENERIC_READ) {
        operation = L"Read";
    }
    if (dwDesiredAccess & GENERIC_WRITE) {
        if (!operation.empty()) { 
            operation += L" & ";
        }
        operation += L"Write";
    }
    if (dwDesiredAccess & GENERIC_EXECUTE) {
        if (!operation.empty()) { 
            operation += L" & ";
        }
        operation += L"Execute";
    }
    if (dwDesiredAccess & GENERIC_ALL) {
        operation = L"All access";
    }

    if (dwCreationDisposition == CREATE_NEW || dwCreationDisposition == CREATE_ALWAYS) {
        operation = L"Create New File";
    }
    else if (dwCreationDisposition == OPEN_EXISTING) {
        operation = L"Open Existing File";
    }

    return operation;
}

void FileCallInterceptor::logCreateFileCall(const std::wstring& operation, LPCWSTR fileName)
{
    std::time_t now = std::time(nullptr);
    std::tm local_time = *std::localtime(&now);

    std::wofstream logFile{logFileName, std::wios::app};
    if (logFile.is_open()) {
        logFile << L"[" << processName << L" -- "
                << std::put_time(&local_time, L"%Y-%m-%d %H:%M:%S") << L"]  "
                << operation << L": " << fileName << std::endl;
        logFile.close();
    }
}

void FileCallInterceptor::changeCreateFileToOriginal()
{
    BYTE* function = (BYTE*) createFileOriginal;
    DWORD oldProtect;

    VirtualProtect(function, sizeof(createFileOriginalBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(function, createFileOriginalBytes, sizeof(createFileOriginalBytes));
    VirtualProtect(function, sizeof(createFileOriginalBytes), oldProtect, &oldProtect);
}

void FileCallInterceptor::changeCreateFileToHooked()
{
    BYTE* function = (BYTE*) createFileOriginal;
    DWORD oldProtect;

    VirtualProtect(function, sizeof(createFileAbsoluteJmpCode), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(function, createFileAbsoluteJmpCode, sizeof(createFileAbsoluteJmpCode));
    VirtualProtect(function, sizeof(createFileAbsoluteJmpCode), oldProtect, &oldProtect);
}

bool FileCallInterceptor::hookDeleteFile(HMODULE kernel32Module)
{
    deleteFileOriginal = (DeleteFileOrig) GetProcAddress(kernel32Module, "DeleteFileW");
    if (!deleteFileOriginal) {
        return false;
    }

    BYTE* function = (BYTE*) deleteFileOriginal;
    memcpy(deleteFileOriginalBytes, function, sizeof(deleteFileOriginalBytes));
    DWORD oldProtect;

    VirtualProtect(function, sizeof(deleteFileAbsoluteJmpCode), PAGE_EXECUTE_READWRITE, &oldProtect);
    *(DWORDLONG*) &deleteFileAbsoluteJmpCode[6] = (DWORDLONG) hookedDeleteFile;
    memcpy(function, deleteFileAbsoluteJmpCode, sizeof(deleteFileAbsoluteJmpCode));
    VirtualProtect(function, sizeof(deleteFileAbsoluteJmpCode), oldProtect, &oldProtect);

    return true;
}

void FileCallInterceptor::unhookDeleteFile()
{
    if (!deleteFileOriginal) {
        return;
    }

    BYTE* function = (BYTE*) deleteFileOriginal;
    DWORD oldProtect;

    VirtualProtect(function, sizeof(deleteFileOriginalBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(function, deleteFileOriginalBytes, sizeof(deleteFileOriginalBytes));
    VirtualProtect(function, sizeof(deleteFileOriginalBytes), oldProtect, &oldProtect);
}

BOOL __stdcall FileCallInterceptor::hookedDeleteFile(LPCWSTR lpFileName)
{
    logDeleteFileCall(lpFileName);
    
    changeDeleteFileToOriginal();
    BOOL result = (deleteFileOriginal)(lpFileName);
    changeDeleteFileToHooked();

    return result;
}

void FileCallInterceptor::logDeleteFileCall(LPCWSTR fileName)
{
    std::time_t now = std::time(nullptr);
    std::tm local_time = *std::localtime(&now);

    std::wofstream logFile{logFileName, std::wios::app};
    if (logFile.is_open()) {
        logFile << L"[" << processName << L" -- "
                << std::put_time(&local_time, L"%Y-%m-%d %H:%M:%S") << L"]  "
                << L"Delete File: " << fileName << std::endl;
        logFile.close();
    }
}

void FileCallInterceptor::changeDeleteFileToOriginal()
{
    BYTE* function = (BYTE*) deleteFileOriginal;
    DWORD oldProtect;

    VirtualProtect(function, sizeof(deleteFileOriginalBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(function, deleteFileOriginalBytes, sizeof(deleteFileOriginalBytes));
    VirtualProtect(function, sizeof(deleteFileOriginalBytes), oldProtect, &oldProtect);
}

void FileCallInterceptor::changeDeleteFileToHooked()
{
    BYTE* function = (BYTE*) deleteFileOriginal;
    DWORD oldProtect;

    VirtualProtect(function, sizeof(deleteFileAbsoluteJmpCode), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(function, deleteFileAbsoluteJmpCode, sizeof(deleteFileAbsoluteJmpCode));
    VirtualProtect(function, sizeof(deleteFileAbsoluteJmpCode), oldProtect, &oldProtect);
}

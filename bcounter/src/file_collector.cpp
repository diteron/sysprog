#include "file_collector.h"

#include <iostream>

FileCollector::FileCollector(std::queue<std::string>& fileQueue,
        HANDLE& queueMutex, HANDLE& taskEvent)
    : fileQueue{fileQueue}, queueMutex{queueMutex}, taskEvent{taskEvent}
{}

void FileCollector::collectFiles(const std::string& directory)
{
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile((directory + "\\*").c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            const std::string fileOrDir = directory + "\\" + findData.cFileName;

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                // Skip '.' and '..'
                if (findData.cFileName[0] != '.') {
                    collectFiles(fileOrDir);
                }
            } 
            else {
                WaitForSingleObject(queueMutex, INFINITE);
                fileQueue.push(fileOrDir);
                ReleaseMutex(queueMutex);
                SetEvent(taskEvent);
            }
        } while (FindNextFile(hFind, &findData));
        
        FindClose(hFind);
    }

    SetEvent(taskEvent);
}

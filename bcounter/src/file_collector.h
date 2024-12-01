#pragma once

#include <queue>
#include <string>
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;

class FileCollector
{
public:
    FileCollector() = delete;
    FileCollector(std::queue<std::string>& fileQueue,
            HANDLE& queueMutex, HANDLE& taskEvent);

    void collectFiles(const std::string& directory);

private:
    std::queue<std::string>& fileQueue;
    HANDLE& queueMutex;
    HANDLE& taskEvent;
};

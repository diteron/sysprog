#pragma once

#include <queue>
#include <string>
#include <windows.h>
#include <fstream>

class ThreadPool
{
public:
    ThreadPool() = delete;
    ThreadPool(int threadsInPool, std::queue<std::string>& fileQueue,
            HANDLE& queueMutex, HANDLE& taskEvent);

    ~ThreadPool();

    [[nodiscard]] bool start();
    void stop();

    void processFiles();

private:
    static DWORD WINAPI processFilesWrapper(LPVOID param);
    void processFile(const std::string& filePath);

    volatile bool isDone = false;
    const int threadsInPool;
    std::vector<HANDLE> threads;

    std::queue<std::string>& fileQueue;
    HANDLE& queueMutex;
    HANDLE& taskEvent;
    CRITICAL_SECTION coutLock;

    const char* OutFileName = "result.txt";
    std::ofstream resultFile;
    CRITICAL_SECTION fileLock;
};

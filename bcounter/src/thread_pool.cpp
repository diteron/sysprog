#include "thread_pool.h"

#include <bitset>
#include <cstdint>
#include <iostream>

ThreadPool::ThreadPool(int threadsInPool, std::queue<std::string>& fileQueue,
        HANDLE& queueMutex, HANDLE& taskEvent)
    : threadsInPool{threadsInPool}, fileQueue{fileQueue},
        queueMutex{queueMutex}, taskEvent{taskEvent},
        resultFile{OutFileName, std::ios::app}
{
    threads.resize(threadsInPool);
    InitializeCriticalSection(&fileLock);
    coutMutex = CreateMutex(nullptr, FALSE, nullptr);
}

ThreadPool::~ThreadPool()
{
    if (!isDone) {
        stop();
    }
    
    DeleteCriticalSection(&fileLock);
    CloseHandle(coutMutex);
}

bool ThreadPool::start()
{
    for (auto& thread : threads) {
        thread = CreateThread(nullptr, 0, &processFilesWrapper, this, 0, nullptr);
        if (thread == nullptr) {
            std::cerr << "Failed to start thread. Error code '" << GetLastError() << "'\n";
            return false;
        }
    }

    return true;
}

void ThreadPool::stop()
{
    isDone = true;

    WaitForMultipleObjects(threads.size(), threads.data(), TRUE, INFINITE);
    for (HANDLE thread : threads) {
        CloseHandle(thread);
    }
}

void ThreadPool::processFiles()
{       
    HANDLE threadHandle = GetCurrentThread();
    DWORD threadId = GetThreadId(threadHandle);

    while (true) {
        #ifndef NDEBUG
            WaitForSingleObject(coutMutex, INFINITE);
            std::cout << "Thread with id " << threadId << " is waiting for task\n";
            ReleaseMutex(coutMutex);
        #endif

        WaitForSingleObject(taskEvent, INFINITE);

        std::string filePath;
        { 
            WaitForSingleObject(queueMutex, INFINITE);
            if (!fileQueue.empty()) {
                filePath = fileQueue.front();
                fileQueue.pop();
                ReleaseMutex(queueMutex);
            }
            else if (!isDone) {
                ReleaseMutex(queueMutex);
                continue;
            }
            else {
                ReleaseMutex(queueMutex);
                return;
            }
        }

        #ifndef NDEBUG
            WaitForSingleObject(coutMutex, INFINITE);
            std::cout << "Thread with id " << threadId << " have started processing file '" << filePath<<  "'\n";
            ReleaseMutex(coutMutex);
        #endif

        processFile(filePath);
    }
}

DWORD __stdcall ThreadPool::processFilesWrapper(LPVOID param)
{
    ThreadPool* processor = static_cast<ThreadPool*>(param);
    processor->processFiles();
    return 0;
}

void ThreadPool::processFile(const std::string& filePath)
{
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (file.is_open()) {
        uint64_t ones = 0, zeros = 0;
        char byte;
        while (file.get(byte)) {
            std::bitset<8> bits(byte);
            ones += bits.count();
            zeros += 8 - bits.count();
        }
        file.close();

        EnterCriticalSection(&fileLock);
        resultFile << filePath.c_str() << "\n1-" << ones << " 0-" << zeros << "\n";
        LeaveCriticalSection(&fileLock);
    }
    else {
        EnterCriticalSection(&fileLock);
        resultFile << filePath.c_str() << "\n" << "Unable to process this file\n";
        LeaveCriticalSection(&fileLock);
    }
}

#include "thread_pool.h"
#include "file_collector.h"

#include <iostream>
#include <chrono>

bool isArgsCorrect(int argc, const char* argv[], long& numberOfThreads, std::string& dirPath);


int main(int argc, const char* argv[])
{
    using namespace std::chrono;

    long numberOfThreads;
    std::string dirPath;
    if (!isArgsCorrect(argc, argv, numberOfThreads, dirPath)) {
        return 1;
    }

    std::queue<std::string> fileQueue;
    HANDLE queueMutex = CreateMutex(nullptr, FALSE, nullptr);
    HANDLE taskEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    auto threadPool = std::make_unique<ThreadPool>(numberOfThreads, fileQueue, queueMutex, taskEvent);
    auto fileCollector = std::make_unique<FileCollector>(fileQueue, queueMutex, taskEvent);
    
    std::cout << "Counting bytes in dir '" << argv[1] << "' is started\n";

    uint64_t start, end;
    start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    if (!threadPool->start()) {
        std::cerr << "Failed to start thread pool\n";
        return 1;
    }
    
    fileCollector->collectFiles(dirPath);
    threadPool->stop();

    CloseHandle(queueMutex);
    CloseHandle(taskEvent);

    end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    uint64_t countTime = end - start;

    std::cout << "Counting bytes in dir '" << argv[1] << "' is completed in " << countTime << " milliseconds\n";

    return 0;
}


bool isArgsCorrect(int argc, const char* argv[], long& numberOfThreads, std::string& dirPath)
{
    static const long MaxNumberOfThreads = 32;

    if (argc < 2) {
        std::cerr << "Directory path not specified\n";
        return false;
    }
    else if (argc < 3) {
        std::cerr << "Number of threads in pool is not specified\n";
        return false;
    }

    dirPath = std::string(argv[1], argv[1] + std::strlen(argv[1]));
    numberOfThreads = std::strtol(argv[2], nullptr, 0);

    if (numberOfThreads <= 0) {
        std::cerr << "Number of threads in pool must be positive number\n";
        return false;
    }
    else if (numberOfThreads > MaxNumberOfThreads) {
        std::cerr << "Too many threads (max " << MaxNumberOfThreads << ")\n";
        return false;
    }

    return true;
}

#include "dll_injector.h"

#include <iostream>


int main(int argc, const char* argv[])
{
    const char* dllPath = "logger.dll";

    if (argc < 2) {
        std::cerr << "Usage: fintercept <path_to_program>\n";
        return 1;
    }


    return 0;
}

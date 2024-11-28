#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Directory path not specified.\n";
        return 1;
    }

    std::cout << "Entered directory path to count bytes: " << argv[1] << "\n";
    return 0;
}

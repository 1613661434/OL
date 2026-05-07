#include "ol_TimeStamp.h"
#include <iostream>
using namespace ol;

#ifdef __unix__
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

int main()
{
    TimeStamp ts;
    std::cout << ts.toInt() << std::endl;
    std::cout << ts.toString() << std::endl;

#ifdef __unix__
    sleep(1);
#elif defined(_WIN32)
    Sleep(1000);
#endif
    std::cout << TimeStamp::now().toInt() << std::endl;
    std::cout << TimeStamp::now().toString() << std::endl;

    return 0;
}

// g++ -o test_ol_TimeStamp test_ol_TimeStamp.cpp ol_TimeStamp.cpp
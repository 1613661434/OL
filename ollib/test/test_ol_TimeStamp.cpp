#include "ol_TimeStamp.h"
#include <iostream>
#include <unistd.h>

using namespace ol;

int main()
{
    TimeStamp ts;
    std::cout << ts.toInt() << std::endl;
    std::cout << ts.toString() << std::endl;

    sleep(1);
    std::cout << TimeStamp::now().toInt() << std::endl;
    std::cout << TimeStamp::now().toString() << std::endl;

    return 0;
}

// g++ -o test_ol_TimeStamp test_ol_TimeStamp.cpp ol_TimeStamp.cpp
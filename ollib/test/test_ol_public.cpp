#include "ol_public.h"
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    ccmdstr cstr;
    cstr.split("asdasd,asd213,43435123,32432", ",");
    cout << cstr;

    return 0;
}
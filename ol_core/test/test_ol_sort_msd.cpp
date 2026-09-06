#include "ol_sort.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace ol;

namespace
{
    bool checkAgainstStdSort(const char* test_name, std::vector<std::string> values)
    {
        auto expected = values;
        std::sort(expected.begin(), expected.end());
        radix_sort_msd(values);

        if (values == expected) return true;

        std::cerr << "[Error] " << test_name << " differs from std::sort.\n";
        return false;
    }
}

int main()
{
    int failures = 0;

    if (!checkAgainstStdSort("duplicate strings", {"aa", "aa", "aa"}))
        ++failures;

    if (!checkAgainstStdSort("empty and prefix strings",
                             {"apple", "app", "", "banana", "ban", "app", "a", ""}))
        ++failures;

    if (!checkAgainstStdSort("embedded zero and high bytes",
                             {std::string("\0", 1), std::string("a\0b", 3),
                              std::string(1, static_cast<char>(0x80)),
                              std::string(1, static_cast<char>(0xff)), "", "a"}))
        ++failures;

    {
        std::vector<std::string> values = {"app", "apple", "app", "banana", "band", ""};
        const auto groups = radix_group_by_prefix(values, 3);
        const std::vector<std::vector<std::string>> expected = {
            {""}, {"app", "apple", "app"}, {"banana", "band"}};

        if (groups != expected)
        {
            std::cerr << "[Error] prefix grouping produced unexpected groups.\n";
            ++failures;
        }
    }

    try
    {
        std::vector<std::string> values = {
            std::string(1, static_cast<char>(200)), "ascii"};
        radix_sort_msd(values, -1, 128);
        std::cerr << "[Error] radix overflow test did not throw invalid_argument.\n";
        ++failures;
    }
    catch (const std::invalid_argument&)
    {
    }
    catch (const std::exception& e)
    {
        std::cerr << "[Error] radix overflow test threw an unexpected exception: "
                  << e.what() << '\n';
        ++failures;
    }

    if (failures != 0)
    {
        std::cerr << failures << " MSD radix-sort test(s) failed.\n";
        return 1;
    }

    std::cout << "All MSD radix-sort tests passed.\n";
    return 0;
}

#include "ol_cqueue.h"

#include <cstdlib>
#include <iostream>
#include <unordered_set>

namespace
{
    // 只记录对象地址，不读取已经结束生命周期的对象成员。
    // 这样能够明确显示同一个数组元素是否被析构了两次。
    std::unordered_set<const void*>& liveObjects()
    {
        static std::unordered_set<const void*> objects;
        return objects;
    }

    class LifetimeProbe
    {
    public:
        LifetimeProbe()
        {
            const bool inserted = liveObjects().insert(this).second;
            std::cout << "construct  object @ " << this << '\n';

            if (!inserted)
            {
                std::cerr << "ERROR: object was constructed twice at the same live address\n";
                std::abort();
            }
        }

        LifetimeProbe(const LifetimeProbe&) : LifetimeProbe() {}
        LifetimeProbe(LifetimeProbe&&) noexcept : LifetimeProbe() {}

        LifetimeProbe& operator=(const LifetimeProbe&) = default;
        LifetimeProbe& operator=(LifetimeProbe&&) noexcept = default;

        ~LifetimeProbe()
        {
            if (liveObjects().erase(this) == 0)
            {
                std::cerr << "\nDOUBLE DESTRUCTION DETECTED @ " << this << '\n';
                std::cerr << "The cqueue destructor manually destroyed this element,\n"
                             "then the T m_data[MAX_SIZE] array destroyed it again.\n";
                std::abort();
            }

            std::cout << "destroy    object @ " << this << '\n';
        }
    };

    class NonDefaultValue
    {
    public:
        explicit NonDefaultValue(int value) : m_value(value) {}

        int value() const { return m_value; }

    private:
        int m_value;
    };
}

int main()
{
    std::cout << "Create cqueue<LifetimeProbe, 1> and push one element.\n";

    {
        ol::cqueue<LifetimeProbe, 1> queue;
        queue.push(LifetimeProbe{});

        std::cout << "Queue size: " << queue.size() << '\n';
        std::cout << "Leaving scope now...\n" << std::flush;
    }

    if (!liveObjects().empty())
    {
        std::cerr << "LEAK DETECTED: " << liveObjects().size()
                  << " object(s) were not destroyed.\n";
        return 1;
    }

    // 原始存储方案不应强迫队列元素提供默认构造函数。
    {
        ol::cqueue<NonDefaultValue, 1> queue;
        queue.emplace(42);
        if (queue.front().value() != 42)
        {
            std::cerr << "Unexpected value in non-default-constructible test.\n";
            return 1;
        }
        queue.pop();
    }

    std::cout << "No double destruction detected.\n";
    return 0;
}

#include <iostream>
#include <thread>
#include <mutex>

volatile bool g_done{ false };
volatile int* g_bounce{ nullptr };
int g_error{};
std::mutex am;

void Bouncer()
{
    for (int i = 0; i < 50000000; ++i) {
        am.lock();
        *g_bounce = -(1 + *g_bounce);
        am.unlock();
    }
    g_done = true;
}

void Checker()
{
    while (g_done == false) {
        am.lock();
        int y = *g_bounce;
        if ((0 != y) && (-1 != y))
            ++g_error;
        am.unlock();
    }
}


int main()
{
    //g_bounce = new int{ 0 };
    int arr[32];
    g_bounce = arr + 16;
    *g_bounce = 0;

    long long temp = reinterpret_cast<long long>(g_bounce);
    temp = ((temp / 64) * 64) - 2;
    g_bounce = reinterpret_cast<int*>(temp);

    std::thread b{ Bouncer };
    std::thread c{ Checker };
    b.join();
    c.join();

    std::cout << "Number of error : " << g_error << '\n';
}
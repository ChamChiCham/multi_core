#include <thread>
#include <iostream>
#include <mutex>
#include <atomic>

volatile bool done = false;
volatile int* bound;
int error;


void ThreadFunc1()
{
	for (int j = 0; j <= 25000000; ++j) *bound = -(1 + *bound);
	done = true;
}

void ThreadFunc2()
{
	while (!done) {
		int v = *bound;
		if ((v != 0) && (v != -1)) {
			error++;
			if (error < 10) {
				std::printf("%X ", v);
			}
		}
	}
}

int main()
{
	int arr[32]{};
	bound = arr + 20;

	long long temp = reinterpret_cast<long long>(bound);
	temp = (temp / 64) * 64;
	temp = temp - 2;
	bound = reinterpret_cast<int*>(temp);

	std::thread t1{ ThreadFunc1 };
	std::thread t2{ ThreadFunc2 };

	t1.join();
	t2.join();
		
	std::cout << error << std::endl;
}
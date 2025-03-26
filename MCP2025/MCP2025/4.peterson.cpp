#include <iostream>
#include <thread>
#include <atomic>

using namespace std;
volatile int victim = 0;
volatile bool flag[2] = { false, false };
volatile int sum;

void Lock(int myID)
{
	int other = 1 - myID;
	flag[myID] = true;
	victim = myID;
	while (flag[other] && victim == myID) { std::this_thread::yield(); }
}
void Unlock(int myID)
{
	flag[myID] = false;
}

void worker_peterson(int thid)
{
	for (auto i = 0; i < 25000000; ++i) {
		Lock(thid);
		sum = sum + 2;
		Unlock(thid);
	}
}
int main()
{
	thread t1 = thread{ worker_peterson, 0 };
	thread t2 = thread{ worker_peterson, 1 };
	auto start = chrono::high_resolution_clock::now();
	t1.join();
	t2.join();
	auto end = chrono::high_resolution_clock::now();
	auto diff = chrono::duration_cast<chrono::milliseconds>(end - start).count();
	std::cout << "time: "<< diff << "ms / res = " << static_cast<int>(sum) << std::endl;
	cout << "Sum = " << sum << "\n";
}

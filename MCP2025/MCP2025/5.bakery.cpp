#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>

constexpr int SIZE = 50'000'000;
constexpr int THREAD = 8;

using namespace std;
volatile int sum;
std::mutex sl;

int g_cur_thread{};

class BakeryVolatile
{
private:
	volatile bool flag[THREAD];
	volatile int label[THREAD];

public:
	BakeryVolatile()
	{
		for (int i = 0; i < THREAD; ++i) {
			flag[i] = false;
			label[i] = 0;
		}
	}

	void lock(const int id)
	{

		flag[id] = true;

		int max_val{-1};
		for (auto val : label) {
			if (max_val < val) {
				max_val = val;
			}
		}

		label[id] = max_val + 1;
		bool loop{ false };
		do {
			loop = false;
			for (int i = 0; i < g_cur_thread; ++i) {
				if (i == id) { continue; }
				if (flag[i] &&
					((label[i] < label[id]) ||
					((label[i] == label[id]) && (i < id)))
					) {
					loop = true;
					break;
				}
			}
		} while (loop);
	}

	void unlock(const int id)
	{
		flag[id] = false;
	}

};

class BakeryAtomic
{
private:
	std::atomic_bool flag[THREAD];
	std::atomic_int label[THREAD];

public:
	BakeryAtomic()
	{
		for (int i = 0; i < THREAD; ++i) {
			flag[i] = false;
			label[i] = 0;
		}
	}

	void lock(const int id)
	{

		flag[id] = true;

		int max_val{ -1 };
		for (auto& val : label) {
			if (max_val < val) {
				max_val = val;
			}
		}

		label[id] = max_val + 1;
		bool loop{ false };
		do {
			loop = false;
			for (int i = 0; i < g_cur_thread; ++i) {
				if (i == id) { continue; }
				if (flag[i] &&
					((label[i] < label[id]) ||
						((label[i] == label[id]) && (i < id)))
					) {
					loop = true;
					break;
				}
			}
		} while (loop);
	}

	void unlock(const int id)
	{
		flag[id] = false;
	}

};



void worker_no_lock(const int thread_num)
{
	for (int i = 0; i < SIZE / thread_num; ++i) {
		sum = sum + 2;
	}
}

void worker_mutex(const int thread_num)
{
	for (int i = 0; i < SIZE / thread_num; ++i) {
		sl.lock();
		sum = sum + 2;
		sl.unlock();
	}
}

BakeryVolatile b_v_l;
void worker_bakery_volatile(const int thread_id, const int thread_num)
{
	for (int i = 0; i < SIZE / thread_num; ++i) {
		b_v_l.lock(thread_id);
		sum = sum + 2;
		b_v_l.unlock(thread_id);
	}
}


BakeryAtomic b_a_l;
void worker_bakery_atomic(const int thread_id, const int thread_num)
{
	for (int i = 0; i < SIZE / thread_num; ++i) {
		b_a_l.lock(thread_id);
		sum = sum + 2;
		b_a_l.unlock(thread_id);
	}
}





int main()
{
	std::cout << "no lock " << std::endl;
	for (int thread_num = 1; thread_num <= THREAD; thread_num *= 2) {
		sum = 0;
		std::vector<std::thread> threads;
		auto start = chrono::high_resolution_clock::now();
		for (int i = 0; i < thread_num; ++i) {
			threads.emplace_back(worker_no_lock, thread_num);
		}
		for (auto& th : threads) {
			th.join();
		}
		auto end = chrono::high_resolution_clock::now();

		auto diff = chrono::duration_cast<chrono::milliseconds>(end - start).count();
		std::cout << "threads " << thread_num << ": time: " << diff << "ms / res = "<< static_cast<int>(sum) << std::endl;
	}

	std::cout << "mutex " << std::endl;
	for (int thread_num = 1; thread_num <= THREAD; thread_num *= 2) {
		sum = 0;
		std::vector<std::thread> threads;
		auto start = chrono::high_resolution_clock::now();
		for (int i = 0; i < thread_num; ++i) {
			threads.emplace_back(worker_mutex, thread_num);
		}
		for (auto& th : threads) {
			th.join();
		}
		auto end = chrono::high_resolution_clock::now();

		auto diff = chrono::duration_cast<chrono::milliseconds>(end - start).count();
		std::cout << "threads " << thread_num << ": time: " << diff << "ms / res = " << static_cast<int>(sum) << std::endl;
	}

	std::cout << "bakery volatile" << std::endl;
	for (int thread_num = 1; thread_num <= THREAD; thread_num *= 2) {
		g_cur_thread = thread_num;
		sum = 0;
		std::vector<std::thread> threads;
		auto start = chrono::high_resolution_clock::now();
		for (int i = 0; i < thread_num; ++i) {
			threads.emplace_back(worker_bakery_volatile, i, thread_num);
		}
		for (auto& th : threads) {
			th.join();
		}
		auto end = chrono::high_resolution_clock::now();

		auto diff = chrono::duration_cast<chrono::milliseconds>(end - start).count();
		std::cout << "threads " << thread_num << ": time: " << diff << "ms / res = " << static_cast<int>(sum) << std::endl;
	}



	std::cout << "bakery atomic" << std::endl;
	for (int thread_num = 1; thread_num <= THREAD; thread_num *= 2) {
		g_cur_thread = thread_num;
		sum = 0;
		std::vector<std::thread> threads;
		auto start = chrono::high_resolution_clock::now();
		for (int i = 0; i < thread_num; ++i) {
			threads.emplace_back(worker_bakery_atomic, i, thread_num);
		}
		for (auto& th : threads) {
			th.join();
		}
		auto end = chrono::high_resolution_clock::now();

		auto diff = chrono::duration_cast<chrono::milliseconds>(end - start).count();
		std::cout << "threads " << thread_num << ": time: " << diff << "ms / res = " << static_cast<int>(sum) << std::endl;
	}


}

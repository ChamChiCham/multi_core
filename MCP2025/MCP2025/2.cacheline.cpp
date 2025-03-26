#include <print>
#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;

std::mutex s_m;
volatile int sum;
atomic_int asum;

volatile int arr_sum[16];

struct NUM {
	alignas(64) volatile int num;
};
NUM cache_arr_sum[16];

void worker(int count)
{
	for (int i = 0; i < 50'000'000 / count; ++i) {
		s_m.lock();
		sum = sum + 2;
		s_m.unlock();
	}
}

void a_worker(int count)
{
	for (int i = 0; i < 50'000'000 / count; ++i) {
		asum += 2;
	}
}

void l_worker(int count)
{
	volatile int l_sum{};
	for (int i = 0; i < 50'000'000 / count; ++i) {
		l_sum = l_sum + 2;
	}
	s_m.lock();
	sum += l_sum;
	s_m.unlock();
}

void o2_worker(const int count, const int thread_id)
{
	arr_sum[thread_id] = 0;
	for (int i = 0; i < 50'000'000 / count; ++i) {
		arr_sum[thread_id] = arr_sum[thread_id] + 2;
	}
}

void o3_worker(const int count, const int thread_id)
{
	cache_arr_sum[thread_id].num = 0;
	for (int i = 0; i < 50'000'000 / count; ++i) {
		cache_arr_sum[thread_id].num = cache_arr_sum[thread_id].num + 2;
	}
}

int main()
{
	worker(1);

	println("local");
	for (int thread_num = 1; thread_num <= 16; thread_num *= 2) {
		sum = 0;
		std::vector<std::thread> threads;
		auto start = chrono::high_resolution_clock::now();
		for (int i = 0; i < thread_num; ++i) {
			threads.emplace_back(l_worker, thread_num);
		}
		for (auto& th : threads) {
			th.join();
		}
		auto end = chrono::high_resolution_clock::now();

		auto diff = chrono::duration_cast<chrono::milliseconds>(end - start).count();
		println("threads {} : time: {}ms / res = {}", thread_num, diff, static_cast<int>(sum));
	}

	println("global array");
	for (int thread_num = 1; thread_num <= 16; thread_num *= 2) {
		sum = 0;
		std::vector<std::thread> threads;
		auto start = chrono::high_resolution_clock::now();
		for (int i = 0; i < thread_num; ++i) {
			threads.emplace_back(o2_worker, thread_num, i);
		}
		for (auto& th : threads) {
			th.join();
		}
		auto end = chrono::high_resolution_clock::now();

		auto diff = chrono::duration_cast<chrono::milliseconds>(end - start).count();
		int res{};
		for (auto value : arr_sum) {
			res += value;
		}
		println("threads {} : time: {}ms / res = {}", thread_num, diff, res);
	}

	println("global cache array");
	for (int thread_num = 1; thread_num <= 16; thread_num *= 2) {
		sum = 0;
		std::vector<std::thread> threads;
		auto start = chrono::high_resolution_clock::now();
		for (int i = 0; i < thread_num; ++i) {
			threads.emplace_back(o3_worker, thread_num, i);
		}
		for (auto& th : threads) {
			th.join();
		}
		auto end = chrono::high_resolution_clock::now();

		auto diff = chrono::duration_cast<chrono::milliseconds>(end - start).count();
		int res{};
		for (auto value : arr_sum) {
			res += value;
		}
		println("threads {} : time: {}ms / res = {}", thread_num, diff, res);
	}
}
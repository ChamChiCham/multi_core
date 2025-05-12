#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>


struct NODE {
	int value;
	NODE* volatile next;
	NODE() : value(-1) { next = nullptr; }
	NODE(int x) : value(x), next(nullptr) {}
};

class CQUEUE {
	NODE* head, * tail;
	std::mutex	hl, tl;
public:
	CQUEUE()
	{
		head = tail = new NODE{ 0 };
	}
	~CQUEUE() {
		clear();
		delete head;
	}

	void clear()
	{
		while (head->next != nullptr) {
			auto ptr = head;
			head = head->next;
			delete ptr;
		}
	}

	void Enq(int x)
	{
		NODE* n = new NODE(x);
		tl.lock();
		tail->next = n;
		tail = n;
		tl.unlock();
	}

	int Deq()
	{
		hl.lock();
		if (head->next == nullptr) {
			hl.unlock();
			return -1;
		}
		int value = head->next->value;
		NODE* old = head;
		head = head->next;
		hl.unlock();
		delete old;
		return value;
	}

	void print20()
	{
		auto p = head->next;

		for (int i = 0; i < 20; ++i) {
			if (nullptr == p) break;
			std::cout << p->value << ", ";
			p = p->next;
		}
		std::cout << std::endl;
	}
};

class LFQUEUE {
	NODE* volatile head, * volatile tail;
public:
	LFQUEUE()
	{
		head = tail = new NODE{ 0 };
	}
	~LFQUEUE() {
		clear();
		delete head;
	}

	void clear()
	{
		while (head->next != nullptr) {
			auto ptr = head;
			head = head->next;
			delete ptr;
		}
	}

	bool CAS(NODE** addr, NODE* old_p, NODE* new_p)
	{
		std::atomic<long long> *a = reinterpret_cast<std::atomic_llong *>(addr);
		long long o = reinterpret_cast<long long>(old_p);
		long long n = reinterpret_cast<long long>(new_p);
		return std::atomic_compare_exchange_strong(a, &o, n);
	}

	void Enq(int x)
	{
		NODE* n = new NODE(x);

		CAS(&tail->next, nullptr, n);
		tail->next = n;
		tail = n;

	}

	int Deq()
	{
		hl.lock();
		if (head->next == nullptr) {
			hl.unlock();
			return -1;
		}
		int value = head->next->value;
		NODE* old = head;
		head = head->next;
		hl.unlock();
		delete old;
		return value;
	}

	void print20()
	{
		auto p = head->next;

		for (int i = 0; i < 20; ++i) {
			if (nullptr == p) break;
			std::cout << p->value << ", ";
			p = p->next;
		}
		std::cout << std::endl;
	}
};

constexpr int MAX_THREADS = 16;

constexpr int NUM_TEST = 10000000;

CQUEUE g_queue;

void benchmark(const int num_thread)
{
	int key = 0;
	const int num_loop = NUM_TEST / num_thread;

	for (int i = 0; i < num_loop; i++) {
		if ((i < 32) || (rand() % 2 == 0))
			g_queue.Enq(key++);
		else
			g_queue.Deq();

	}
}

int main()
{
	using namespace std::chrono;

	{
		for (int i = 1; i <= 16; i = i * 2) {
			std::vector <std::thread> threads;
			g_queue.clear();
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j)
				threads.emplace_back(benchmark, i);
			for (auto& th : threads)
				th.join();
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();

			std::cout << i << " Threads, Exec time = " << exec_ms << "ms.  ";
			std::cout << "QUEUE = ";
			g_queue.print20();
			std::cout << std::endl;
		}
	}

	{
		g_queue.clear();
		auto start_t = system_clock::now();
		benchmark(1);
		auto end_t = system_clock::now();
		auto exec_t = end_t - start_t;
		auto exec_ms = duration_cast<milliseconds>(exec_t).count();

		std::cout << "Single Thread QUEUE = ";
		g_queue.print20();
		std::cout << ", Exec time = " << exec_ms << "ms.\n;";
	}
}


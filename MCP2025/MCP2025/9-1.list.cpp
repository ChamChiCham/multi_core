#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>

class NODE {	
public:
	int key;
	NODE* next;

	NODE() { next = nullptr; }

	NODE(int key_value) {
		next = nullptr;
		key = key_value;
	}

	~NODE() {}
};

class DummyMutex {
public:
	void lock() {}
	void unlock() {}
};

class CLIST {
	NODE *head, *tail;
	std::mutex glock;

public:
	CLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~CLIST() {}

	void clear()
	{
		while (head->next != tail) {
			auto ptr = head->next;
			head->next = head->next->next;
			delete ptr;
		}
	}
	bool Add(int key)
	{
		NODE* pred = head;

		glock.lock();
		NODE* curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}
		
		if (curr->key == key) {
			glock.unlock();
			return false;
		}
		else {
			auto n = new NODE{ key };
			n->next = curr;
			pred->next = n;
			glock.unlock();
			return true;
		}
	}
	bool Remove(int key)
	{
		NODE* pred = head;
		glock.lock();
		NODE* curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->key == key) {
			auto n = curr;
			pred->next = n->next;
			glock.unlock();
			delete n;
			return true;
		} else {
			glock.unlock();
			return false;
		}
	}
	bool Contains(int key)
	{
		NODE* pred = head;
		glock.lock();
		NODE* curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->key == key) {
			glock.unlock();
			return true;
		}
		else {
			glock.unlock();
			return false;
		}
	}
	void print20()
	{
		auto p = head->next;

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next;
		}
		std::cout << std::endl;
	}
};

constexpr int NUM_TEST = 4000000;
constexpr int KEY_RANGE = 1000;

CLIST g_set;

void benchmark(int num_thread)
{
	int key;
	const int num_loop = NUM_TEST / num_thread;

	for (int i = 0; i < num_loop; i++) {
		switch (rand() % 3) {
		case 0: key = rand() % KEY_RANGE;
			g_set.Add(key);
			break;
		case 1: key = rand() % KEY_RANGE;
			g_set.Remove(key);
			break;
		case 2: key = rand() % KEY_RANGE;
			g_set.Contains(key);
			break;
		default: std::cout << "Error\n";
			exit(-1);
		}
	}
}

int main()
{
	using namespace std::chrono;
	
	{
		auto start_t = system_clock::now();
		benchmark(1);
		auto end_t = system_clock::now();
		auto exec_t = end_t - start_t;
		auto exec_ms = duration_cast<milliseconds>(exec_t).count();

		std::cout << "Single Thread SET = ";
		g_set.print20();
		std::cout << ", Exec time = " << exec_ms << "ms.\n;";
	}

{
		for (int i = 1; i <= 16; i = i * 2) {
			std::vector <std::thread> threads;
			g_set.clear();
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j)
				threads.emplace_back(benchmark, i);
			for (auto& th : threads)
				th.join();
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();

			std::cout << i << " Threads : SET = ";
			g_set.print20();
			std::cout << ", Exec time = " << exec_ms << "ms.\n;";
		}
	}
}


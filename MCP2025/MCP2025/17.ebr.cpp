#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <array>


class HISTORY {
public:
	int op;
	int i_value;
	bool o_value;
	HISTORY(int o, int i, bool re) : op(o), i_value(i), o_value(re) {}
};

std::array<std::vector<HISTORY>, 16> history;

//struct NODE {
//public:
//	int key;
//	NODE* next;
//	std::mutex glock;
//	bool marked{ false };
//
//	NODE() { next = nullptr; }
//
//	NODE(int key_value) {
//		next = nullptr;
//		key = key_value;
//	}
//
//	~NODE() {}
//
//	void lock() { glock.lock(); }
//	void unlock() { glock.unlock(); }
//};

struct LFNODE;

class AMR
{
private:
	long long data{ 0 };

public:
	bool get_mark() const
	{
		return data & 0x01;
	}

	LFNODE* get_ptr() const
	{
		long long temp = data & 0xFFFFFFFFFFFFFFE;
		return reinterpret_cast<LFNODE*>(temp);
	}

	LFNODE* get_ptr(bool* get_removed)
	{
		long long temp = data;
		*get_removed = (temp & 1);
		temp = temp & 0xFFFFFFFFFFFFFFE;
		return reinterpret_cast<LFNODE*>(temp);
	}

	void set_ptr(LFNODE* p)
	{
		long long temp = reinterpret_cast<long long>(p);
		temp = temp & 0xFFFFFFFFFFFFFFE;
		data = temp;
	}

	bool CAS(LFNODE* old_ptr, LFNODE* new_ptr, bool old_marked, bool new_marked)
	{
		long long old_temp = reinterpret_cast<long long>(old_ptr);
		if (old_marked) {
			old_temp = old_temp | 0x01;
		}
		else {
			old_temp = old_temp & 0xFFFFFFFFFFFFFFE;
		}

		long long new_temp = reinterpret_cast<long long>(new_ptr);
		if (new_marked) {
			new_temp = new_temp | 0x01;
		}
		else {
			new_temp = new_temp & 0xFFFFFFFFFFFFFFE;
		}

		std::atomic_compare_exchange_strong(
			reinterpret_cast<std::atomic<long long>*>(data), &old_temp, new_temp);
	}

};

struct LFNODE {
	int key;
	AMR next;

	LFNODE() { next.set_ptr(nullptr); }

	LFNODE(int key_value) {
		next.set_ptr(nullptr);
		key = key_value;
	}

	~LFNODE() {}
};


class LLIST {
	LFNODE* head, * tail;
public:
	LLIST()
	{
		head = new LFNODE{ std::numeric_limits<int>::min() };
		tail = new LFNODE{ std::numeric_limits<int>::max() };
		head->next.set_ptr(tail);
	}
	~LLIST()
	{
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next.get_ptr() != tail) {
			auto ptr = head->next;
			head->next = head->next.get_ptr()->next;
			delete ptr.get_ptr();
		}
	}

	void Find(LFNODE*& pred, LFNODE*& curr, int x)
	{
	retry:
		pred = head;
		while (true) {
			curr = pred->next.get_ptr();
			bool is_removed{ false };
			LFNODE* succ = curr->next.get_ptr(&is_removed);
			while (true == is_removed) {
				if (false == pred->next.CAS(curr, succ, false, false)) {
					goto retry;
				}
				curr = succ;
				succ = curr->next.get_ptr(&is_removed);
			}
			if (curr->key >= x) return;
			pred = curr;
			curr = succ;
		}
	}


	bool Add(int key)
	{
		while (true) {
			LFNODE* pred{ nullptr };
			LFNODE* curr{ nullptr };
			Find(pred, curr, key);

			// curr->lock(); pred->lock();
			if (curr->key == key) {
				// curr->unlock(); pred->unlock();
				return false;
			}
			else {
				auto n = new LFNODE{ key };
				n->next.set_ptr(curr);
				if (false == pred->next.CAS(curr, n, false, false)) {
					delete n;
					continue;
				}
				// curr->unlock(); pred->unlock();
				return true;
			}

			// curr->unlock(); pred->unlock();
		}
	}
	bool Remove(int key)
	{
		while (true) {
			LFNODE* pred{ nullptr };
			LFNODE* curr{ nullptr };

			Find(pred, curr, key);

			if (curr->key != key) return false;

			if (curr->key == key) {
				LFNODE* succ = curr->next.get_ptr();
				if (false == curr->next.CAS(succ, succ, false, true)) {
					continue;
				}
				pred->next.CAS(curr, succ, false, false);
				return true;
			}
		}
	}
	bool Contains(int key)
	{
		LFNODE* curr = head;
		while (true) {
			bool marked = false;
			while (curr->key < key) {
				curr = curr->next.get_ptr(&marked);
			}
			return key == curr->key && not marked;
		}
	}
	void print20()
	{
		auto p = head->next.get_ptr();

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next.get_ptr();
		}
		std::cout << std::endl;
	}
};

constexpr int NUM_TEST = 4000000;
constexpr int KEY_RANGE = 1000;

LLIST g_set;

void check_history(int num_threads)
{
	std::array <int, KEY_RANGE> survive = {};
	std::cout << "Checking Consistency : ";
	if (history[0].size() == 0) {
		std::cout << "No history.\n";
		return;
	}
	for (int i = 0; i < num_threads; ++i) {
		for (auto& op : history[i]) {
			if (false == op.o_value) continue;
			if (op.op == 3) continue;
			if (op.op == 0) survive[op.i_value]++;
			if (op.op == 1) survive[op.i_value]--;
		}
	}
	for (int i = 0; i < KEY_RANGE; ++i) {
		int val = survive[i];
		if (val < 0) {
			std::cout << "ERROR. The value " << i << " removed while it is not in the set.\n";
			exit(-1);
		}
		else if (val > 1) {
			std::cout << "ERROR. The value " << i << " is added while the set already have it.\n";
			exit(-1);
		}
		else if (val == 0) {
			if (g_set.Contains(i)) {
				std::cout << "ERROR. The value " << i << " should not exists.\n";
				exit(-1);
			}
		}
		else if (val == 1) {
			if (false == g_set.Contains(i)) {
				std::cout << "ERROR. The value " << i << " shoud exists.\n";
				exit(-1);
			}
		}
	}
	std::cout << " OK\n";
}


void benchmark_check(int num_threads, int th_id)
{
	for (int i = 0; i < NUM_TEST / num_threads; ++i) {
		int op = rand() % 3;
		switch (op) {
		case 0: {
			int v = rand() % KEY_RANGE;
			history[th_id].emplace_back(0, v, g_set.Add(v));
			break;
		}
		case 1: {
			int v = rand() % KEY_RANGE;
			history[th_id].emplace_back(1, v, g_set.Remove(v));
			break;
		}
		case 2: {
			int v = rand() % KEY_RANGE;
			history[th_id].emplace_back(2, v, g_set.Contains(v));
			break;
		}
		}
	}
}
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

	// 알고리즘 정확성 검사
	{
		for (int i = 1; i <= 16; i = i * 2) {
			std::vector <std::thread> threads;
			g_set.clear();
			for (auto& h : history) h.clear();
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j)
				threads.emplace_back(benchmark_check, i, j);
			for (auto& th : threads)
				th.join();
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();

			std::cout << i << " Threads : SET = ";
			g_set.print20();
			std::cout << ", Exec time = " << exec_ms << "ms.\n;";
			check_history(i);
		}
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


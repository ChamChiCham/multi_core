#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <queue>


constexpr int MAX_THREADS = 16;

class NODE {
public:
	int	key;
	NODE* volatile next;
	NODE(int x) : key(x), next(nullptr) {}
};

class DUMMYMUTEX
{
public:
	void lock() {}
	void unlock() {}
};

class C_QUEUE {
	NODE* volatile head;
	NODE * volatile tail;
	std::mutex head_lock, tail_lock;
public:
	C_QUEUE()
	{
		head = tail = new NODE{ -1 };
	}
	void clear()
	{
		while (-1 != Dequeue());
	}
	void Enqueue(int x)
	{
		NODE* e = new NODE{ x };
		tail_lock.lock();
		tail->next = e;
		tail = e;
		tail_lock.unlock();
	}
	int Dequeue()
	{
		head_lock.lock();
		if (nullptr == head->next) {
			head_lock.unlock();
			return -1;
		}
		int v = head->next->key;
		NODE* e = head;
		head = head->next;
		head_lock.unlock();
		delete e;
		return v;
	}
	void print20()
	{
		for (int i = 0; i < 20; ++i) {
			int v = Dequeue();
			if (-1 == v) break;
			std::cout << v << ", ";
		}
		std::cout << std::endl;
	}
};

std::atomic_bool stop = false;
thread_local std::queue<NODE*> node_pool;

class LF_QUEUE {
	NODE* volatile head;
	NODE* volatile tail;
public:
	LF_QUEUE()
	{
		head = tail = new NODE{ -1 };
	}
	void clear()
	{
		while (-1 != Dequeue());
	}
	bool CAS(NODE* volatile* ptr, NODE* old_ptr, NODE* new_ptr)
	{
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic_llong*>(ptr),
			reinterpret_cast<long long*>(&old_ptr),
			reinterpret_cast<long long>(new_ptr)
		);
	}
	void Enqueue(int x)
	{
		NODE* e = nullptr;
		if (node_pool.empty()) e = new NODE{x};
		else {
			e = node_pool.front();
			node_pool.pop();
		}
		while (true) {
			auto last = tail;
			auto next = last->next;
			if (tail != last) continue;
			if (next != nullptr) {
				CAS(&tail, last, next);
				continue;
			}

			if (true == CAS(&last->next, nullptr, e)) {
				CAS(&tail, last, e);
				return;
			}
		}
	}
	int Dequeue()
	{
		while (true) {
			auto first = head;
			auto next = first->next;
			auto last = tail;
			if (first != head)
				continue;
			if (nullptr == next) 
				return -1;
			if (first == last) {
				CAS(&tail, last, next);
				continue;
			}
			auto v = next->key;
			if (true == CAS(&head, first, next)) {
				// node_pool.push(first);
				return v;
			}
		}
	}
	void print20()
	{
		for (int i = 0; i < 20; ++i) {
			int v = Dequeue();
			if (-1 == v) break;
			std::cout << v << ", ";
		}
		std::cout << std::endl;
	}
};


// 64bit stamp
class STNODE;
class STPTR;
bool CAS(STPTR* next1, STNODE* old_ptr, STNODE* new_ptr, int64_t old_st, int64_t new_st);

thread_local std::queue<STNODE*> stnode_pool;

class alignas(16) STPTR {
	// NODE* volatile m_ptr;   ====> 프로그래밍 할 때 실수할 확률이 매우 높다.
	// int m_stamp;
public:
	// std::atomic_llong m_stpr;

	STNODE* volatile _ptr;
	int64_t volatile _stamp;

	STPTR(STNODE* p, int64_t st)
	{
		set_ptr(p, st);
	}


	void set_ptr(STNODE* p, int64_t st)
	{
		_ptr = p;
		_stamp = st;
	}

	STNODE* get_ptr()
	{
		return _ptr;
	}
	int64_t get_stamp()
	{
		return _stamp;
	}
};


// next1에 있는 포인터를 예전 ptr + st에서 최신 ptr + st으로 바꾼다.
bool CAS(STPTR* next1, STNODE* old_ptr, STNODE* new_ptr, int64_t old_st, int64_t new_st)
{
	STPTR old_sptr{ old_ptr, old_st };

	return _InterlockedCompareExchange128(reinterpret_cast<int64_t volatile*>(next1),
		new_st, reinterpret_cast<int64_t>(new_ptr),
		reinterpret_cast<int64_t*>(&old_sptr));

}


class STNODE {
public:
	int	key;
	STPTR next;
	STNODE(int x) : key(x), next(nullptr, 0) {}
};

class ST_LF_QUEUE {
	STPTR head{ nullptr, 0 }, tail{ nullptr, 0 };
public:
	ST_LF_QUEUE()
	{
		auto n = new STNODE{ -1 };
		head.set_ptr(n, 0);
		tail.set_ptr(n, 0);
	}
	void clear()
	{
		while (-1 != Dequeue());
	}
	void Enqueue(int x)
	{
		STNODE* e = nullptr;
		if (stnode_pool.empty()) e = new STNODE{ x };
		else {
			e = stnode_pool.front();
			stnode_pool.pop();
			e->key = x;
			int64_t stamp = e->next.get_stamp();
			e->next.set_ptr(nullptr, stamp + 1);
		}
		while (true) {
			STPTR last{ tail };
			STPTR next{ last.get_ptr()->next };
			if (tail.get_ptr() != last.get_ptr()) continue;
			if (next.get_ptr() != nullptr) {
				CAS(&tail, last.get_ptr(), next.get_ptr(),
					last.get_stamp(), last.get_stamp() + 1);
				continue;
			}

			if (true == CAS(&last.get_ptr()->next, nullptr, e,
				next.get_stamp(), next.get_stamp() + 1)) {
				CAS(&tail, last.get_ptr(), e, last.get_stamp(), last.get_stamp() + 1);
				return;
			}
		}
	}
	int Dequeue()
	{
		while (true) {
			auto first = head;
			auto next = first.get_ptr()->next;
			auto last = tail;
			if (first.get_ptr() != head.get_ptr())
				continue;
			if (nullptr == next.get_ptr())
				return -1;
			if (first.get_ptr() == last.get_ptr()) {
				CAS(&tail, last.get_ptr(), next.get_ptr(),
					last.get_stamp(), last.get_stamp() + 1);
				continue;
			}
			auto v = next.get_ptr()->key;
			if (true == CAS(&head, first.get_ptr(), next.get_ptr(), first.get_stamp(), first.get_stamp() + 1)) {
				stnode_pool.push(first.get_ptr());
				return v;
			}
		}
	}
	void print20()
	{
		for (int i = 0; i < 20; ++i) {
			int v = Dequeue();
			if (-1 == v) break;
			std::cout << v << ", ";
		}
		std::cout << std::endl;
	}
};





ST_LF_QUEUE my_queue;
thread_local int thread_id;

const int NUM_TEST = 10000000;

std::atomic_int loop_count = NUM_TEST;

void benchmark(const int th_id, const int num_thread)
{
	int key = 0;

	thread_id = th_id;
	while  (loop_count-- > 0) {
		if (rand() % 2 == 0)
			my_queue.Enqueue(key++);
		else
			my_queue.Dequeue();
	}
}


int main()
{
	using namespace std::chrono;

	for (int n = 1; n <= MAX_THREADS; n = n * 2) {
		loop_count = NUM_TEST;
		my_queue.clear();
		std::vector<std::thread> tv;
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < n; ++i) {
			tv.emplace_back(benchmark, i, n);
		}
		for (auto& th : tv)
			th.join();
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		size_t ms = duration_cast<milliseconds>(exec_t).count();
		std::cout << n << " Threads,  " << ms << "ms.";
		my_queue.print20();
	}
}
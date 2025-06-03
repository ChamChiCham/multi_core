#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <array>
#include <queue>
#include <set>


class HISTORY {
public:
	int op;
	int i_value;
	bool o_value;
	HISTORY(int o, int i, bool re) : op(o), i_value(i), o_value(re) {}
};

std::array<std::vector<HISTORY>, 16> history;

struct NODE {
	int key;
	NODE* next;
	std::mutex sm;
	volatile bool removed{ false };
	NODE() : key(-1) { next = nullptr; }
	NODE(int x) : key(x), next(nullptr) {}
	void lock()
	{
		sm.lock();
	}
	void unlock()
	{
		sm.unlock();
	}
};

class CLIST {
	NODE *head, *tail;
	//std::mutex	sm;
public:
	CLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~CLIST() {
		delete head;
		delete tail;
	}

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
		//sm.lock();
		NODE* curr = pred->next;

		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}
		
		if (curr->key == key) {
			//sm.unlock();
			return false;
		}
		else {
			auto n = new NODE{ key };
			n->next = curr;
			pred->next = n;
			//sm.unlock();
			return true;
		}
	}
	bool Remove(int key)
	{
		NODE* pred = head;
		//sm.lock();
		NODE* curr = pred->next;

		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->key == key) {
			auto n = curr;
			pred->next = n->next;
			//sm.unlock();
			delete n;
			return true;
		} else {
			//sm.unlock();
			return false;
		}
	}
	bool Contains(int key)
	{
		NODE* pred = head;
		//sm.lock();
		NODE* curr = pred->next;

		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->key == key) {
			//sm.unlock();
			return true;
		}
		else {
			//sm.unlock();
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

class FLIST {
	NODE* head, * tail;
public:
	FLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~FLIST()
	{
		delete head;
		delete tail;
	}

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
		pred->lock();
		NODE* curr = pred->next;
		curr->lock();

		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->key == key) {
			pred->unlock(); curr->unlock();
			return false;
		}
		else {
			auto n = new NODE{ key };
			n->next = curr;
			pred->next = n;
			pred->unlock(); curr->unlock();
			return true;
		}
	}
	bool Remove(int key)
	{
		NODE* pred = head;
		pred->lock();
		NODE* curr = pred->next;
		curr->lock();
		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->key == key) {
			auto n = curr;
			pred->next = n->next;
			pred->unlock(); curr->unlock();
			delete n;
			return true;
		}
		else {
			pred->unlock(); curr->unlock();
			return false;
		}
	}
	bool Contains(int key)
	{
		NODE* pred = head;
		pred->lock();
		NODE* curr = pred->next;
		curr->lock();

		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->key == key) {
			pred->unlock(); curr->unlock();
			return true;
		}
		else {
			pred->unlock(); curr->unlock();
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

class OLIST {
	NODE* head, * tail;
public:
	OLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~OLIST()
	{
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next != tail) {
			auto ptr = head->next;
			head->next = head->next->next;
			delete ptr;
		}
	}

	bool validate(NODE* pred, NODE* curr)
	{
		NODE* n = head;
		while (n->key <= pred->key) {
			if (pred == n)
				return pred->next == curr;
			n = n->next;
		}
		return false;
	}

	bool Add(int key)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (true == validate(pred, curr)) {
				if (curr->key == key) {
					pred->unlock(); curr->unlock();
					return false;
				}
				else {
					auto n = new NODE{ key };
					n->next = curr;
					pred->next = n;
					pred->unlock(); curr->unlock();
					return true;
				}
			}
			else {
				pred->unlock(); curr->unlock();
			}
		}
	}
	bool Remove(int key)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (true == validate(pred, curr)) {
				if (curr->key == key) {
					auto n = curr;
					pred->next = n->next;
					pred->unlock(); curr->unlock();
					// delete n;
					return true;
				}
				else {
					pred->unlock(); curr->unlock();
					return false;
				}
			}
			else {
				pred->unlock(); curr->unlock();
				continue;
			}
		}
	}
	bool Contains(int key)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;

			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			std::lock_guard <std::mutex> pl{ pred->sm };
			std::lock_guard <std::mutex> cl{ curr->sm };

			if (curr->key == key) {
				return true;
			}
			else {
				return false;
			}
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

class LLIST {
	NODE* head, * tail;
public:
	LLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~LLIST()
	{
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next != tail) {
			auto ptr = head->next;
			head->next = head->next->next;
			delete ptr;
		}
	}

	bool validate(NODE* pred, NODE* curr)
	{
		return (pred->removed == false) && (curr->removed == false)
			&& (pred->next == curr);
	}

	bool Add(int key)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (true == validate(pred, curr)) {
				if (curr->key == key) {
					pred->unlock(); curr->unlock();
					return false;
				}
				else {
					auto n = new NODE{ key };
					n->next = curr;
					pred->next = n;
					pred->unlock(); curr->unlock();
					return true;
				}
			}
			else {
				pred->unlock(); curr->unlock();
			}
		}
	}
	bool Remove(int key)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (true == validate(pred, curr)) {
				if (curr->key == key) {
					auto n = curr;
					curr->removed = true;
					pred->next = n->next;
					pred->unlock(); curr->unlock();
					//delete n;
					return true;
				}
				else {
					pred->unlock(); curr->unlock();
					return false;
				}
			}
			else {
				pred->unlock(); curr->unlock();
				continue;
			}
		}
	}
	bool Contains(int key)
	{
		NODE* curr = head;
		while (curr->key < key) {
			curr = curr->next;
		}
		return (curr->removed == false) && (curr->key == key);
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

struct NODE_SP {
	int key;
	std::shared_ptr<NODE_SP> next{ nullptr };
	std::mutex sm;
	volatile bool removed{ false };
	NODE_SP() : key(-1) {}
	NODE_SP(int x) : key(x) {}
	void lock()
	{
		sm.lock();
	}
	void unlock()
	{
		sm.unlock();
	}
};

class LLIST_SP {
	std::shared_ptr<NODE_SP> head, tail;
public:
	LLIST_SP()
	{
		head = std::make_shared<NODE_SP>(std::numeric_limits<int>::min());
		tail = std::make_shared<NODE_SP>(std::numeric_limits<int>::max());
		head->next = tail;
	}
	~LLIST_SP()
	{
	}

	void clear()
	{
		head->next = tail;
	}

	bool validate(const std::shared_ptr<NODE_SP> &pred, const std::shared_ptr<NODE_SP> &curr)
	{
		return (pred->removed == false) && (curr->removed == false)
			&& (pred->next == curr);
	}

	bool Add(int key)
	{
		while (true) {
			std::shared_ptr<NODE_SP> pred = head;
			std::shared_ptr<NODE_SP> curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (true == validate(pred, curr)) {
				if (curr->key == key) {
					pred->unlock(); curr->unlock();
					return false;
				}
				else {
					auto n = std::make_shared<NODE_SP>( key );
					n->next = curr;
					pred->next = n;
					pred->unlock(); curr->unlock();
					return true;
				}
			}
			else {
				pred->unlock(); curr->unlock();
			}
		}
	}
	bool Remove(int key)
	{
		while (true) {
			std::shared_ptr<NODE_SP> pred = head;
			std::shared_ptr<NODE_SP> curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (true == validate(pred, curr)) {
				if (curr->key == key) {
					auto n = curr;
					curr->removed = true;
					pred->next = n->next;
					pred->unlock(); curr->unlock();
					//delete n;
					return true;
				}
				else {
					pred->unlock(); curr->unlock();
					return false;
				}
			}
			else {
				pred->unlock(); curr->unlock();
				continue;
			}
		}
	}
	bool Contains(int key)
	{
		std::shared_ptr<NODE_SP> curr = head;
		while (curr->key < key) {
			curr = curr->next;
		}
		return (curr->removed == false) && (curr->key == key);
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

//class LLIST_ASP {
//	std::shared_ptr<NODE_SP> head, tail;
//public:
//	LLIST_ASP()
//	{
//		head = std::make_shared<NODE_SP>(std::numeric_limits<int>::min());
//		tail = std::make_shared<NODE_SP>(std::numeric_limits<int>::max());
//		head->next = tail;
//	}
//	~LLIST_ASP()
//	{
//	}
//
//	void clear()
//	{
//		head->next = tail;
//	}
//
//	bool validate(const std::shared_ptr<NODE_SP> &pred, const std::shared_ptr<NODE_SP> &curr)
//	{
//		return (pred->removed == false) && (curr->removed == false)
//			&& (std::atomic_load(&pred->next) == curr);
//	}
//
//	bool Add(int key)
//	{
//		while (true) {
//			std::shared_ptr<NODE_SP> pred = head;
//			std::shared_ptr<NODE_SP> curr = std::atomic_load(&pred->next);
//			while (curr->key < key) {
//				pred = curr;
//				curr = std::atomic_load(&curr->next);
//			}
//
//			pred->lock(); curr->lock();
//			if (true == validate(pred, curr)) {
//				if (curr->key == key) {
//					pred->unlock(); curr->unlock();
//					return false;
//				}
//				else {
//					auto n = std::make_shared<NODE_SP>(key);
//					n->next = curr;
//					std::atomic_exchange(&pred->next, n);
//					pred->unlock(); curr->unlock();
//					return true;
//				}
//			}
//			else {
//				pred->unlock(); curr->unlock();
//			}
//		}
//	}
//	bool Remove(int key)
//	{
//		while (true) {
//			std::shared_ptr<NODE_SP> pred = head;
//			std::shared_ptr<NODE_SP> curr = std::atomic_load(&pred->next);
//			while (curr->key < key) {
//				pred = curr;
//				curr = std::atomic_load(&curr->next);
//			}
//
//			pred->lock(); curr->lock();
//			if (true == validate(pred, curr)) {
//				if (curr->key == key) {
//					curr->removed = true;
//					std::atomic_exchange(&pred->next, std::atomic_load(&curr->next));
//					pred->unlock(); curr->unlock();
//					//delete n;
//					return true;
//				}
//				else {
//					pred->unlock(); curr->unlock();
//					return false;
//				}
//			}
//			else {
//				pred->unlock(); curr->unlock();
//				continue;
//			}
//		}
//	}
//	bool Contains(int key)
//	{
//		std::shared_ptr<NODE_SP> curr = head;
//		while (curr->key < key) {
//			curr = std::atomic_load(&curr->next);
//		}
//		return (curr->removed == false) && (curr->key == key);
//	}
//	void print20()
//	{
//		auto p = head->next;
//
//		for (int i = 0; i < 20; ++i) {
//			if (tail == p) break;
//			std::cout << p->key << ", ";
//			p = p->next;
//		}
//		std::cout << std::endl;
//	}
//};
struct NODE_ASP20 {
	int key;
	std::atomic<std::shared_ptr<NODE_ASP20>> next{ nullptr };
	std::mutex sm;
	volatile bool removed{ false };
	NODE_ASP20() : key(-1) {}
	NODE_ASP20(int x) : key(x) {}
	void lock()
	{
		sm.lock();
	}
	void unlock()
	{
		sm.unlock();
	}
};

class LLIST_ASP20 {
	std::shared_ptr<NODE_ASP20> head, tail;
public:
	LLIST_ASP20()
	{
		head = std::make_shared<NODE_ASP20>(std::numeric_limits<int>::min());
		tail = std::make_shared<NODE_ASP20>(std::numeric_limits<int>::max());
		head->next = tail;
	}
	~LLIST_ASP20()
	{
	}

	void clear()
	{
		head->next = tail;
	}

	bool validate(const std::shared_ptr<NODE_ASP20>& pred, const std::shared_ptr<NODE_ASP20>& curr)
	{
		return (pred->removed == false) && (curr->removed == false)
			&& (pred->next.load() == curr);
	}

	bool Add(int key)
	{
		while (true) {
			std::shared_ptr<NODE_ASP20> pred = head;
			std::shared_ptr<NODE_ASP20> curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (true == validate(pred, curr)) {
				if (curr->key == key) {
					pred->unlock(); curr->unlock();
					return false;
				}
				else {
					auto n = std::make_shared<NODE_ASP20>(key);
					n->next = curr;
					pred->next = n;
					pred->unlock(); curr->unlock();
					return true;
				}
			}
			else {
				pred->unlock(); curr->unlock();
			}
		}
	}
	bool Remove(int key)
	{
		while (true) {
			std::shared_ptr<NODE_ASP20> pred = head;
			std::shared_ptr<NODE_ASP20> curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (true == validate(pred, curr)) {
				if (curr->key == key) {
					curr->removed = true;
					pred->next = curr->next.load();
					pred->unlock(); curr->unlock();
					//delete n;
					return true;
				}
				else {
					pred->unlock(); curr->unlock();
					return false;
				}
			}
			else {
				pred->unlock(); curr->unlock();
				continue;
			}
		}
	}
	bool Contains(int key)
	{
		std::shared_ptr<NODE_ASP20> curr = head;
		while (curr->key < key) {
			curr = curr->next;
		}
		return (curr->removed == false) && (curr->key == key);
	}
	void print20()
	{
		std::shared_ptr<NODE_ASP20> p = head->next;

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next;
		}
		std::cout << std::endl;
	}
};

struct LFNODE;

class AMR
{
	std::atomic_llong data;
public:
	AMR() : data(0) {}
	~AMR() {}

	bool get_mark()
	{
		return (data & 1) == 1;
	}
	LFNODE* get_ptr()
	{
		long long temp = data & 0xFFFFFFFFFFFFFFFE;
		return reinterpret_cast<LFNODE *>(temp);
	}

	LFNODE* get_ptr(bool *is_removed)
	{
		long long temp = data;
		*is_removed = (temp & 1 ) == 1;
		return reinterpret_cast<LFNODE*>(temp & 0xFFFFFFFFFFFFFFFE);
	}
	void set_ptr(LFNODE* p)
	{
		long long temp = reinterpret_cast<long long>(p);
		temp = temp & 0xFFFFFFFFFFFFFFFE;
		data = temp;
	}
	bool CAS(LFNODE* old_p, LFNODE* new_p, bool old_m, bool new_m)
	{
		long long old_value = reinterpret_cast<long long>(old_p);
		long long new_value = reinterpret_cast<long long>(new_p);
		if (true == old_m)
			old_value = old_value | 1;
		else
			old_value = old_value & 0xFFFFFFFFFFFFFFFE;
		if (true == new_m) 
			new_value = new_value | 1;
		else
			new_value = new_value & 0xFFFFFFFFFFFFFFFE;

		return std::atomic_compare_exchange_strong(&data, &old_value, new_value);
	}
};

struct LFNODE {
	int key;
	AMR next;
	int ebr_counter;
	LFNODE() : key(-1) {  }
	LFNODE(int x) : key(x) {}
};

class LFLIST {
	LFNODE* head, * tail;
public:
	LFLIST()
	{
		head = new LFNODE{ std::numeric_limits<int>::min() };
		tail = new LFNODE{ std::numeric_limits<int>::max() };
		head->next.set_ptr(tail);
	}
	~LFLIST()
	{
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next.get_ptr() != tail) {
			auto ptr = head->next.get_ptr();
			head->next.set_ptr(head->next.get_ptr()->next.get_ptr());
			delete ptr;
		}
	}

	void Find(LFNODE*& pred, LFNODE*& curr, int x)
	{
	retry:
		pred = head;
		while (true) {
			curr = pred->next.get_ptr();
			bool is_removed = false;
			LFNODE* succ = curr->next.get_ptr(&is_removed);
			while (true == is_removed) {
				if (false == pred->next.CAS(curr, succ, false, false))
					goto retry;
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
			LFNODE* pred = head;
			LFNODE* curr = pred->next.get_ptr();

			Find(pred, curr, key);

			if (curr->key == key)
				return false;
			else {
				auto n = new LFNODE{ key };
				n->next.set_ptr(curr);
				if (true == pred->next.CAS(curr, n, false, false))
					return true;
			}
		}
	}
	bool Remove(int key)
	{
		while (true) {
			LFNODE* pred, * curr;
			Find(pred, curr, key);
			if (curr->key == key) {
				LFNODE* succ = curr->next.get_ptr();
				if (false == curr->next.CAS(succ, succ, false, true))
					continue;
				pred->next.CAS(curr, succ, false, false);
				return true;
			}
			else return false;
		}
	}
	bool Contains(int key)
	{
		LFNODE* curr = head;
		while (curr->key < key) {
			curr = curr->next.get_ptr();
		}
		return (curr->next.get_mark() == false) && (curr->key == key);
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


constexpr int MAX_THREADS = 16;

std::atomic<int> g_ebr_counter = 0;
volatile int num_threads = 1;

thread_local std::queue<LFNODE*> free_list;
thread_local int thread_id;
std::atomic_int thread_ebr[MAX_THREADS * 16];

LFNODE* ebr_new(int x)
{
	if (free_list.empty()) return new LFNODE(x);
	LFNODE* p = free_list.front();

	int ebr_counter = p->ebr_counter;

	for (int i = 0; i < num_threads; ++i)
		if (thread_ebr[i * 16] < ebr_counter) {
			return new LFNODE(x);
		}
	free_list.pop();
	p->key = x;
	p->next.set_ptr(nullptr);
	p->ebr_counter = 0;
	return p;
}

void ebr_delete(LFNODE *p)
{
	p->ebr_counter = g_ebr_counter;
	free_list.push(p);
}

class EBR_LFLIST {
	LFNODE* head, * tail;
public:
	EBR_LFLIST()
	{
		head = new LFNODE{ std::numeric_limits<int>::min() };
		tail = new LFNODE{ std::numeric_limits<int>::max() };
		head->next.set_ptr(tail);
	}
	~EBR_LFLIST()
	{
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next.get_ptr() != tail) {
			auto ptr = head->next.get_ptr();
			head->next.set_ptr(head->next.get_ptr()->next.get_ptr());
			delete ptr;
		}
	}

	void Find(LFNODE*& pred, LFNODE*& curr, int x)
	{
	retry:
		pred = head;
		while (true) {
			curr = pred->next.get_ptr();
			bool is_removed = false;
			LFNODE* succ = curr->next.get_ptr(&is_removed);
			while (true == is_removed) {
				if (false == pred->next.CAS(curr, succ, false, false))
					goto retry;
				ebr_delete(curr);
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
		thread_ebr[thread_id * 16] = ++g_ebr_counter;
		while (true) {
			LFNODE* pred = head;
			LFNODE* curr = pred->next.get_ptr();

			Find(pred, curr, key);

			if (curr->key == key) {
				thread_ebr[thread_id * 16] = std::numeric_limits<int>::max();
				return false;
			}
			else {
				auto n = ebr_new(key);
				n->next.set_ptr(curr);
				if (true == pred->next.CAS(curr, n, false, false)) {
					thread_ebr[thread_id * 16] = std::numeric_limits<int>::max();
					return true;
				}
			}
		}
	}
	bool Remove(int key)
	{
		while (true) {
			LFNODE* pred, * curr;
			Find(pred, curr, key);
			if (curr->key == key) {
				LFNODE* succ = curr->next.get_ptr();
				if (false == curr->next.CAS(succ, succ, false, true))
					continue;
				if (true == pred->next.CAS(curr, succ, false, false))
					ebr_delete(curr);
				return true;
			}
			else return false;
		}
	}
	bool Contains(int key)
	{
		LFNODE* curr = head;
		while (curr->key < key) {
			curr = curr->next.get_ptr();
		}
		return (curr->next.get_mark() == false) && (curr->key == key);
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

class STD_SET {
	std::set <int> m_set;
	std::mutex set_lock;
public:
	STD_SET()
	{
	}
	void clear()
	{
		m_set.clear();
	}
	bool Add(int x)
	{
		//std::lock_guard<std::mutex> aa { set_lock };
		return m_set.insert(x).second;
	}
	bool Remove(int x)
	{
		//std::lock_guard<std::mutex> aa{ set_lock };
		if (0 != m_set.count(x)) {
			m_set.erase(x);
			return true;
		}
		else return false;
	}
	bool Contains(int x)
	{
		//std::lock_guard<std::mutex> aa{ set_lock };
		return 0 != m_set.count(x);

	}
	void print20()
	{
		int count = 20;
		for (auto x : m_set) {
			if (count-- == 0) break;
			std::cout << x << ", ";
		}
		std::cout << std::endl;
	}
};

struct RESPONSE {
	bool m_bool;
};

enum METHOD { M_ADD, M_REMOVE, M_CONTAINS, M_CLEAR, M_PRINT20 };

struct INVOCATION {
	METHOD m_method;
	int x;
};

class SEQOBJECT {
	std::set <int> m_set;
public:
	SEQOBJECT()
	{
	}
	void clear()
	{
		m_set.clear();
	}
	RESPONSE apply(INVOCATION& inv)
	{
		RESPONSE r{ true };
		switch (inv.m_method) {
		case M_ADD:
			r.m_bool = m_set.insert(inv.x).second;
			break;
		case M_REMOVE:
			r.m_bool = (0 != m_set.count(inv.x));
			if (r.m_bool == true)
				m_set.erase(inv.x);
			break;
		case M_CONTAINS:
			r.m_bool = (0 != m_set.count(inv.x));
			break;
		case M_CLEAR:
			m_set.clear();
			break;
		case M_PRINT20: {
			int count = 20;
			for (auto x : m_set) {
				if (count-- == 0) break;
				std::cout << x << ", ";
			}
			std::cout << std::endl;
		}
					  break;
		}
		return r;
	}
};

class STD_SEQ_SET {
	SEQOBJECT m_set;
public:
	STD_SEQ_SET()
	{
	}
	void clear()
	{
		INVOCATION inv{ M_CLEAR, 0 };
		m_set.apply(inv);
	}
	bool Add(int x)
	{
		INVOCATION inv{ M_ADD, x };
		return m_set.apply(inv).m_bool;
	}
	bool Remove(int x)
	{
		INVOCATION inv{ M_REMOVE, x };
		return m_set.apply(inv).m_bool;
	}
	bool Contains(int x)
	{
		INVOCATION inv{ M_CONTAINS, x };
		return m_set.apply(inv).m_bool;

	}
	void print20()
	{
		INVOCATION inv{ M_PRINT20, 0 };
		m_set.apply(inv);
	}
};

class U_NODE {
public:
	INVOCATION m_inv;
	int m_seq;
	U_NODE* volatile next;
};

class LFUNV_OBJECT {
	U_NODE* volatile m_head[MAX_THREADS];
	U_NODE tail;
	U_NODE* get_max_head()
	{
		U_NODE* h = m_head[0];
		for (int i = 1; i < MAX_THREADS; ++i)
			if (h->m_seq < m_head[i]->m_seq)
				h = m_head[i];
		return h;
	}
public:
	LFUNV_OBJECT() {
		tail.m_seq = 0;
		tail.next = nullptr;
		for (auto& h : m_head) h = &tail;
	}
	void clear()
	{
		U_NODE* p = tail.next;
		while (nullptr != p) {
			U_NODE* old_p = p;
			p = p->next;
			delete old_p;
		}
		tail.next = nullptr;
		for (auto& h : m_head) h = &tail;
	}

	void print20()
	{
		SEQOBJECT std_set;
		U_NODE* p = tail.next;
		while (p != nullptr) {
			std_set.apply(p->m_inv);
			p = p->next;
		}
		INVOCATION inv{ M_PRINT20, 0 };
		std_set.apply(inv);
	}

	RESPONSE apply(INVOCATION& inv)
	{
		U_NODE* prefer = new U_NODE{ inv, 0, nullptr };
		while (0 == prefer->m_seq) {
			U_NODE* head = get_max_head();
			long long temp = 0;
			std::atomic_compare_exchange_strong(
				reinterpret_cast<volatile std::atomic_llong*>(&head->next),
				&temp,
				reinterpret_cast<long long>(prefer));
			U_NODE* after = head->next;
			after->m_seq = head->m_seq + 1;
			m_head[thread_id] = after;
		}

		SEQOBJECT std_set;
		U_NODE* p = tail.next;
		while (p != prefer) {
			std_set.apply(p->m_inv);
			p = p->next;
		}
		return std_set.apply(inv);
	}
};

class STD_LF_SET {
	LFUNV_OBJECT m_set;
public:
	STD_LF_SET()
	{
	}
	void clear()
	{
		//INVOCATION inv{ M_CLEAR, 0 };
		//m_set.apply(inv);
		m_set.clear();
	}
	bool Add(int x)
	{
		INVOCATION inv{ M_ADD, x };
		return m_set.apply(inv).m_bool;
	}
	bool Remove(int x)
	{
		INVOCATION inv{ M_REMOVE, x };
		return m_set.apply(inv).m_bool;
	}
	bool Contains(int x)
	{
		INVOCATION inv{ M_CONTAINS, x };
		return m_set.apply(inv).m_bool;

	}
	void print20()
	{
		//INVOCATION inv{ M_PRINT20, 0 };
		//m_set.apply(inv);
		m_set.print20();
	}
};

constexpr int MAX_LEVEL = 9;

struct SKNODE {
	int key;
	SKNODE* next[MAX_LEVEL + 1];
	int top_level;
	std::mutex nl;
	volatile bool removed;
	volatile bool fullylinked;
	SKNODE() : key(-1), top_level(0), removed (false), fullylinked (false)
	{ 
		for (auto &n : next) 
			n = nullptr; 
	}
	SKNODE(int x, int lv) : key(x), top_level(lv), removed (false), fullylinked(false)
	{
		for (auto& n : next)
			n = nullptr;
	}
	void lock()
	{
		nl.lock();
	}
	void unlock()
	{
		nl.unlock();
	}
};

class CSKLIST {
	SKNODE * head, * tail;
	//std::null_mutex	sm;
public:
	CSKLIST()
	{
		head = new SKNODE{ std::numeric_limits<int>::min(), MAX_LEVEL };
		tail = new SKNODE{ std::numeric_limits<int>::max(), MAX_LEVEL };
		for (auto& n : head->next) n = tail;
	}
	~CSKLIST() {
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next[0] != tail) {
			auto ptr = head->next[0];
			head->next[0] = head->next[0]->next[0];
			delete ptr;
		}
		for (auto& n : head->next) n = tail;
	}

	void Find(int key, SKNODE *pred[], SKNODE *curr[])
	{
		for (int i = MAX_LEVEL; i >= 0; --i) {
			if (i == MAX_LEVEL)
				pred[i] = head;
			else
				pred[i] = pred[i + 1];
			curr[i] = pred[i]->next[i];
			while (curr[i]->key < key) {
				pred[i] = curr[i];
				curr[i] = curr[i]->next[i];
			}
		}
	}

	bool Add(int key)
	{
		SKNODE* pred[MAX_LEVEL + 1], * curr[MAX_LEVEL + 1];
		//sm.lock();
		Find(key, pred, curr);

		if (curr[0]->key == key) {
			//sm.unlock();
			return false;
		}
		else {
			int top_level = 0;
			for (int i = 0; i < MAX_LEVEL; ++i) {
				if (rand() % 2 == 0) break;
				top_level++;
			}
			auto n = new SKNODE{ key, top_level };
			for (int i = 0; i <= top_level; ++i) {
				pred[i]->next[i] = n;
				n->next[i] = curr[i];
			}
			//sm.unlock();
			return true;
		}
	}
	bool Remove(int key)
	{
		SKNODE* pred[MAX_LEVEL + 1], *curr[MAX_LEVEL + 1];
		//sm.lock();
		Find(key, pred, curr);
		
		if (curr[0]->key == key) {
			auto n = curr[0];
			for (int i = 0; curr[0]->top_level >= i; ++i)
				pred[i]->next[i] = curr[i]->next[i];
			//sm.unlock();
			delete n;
			return true;
		}
		else {
			//sm.unlock();
			return false;
		}
	}
	bool Contains(int key)
	{
		SKNODE* pred[MAX_LEVEL + 1], * curr[MAX_LEVEL + 1];
		//sm.lock();
		Find(key, pred, curr);

		if (curr[0]->key == key) {
			//sm.unlock();
			return true;
		}
		else {
			//sm.unlock();
			return false;
		}
	}
	void print20()
	{
		auto p = head->next[0];

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next[0];
		}
		std::cout << std::endl;
	}
};

class LSKLIST {
	SKNODE* head, * tail;
public:
	LSKLIST()
	{
		head = new SKNODE{ std::numeric_limits<int>::min(), MAX_LEVEL };
		tail = new SKNODE{ std::numeric_limits<int>::max(), MAX_LEVEL };
		for (auto& n : head->next) n = tail;
	}
	~LSKLIST() {
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next[0] != tail) {
			auto ptr = head->next[0];
			head->next[0] = head->next[0]->next[0];
			delete ptr;
		}
		for (auto& n : head->next) n = tail;
	}

	int Find(int key, SKNODE* pred[], SKNODE* curr[])
	{
		int found_level = -1;
		for (int i = MAX_LEVEL; i >= 0; --i) {
			if (i == MAX_LEVEL)
				pred[i] = head;
			else
				pred[i] = pred[i + 1];
			curr[i] = pred[i]->next[i];
			while (curr[i]->key < key) {
				pred[i] = curr[i];
				curr[i] = curr[i]->next[i];
			}
			if (curr[i]->key == key && found_level == -1)
				found_level = i;
		}
		return found_level;
	}

	bool Add(int key)
	{
		SKNODE* pred[MAX_LEVEL + 1], * curr[MAX_LEVEL + 1];
		while (true) {
			int found_level = Find(key, pred, curr);

			if (-1 != found_level) {
				while (curr[found_level]->fullylinked == false);
				return false;
			}

			int top_level = curr[found_level]->top_level;
			bool valid = true;

			for (int level = 0; level <= top_level; ++level) {
				if (false == valid) break;
				pred[level]->lock();
				if (pred[level]->removed == true || pred[level]->next[level] != curr[found_level] || curr[found_level]->removed == true) {
					valid = false;
					break;
				}

				if (false == valid) {
					//unlock();;
					continue;
				}

			}

			if (curr[0]->key == key) {
				//sm.unlock();
				return false;
			}
			else {
				int top_level = 0;
				for (int i = 0; i < MAX_LEVEL; ++i) {
					if (rand() % 2 == 0) break;
					top_level++;
				}
				auto n = new SKNODE{ key, top_level };
				for (int i = 0; i <= top_level; ++i) {
					pred[i]->next[i] = n;
					n->next[i] = curr[i];
				}
				//sm.unlock();
				return true;
			}
		}
	}
	bool Remove(int key)
	{
		SKNODE* pred[MAX_LEVEL + 1], * curr[MAX_LEVEL + 1];
		//sm.lock();
		Find(key, pred, curr);

		if (curr[0]->key == key) {
			auto n = curr[0];
			for (int i = 0; curr[0]->top_level >= i; ++i)
				pred[i]->next[i] = curr[i]->next[i];
			//sm.unlock();
			delete n;
			return true;
		}
		else {
			//sm.unlock();
			return false;
		}
	}
	bool Contains(int key)
	{
		SKNODE* pred[MAX_LEVEL + 1], * curr[MAX_LEVEL + 1];
		//sm.lock();
		Find(key, pred, curr);

		if (curr[0]->key == key) {
			//sm.unlock();
			return true;
		}
		else {
			//sm.unlock();
			return false;
		}
	}
	void print20()
	{
		auto p = head->next[0];

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next[0];
		}
		std::cout << std::endl;
	}
};

struct LFSKNODE;

class SKAMR
{
	std::atomic_llong data;
public:
	SKAMR() : data(0) {}
	~SKAMR() {}

	bool get_mark()
	{
		return (data & 1) == 1;
	}
	LFSKNODE* get_ptr()
	{
		long long temp = data & 0xFFFFFFFFFFFFFFFE;
		return reinterpret_cast<LFSKNODE*>(temp);
	}

	LFSKNODE* get_ptr(bool* is_removed)
	{
		long long temp = data;
		*is_removed = (temp & 1) == 1;
		return reinterpret_cast<LFSKNODE*>(temp & 0xFFFFFFFFFFFFFFFE);
	}
	void set_ptr(LFSKNODE* p)
	{
		long long temp = reinterpret_cast<long long>(p);
		temp = temp & 0xFFFFFFFFFFFFFFFE;
		data = temp;
	}
	bool CAS(LFSKNODE* old_p, LFSKNODE* new_p, bool old_m, bool new_m)
	{
		long long old_value = reinterpret_cast<long long>(old_p);
		long long new_value = reinterpret_cast<long long>(new_p);
		if (true == old_m)
			old_value = old_value | 1;
		else
			old_value = old_value & 0xFFFFFFFFFFFFFFFE;
		if (true == new_m)
			new_value = new_value | 1;
		else
			new_value = new_value & 0xFFFFFFFFFFFFFFFE;

		return std::atomic_compare_exchange_strong(&data, &old_value, new_value);
	}
};

struct LFSKNODE {
	int key;
	SKAMR next[MAX_LEVEL + 1];
	int top_level;
	int ebr_counter;
	LFSKNODE() : key(-1), top_level(MAX_LEVEL) {}
	LFSKNODE(int x, int tl) : key(x), top_level(tl) {}
};

class LFSKLIST {
	LFSKNODE* head, * tail;
public:
	LFSKLIST()
	{
		head = new LFSKNODE{ std::numeric_limits<int>::min(), MAX_LEVEL };
		tail = new LFSKNODE{ std::numeric_limits<int>::max(), MAX_LEVEL };
		for (auto& n : head->next) n.set_ptr(tail);
	}
	~LFSKLIST() {
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next[0].get_ptr() != tail) {
			auto ptr = head->next[0].get_ptr();
			head->next[0].set_ptr(ptr->next[0].get_ptr());
			delete ptr;
		}
		for (auto& n : head->next) n.set_ptr(tail);
	}

	bool Find(int key, LFSKNODE* pred[], LFSKNODE* curr[])
	{
	retry:
		for (int i = MAX_LEVEL; i >= 0; --i) {
			if (i == MAX_LEVEL)
				pred[i] = head;
			else
				pred[i] = pred[i + 1];
			curr[i] = pred[i]->next[i].get_ptr();

			while (true) {
				bool removed = false;
				LFSKNODE* succ = curr[i]->next[i].get_ptr(&removed);
				while (true == removed) {
					if (false == pred[i]->next[i].CAS(curr[i], succ, false, false))
						goto retry;
					curr[i] = succ;
					succ = curr[i]->next[i].get_ptr(&removed);
				}
				if (curr[i]->key >= key) break;
				pred[i] = curr[i];
				curr[i] = succ;
			}
		}
		return curr[0]->key == key;
	}

	bool Add(int key)
	{
		LFSKNODE* pred[MAX_LEVEL + 1], * curr[MAX_LEVEL + 1];

		while (true) {
			if (true == Find(key, pred, curr))
				return false;

			int top_level = 0;
			for (int i = 0; i < MAX_LEVEL; ++i) {
				if (rand() % 2 == 0) break;
				top_level++;
			}
			auto n = new LFSKNODE{ key, top_level };
			for (int i = 0; i <= top_level; ++i)
				n->next[i].set_ptr(curr[i]);

			if (false == pred[0]->next[0].CAS(curr[0], n, false, false))
				continue;

			for (int i = 1; i <= top_level; ++i) {
				while (false == pred[i]->next[i].CAS(curr[i], n, false, false)) {
					Find(key, pred, curr);
				}
			}
			return true;
		}
	}
	bool Remove(int key)
	{
		LFSKNODE* pred[MAX_LEVEL + 1], * curr[MAX_LEVEL + 1];

		if (false == Find(key, pred, curr)) return false;

		LFSKNODE* victim = curr[0];
		int top_level = victim->top_level;
		for (int i = top_level; i > 0; --i) {
			while (false == victim->next[i].CAS(curr[i], curr[i], true, false)) {
				if (victim->next[i].get_mark() == true)
					break;
				if (false == Find(key, pred, curr)) return false;
			}
		}

		// 숙제
	}
	bool Contains(int key)
	{
		LFSKNODE* pred, *curr;
		pred = head;
		curr = pred->next[MAX_LEVEL].get_ptr();
		for (int i = MAX_LEVEL; i >= 0; --i) {
			while (true) {
				bool removed = false;
				LFSKNODE* succ = curr->next[i].get_ptr(&removed);
				while (true == removed) {
					curr = succ;
					succ = curr->next[i].get_ptr(&removed);
				}
				if (curr->key >= key) break;
				curr = succ;
				pred = curr;
			}
		}
		return curr->key == key;
	}

	void print20()
	{
		auto p = head->next[0].get_ptr();

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next[0].get_ptr();
		}
		std::cout << std::endl;
	}
};


constexpr int NUM_TEST = 4000000;
constexpr int KEY_RANGE = 1000;

LFSKLIST g_set;

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
	thread_id = th_id;
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

void benchmark(int num_thread, int th_id)
{
	thread_id = th_id;
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
	
	while(false)
	 // 알고리즘 정확성 검사
	{
		for (int i = 1; i <= 16; i = i * 2) {
			num_threads = i;
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
		for (int i = 1; i <= 16; i = i) {
			num_threads = i;
			std::vector <std::thread> threads;
			g_set.clear();
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j)
				threads.emplace_back(benchmark, i, j);
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


#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <array>
#include <queue>


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
	int ebr_counter;
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


// FROM HERE

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
thread_local std::queue<LFNODE*> free_list;
thread_local std::queue<NODE*> free_list_lazy;
thread_local int thread_id;
std::atomic_int thread_ebr[MAX_THREADS * 16];
int g_num_threads;



LFNODE* ebr_new(int x)
{
	if (free_list.empty()) return new LFNODE(x);

	LFNODE* p = free_list.front();

	int ebr_counter = p->ebr_counter;

	for (int i = 0; i < g_num_threads; ++i)
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
		thread_ebr[thread_id * 16] = ++g_ebr_counter;
		while (true) {
			LFNODE* pred, * curr;
			Find(pred, curr, key);
			if (curr->key == key) {
				LFNODE* succ = curr->next.get_ptr();
				if (false == curr->next.CAS(succ, succ, false, true))
					continue;
				if (true == pred->next.CAS(curr, succ, false, false))
					ebr_delete(curr);

				thread_ebr[thread_id * 16] = std::numeric_limits<int>::max();
				return true;
			}
			else {
				thread_ebr[thread_id * 16] = std::numeric_limits<int>::max();
				return false;
			}
		}
	}
	bool Contains(int key)
	{
		thread_ebr[thread_id * 16] = ++g_ebr_counter;
		LFNODE* curr = head;
		while (curr->key < key) {
			curr = curr->next.get_ptr();
		}

		bool ret{ (curr->next.get_mark() == false) && (curr->key == key) };
		thread_ebr[thread_id * 16] = std::numeric_limits<int>::max();
		return ret;
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


// EBR LAZY

NODE* ebr_new_lazy(int x)
{
	if (free_list_lazy.empty()) return new NODE(x);

	NODE* p = free_list_lazy.front();

	int ebr_counter = p->ebr_counter;

	for (int i = 0; i < g_num_threads; ++i)
		if (thread_ebr[i * 16] < ebr_counter) {
			return new NODE(x);
		}

	free_list_lazy.pop();
	p->key = x;
	p->next = nullptr;
	p->ebr_counter = 0;
	p->removed = false;
	return p;
}

void ebr_delete(NODE* p)
{
	p->ebr_counter = g_ebr_counter;
	free_list_lazy.push(p);
}

class EBR_LLIST {
	NODE* head, *tail;
public:
	EBR_LLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~EBR_LLIST()
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
		thread_ebr[thread_id * 16] = ++g_ebr_counter;
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
					thread_ebr[thread_id * 16] = std::numeric_limits<int>::max();
					return false;
				}
				else {
					auto n{ ebr_new_lazy(key) };
					n->next = curr;
					pred->next = n;
					pred->unlock(); curr->unlock();
					thread_ebr[thread_id * 16] = std::numeric_limits<int>::max();
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
		thread_ebr[thread_id * 16] = ++g_ebr_counter;
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
					ebr_delete(n);
					thread_ebr[thread_id * 16] = std::numeric_limits<int>::max();
					return true;
				}
				else {
					pred->unlock(); curr->unlock();
					thread_ebr[thread_id * 16] = std::numeric_limits<int>::max();
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
		thread_ebr[thread_id * 16] = ++g_ebr_counter;
		NODE* curr = head;
		while (curr->key < key) {
			curr = curr->next;
		}

		bool ret{ (curr->removed == false) && (curr->key == key) };
		thread_ebr[thread_id * 16] = std::numeric_limits<int>::max();
		return ret;
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
constexpr int KEY_RANGE = 100;


// LLIST / LFLIST / EBR_LLIST / EBR_LFLIST
EBR_LFLIST g_set;


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
	{
		auto start_t = system_clock::now();
		g_num_threads = 1;
		benchmark(1, 0);
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
			g_num_threads = i;
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
			g_num_threads = i;
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

	getchar();
}


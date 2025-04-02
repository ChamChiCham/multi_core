#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;
constexpr int THREAD = 16;



const auto NUM_TEST = 4000000;
const auto KEY_RANGE = 1000;

class NODE {
public:
	int key{};
	NODE* next{nullptr};

	NODE() = default;

	NODE(int key_value) :
		key{ key_value }
	{}

	~NODE() {}
};


class CLIST {
	NODE head, tail;
	mutex glock;
public:
	CLIST()
	{
		head.key = std::numeric_limits<int>::min();
		tail.key = std::numeric_limits<int>::max();
		head.next = &tail;
	}
	~CLIST() {}
	void Init()
	{
		NODE* ptr;
		while (head.next != &tail) {
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}
	bool Add(int key)
	{
		NODE* new_node = new NODE;
		new_node->key = key;
		NODE* current = &head;

		glock.lock();
		while (current != &tail) {
			if (current->next->key == key) {
				glock.unlock();
				delete new_node;
				return false;
			}
			if (current->next->key < key) {
				current = current->next;
			}
			else {
				new_node->next = current->next;
				current->next = new_node;
				glock.unlock();
				return true;
			}
		}
		glock.unlock();
		return false;
	}
	bool Remove(int key)
	{
		NODE* prev = &head;
		glock.lock();
		NODE* ptr = head.next;
		while (ptr != &tail) {
			if (ptr->key == key) {
				prev->next = ptr->next;
				glock.unlock();
				delete ptr;
				return true;
			}
			if (ptr->key > key) {
				glock.unlock();
				return false;
			}
			prev = ptr;
			ptr = ptr->next;
		}
		glock.unlock();
		return false;
	}
	bool Contains(int key)
	{
		glock.lock();
		NODE* ptr = head.next;
		while (ptr != &tail) {
			if (ptr->key == key) {
				glock.unlock();
				return true;
			}
			if (ptr->key > key) {
				glock.unlock();
				return false;
			}
			ptr = ptr->next;
		}
		glock.unlock();
		return false;
	}

	void print20()
	{
		NODE* ptr = head.next;
		for (int i = 0; i < 20; ++i) {
			if (ptr == &tail) { break; }
			std::cout << ptr->key << ", ";
			ptr = ptr->next;
		}
		std::cout << std::endl;
	}
};


CLIST clist;
void ThreadFunc(int num_thread)
{
	int key;
	for (int i = 0; i < NUM_TEST / num_thread; i++) {
		switch (rand() % 3) {
		case 0: key = rand() % KEY_RANGE;
			clist.Add(key);
			break;
		case 1: key = rand() % KEY_RANGE;
			clist.Remove(key);
			break;
		case 2: key = rand() % KEY_RANGE;
			clist.Contains(key);
			break;
		default: cout << "Error\n";
			exit(-1);
		}
	}
}




int main()
{
	std::cout << "coarse set" << std::endl;
	for (int thread_num = 1; thread_num <= THREAD; thread_num *= 2) {
		clist.Init();
		std::vector<std::thread> threads;
		auto start = chrono::high_resolution_clock::now();
		for (int i = 0; i < thread_num; ++i) {
			threads.emplace_back(ThreadFunc, thread_num);
		}
		for (auto& th : threads) {
			th.join();
		}
		auto end = chrono::high_resolution_clock::now();

		auto diff = chrono::duration_cast<chrono::milliseconds>(end - start).count();
		std::cout << "threads " << thread_num << ": time: " << diff << "ms" << std::endl;
		clist.print20();
	}

}

#include <thread>
#include <iostream>



bool g_ready = false;
int g_data = 0;

void Receiver()
{
	while (false == g_ready) ;
	std::cout << "I got " << g_data << std::endl;
}

void Sender()
{
	std::cin >> g_data;
	g_ready = true;
}

int main()
{
	std::thread t1{ Receiver };
	std::thread t2{ Sender };



	t1.join();
	t2.join();
}

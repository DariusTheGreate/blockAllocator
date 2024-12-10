#include <iostream>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>

#include "BlockAllocator.hpp"

BlockAllocator globalAlloc{256, 1024 * 1024};

std::vector<void*> blocks;
std::mutex blocksListMtx;

void alloc(int)
{
	try{
		while(1){
			std::random_device dev;
		    std::mt19937 rng(dev());
		    std::uniform_int_distribution<std::mt19937::result_type> dist(1,10); 
		    std::this_thread::sleep_for(std::chrono::milliseconds(100 * dist(rng)));
		    
		    std::cout << "allocate\n";
		    std::unique_lock<std::mutex>(blocksListMtx);
			blocks.push_back(globalAlloc.allocate());

			globalAlloc.LogInfo();
		}
	}
	catch(std::exception& e)
	{
		std::cout << e.what() << "\n";
	}
}

void dealloc(int)
{
	try{
		while(1){
			std::random_device dev;
		    std::mt19937 rng(dev());
		    std::uniform_int_distribution<std::mt19937::result_type> dist(1,10); 
		    std::this_thread::sleep_for(std::chrono::milliseconds(1000 * dist(rng)));

		    std::cout << "deallocate\n";
		    std::unique_lock<std::mutex>(blocksListMtx);
			if(blocks.size() > 0)
				globalAlloc.deallocate(*(blocks.end() - 1));

			globalAlloc.LogInfo();
		}
	}
	catch(std::exception& e)
	{
		std::cout << e.what() << "\n";
	}
}

void testMultithreadedRandomized()
{
	std::thread t1{alloc, 0};
	std::thread t2{dealloc, 0};

	std::thread t3{alloc, 0};
	std::thread t4{alloc, 0};
	t1.join();
	t2.join();
	t3.join();
	t4.join();
}

int main()
{
	BlockAllocator alloc{1024, 1024};

	testMultithreadedRandomized();

	return 0;
}
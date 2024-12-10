#include <iostream>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>
#include <cassert>
#include <cstring>

#include "BlockAllocator.hpp"

BlockAllocator globalAlloc{256, 1024 * 1024};

std::vector<void*> blocks;
std::mutex blocksListMtx;


template <typename Arg, typename... Args>
void print(Arg&& arg, Args&&... args)
{
    std::cout << std::forward<Arg>(arg);
    using expander = int[];
    (void)expander{0, (void(std::cout << ' ' << std::forward<Args>(args)), 0)...};
}


void allocWithRandomDelay(int)
{
	try{
		while(1){
			std::random_device dev;
		    std::mt19937 rng(dev());
		    std::uniform_int_distribution<std::mt19937::result_type> dist(1,10); 
		    std::this_thread::sleep_for(std::chrono::milliseconds(100 * dist(rng)));
		    
		    print("allocate\n");
		    std::unique_lock<std::mutex> lck(blocksListMtx);
			blocks.push_back(globalAlloc.allocate());

			globalAlloc.LogInfo();
		}
	}
	catch(std::exception& e)
	{
		std::cout << e.what() << "\n";
	}
}

void deallocWithRandomDelay(int)
{
	try{
		while(1){
			std::random_device dev;
		    std::mt19937 rng(dev());
		    std::uniform_int_distribution<std::mt19937::result_type> dist(1,10); 
		    std::this_thread::sleep_for(std::chrono::milliseconds(1000 * dist(rng)));

		    print("deallocate\n");
		    std::unique_lock<std::mutex> lck(blocksListMtx);
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


void testAllocation()
{
	print("---testAllocation()---\n");
	BlockAllocator alloc{1024, 1024};
	void* data = alloc.allocate();	
	std::memset(data, 1, 1024);

	for(uint32_t i = 0; i < 1024 * 2; ++i)
		alloc.allocate();	

	assert(alloc.GetFreeListSize() == 0);
	assert(alloc.GetUsedMem() == 1024 * 1024);
	assert(alloc.allocate() == nullptr);	

	alloc.LogInfo();
}

void testMove()
{
	print("---testMove()---\n");
	BlockAllocator alloc(1024, 1024);
	BlockAllocator alloc2 = std::move(alloc);

	assert(alloc.GetBlockSize() == 0);
	assert(alloc.GetBlockCount() == 0);
	assert(alloc.GetUsedMem() == 0);
	assert(alloc.GetFreeListSize() == 0);

	assert(alloc2.GetBlockSize() == 1024);
	assert(alloc2.GetBlockCount() == 1024);
	assert(alloc2.GetUsedMem() == 0);
	assert(alloc2.GetFreeListSize() == 1024);

	alloc = std::move(alloc2);

	assert(alloc2.GetBlockSize() == 0);
	assert(alloc2.GetBlockCount() == 0);
	assert(alloc2.GetUsedMem() == 0);
	assert(alloc2.GetFreeListSize() == 0);

	assert(alloc.GetBlockSize() == 1024);
	assert(alloc.GetBlockCount() == 1024);
	assert(alloc.GetUsedMem() == 0);
	assert(alloc.GetFreeListSize() == 1024);
}

void testShuffle()
{
	print("---testShuffle()---\n");
	for(uint32_t i = 0; i < 10000; ++i)
	{
		std::random_device dev;
	    std::mt19937 rng(dev());
	    std::uniform_int_distribution<std::mt19937::result_type> dist(1,2); 
	    if(dist(rng) == 1)
	    {
		    print("allocate\n");
		    std::unique_lock<std::mutex> lck(blocksListMtx);
			blocks.push_back(globalAlloc.allocate());
			globalAlloc.LogInfo();
	    }
	    else
	    {
	    	
		    print("deallocate\n");
		    std::unique_lock<std::mutex> lck(blocksListMtx);
			if(blocks.size() > 0)
				globalAlloc.deallocate(*(blocks.end() - 1));

			globalAlloc.LogInfo();
	    }
	}
}

void testMultithreadedRandomized()
{
	print("---testMultithreadedRandomized()---\n");
	std::thread t1{allocWithRandomDelay, 0};
	std::thread t2{deallocWithRandomDelay, 0};
	std::thread t3{allocWithRandomDelay, 0};
	std::thread t4{deallocWithRandomDelay, 0};
	t1.join();
	t2.join();
	t3.join();
	t4.join();
}


int main()
{
	testAllocation();
	testMove();
	testShuffle();
	testMultithreadedRandomized();
	return 0;
}
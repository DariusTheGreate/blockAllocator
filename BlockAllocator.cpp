#include <thread>
#include <mutex>
#include <random>
#include <chrono>
#include <iostream>

#include "BlockAllocator.hpp"

BlockAllocator::BlockAllocator(uint32_t blockSize, uint32_t blockCount) : _blockSize(blockSize), _blockCount(blockCount)
{
	if(blockSize == 0 || blockCount == 0)
		throw std::runtime_error("BlockAllocator::BlockAllocator(uint32_t, uint32_t) Won't allocate 0 bytes of memory");// NOT COOL

	_dataPtr = malloc(blockSize * blockCount); // CHECK ERRNO MB?

	if(!_dataPtr)
		throw std::runtime_error("BlockAllocator::BlockAllocator(uint32_t, uint32_t) Couldn't allocate memory");// NOT COOL

	for (uint32_t i = 0; i < blockCount; i++)
    {
        const uint64_t chunkAddress = reinterpret_cast<uint64_t>(_dataPtr) + i * blockSize;
        Chunk* newFreeChunk = reinterpret_cast<Chunk*>(chunkAddress);
        newFreeChunk->next = _chunksHead;
        _chunksHead = newFreeChunk;
    }
    _usedMem = 0;
}


BlockAllocator::BlockAllocator(BlockAllocator&& alloc_in) noexcept
{
    _blockSize = alloc_in._blockSize;
    _usedMem = alloc_in._usedMem;
    _dataPtr = alloc_in._dataPtr;
    alloc_in._dataPtr= nullptr;
    alloc_in._usedMem = alloc_in._blockSize = alloc_in._blockCount = 0;
}

BlockAllocator& BlockAllocator::operator = (BlockAllocator&& alloc_in) noexcept
{
    if (this != &alloc_in)
    {
        _blockSize = alloc_in._blockSize;
        _usedMem = alloc_in._usedMem;
        _dataPtr = alloc_in._dataPtr;
        alloc_in._dataPtr = nullptr;
        alloc_in._usedMem = alloc_in._blockSize = alloc_in._blockCount = 0;
    }
    return *this;
}

BlockAllocator::~BlockAllocator()
{
	free(_dataPtr);	
}

void* BlockAllocator::allocate()
{
	std::unique_lock<std::mutex>{_lock};
	if(_blockSize + _usedMem > _blockSize * _blockCount)
		return nullptr;

	auto* curr = _chunksHead;
	auto* next = _chunksHead->next;
	_chunksHead = next;
	_usedMem += _blockSize;

	return curr;
}

void* BlockAllocator::allocate(uint32_t sz_in)// Think about it
{
	std::unique_lock<std::mutex>{_lock};
	if(sz_in % _blockSize != 0)	
		throw std::runtime_error("BlockAllocator::allocate(uint32_t): Trying to allocate size not aligned with blockSize");
	return nullptr;
}

void BlockAllocator::deallocate(void* ptr)
{
	if(_usedMem == 0)
		return;
	std::unique_lock<std::mutex>{_lock};
	auto* newChunk = reinterpret_cast<Chunk*>(ptr); // NULLIFY HERE?
    newChunk->next = _chunksHead;
    _chunksHead = newChunk;
    _usedMem -= _blockSize;
}

void BlockAllocator::LogInfo()
{
	std::cout << "BlockAllocator State: UsedMem - " << _usedMem << "\n"; //"; FreeList size - " << GetFreeListSize() << "\n";
}

uint32_t BlockAllocator::GetFreeListSize()
{
	std::unique_lock<std::mutex>(_lock);
	if(!_chunksHead)
		return 0;

	uint32_t sz = 1;
	auto* it = _chunksHead;
	while(it->next){
		it = it->next;
		sz++;
	}
	return sz;
}

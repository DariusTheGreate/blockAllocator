#include "BlockAllocator.hpp"

#include <iostream>

/*!
	@brief Constructor O(n), where n is blockCount + malloc overhead.
	throws runtime_error in case blockCount + blockSize is zero or malloc failed.

	@param blocksSize Size of one chunk.
	@param blockCount Number of chunks.
*/
BlockAllocator::BlockAllocator(uint32_t blockSize, uint32_t blockCount) : _blockSize(blockSize), _blockCount(blockCount)
{
	if (blockSize == 0 || blockCount == 0)
		throw std::runtime_error("BlockAllocator::BlockAllocator(uint32_t, uint32_t) Won't allocate 0 bytes of memory");

	_dataPtr = malloc(blockSize * blockCount);

	if (!_dataPtr)
		throw std::runtime_error("BlockAllocator::BlockAllocator(uint32_t, uint32_t) Couldn't allocate memory");

	for (uint32_t i = 0; i < blockCount; i++)
	{
		const uint64_t chunkAddress = reinterpret_cast<uint64_t>(_dataPtr) + i * blockSize;
		Chunk* newFreeChunk = reinterpret_cast<Chunk*>(chunkAddress);
		newFreeChunk->next = _chunksHead;
		_chunksHead = newFreeChunk;
	}
	_usedMem = 0;
}


/*!
	@brief Move constructor.
*/
BlockAllocator::BlockAllocator(BlockAllocator&& alloc_in) noexcept
{
	_blockSize = alloc_in._blockSize;
	_blockCount = alloc_in._blockCount;
	_usedMem = alloc_in._usedMem;
	_dataPtr = alloc_in._dataPtr;
	_chunksHead = alloc_in._chunksHead;

	alloc_in._dataPtr = nullptr;
	alloc_in._usedMem = alloc_in._blockSize = alloc_in._blockCount = 0;
	alloc_in._chunksHead = nullptr;
}

/*!
	@brief Move assignment.
*/
BlockAllocator& BlockAllocator::operator = (BlockAllocator&& alloc_in) noexcept
{
	if (this != &alloc_in)
	{
		_blockSize = alloc_in._blockSize;
		_blockCount = alloc_in._blockCount;
		_usedMem = alloc_in._usedMem;
		_dataPtr = alloc_in._dataPtr;
		_chunksHead = alloc_in._chunksHead;

		alloc_in._dataPtr = nullptr;
		alloc_in._usedMem = alloc_in._blockSize = alloc_in._blockCount = 0;
		alloc_in._chunksHead = nullptr;
	}
	return *this;
}

/*!
	@brief Destructor.
*/
BlockAllocator::~BlockAllocator()
{
	free(_dataPtr);
}

/*!
	@brief Increment _usedMem value and gives free chunk.
*/
void* BlockAllocator::allocate() noexcept
{
	std::unique_lock<std::mutex> lck(_lock);

	if (_blockSize + _usedMem > _blockSize * _blockCount)
		return nullptr;

	auto* curr = _chunksHead;
	auto* next = _chunksHead->next;
	_chunksHead = next;
	_usedMem += _blockSize;

	return curr;
}

/*!
	@brief Decrement _usedMem value and stores free chunk.
*/
void BlockAllocator::deallocate(void* ptr) noexcept
{
	if (_usedMem == 0)
		return;

	std::unique_lock<std::mutex> lck(_lock);
	auto* newChunk = reinterpret_cast<Chunk*>(ptr);
	newChunk->next = _chunksHead;
	_chunksHead = newChunk;
	_usedMem -= _blockSize;
}

/*!
	@brief Logs Info
*/
void BlockAllocator::LogInfo()
{
	std::cout << "BlockAllocator State: UsedMem - " << _usedMem << "\n";
}

/*!
	@brief Traverse free list and counts number of nodes.
*/
uint32_t BlockAllocator::GetFreeListSize() noexcept
{
	std::unique_lock<std::mutex> lck(_lock);
	if (!_chunksHead)
		return 0;

	uint32_t sz = 1;
	auto* it = _chunksHead;
	while (it->next) {
		it = it->next;
		sz++;
	}
	return sz;
}

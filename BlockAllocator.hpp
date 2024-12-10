#include <thread>
#include <mutex>

/*!
	@brief Custom allocator based on malloc/free.
	Allocates _blockCount number of chunks with size of _blockSize.
	Stores list of free chunks, and gives free one on allocate(). Put one back on deallocate().
	Free list goes from the end to start.
	Not STL compatible yet:(
	Stores mutex for synchronization.
	Possible improvement - use lock-free list for free chunks.
*/

class BlockAllocator
{
	public:
		BlockAllocator(uint32_t blockSize, uint32_t blockCount);
		BlockAllocator(BlockAllocator&& alloc_in) noexcept;
		BlockAllocator& operator =(BlockAllocator&& alloc_in) noexcept;
		BlockAllocator(const BlockAllocator& alloc_in) = delete;
		BlockAllocator& operator =(const BlockAllocator& alloc_in) = delete;
		~BlockAllocator();

		void* allocate() noexcept;
		void deallocate(void* ptr) noexcept;

		void LogInfo();
		uint32_t GetBlockSize() const noexcept { return _blockSize; }
		uint32_t GetBlockCount() const noexcept { return _blockCount; }
		uint32_t GetUsedMem() const noexcept { return _usedMem; }
		uint32_t GetFreeListSize() noexcept;

	private:
		uint32_t   _blockSize  = 0;
		uint32_t   _blockCount = 0;
		uint32_t   _usedMem    = 0;
		void*      _dataPtr    = nullptr;
		std::mutex _lock;

	    struct Chunk { Chunk* next; };
	    Chunk*     _chunksHead = nullptr;
};

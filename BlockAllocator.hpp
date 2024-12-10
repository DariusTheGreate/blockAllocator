#include <thread>
#include <mutex>


class BlockAllocator
{
	public:
		BlockAllocator(uint32_t blockSize, uint32_t blockCount);
		BlockAllocator(BlockAllocator&& alloc_in) noexcept;
		BlockAllocator& operator =(BlockAllocator&& alloc_in) noexcept;
		BlockAllocator(const BlockAllocator& alloc_in) = delete;
		BlockAllocator& operator =(const BlockAllocator& alloc_in) = delete;
		~BlockAllocator();

		void* allocate();
		void* allocate(uint32_t sz_in);// Think about it
		void deallocate(void* ptr);
		void LogInfo();
		uint32_t GetFreeListSize();

	private:
		uint32_t   _blockSize  = 0;
		uint32_t   _blockCount = 0;
		uint32_t   _usedMem    = 0;
		void*      _dataPtr    = nullptr;
		std::mutex _lock;

		// Stack linked list for free chunks
	    struct Chunk { Chunk* next; };
	    Chunk*     _chunksHead = nullptr;
};

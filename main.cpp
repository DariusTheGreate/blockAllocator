#include <iostream>
#include <thread>
#include <mutex>

class BlockAllocator
{
	public:
		BlockAllocator(uint32_t blockSize, uint32_t blockCount) : _blockSize(blockSize), _blockCount(blockCount)
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
		}

		void* allocate()
		{
			return _chunksHead->next;
		}

		void deallocate()
		{

		}

	private:
		uint32_t   _blockSize  = 0;
		uint32_t   _blockCount = 0;
		uint32_t   _currBlock  = 0;
		void*      _dataPtr    = nullptr;
		std::mutex _lock;

		 // Stack linked list for free chunks
	    struct Chunk { Chunk* next; };
	    Chunk* _chunksHead = nullptr;
};

int main()
{
	BlockAllocator alloc{1024, 1024};

	return 0;
}
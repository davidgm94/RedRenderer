#include "common.h"

#define OWN_GENERAL_PURPOSE_ALLOCATOR 0
#define OWN_ALLOCATOR_FOR_EASTL 0
#define EASTL_ALLOCATION_DEBUGGING_INFO 1
#define GENERAL_PURPOSE_ALLOCATION_DEBUGGING_INFO 1

#ifndef STATIC_MEMORY_SIZE 
#define STATIC_MEMORY_SIZE (20 * MEGABYTE)
#endif

size_t staticMemorySize = STATIC_MEMORY_SIZE;
char staticBlob[STATIC_MEMORY_SIZE];

struct MemoryBlock
{
    void* blob;
    void* freePointer;
};

struct StaticMemory : MemoryBlock
{
    StaticMemory()
	   : MemoryBlock({ (void*)staticBlob, (void*)staticBlob })
    {}
};

const size_t staticBigPoolSize = 4 * GIGABYTE;
char bigPool[staticBigPoolSize];

struct StaticBigPool : MemoryBlock
{
    StaticBigPool()
	   :MemoryBlock({ (void*)bigPool, (void*)bigPool })
    {}
};

StaticMemory staticMemory;
//StaticBigPool bigPool;

static inline void* allocateStaticMemory(size_t size)
{
    size_t addressAfterAllocation = (size_t)(staticMemory.freePointer) + size;
    size_t highestAddressAvailable = (size_t)(staticMemory.blob) + staticMemorySize;
    if (!(addressAfterAllocation > highestAddressAvailable))
    {
	   void* allocatedMemory = staticMemory.freePointer;
	   staticMemory.freePointer = (char*)staticMemory.freePointer + size;

	   return allocatedMemory;
    }
    else
	   // static memory allocation failed
    {
#ifdef _WIN64
	   OutputDebugString("Static allocation failed");
#endif
	   return nullptr;
    }
}

static inline void* allocateHeapMemory(size_t size)
{
    return malloc(size);
}

/*
This function returns the total system memory in kilobytes
*/
static inline u64 getTotalRAMSize()
{
    u64 kilobytes;
    BOOL success = GetPhysicallyInstalledSystemMemory(&kilobytes);
    assert(success == TRUE);
    return kilobytes;
}

static inline void* allocateMemoryInBlock(MemoryBlock* block, size_t size)
{
    void* allocatedMemory = block->freePointer;
    block->freePointer = (char*)block->freePointer + size;

    return allocatedMemory;
}

static inline void allocateMemoryBlock(MemoryBlock* block, size_t size)
{
    block->blob = allocateHeapMemory(size);
    assert(block->blob);
    block->freePointer = block->blob;
}

static inline void deallocateMemoryBlock(MemoryBlock* block)
{
    // it gives error because alignment is not implemented
    free(block->blob);
    block->blob = nullptr;
    block->freePointer = nullptr;
}

struct BlockAllocator
{
    u64 RAM_Kilobytes;
    MemoryBlock* memoryBlocks; // array of block whose count depends on system total RAM
    u64 blockCount;
    u64 allocatedBlockCount;
    u64 blockSize;

    BlockAllocator(size_t _blockSize = (256 * MEGABYTE))
	   :
	   RAM_Kilobytes(getTotalRAMSize()),
	   blockSize(_blockSize)
    {
	   double memoryPercentageToBeUsed = 0.4;
	   u64 memoryToBeUsedInKilobytes = u64(memoryPercentageToBeUsed * RAM_Kilobytes);
	   u64 memoryToBeUsed = memoryToBeUsedInKilobytes * KILOBYTE;

	   blockCount = memoryToBeUsed / blockSize;

	   memoryBlocks = (MemoryBlock*)allocateStaticMemory(blockCount * sizeof(MemoryBlock));


	   for (u64 i = 0; i < blockCount; i++)
	   {
		  MemoryBlock* block = &memoryBlocks[i];
		  block->blob = nullptr;
		  block->freePointer = nullptr;
	   }

	   MemoryBlock* firstBlock = &memoryBlocks[0];
	   allocateMemoryBlock(firstBlock, blockSize);
	   allocatedBlockCount = 1;
    }

    void* allocate(size_t size)
    {
	   if (size < blockSize)
	   {
		  for (int i = 0; i < allocatedBlockCount; i++)
		  {
			 MemoryBlock* block = &memoryBlocks[i];
			 size_t addressAfterAllocation = (size_t)(block->freePointer) + size;
			 size_t highestAddressAvailable = (size_t)(block->blob) + blockSize;

			 if (!(addressAfterAllocation > highestAddressAvailable))
			 {
				// do allocation
				void* allocatedMemory = allocateMemoryInBlock(block, size);

				return allocatedMemory;
			 }
		  }

		  if (allocatedBlockCount < blockCount)
		  {
			 // Allocate a new block
			 MemoryBlock* blockToAllocate = &memoryBlocks[allocatedBlockCount];
			 allocateMemoryBlock(blockToAllocate, blockSize);
			 allocatedBlockCount++;

			 void* allocatedMemory = allocateMemoryInBlock(blockToAllocate, size);
			 return allocatedMemory;
		  }
		  else
		  {
			 return nullptr;
		  }
	   }
	   else
	   {
		  // TODO: implement in cases the total memory needed is bigger than the actual block
	   }
    }

    ~BlockAllocator()
    {
	   for (u64 i = 0; i < allocatedBlockCount; i++)
	   {
		  MemoryBlock* block = &memoryBlocks[i];
		  deallocateMemoryBlock(block);
	   }
    }
};

BlockAllocator blockAllocator;

#define ALLOC_DBG_INFO(msg) ( OutputDebugString(msg) )

void* __cdecl operator new(size_t size)
{
#if _DEBUG
#if EASTL_ALLOCATION_DEBUGGING_INFO
    char message[512];
    sprintf(message, "General purpose allocation of %zu bytes\n", size);
    ALLOC_DBG_INFO(message);
#endif
#endif

#if OWN_GENERAL_PURPOSE_ALLOCATOR 
    void* allocatedMemory = blockAllocator.allocate(size);
    return allocatedMemory;
#else
    return malloc(size);
#endif
}

void* __cdecl operator new[](size_t size)
{
#if _DEBUG
#if EASTL_ALLOCATION_DEBUGGING_INFO
    char message[512];
    sprintf(message, "General purpose allocation of %zu bytes\n", size);
    ALLOC_DBG_INFO(message);
#endif
#endif

#if OWN_GENERAL_PURPOSE_ALLOCATOR 
    void* allocatedMemory = blockAllocator.allocate(size);
    return allocatedMemory;
#else
    return malloc(size);
#endif
}

void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
#if _DEBUG
#if EASTL_ALLOCATION_DEBUGGING_INFO
    static char message[128];
    sprintf(message, "Allocation of %s (size %zu) of new[] (no alignment)\n", name, size);
    ALLOC_DBG_INFO(message);
#endif
#endif

#if OWN_ALLOCATOR_FOR_EASTL
    void* allocatedMemory = blockAllocator.allocate(size);
    return allocatedMemory;
#else
    return malloc(size);
#endif
   
}

void* __cdecl operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
#if _DEBUG
#if EASTL_ALLOCATION_DEBUGGING_INFO
    ALLOC_DBG_INFO("new[] (alignment)");
#endif
#endif
    
#if OWN_ALLOCATOR_FOR_EASTL
    void* allocatedMemory = blockAllocator.allocate(size);
    return allocatedMemory;
#else
    return _aligned_offset_malloc(size, alignment, alignmentOffset);
#endif
}


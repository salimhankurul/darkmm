#pragma once

#define KB(number) number * 1024 
#define MB(number) KB(number) * 1024 

#define ALLOC_BUFFER_SIZE MB(4) 

// Size of 1 chunk
#define ALLOC_CHUNK_SIZE 128

// How Many Chuck 
#define CHUNK_COUNT ALLOC_BUFFER_SIZE / ALLOC_CHUNK_SIZE

// One Chunk can hold data for it self or multi chunks belongs to its self
// Yes For ever MemBlock We only need to know how many AllocatedChunks it has
// WE dont need any other information in our InfoTable
struct MemBlockInfo
{
	// Current one is allocated or next 5 for 1 request ....
	unsigned short AllocatedChunks;
};

struct MemBlock
{
	char data[ALLOC_CHUNK_SIZE];
};

class MemoryAllocator
{
public:

	static void Init()
	{
		RtlZeroMemory(InfoTables, sizeof(InfoTables));
		RtlZeroMemory(MainBuffer, sizeof(MainBuffer));
		CurrentAllocatedChunks = 0;
		KeInitializeGuardedMutex(&g_globalLock);
	}

	static void* Alloc(u32 Size);
	static void* FastAlloc(u32 Size);
	static void  Free(void* Mem);

	static MemBlockInfo InfoTables[CHUNK_COUNT];
	static MemBlock MainBuffer[CHUNK_COUNT];
	static KGUARDED_MUTEX g_globalLock;

	// For Debugging
	static u32 CurrentAllocatedChunks;
};

#define USE_SELF_MEMORY_ALLOCATOR 

#if defined(USE_SELF_MEMORY_ALLOCATOR)
#define ALLOCATE(Size) MemoryAllocator::FastAlloc(Size)
#define FREE(ptr) MemoryAllocator::Free(ptr)
#else
#define ALLOCATE(Size) ExAllocatePool(NonPagedPool, Size)
#define FREE(ptr) ExFreePool(ptr)
#endif
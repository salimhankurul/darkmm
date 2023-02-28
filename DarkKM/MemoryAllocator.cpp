#include "Main.h"

#define MEM_FREE  0
#define MEM_TAKEN 1

#define ALLOC_PRINT(format, ...) //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, __VA_ARGS__)

MemBlockInfo MemoryAllocator::InfoTables[CHUNK_COUNT];
MemBlock MemoryAllocator::MainBuffer[CHUNK_COUNT];
KGUARDED_MUTEX MemoryAllocator::g_globalLock;
u32 MemoryAllocator::CurrentAllocatedChunks;

// Can free Multi or Single Chunks
void MemoryAllocator::Free(void* Mem)
{
	if (!Mem)
		return;
	// Multi Threading 
	KeAcquireGuardedMutex(&g_globalLock);

	// Based on Memory Address of requested mem we can find index of it on our infotable
	auto Distance = reinterpret_cast<uint8_t*>(Mem) - reinterpret_cast<uint8_t*>(MainBuffer);

	// Get the index from distance
	auto Index = Distance / ALLOC_CHUNK_SIZE;

	// Finding correct memory block with 1 calculation, it will find all block for same amount of time
	MemBlockInfo* CurrentInfoTable = &InfoTables[Index];

	ALLOC_PRINT("[%ws]::%hs:: Distance %i Index %i\r\n", PROJECT_NAME, __FUNCTION__, Distance, Index);
	ALLOC_PRINT("[%ws]::%hs:: Freed %i Chunks \r\n", PROJECT_NAME, __FUNCTION__, CurrentInfoTable->AllocatedChunks);

	// Just For Debugging , Can be removed
	CurrentAllocatedChunks -= CurrentInfoTable->AllocatedChunks;

	// Clear All Chunks at once
	RtlZeroMemory(CurrentInfoTable, sizeof(MemBlockInfo) * CurrentInfoTable->AllocatedChunks);

	// Multi Threading 
	KeReleaseGuardedMutex(&g_globalLock);
}

void* MemoryAllocator::Alloc(u32 Size)
{
	if (Size == 0)
		return nullptr;

	// For Multi Threading 
	KeAcquireGuardedMutex(&g_globalLock);

	PVOID mem = nullptr;

	// Like if we need ALLOC_CHUNK_SIZE + 1 bytes we need + 1 chunks, this adds 2 chunk
	int NeedChunkCount = Size / ALLOC_CHUNK_SIZE;

	// Like if we need ALLOC_CHUNK_SIZE + 1 bytes we need + 1 chunks, this adds 1 chunk that is left over
	NeedChunkCount += Size % ALLOC_CHUNK_SIZE == 0 ? 0 : 1;

	ALLOC_PRINT("[%ws]::%hs:: Size %i \r\n", PROJECT_NAME, __FUNCTION__, Size);
	ALLOC_PRINT("[%ws]::%hs:: NeedChunkCount %i \r\n", PROJECT_NAME, __FUNCTION__, NeedChunkCount);

	auto foundIndex = 0;
	bool IndexIsGoodForAllocation = false;

	for (u32 index = 0; index < CHUNK_COUNT; index++)
	{
		// Check CurrentIndex
		bool CurrentIndexGood = true;
		for (u16 a = index; a < index + NeedChunkCount; a++)
		{			
			if (InfoTables[a].AllocatedChunks > 0)
			{
				index = index + NeedChunkCount;
				CurrentIndexGood = false;
				break;
			}			
		}

		// Current Index is good
		if (CurrentIndexGood)
		{
			// Just For Debugging - Can be removed
			CurrentAllocatedChunks += NeedChunkCount;
			// First One will be set to total chunk count
			InfoTables[index].AllocatedChunks = NeedChunkCount;

			// Rest Will be just marked as filled
			for (short a = index + 1; a < index + NeedChunkCount; a++)
				InfoTables[a].AllocatedChunks = 1;

			mem = &MainBuffer[index];

			// For Multi Threading 
			KeReleaseGuardedMutex(&g_globalLock);

			return mem;
		}
	}
	
	// For Multi Threading 
	KeReleaseGuardedMutex(&g_globalLock);
	return nullptr;
}

void* MemoryAllocator::FastAlloc(u32 Size)
{
	if (Size == 0)
		return nullptr;

	// For Multi Threading 
	KeAcquireGuardedMutex(&g_globalLock);

	PVOID mem = nullptr;

	// Like if we need 65 bytes we need 3 chunks, this adds 2 chunk
	int NeedChunkCount = Size / ALLOC_CHUNK_SIZE;

	// Like if we need 65 bytes we need 3 chunks, this adds 1 chunk that is left over
	NeedChunkCount += Size % ALLOC_CHUNK_SIZE == 0 ? 0 : 1;

	ALLOC_PRINT("[%ws]::%hs:: Size %i \r\n", PROJECT_NAME, __FUNCTION__, Size);
	ALLOC_PRINT("[%ws]::%hs:: NeedChunkCount %i \r\n", PROJECT_NAME, __FUNCTION__, NeedChunkCount);

	auto RandomIndex = 0;
	bool IndexIsGoodForAllocation = false;

	do {
		// Most of our memory will be empty so we find empty table by a random Index, will be faster than for loop
		static ULONG Seed = 3432546;
		RandomIndex = RtlRandomEx(&Seed) % (CHUNK_COUNT - NeedChunkCount - 1);

		ALLOC_PRINT("[%ws]::%hs:: RandomIndex %i \r\n", PROJECT_NAME, __FUNCTION__, RandomIndex);

		bool RandomIndex_Empty = true;

		// lets Check NeededChunkCounts to see if they are free
		for (short a = RandomIndex; a < RandomIndex + NeedChunkCount; a++)
		{
			// Current i is bad
			if (InfoTables[a].AllocatedChunks > 0)
			{
				RandomIndex_Empty = false;
				break;
			}
		}

		if (RandomIndex_Empty)
		{
			// Just For Debugging - Can be removed
			CurrentAllocatedChunks += NeedChunkCount;

			// First One will be set to total chunk count
			InfoTables[RandomIndex].AllocatedChunks = NeedChunkCount;

			// Rest Will be just marked as filled
			for (short a = RandomIndex + 1; a < RandomIndex + NeedChunkCount; a++)
				InfoTables[a].AllocatedChunks = 1;

			IndexIsGoodForAllocation = true;
		}
		else
			ALLOC_PRINT("[%ws]::%hs:: RandomIndex_Empty Not \r\n", PROJECT_NAME, __FUNCTION__);

	} while (!IndexIsGoodForAllocation);


	mem = &MainBuffer[RandomIndex];

	// For Multi Threading 
	KeReleaseGuardedMutex(&g_globalLock);

	return mem;
}


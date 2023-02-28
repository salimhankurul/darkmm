#include "Main.h"

NTSTATUS FMF::SearchProcess(wchar_t* ProcessName, uint16_t SleepTime, uint32_t DesiredEmptySpace, uint64_t& Output)
{
	DEBUG_PRINT("[%ws]::%hs:: Started \r\n", PROJECT_NAME, __FUNCTION__);

	Process* Game = new Process();
	
	auto Status = KernelProcess::GetProcess(ProcessName, *Game);

	if (!NT_SUCCESS(Status))
	{
		DEBUG_PRINT("[%ws]::%hs Error GetProcess Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, Status);
		return Status;
	}
	
	Status = FindFreeMemory(ProcessName ,*Game, SleepTime, DesiredEmptySpace, Output);
	
	DEBUG_PRINT("[%ws]::%hs Output %llX \r\n", PROJECT_NAME, __FUNCTION__, Output);

	DEBUG_RESULT_WITH_RETURN(DEBUG_PRINT, Status, "FindFreeMemory");

	delete Game;

	DEBUG_PRINT("[%ws]::%hs Handles Closed \r\n", PROJECT_NAME, __FUNCTION__);
	DEBUG_PRINT("[%ws]::%hs:: Finished \r\n", PROJECT_NAME, __FUNCTION__);

	return Status;
}

uint64_t FMF::CheckRegionForEmptySpaces(wchar_t* ModuleName, uint64_t ModuleBase,  uint64_t LocalAllocatedAddress, uint64_t LocalAllocatedSize, DWORD Protection, uint64_t DesiredEmptySpace)
{
	uint64_t CurrentEmptySpace = 0;
	for (uint64_t i = LocalAllocatedAddress; i < LocalAllocatedAddress + LocalAllocatedSize; i++)
	{

		if (*reinterpret_cast<BYTE*>(i) == 0)
			CurrentEmptySpace++;

		if (*reinterpret_cast<BYTE*>(i) != 0)
		{
			if (CurrentEmptySpace > DesiredEmptySpace) // Empty Memory End
			{
				// Rebase EmptyStart Address to Games Memory instead of our malloc address
				uint64_t EmptyStart = (i - LocalAllocatedAddress + ModuleBase) - CurrentEmptySpace;


				DEBUG_PRINT("[%ws]::%hs:: %ws-[0x%llX] - EmptyStart: 0x%llX  EmptySize : 0x%llX Protection %i EmpytSpacePointer: 0x%llX\r\n", PROJECT_NAME, __FUNCTION__,
					ModuleName, ModuleBase, EmptyStart, CurrentEmptySpace, Protection, EmptyStart - ModuleBase);
				return EmptyStart;
			}
			CurrentEmptySpace = 0;
		}
	}

	return CurrentEmptySpace;
}

NTSTATUS FMF::FindFreeMemory(wchar_t* ModuleName, Process& pProcess, uint32_t SleepTime, uint32_t DesiredEmptySpace, uint64_t& Output)
{
	// ******************************************* 
	// Allocate Memory
	// ******************************************* 

	KernelUtilities::Sleep(SleepTime);

	PVOID LocalAllocatedAddress = MmAllocateNonCachedMemory(pProcess.Size);

	if (!LocalAllocatedAddress)
	{
		DEBUG_PRINT("[%ws]::%hs Error: MmAllocateNonCachedMemory \r\n", PROJECT_NAME, __FUNCTION__);
		return STATUS_ACCESS_DENIED;
	}

	DEBUG_PRINT("[%ws]::%hs Memory Allocated via MmAllocateNonCachedMemory \r\n", PROJECT_NAME, __FUNCTION__);

	// ******************************************* 
	// Read Process
	// ******************************************* 

	auto status = UserMemory::MyReadProcessMemory(pProcess.MmGameHandle, (LPVOID)pProcess.Base.QuadPart, (PVOID)LocalAllocatedAddress, pProcess.Size);

	DEBUG_RESULT_WITH_RETURN(DEBUG_PRINT, status, "SSDT::MyReadProcessMemory");

	Output = CheckRegionForEmptySpaces(ModuleName, pProcess.Base.QuadPart, (uint64_t)LocalAllocatedAddress, pProcess.Size, 0x0, DesiredEmptySpace);

	// ******************************************* 
	// Free Memory
	// ******************************************* 

	MmFreeNonCachedMemory(LocalAllocatedAddress, pProcess.Size);

	DEBUG_PRINT("[%ws]::%hs Memory Allocated via MmFreeNonCachedMemory \r\n", PROJECT_NAME, __FUNCTION__);

	return status;
}
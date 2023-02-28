#pragma once

class Process;
class FMF
{
public:

		static uint64_t CheckRegionForEmptySpaces(wchar_t* ModuleName, uint64_t ModuleBase, uint64_t LocalAllocatedAddress, uint64_t LocalAllocatedSize, DWORD Protection, uint64_t DesiredEmptySpace);
		static NTSTATUS SearchProcess(wchar_t* ProcessName, uint16_t SleepTime, uint32_t DesiredEmptySpace, uint64_t& Output);
		static NTSTATUS FindFreeMemory(wchar_t* ModuleName, Process& pProcess, uint32_t SleepTime, uint32_t DesiredEmptySpace, uint64_t& Output);
};


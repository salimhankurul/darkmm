#include "Main.h"

NTSTATUS ProcessDumper::DumpProcess(wchar_t* ProcessName, wchar_t* TargetPath, uint32_t SleepTime)
{
//	DEBUG_PRINT("[%ws]::%hs:: Started Dumping %ws \r\n", PROJECT_NAME, __FUNCTION__, ProcessName);
//
//	Process* Game = new Process(ProcessName);
//
//	DEBUG_PRINT("[%ws]::%hs Waiting For %wZ... \r\n", PROJECT_NAME, __FUNCTION__, &Game->FullName);
//
//	auto status = KernelProcess::CatchProcess(*Game);
//
//	Game->Debug();
//
//	if (!NT_SUCCESS(status))
//	{
//		DEBUG_PRINT("[%ws]::%hs Error KernelProcess::CatchProcess Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
//		return status;
//	}
//
//	DEBUG_PRINT("[%ws]::%hs Target Found, Sleeping for %i seconds \r\n", PROJECT_NAME, __FUNCTION__, SleepTime);
//
//	KernelUtilities::Sleep(SleepTime);
//
//	PEPROCESS MmGameHandle = KernelProcess::GetPEPProcess(Game->ID);
//	HANDLE ZwGameHandle = KernelProcess::GetProcessHandle(Game->ID);
//
//	DEBUG_PRINT("[%ws]::%hs Recived Game Handle \r\n", PROJECT_NAME, __FUNCTION__);
//
//	KernelFile* pFile = new KernelFile(TargetPath);
//
//	// ******************************************* 
//	// Allocate memory for File
//	// ******************************************* 
//	pFile->OpenFile();
//	
//	pFile->Allocate(Game->Size);
//
//	DEBUG_PRINT("[%ws]::%hs Allocate memory for File \r\n", PROJECT_NAME, __FUNCTION__);
//
//	// Copy Header
//	status = UserMemory::MyReadProcessMemory(MmGameHandle,(PVOID)Game->Base.QuadPart, pFile->data, 0x1000);
//		
//	if (!NT_SUCCESS(status))
//	{
//		DEBUG_PRINT("[%ws]::%hs Error To MyReadProcessMemory Header -> Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
//		delete pFile;
//		return status;
//	}
//
//	DEBUG_PRINT("[%ws]::%hs Readed Process Header \r\n", PROJECT_NAME, __FUNCTION__);
//	
//	PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(pFile->data);
//
//	PVOID isWow64 = PsGetProcessWow64Process(MmGameHandle);
//
//	if (isWow64)
//	{
//		DEBUG_PRINT("[%ws]::%hs Dumping x86 \r\n", PROJECT_NAME, __FUNCTION__);
//		PIMAGE_NT_HEADERS32 pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS32>((reinterpret_cast<LPBYTE>(pFile->data) + pDosHeader->e_lfanew));
//		uint32_t i = 0;
//		IMAGE_SECTION_HEADER* pSecHeader;
//		uint32_t bufPtr = pNtHeaders->OptionalHeader.SizeOfHeaders;
//		for (pSecHeader = IMAGE_FIRST_SECTION(pNtHeaders); i < pNtHeaders->FileHeader.NumberOfSections; ++i, ++pSecHeader)
//		{
//			pSecHeader->Misc.VirtualSize = pSecHeader->SizeOfRawData;
//			memcpy(reinterpret_cast<LPBYTE>(pFile->data) + bufPtr, pSecHeader, sizeof(IMAGE_SECTION_HEADER));
//			bufPtr += sizeof(IMAGE_SECTION_HEADER);
//			status = UserMemory::MyReadProcessMemory(MmGameHandle, reinterpret_cast<LPBYTE>(pNtHeaders->OptionalHeader.ImageBase + pSecHeader->VirtualAddress), reinterpret_cast<LPBYTE>(pFile->data) + pSecHeader->PointerToRawData, pSecHeader->SizeOfRawData);
//			if (!NT_SUCCESS(status))
//			{
//				DEBUG_PRINT("[%ws]::%hs Error To MyReadProcessMemory -> Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
//				goto cleanup;
//			}
//		}
//	}
//	else
//	{
//		DEBUG_PRINT("[%ws]::%hs Dumping x64 \r\n", PROJECT_NAME, __FUNCTION__);
//		PIMAGE_NT_HEADERS pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>((reinterpret_cast<LPBYTE>(pFile->data) + pDosHeader->e_lfanew));
//		uint32_t i = 0;
//		IMAGE_SECTION_HEADER* pSecHeader;
//		uint32_t bufPtr = pNtHeaders->OptionalHeader.SizeOfHeaders;
//		for (pSecHeader = IMAGE_FIRST_SECTION(pNtHeaders); i < pNtHeaders->FileHeader.NumberOfSections; ++i, ++pSecHeader)
//		{
//			pSecHeader->Misc.VirtualSize = pSecHeader->SizeOfRawData;
//			memcpy(reinterpret_cast<LPBYTE>(pFile->data) + bufPtr, pSecHeader, sizeof(IMAGE_SECTION_HEADER));
//			bufPtr += sizeof(IMAGE_SECTION_HEADER);
//			status = UserMemory::MyReadProcessMemory(MmGameHandle, (void*)(pNtHeaders->OptionalHeader.ImageBase + pSecHeader->VirtualAddress), reinterpret_cast<LPBYTE>(pFile->data) + pSecHeader->PointerToRawData, pSecHeader->SizeOfRawData);
//			if (!NT_SUCCESS(status))
//			{
//				DEBUG_PRINT("[%ws]::%hs Error To MyReadProcessMemory -> Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
//				goto cleanup;
//			}
//		}
//	}
//
//	DEBUG_PRINT("[%ws]::%hs Writed Sections \r\n", PROJECT_NAME, __FUNCTION__);
//	
//
//	status = pFile->WriteFile(); 
//
//	if (!NT_SUCCESS(status))
//	{
//		DEBUG_PRINT("[%ws]::%hs Error To CreateDump -> Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
//	}
//
//cleanup:
//	KernelUtilities::CloseHandle(ZwGameHandle);
//	ObDereferenceObject(MmGameHandle);
//	delete Game;
//
//	delete pFile;
//
//	return status;
	return 0;
}


/* Live Dumping */

NTSTATUS ProcessDumper::DumpModule(PEPROCESS MmGameHandle,KModule Module, wchar_t* ModuleTargetPath)
{
	
	KernelFile* pFile = new KernelFile(ModuleTargetPath);

	// ******************************************* 
	// Allocate memory for File
	// ******************************************* 
	pFile->OpenFile();

	pFile->Allocate(Module.Size.LowPart);

	DEBUG_PRINT("[%ws]::%hs Allocate memory for File \r\n", PROJECT_NAME, __FUNCTION__);

	// Copy Header
	auto status = UserMemory::MyReadProcessMemory(MmGameHandle, (PVOID)Module.Base.QuadPart, pFile->data, 0x1000);

	if (!NT_SUCCESS(status))
	{
		DEBUG_PRINT("[%ws]::%hs Error To MyReadProcessMemory Header -> Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
		delete pFile;
		return status;
	}

	DEBUG_PRINT("[%ws]::%hs Readed Process Header \r\n", PROJECT_NAME, __FUNCTION__);

	PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(pFile->data);

	PVOID isWow64 = PsGetProcessWow64Process(MmGameHandle);

	if (isWow64)
	{
		DEBUG_PRINT("[%ws]::%hs Dumping x86 \r\n", PROJECT_NAME, __FUNCTION__);
		PIMAGE_NT_HEADERS32 pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS32>((reinterpret_cast<LPBYTE>(pFile->data) + pDosHeader->e_lfanew));
		uint32_t i = 0;
		IMAGE_SECTION_HEADER* pSecHeader;
		uint32_t bufPtr = pNtHeaders->OptionalHeader.SizeOfHeaders;
		for (pSecHeader = IMAGE_FIRST_SECTION(pNtHeaders); i < pNtHeaders->FileHeader.NumberOfSections; ++i, ++pSecHeader)
		{
			pSecHeader->Misc.VirtualSize = pSecHeader->SizeOfRawData;
			memcpy(reinterpret_cast<LPBYTE>(pFile->data) + bufPtr, pSecHeader, sizeof(IMAGE_SECTION_HEADER));
			bufPtr += sizeof(IMAGE_SECTION_HEADER);
			status = UserMemory::MyReadProcessMemory(MmGameHandle, reinterpret_cast<LPBYTE>(pNtHeaders->OptionalHeader.ImageBase + pSecHeader->VirtualAddress), reinterpret_cast<LPBYTE>(pFile->data) + pSecHeader->PointerToRawData, pSecHeader->SizeOfRawData);
			if (!NT_SUCCESS(status))
			{
				DEBUG_PRINT("[%ws]::%hs Error To MyReadProcessMemory -> Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
				goto cleanup;
			}
		}
	}
	else
	{
		DEBUG_PRINT("[%ws]::%hs Dumping x64 \r\n", PROJECT_NAME, __FUNCTION__);
		PIMAGE_NT_HEADERS pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>((reinterpret_cast<LPBYTE>(pFile->data) + pDosHeader->e_lfanew));
		uint32_t i = 0;
		IMAGE_SECTION_HEADER* pSecHeader;
		uint32_t bufPtr = pNtHeaders->OptionalHeader.SizeOfHeaders;
		for (pSecHeader = IMAGE_FIRST_SECTION(pNtHeaders); i < pNtHeaders->FileHeader.NumberOfSections; ++i, ++pSecHeader)
		{
			pSecHeader->Misc.VirtualSize = pSecHeader->SizeOfRawData;
			memcpy(reinterpret_cast<LPBYTE>(pFile->data) + bufPtr, pSecHeader, sizeof(IMAGE_SECTION_HEADER));
			bufPtr += sizeof(IMAGE_SECTION_HEADER);
			status = UserMemory::MyReadProcessMemory(MmGameHandle, (void*)(pNtHeaders->OptionalHeader.ImageBase + pSecHeader->VirtualAddress), reinterpret_cast<LPBYTE>(pFile->data) + pSecHeader->PointerToRawData, pSecHeader->SizeOfRawData);
			if (!NT_SUCCESS(status))
			{
				DEBUG_PRINT("[%ws]::%hs Error To MyReadProcessMemory -> Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
				goto cleanup;
			}
		}
	}

	DEBUG_PRINT("[%ws]::%hs Writed Sections \r\n", PROJECT_NAME, __FUNCTION__);


	status = pFile->WriteFile();

	if (!NT_SUCCESS(status))
	{
		DEBUG_PRINT("[%ws]::%hs Error To CreateDump -> Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
	}

cleanup:

	
	delete pFile;

	return status;
}

NTSTATUS ProcessDumper::DumpAllProcessNamed(wchar_t* ProcessName, wchar_t* TargetPath, uint32_t SleepTime)
{
	DEBUG_PRINT("[%ws]::%hs Waiting %ws  \r\n", PROJECT_NAME, __FUNCTION__, ProcessName);

	while(!KernelProcess::GetProcessId(ProcessName))
		KernelUtilities::Sleep(100);
	
	DEBUG_PRINT("[%ws]::%hs %ws Found -> Sleeping for %i seconds \r\n", PROJECT_NAME, __FUNCTION__, ProcessName, SleepTime);

	KernelUtilities::Sleep(SleepTime);

	ULONG bytes = 0;

	auto status = ZwQuerySystemInformation(SystemProcessInformation, 0, bytes, &bytes);

	if (!bytes)
		return 0;

	auto Allocated = ALLOCATE(bytes);

	PSYSTEM_PROCESS_INFO pProcessInformation = reinterpret_cast<PSYSTEM_PROCESS_INFO>(Allocated);

	status = ZwQuerySystemInformation(SystemProcessInformation, pProcessInformation, bytes, &bytes);

	while (pProcessInformation->NextEntryOffset)
	{
		if (pProcessInformation->ImageName.Buffer != nullptr)
		{
			if (wcscmp(pProcessInformation->ImageName.Buffer, ProcessName) == 0)
			{	
				auto processId = pProcessInformation->UniqueProcessId;
				
				PEPROCESS MmGameHandle = KernelProcess::GetPEPProcess(processId);
				
				if (!MmGameHandle)
				{
					DEBUG_PRINT("[%ws]::%hs Failed To Get Handle \r\n", PROJECT_NAME, __FUNCTION__);
					continue;
				}

				KeAttachProcess(MmGameHandle);
				
				DEBUG_PRINT("[%ws]::%hs Target Found ID: %i Handle %X \r\n", PROJECT_NAME, __FUNCTION__, processId, MmGameHandle);

				auto lpModule = new KModule();
				
				auto Status = KernelModule::GetUserModule(MmGameHandle, ProcessName, lpModule);

				if (!NT_SUCCESS(Status))
				{
					DEBUG_PRINT("[%ws]::%hs:: Error To Get Module %hs \r\n", PROJECT_NAME, __FUNCTION__, ProcessName);
					delete lpModule;
					KeDetachProcess();
					continue;
				}

				if (lpModule->Base.LowPart)
				{
					DEBUG_PRINT("[%ws]::%hs MainModule Found \r\n", PROJECT_NAME, __FUNCTION__);

					auto info = new DK::DString<WCHAR>(L"%ws%i.exe", TargetPath, processId);

					DEBUG_PRINT("[%ws]::%hs Creating Module At: %ws \r\n", PROJECT_NAME, __FUNCTION__, info->Buffer);
					
					DumpModule(MmGameHandle, *lpModule, info->Buffer);
					delete info;
				} else
					DEBUG_PRINT("[%ws]::%hs MainModule Not Found \r\n", PROJECT_NAME, __FUNCTION__);
							
				delete lpModule;
				KeDetachProcess();
				ObDereferenceObject(MmGameHandle);
			}
		}

		pProcessInformation = reinterpret_cast<PSYSTEM_PROCESS_INFO>(reinterpret_cast<PUCHAR>(pProcessInformation) + pProcessInformation->NextEntryOffset);
	}

	FREE(Allocated);

	return status;
}
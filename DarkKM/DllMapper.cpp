#include "Main.h"
#include "Loader.h"

#define MAPPER_DEBUG DEBUG_PRINT

NTSTATUS Mapper::ManuelMapImage86(
	PEPROCESS MmGameHandle,
	KernelFile* pFile,
	DWORD LoadLibrary,
	DWORD GetProcAddress,
	DWORD& ImageAddress,
	DWORD& ParametersAddress,
	DWORD& LoaderAddress
)
{
	NTSTATUS status = STATUS_SUCCESS;
	// ********************************** Start Writing DLL & Loader
	PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(pFile->data);
	PIMAGE_NT_HEADERS32 pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS32>((reinterpret_cast<LPBYTE>(pFile->data) + pDosHeader->e_lfanew));

	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		status = STATUS_NO_TOKEN;

	if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		status = STATUS_NO_TOKEN;

	if (!(pNtHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL))
		status = STATUS_NO_TOKEN;

	DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, status, "Integrity Of the DLL");

	// ************************************** Write The DLL Into The Game

	status = UserMemory::MyWriteProcessMemory(MmGameHandle, (PVOID)ImageAddress, (PVOID)pFile->data, pNtHeaders->OptionalHeader.SizeOfHeaders);

	DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, status, "Write The Image Header Into The Game");

	PIMAGE_SECTION_HEADER pSectionHeaders = reinterpret_cast<PIMAGE_SECTION_HEADER>(pNtHeaders + 1);

	for (DWORD i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++)
	{
		status = UserMemory::MyWriteProcessMemory(MmGameHandle,
			reinterpret_cast<PVOID>(reinterpret_cast<LPBYTE>(ImageAddress) + pSectionHeaders[i].VirtualAddress),
			reinterpret_cast<PVOID>(reinterpret_cast<LPBYTE>(pFile->data) + pSectionHeaders[i].PointerToRawData),
			pSectionHeaders[i].SizeOfRawData);

		//MAPPER_DEBUG("[%ws]::%hs Section is written to 0x%X \r\n", PROJECT_NAME, __FUNCTION__, reinterpret_cast<PVOID>(reinterpret_cast<LPBYTE>(ImageAddress) + pSectionHeaders[i].VirtualAddress));

		if (!NT_SUCCESS(status))
		{
			MAPPER_DEBUG("[%ws]::%hs Error To Write NumberOfSections. Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
			return status;
		}
	}

	// ************************************** Manualy Write Loader since target game is x86

	DWORD pLoader = ImageAddress + pNtHeaders->OptionalHeader.SizeOfImage + 0x32;

	status = UserMemory::MyWriteProcessMemory(MmGameHandle, (PVOID)pLoader, &SimpleLoader32, sizeof(SimpleLoader32));

	DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, status, "Write The Loader Into The Game");

	// ************************************** Manualy Write Parameters since target game is x86
	DWORD pParameters = pLoader + sizeof(SimpleLoader32) + 0x32;

	LoaderParams86* parameters = new LoaderParams86();

	parameters->pImageBase = ImageAddress;
	parameters->pNtHeaders = ImageAddress + pDosHeader->e_lfanew;
	parameters->pBaseRelocation = ImageAddress + pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
	parameters->pImportDescriptor = ImageAddress + pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	parameters->fLoadLibrary = LoadLibrary;
	parameters->fGetProcAddress = GetProcAddress;
	parameters->Inited = false;

	status = UserMemory::MyWriteProcessMemory(MmGameHandle, (PVOID)pParameters, parameters, sizeof(LoaderParams86));

	delete parameters;

	DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, status, "Write The Parameters Into The Game");

	ParametersAddress = pParameters;
	LoaderAddress = pLoader;

	MAPPER_DEBUG("[%ws]::%hs Successfull: I:0x%X L:0x%X P:0x%X\r\n", PROJECT_NAME, __FUNCTION__, ImageAddress, LoaderAddress, ParametersAddress);

	return status;
}

NTSTATUS Mapper::ManuelMapImage64(
	PEPROCESS MmGameHandle,
	KernelFile* pFile,
	LONGLONG LoadLibrary,
	LONGLONG GetProcAddress,
	LONGLONG& ImageAddress,
	LONGLONG& ParametersAddress,
	LONGLONG& LoaderAddress
)
{

	NTSTATUS status = STATUS_SUCCESS;
	// ********************************** Start Writing DLL & Loader
	PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(pFile->data);
	PIMAGE_NT_HEADERS pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>((reinterpret_cast<LPBYTE>(pFile->data) + pDosHeader->e_lfanew));

	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		status = STATUS_NO_TOKEN;

	if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		status = STATUS_NO_TOKEN;

	if (!(pNtHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL))
		status = STATUS_NO_TOKEN;

	if (!NT_SUCCESS(status))
	{
		MAPPER_DEBUG("[%ws]::%hs Error Image Header CHECK. Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
		return status;
	}

	// ************************************** Write The DLL Into The Game

	status = UserMemory::MyWriteProcessMemory(MmGameHandle, (PVOID)ImageAddress, (PVOID)pFile->data, pNtHeaders->OptionalHeader.SizeOfHeaders);

	DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, status, "Write The Image Into The Game");

	PIMAGE_SECTION_HEADER pSectionHeaders = reinterpret_cast<PIMAGE_SECTION_HEADER>(pNtHeaders + 1);

	for (DWORD64 i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++)
	{
		status = UserMemory::MyWriteProcessMemory(MmGameHandle,
			reinterpret_cast<PVOID>(reinterpret_cast<LPBYTE>(ImageAddress) + pSectionHeaders[i].VirtualAddress),
			reinterpret_cast<PVOID>(reinterpret_cast<LPBYTE>(pFile->data) + pSectionHeaders[i].PointerToRawData),
			pSectionHeaders[i].SizeOfRawData);


		if (!NT_SUCCESS(status))
		{
			MAPPER_DEBUG("[%ws]::%hs Error To Write NumberOfSections. Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
			return status;
		}
	}

	// ************************************** Manualy Write Loader since target game is x86

	DWORD64 pLoader = ImageAddress + pNtHeaders->OptionalHeader.SizeOfImage + 0x32;

	status = UserMemory::MyWriteProcessMemory(MmGameHandle, (PVOID)pLoader, &SimpleLoader64, sizeof(SimpleLoader64));

	DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, status, "Write The Loader Into The Game");

	// ************************************** Manualy Write Parameters since target game is x86
	DWORD64 pParameters = pLoader + sizeof(SimpleLoader64) + 0x32;

	LoaderParams64* parameters = new LoaderParams64(); 

	parameters->pImageBase = ImageAddress;
	parameters->pNtHeaders = ImageAddress + pDosHeader->e_lfanew;
	parameters->pBaseRelocation = ImageAddress + pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
	parameters->pImportDescriptor = ImageAddress + pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	parameters->fLoadLibrary = LoadLibrary;
	parameters->fGetProcAddress = GetProcAddress;
	parameters->Inited = false;

	status = UserMemory::MyWriteProcessMemory(MmGameHandle, (PVOID)pParameters, parameters, sizeof(LoaderParams64));
	delete parameters;

	DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, status, "Write The Parameters Into The Game");

	ParametersAddress = pParameters;
	LoaderAddress = pLoader;

	MAPPER_DEBUG("[%ws]::%hs Successfull: I:0x%llX L:0x%llX P:0x%llX \r\n", PROJECT_NAME, __FUNCTION__, ImageAddress, LoaderAddress, ParametersAddress);

	return status;
};

NTSTATUS Mapper::FixProtection(
	HANDLE ZwGameHandle,
	uint64_t TempAddress)

{
	ULONG oldProtect;
	PVOID ProtectAddress = reinterpret_cast<PVOID>(TempAddress);

	MEMORY_BASIC_INFORMATION mbi = { 0 };
	SIZE_T ReturnSize;

	auto Status = ZwQueryVirtualMemory(ZwGameHandle, ProtectAddress, MEMORY_INFORMATION_CLASS::MemoryBasicInformation, reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&mbi), sizeof(MEMORY_BASIC_INFORMATION), &ReturnSize);

	DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, Status, "ZwQueryVirtualMemory");

	SIZE_T Size = mbi.RegionSize;
	
	Status = ZwProtectVirtualMemory(ZwGameHandle, &ProtectAddress, &Size, PAGE_EXECUTE_READWRITE, &oldProtect);

	DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, Status, "ZwProtectVirtualMemory");

	DEBUG_PRINT("[%ws]::%hs:: Fixed Protection for %p Size: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, ProtectAddress, Size);

	return Status;
}

NTSTATUS Mapper::Inject(
	const wchar_t* ProcessName, 
	DLLMemoryType MemoryType, 
	KernelFile* Dll,
	DWORD64 EmptyAddress,
	DWORD SleepTime)

{
	DarkInteger ImageAddress;
	DarkInteger ParametersAddress;
	DarkInteger LoaderAddress;
	DarkInteger ShellCodeAddress;
	DarkInteger LoadLibrary, GetProcAddress;

	MAPPER_DEBUG("[%ws]::%hs Injecting: %wZ to %ws \r\n", PROJECT_NAME, __FUNCTION__, &Dll->path, ProcessName);

	//********************************************
	//	Get Process
	//********************************************
	
	Process* Game = new Process();
	
	auto Status = KernelProcess::GetProcess(ProcessName, *Game);

	if (!NT_SUCCESS(Status))
	{
		DEBUG_PRINT("[%ws]::%hs Error KernelProcess::CatchProcess Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, Status);
		return Status;
	}


	//********************************************
	//	Check DLL file is Valid & check its architecture
	//********************************************

	PVOID isWow64 = PsGetProcessWow64Process(Game->MmGameHandle);

	bool IsX64 = isWow64 == 0x0 ? true : false;
	bool FileIsX64 = false;

	Status = Dll->FileIsx64(FileIsX64);

	if (!NT_SUCCESS(Status))
	{
		DEBUG_PRINT("[%ws]::%hs Error KernelFile::FileIsx64 Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, Status);
		return Status;
	}

	MAPPER_DEBUG("[%ws]::%hs Game64-> %i File64-> %i \r\n", PROJECT_NAME, __FUNCTION__, IsX64, FileIsX64);
	
	//********************************************
	//	Get Exports From kernel32
	//********************************************

	KeAttachProcess(Game->MmGameHandle);

	PVOID pGetProcAddress = KernelModule::GetUserModuleExport(L"KERNEL32.DLL", "GetProcAddress", Game->MmGameHandle); //KERNEL32.DLL
	PVOID pLoadLibrary = KernelModule::GetUserModuleExport(L"KERNEL32.DLL", "LoadLibraryA", Game->MmGameHandle);

	if (!pLoadLibrary || !pGetProcAddress)
	{
		DEBUG_PRINT("[%ws]::%hs Error Error To Get Kernel32 exports \r\n", PROJECT_NAME, __FUNCTION__);
		return STATUS_ACCESS_DENIED;
	}
	
	if (IsX64)
	{
		LoadLibrary.QuadPart = (DWORD64)pLoadLibrary;
		GetProcAddress.QuadPart = (DWORD64)pGetProcAddress;
	}
	else
	{
		LoadLibrary.LowPart = (DWORD)pLoadLibrary;
		GetProcAddress.LowPart = (DWORD)pGetProcAddress;
	}

	KeDetachProcess();

	MAPPER_DEBUG("[%ws]::%hs Successfull: LoadLibraryA:0x%p GetProcAddress:0x%p \r\n", PROJECT_NAME, __FUNCTION__,  LoadLibrary.QuadPart, GetProcAddress.QuadPart);

	//********************************************
	//	Get DLL address space in Game
	//********************************************
	
	MAPPER_DEBUG("[%ws]::%hs Sleeping for %i seconds \r\n", PROJECT_NAME, __FUNCTION__, SleepTime);

	KernelUtilities::Sleep(SleepTime);

	//********************************************
	//	Get DLL address space in Game
	//********************************************

	if (MemoryType == DLLMemoryType::ZwAllocate)
	{
		PVOID ProtectAddress = reinterpret_cast<PVOID>(0);

		SIZE_T RegionSize = 0x100000;

		Status = ZwAllocateVirtualMemory(Game->ZwGameHandle, &ProtectAddress, 0x0, &RegionSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, Status, "ZwAllocateVirtualMemory");

		MAPPER_DEBUG("[%ws]::%hs Allocated ImageAddress %p \r\n", PROJECT_NAME, __FUNCTION__, ProtectAddress);

		if (IsX64)
			ImageAddress.QuadPart = reinterpret_cast<uint64_t>(ProtectAddress);	
		else
			ImageAddress.LowPart = reinterpret_cast<uint32_t>(ProtectAddress);
		
	}
	else if (MemoryType == DLLMemoryType::VadAllocate)
	{
		/*DWORD64 ProtectAddress = 0;

		Status = ModuleExtender::ExtendModule(MmGameHandle, Game->FullName.Buffer, Dll.size * 6, ProtectAddress);

		DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, Status, "ModuleExtender::ExtendModule");

		Status = Mapper::FixProtection(ZwGameHandle,ProtectAddress);

		DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, Status, "Mapper::FixProtection");

		MMAP_DEBUG("[%ws]::%hs Allocated ImageAddress %p \r\n", PROJECT_NAME, __FUNCTION__, ProtectAddress);

		if (IsX64)
			ImageAddress.QuadPart = static_cast<uint64_t>(ProtectAddress);
		else
			ImageAddress.LowPart = static_cast<uint32_t>(ProtectAddress);*/

	}
	else if (MemoryType == DLLMemoryType::FreeMemory)
	{

		if (!EmptyAddress)
		{
			MAPPER_DEBUG("[%ws]::%hs Error  EmptyAddress is not provided \r\n", PROJECT_NAME, __FUNCTION__);
			return STATUS_ACCESS_DENIED;
		}
			
		uint64_t TempAddress = Game->Base.QuadPart + EmptyAddress;
		
		MAPPER_DEBUG("[%ws]::%hs FreeMem ImageAddress %p \r\n", PROJECT_NAME, __FUNCTION__, TempAddress);

		Status = Mapper::FixProtection(Game->ZwGameHandle, TempAddress);

		DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, Status, "Mapper::FixProtection");
	
		if (IsX64)
			ImageAddress.QuadPart = static_cast<uint64_t>(TempAddress);
		else
			ImageAddress.LowPart = static_cast<uint32_t>(TempAddress);
	}

	if (!ImageAddress.QuadPart)
	{
		MAPPER_DEBUG("[%ws]::%hs ERROR:: !ImageAddress. Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, Status);
		return Status;
	}

	//********************************************
	//	Write DLl
	//********************************************

	KernelUtilities::Sleep(2000);

	if (FileIsX64)
		Status = ManuelMapImage64(Game->MmGameHandle, Dll, LoadLibrary.QuadPart, GetProcAddress.QuadPart, ImageAddress.QuadPart, ParametersAddress.QuadPart, LoaderAddress.QuadPart);
	else
		Status = ManuelMapImage86(Game->MmGameHandle, Dll, LoadLibrary.LowPart, GetProcAddress.LowPart, ImageAddress.LowPart, ParametersAddress.LowPart, LoaderAddress.LowPart);
	
	if (FileIsX64)
		DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, Status, "Write The x64 DLL")
	else
		DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, Status, "Write The x86 DLL")

	//********************************************
	//	Call Loader
	//********************************************

	KernelUtilities::Sleep(100);

	HANDLE thread;
	
	if (IsX64)
		Status = KernelImporter::MyNtCreateThreadEx(Game->ZwGameHandle, (PVOID)LoaderAddress.QuadPart, (PVOID)ParametersAddress.QuadPart, &thread);
	else
		Status = KernelImporter::MyNtCreateThreadEx(Game->ZwGameHandle, (PVOID)LoaderAddress.LowPart, (PVOID)ParametersAddress.LowPart, &thread);

	DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, Status, "MyNtCreateThreadEx");


	//********************************************
	//	 Cleaning Up
	//********************************************
cleanup:
	// Close Opened Handles
	delete Game;

	DEBUG_RESULT_WITH_RETURN(MAPPER_DEBUG, Status, "Cleaned");

	return Status;
}
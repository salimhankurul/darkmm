#include "Main.h"

Process* TargetProcess;

#define PROCESS_PRINT(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, __VA_ARGS__)


NTSTATUS KernelProcess::GetProcess(
	const wchar_t* ProcessName,
	Process& Game)
{
	// Get Game ID
	while (!Game.ID)
	{
		PROCESS_PRINT("[%ws]::%hs Looking for %ws.... \r\n", PROJECT_NAME, __FUNCTION__, ProcessName);	
		
		Game.ID = KernelProcess::GetProcessId(ProcessName);
		
		if (!Game.ID)
			KernelUtilities::Sleep(1000);
		else
			PROCESS_PRINT("[%ws]::%hs Game.ID Found: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, Game.ID);
	}

	// Than Get Game Base & Size
	if (Game.ID)
	{
		// Lets Sleep For a sec man....

		KernelUtilities::Sleep(1000);

		Game.MmGameHandle = KernelProcess::GetPEPProcess(Game.ID);
		Game.ZwGameHandle = KernelProcess::GetProcessHandle(Game.ID);

		if (!Game.MmGameHandle || !Game.ZwGameHandle)
		{
			PROCESS_PRINT("[%ws]::%hs Error Getting MMHandle \r\n", PROJECT_NAME, __FUNCTION__);
			return STATUS_ACCESS_DENIED;
		}


		KeAttachProcess(Game.MmGameHandle);

		auto lpModule = new KModule();

		auto Status = KernelModule::GetUserModule(Game.MmGameHandle, ProcessName, lpModule);

		KeDetachProcess();
		
		if (!NT_SUCCESS(Status))
		{
			PROCESS_PRINT("[%ws]::%hs:: Error To Getting module -> %ws \r\n", PROJECT_NAME, __FUNCTION__, ProcessName);
			delete lpModule;
			return STATUS_ACCESS_DENIED;
		}

		PVOID isWow64 = PsGetProcessWow64Process(Game.MmGameHandle);

		if (!isWow64)
			Game.Base.QuadPart = lpModule->Base.QuadPart;
		else		
			Game.Base.LowPart = lpModule->Base.LowPart;

		Game.Size = lpModule->Size.LowPart;	

		delete lpModule;

		if (!isWow64)
			PROCESS_PRINT("[%ws]::%hs:: %ws Base %llX Size %X -> \r\n", PROJECT_NAME, __FUNCTION__, ProcessName, Game.Base.QuadPart, Game.Size);
		else
			PROCESS_PRINT("[%ws]::%hs:: %ws Base %X Size %X -> \r\n", PROJECT_NAME, __FUNCTION__, ProcessName, Game.Base.QuadPart, Game.Size);	
	}
	
	return STATUS_SUCCESS;
}

HANDLE KernelProcess::GetProcessHandle(const wchar_t* process_name)
{
	HANDLE GameId = KernelProcess::GetProcessId(process_name);

	HANDLE ZwGameHandle;

	CLIENT_ID clientId;
	OBJECT_ATTRIBUTES oa;
	InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);
	clientId.UniqueProcess = (HANDLE)GameId;
	clientId.UniqueThread = NULL;

	NTSTATUS status = ZwOpenProcess(&ZwGameHandle, PROCESS_ALL_ACCESS, &oa, &clientId);

	if (NT_SUCCESS(status))
		return ZwGameHandle;

	return 0;
}

HANDLE KernelProcess::GetProcessHandle(HANDLE GameId)
{
	HANDLE ZwGameHandle;

	CLIENT_ID clientId;
	OBJECT_ATTRIBUTES oa;
	InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);
	clientId.UniqueProcess = (HANDLE)GameId;
	clientId.UniqueThread = NULL;

	NTSTATUS status = ZwOpenProcess(&ZwGameHandle, PROCESS_ALL_ACCESS, &oa, &clientId);

	if (NT_SUCCESS(status))
		return ZwGameHandle;

	return 0;
}

PEPROCESS KernelProcess::GetPEPProcess(const wchar_t* process_name)
{
	HANDLE GameId = KernelProcess::GetProcessId(process_name);

	if (!GameId)
		return 0;

	PEPROCESS csrss_process;

	PsLookupProcessByProcessId(GameId, &csrss_process);

	return csrss_process;
}

PEPROCESS KernelProcess::GetPEPProcess(HANDLE GameId)
{
	PEPROCESS process;

	PsLookupProcessByProcessId(GameId, &process);

	return process;
}

PEPROCESS KernelProcess::GetPEPProcess(const char* process_name)
{	
	/* we are scanning for a conditional jump, that jumps to a call to the unexported function that we want, so we follow the jump, then follow the call to get to the function. */
	auto calltoPsGetNextProcess = KernelModule::FindPatternInModule("ntoskrnl.exe", (BYTE*)"\xE8\x00\x00\x00\x00\x48\x8B\xD8\x48\x85\xC0\x74\x23\xF7", "x????xxxxxxxxx");
	
	auto PsGetNextProcess = KernelMemory::follow_call< PEPROCESS(__stdcall*)(PEPROCESS) >((BYTE*)calltoPsGetNextProcess); 
	
	if (!PsGetNextProcess)
		return nullptr;
	
	PEPROCESS previous_process = PsGetNextProcess(nullptr);

	while (previous_process)
	{
		if (!strcmp(PsGetProcessImageFileName(previous_process), process_name))
			return previous_process;

		previous_process = PsGetNextProcess(previous_process);
	}

	return nullptr;
}

NTSTATUS KernelProcess::SuspendAllProcessesNamed(const wchar_t* process_name) 
{
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
			if (wcscmp(pProcessInformation->ImageName.Buffer, process_name) == 0)
			{
		
				PEPROCESS m_process;
				
				auto status = PsLookupProcessByProcessId(pProcessInformation->UniqueProcessId, &m_process);

				if (NT_SUCCESS(status))
				{
					PsSuspendProcess(m_process);
					ObDereferenceObject(m_process);
				}			
			}
		}

		pProcessInformation = reinterpret_cast<PSYSTEM_PROCESS_INFO>(reinterpret_cast<PUCHAR>(pProcessInformation) + pProcessInformation->NextEntryOffset);
	}

	FREE(Allocated);

	return status;
}

HANDLE KernelProcess::GetProcessId(const WCHAR* processName)
{
	ULONG bytes = 0;
	
	auto status = ZwQuerySystemInformation(SystemProcessInformation, 0, bytes, &bytes);
	
	if (!bytes)
		return 0;
	
	//auto Allocated = ALLOCATE(bytes);
	auto Allocated = ExAllocatePool(PagedPool, bytes);

	PSYSTEM_PROCESS_INFO pProcessInformation = reinterpret_cast<PSYSTEM_PROCESS_INFO>(Allocated);

	NTSTATUS ntStatus = ZwQuerySystemInformation(SystemProcessInformation, pProcessInformation, bytes, &bytes);

	HANDLE processId = nullptr;

	for (int i = 0; i < 1000; ++i)
	{
		if (pProcessInformation->ImageName.Buffer != nullptr)
		{
			if (wcscmp(pProcessInformation->ImageName.Buffer, processName) == 0)
			{
				processId = pProcessInformation->UniqueProcessId;
				break;
			}
		}

		pProcessInformation = reinterpret_cast<PSYSTEM_PROCESS_INFO>(reinterpret_cast<PUCHAR>(pProcessInformation) + pProcessInformation->NextEntryOffset);
	}

	ExFreePool(Allocated);

	return processId;
}
//
//HANDLE KernelProcess::GetProcessId(const WCHAR* processName)
//{
//	ULONG bytes = 0;
//
//	auto status = ZwQuerySystemInformation(SystemProcessInformation, 0, bytes, &bytes);
//
//	if (!bytes)
//		return 0;
//
//	//auto Allocated = ALLOCATE(bytes);
//	auto Allocated = ExAllocatePool(PagedPool, bytes);
//
//	PSYSTEM_PROCESS_INFO pProcessInformation = reinterpret_cast<PSYSTEM_PROCESS_INFO>(Allocated);
//
//	status = ZwQuerySystemInformation(SystemProcessInformation, pProcessInformation, bytes, &bytes);
//
//	HANDLE processId = nullptr;
//
//	int a = 0;
//	
//	while (true)
//	{
//		if (pProcessInformation->ImageName.Buffer != nullptr)
//		{
//			PROCESS_PRINT("[%ws]::%hs %i Process %ws \r\n", PROJECT_NAME, __FUNCTION__,a, pProcessInformation->ImageName.Buffer);
//			
//			if (wcscmp(pProcessInformation->ImageName.Buffer, processName) == 0)
//			{
//				processId = pProcessInformation->UniqueProcessId;
//				break;
//			}
//		}
//
//		pProcessInformation = reinterpret_cast<PSYSTEM_PROCESS_INFO>(reinterpret_cast<PUCHAR>(pProcessInformation) + pProcessInformation->NextEntryOffset);
//		a++;
//
//		if (!pProcessInformation->NextEntryOffset)
//			break;
//	} 
//
//	//FREE(Allocated);
//	ExFreePool(Allocated);
//
//	return processId;
//}

WCHAR* KernelProcess::GetProcessNameFromID(HANDLE ID)
{
	ULONG bytes = 0;

	auto status = ZwQuerySystemInformation(SystemProcessInformation, 0, bytes, &bytes);

	if (!bytes)
		return 0;

	auto Allocated = ALLOCATE(bytes);

	PSYSTEM_PROCESS_INFO pProcessInformation = reinterpret_cast<PSYSTEM_PROCESS_INFO>(Allocated);

	status = ZwQuerySystemInformation(SystemProcessInformation, pProcessInformation, bytes, &bytes);

	WCHAR* processId = L"";

	while (pProcessInformation->NextEntryOffset)
	{
		if (pProcessInformation->UniqueProcessId == ID)
		{
			return pProcessInformation->ImageName.Buffer;
		}

		pProcessInformation = reinterpret_cast<PSYSTEM_PROCESS_INFO>(reinterpret_cast<PUCHAR>(pProcessInformation) + pProcessInformation->NextEntryOffset);
	}

	FREE(Allocated);

	return processId;
}



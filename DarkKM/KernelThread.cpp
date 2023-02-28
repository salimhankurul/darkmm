#include "Main.h"

//#define THREAD_DEBUG(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, __VA_ARGS__)
#define THREAD_DEBUG //DEBUG_PRINT

NTSTATUS KernelThread::SuspendAllThreadsBelogsTo(const WCHAR* ModuleName)
{
	ULONG bytes = 0;

	auto status = ZwQuerySystemInformation(SystemProcessInformation, 0, bytes, &bytes);

	if (!bytes)
		return STATUS_ACCESS_DENIED;

	auto Allocated = ALLOCATE(bytes);

	PSYSTEM_PROCESS_INFO pInfo = reinterpret_cast<PSYSTEM_PROCESS_INFO>(Allocated);

	status = ZwQuerySystemInformation(SystemProcessInformation, pInfo, bytes, &bytes);

	if (!NT_SUCCESS(status))
		return STATUS_INVALID_INFO_CLASS;

	bool AnyThreadFound = false;
	
	while (pInfo->NextEntryOffset)
	{
		if (pInfo->UniqueProcessId == (HANDLE)0x4)
		{	
			for (ULONG i = 0; i < pInfo->NumberOfThreads; i++)
			{

				if (PsGetCurrentThreadId() == pInfo->Threads[i].ClientId.UniqueThread)
					continue;

				auto lpModule = new KModule();

				auto Status = KernelModule::GetKernelModule(reinterpret_cast<uint64_t>(pInfo->Threads[i].StartAddress), lpModule);

				if (!NT_SUCCESS(Status))
				{
					THREAD_DEBUG("[%ws]::%hs:: Failed Find Module For this Address 0x%llX  \r\n", PROJECT_NAME, __FUNCTION__, pInfo->Threads[i].StartAddress);
					delete lpModule;
					continue;
				}

				if (wcsstr(ModuleName, lpModule->Name->Buffer))
				{
					THREAD_DEBUG("[%ws]::%hs:: %ws's Thread Found \r\n", PROJECT_NAME, __FUNCTION__, ModuleName);
					ULONG ali = 0;		
					status = KernelImporter::MyNtSuspendThread(pInfo->Threads[i].ClientId.UniqueThread, ali);	
					if (!NT_SUCCESS(status))
					{
						THREAD_DEBUG("[%ws]::%hs:: Failed To Suspend %X \r\n", PROJECT_NAME, __FUNCTION__, status);
					}			
					AnyThreadFound = true;
				}

				delete lpModule->Name;
				delete lpModule;
			}

			break;
		}
		pInfo = (PSYSTEM_PROCESS_INFO)((PUCHAR)pInfo + pInfo->NextEntryOffset);
	}

	FREE(Allocated);
	
	if (!AnyThreadFound)
		return STATUS_BAD_INITIAL_PC;

	return status;
}

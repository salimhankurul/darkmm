#include "Main.h"

KernelUtilities::KernelUtilities()
{
}


KernelUtilities::~KernelUtilities()
{
}

#define UTILS_DEBUG(format, ...) //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, __VA_ARGS__)

NTSTATUS KernelUtilities::DumpAllThreads()
{
	NTSTATUS status = STATUS_SUCCESS;
	PVOID pBuf = ALLOCATE( 1024 * 1024);
	RtlZeroMemory(pBuf, 1024 * 1024);
	PSYSTEM_PROCESS_INFO pInfo = (PSYSTEM_PROCESS_INFO)pBuf;

	if (!pInfo)
		return STATUS_NO_MEMORY;


	// Get the process thread list
	status = ZwQuerySystemInformation(SystemProcessInformation, pInfo, 1024 * 1024, NULL);

	if (!NT_SUCCESS(status))
	{
		FREE(pBuf);
		return status;
	}

	// Find target thread
	if (NT_SUCCESS(status))
	{
		status = STATUS_NOT_FOUND;
		while (pInfo->NextEntryOffset)
		{

			UTILS_DEBUG("[%ws]::%hs:: Current Process ID pInfo->ProcessId %d -> Number Of Threads %d \r\n", PROJECT_NAME, __FUNCTION__, pInfo->UniqueProcessId, pInfo->NumberOfThreads);
	
				for (ULONG i = 0; i < pInfo->NumberOfThreads; i++)
				{
					// Skip current thread
					if (pInfo->Threads[i].ClientId.UniqueThread == PsGetCurrentThreadId())
						continue;

					PETHREAD pThread;

					status = PsLookupThreadByThreadId(pInfo->Threads[i].ClientId.UniqueThread, &pThread);

					if (!NT_SUCCESS(status))
						continue;

					UTILS_DEBUG("[%ws]::%hs:: ****************Thread ID %d -> Starts At 0x%llX \r\n", PROJECT_NAME, __FUNCTION__, pInfo->Threads[i].ClientId.UniqueThread, pInfo->Threads[i].StartAddress);

				}
			pInfo = (PSYSTEM_PROCESS_INFO)((PUCHAR)pInfo + pInfo->NextEntryOffset);
		}
	}

	if (pBuf)
		FREE(pBuf);

	return status;
}

NTSTATUS KernelUtilities::BBGetBuildNO(OUT PULONG pBuildNo)
{
	KernelRegisteryData* pData = new KernelRegisteryData();

	NTSTATUS status = KernelRegistery::ReadRegistryData(L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion", L"CurrentBuild",*pData);
	
	if (!NT_SUCCESS(status))
	{
		UTILS_DEBUG("[%ws]::%hs:: Error: ReadRegistryString. Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
		delete pData;
		return status;
	}

	UNICODE_STRING Signal;
	RtlInitUnicodeString(&Signal, (wchar_t*)pData->Data);

	status = RtlUnicodeStringToInteger(&Signal, 10, pBuildNo);
	
	delete pData;

	return status;

}

void KernelUtilities::CloseHandle(const HANDLE& handle)
{
	if (handle && handle != reinterpret_cast<HANDLE>(-1))
		ZwClose(handle);
};

void KernelUtilities::Sleep(ULONGLONG milliseconds)
{
#define MILLISECOND 10000             // 100 nanosecs * 10,000 = 1 ms
#define RELATIVE_MILLISECOND (-MILLISECOND)     // minus means relative time
	
	LARGE_INTEGER Timeout;  /* Units of 100 ns */
	Timeout.QuadPart = RELATIVE_MILLISECOND;
	Timeout.QuadPart *= milliseconds;

	KeDelayExecutionThread(KernelMode, FALSE, &Timeout);
}

void KernelUtilities::Sleep100ns(ULONGLONG ns100seconds)
{
	LARGE_INTEGER Timeout;  /* Units of 100 ns */
	Timeout.QuadPart = -ns100seconds;
	
	KeDelayExecutionThread(KernelMode, FALSE, &Timeout);
}

u64 KernelUtilities::Now()
{
	return *((volatile u64*)(SharedSystemTime));;
}

BOOLEAN KernelUtilities::bDataCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask)
			return 0;

	return (*szMask) == 0;
}

UINT64 KernelUtilities::FindPattern(UINT64 dwAddress, UINT32 dwLen, BYTE *bMask, char * szMask)
{
	for (UINT64 i = 0; i < dwLen; i++)
		if (MmIsAddressValid((PVOID)dwAddress))
			if (bDataCompare((BYTE*)(dwAddress + i), bMask, szMask))
				return (UINT64)(dwAddress + i);

	return 0;
}

HANDLE KernelUtilities::CreateSystemThread(PKSTART_ROUTINE pRoutine, void* pArgument)
{
	OBJECT_ATTRIBUTES ThreadObject;
	InitializeObjectAttributes(&ThreadObject, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

	HANDLE hThread;
	CLIENT_ID clientId;

	auto status = PsCreateSystemThread(&hThread, MAXIMUM_ALLOWED | SYNCHRONIZE | GENERIC_ALL, &ThreadObject, nullptr, &clientId, pRoutine, pArgument); //| SYNCHRONIZE | GENERIC_ALL

	if (status == STATUS_SUCCESS)
		return hThread;

	return nullptr;
}

void KernelUtilities::WaitForSystemThread(HANDLE hMain)
{
	PETHREAD pThread = NULL;
	OBJECT_HANDLE_INFORMATION handleInfo = { 0 };

	// Wait on worker thread
	NTSTATUS status = ObReferenceObjectByHandle(hMain, THREAD_ALL_ACCESS, *PsThreadType, KernelMode, (PVOID*)& pThread, &handleInfo);

	if (NT_SUCCESS(status))
		status = KeWaitForSingleObject(pThread, Executive, KernelMode, TRUE, NULL);

	KernelUtilities::CloseHandle(hMain);
	ObDereferenceObject(pThread);
}

NTSTATUS KernelUtilities::BBCheckProcessStatus(PEPROCESS pProcess)
{

	if (!pProcess)
		return STATUS_ACCESS_DENIED;

	LARGE_INTEGER zeroTime = { 0 };
	return KeWaitForSingleObject(pProcess, Executive, KernelMode, FALSE, &zeroTime); //== STATUS_WAIT_0
}

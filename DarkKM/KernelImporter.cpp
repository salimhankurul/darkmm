#include "Main.h"
// You can use
//MySuspendProcess
//PsResumeProcess

ULONG KernelImporter::BuildNo;

#define IMPORT_DBG(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, __VA_ARGS__)

NTSTATUS KernelImporter::MyNtCreateThreadEx(
	HANDLE ProcessHandle, 
	PVOID lpStartAddress, 
	PVOID lpParameter, 
	PHANDLE ThreadHandle)
{
	NTSTATUS status = STATUS_SUCCESS;

	auto pNtCreateThreadEx = KernelModule::GetKernelModuleExport("ntoskrnl.exe", "NtCreateThreadEx");

	if (!pNtCreateThreadEx)
		pNtCreateThreadEx = KernelModule::FindPatternInModule("ntoskrnl.exe", (BYTE*)"\xE8\x00\x00\x00\x00\x33\xF6\x48\x89\x75\x08", "x????xxxxxx");

	if (!pNtCreateThreadEx)
	{
		IMPORT_DBG("[%ws]::%hs:: Error: Cant Find NtCreateThreadEx \r\n", PROJECT_NAME, __FUNCTION__);
		return STATUS_ACCESS_DENIED;
	}
	else
		pNtCreateThreadEx = pNtCreateThreadEx - 0x56;

	IMPORT_DBG("[%ws]::%hs:: NtCreateThreadEx %llX \r\n", PROJECT_NAME, __FUNCTION__, pNtCreateThreadEx);

	//return STATUS_ACCESS_DENIED;

	static const auto NtCreateThreadEx = reinterpret_cast<NTSTATUS(NTAPI*)(
		OUT PHANDLE hThread,
		IN ACCESS_MASK DesiredAccess,
		IN PVOID ObjectAttributes,
		IN HANDLE ProcessHandle,
		IN PVOID lpStartAddress,
		IN PVOID lpParameter,
		IN ULONG Flags,
		IN SIZE_T StackZeroBits,
		IN SIZE_T SizeOfStackCommit,
		IN SIZE_T SizeOfStackReserve,
		OUT PVOID lpBytesBuffer)>(pNtCreateThreadEx);

	NTSTATUS Status;

	OBJECT_ATTRIBUTES oa;
	InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);
	
	Status = NtCreateThreadEx(ThreadHandle, THREAD_ALL_ACCESS, NULL, ProcessHandle, lpStartAddress, lpParameter, FALSE, NULL, NULL, NULL, NULL);

	return status;
}

NTSTATUS KernelImporter::MyZwOpenThread(
	PHANDLE ThreadHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PCLIENT_ID ClientId
) {

	static const auto ZwOpenThread = reinterpret_cast<NTSTATUS(NTAPI*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID)>(
		KernelModule::GetKernelModuleExport("ntoskrnl.exe", "ZwOpenThread")
		);

	if (!ZwOpenThread)
	{
		IMPORT_DBG("[%ws]::%hs:: ZwOpenThread \r\n", PROJECT_NAME, __FUNCTION__);
		return STATUS_ACCESS_DENIED;
	}

	return ZwOpenThread(ThreadHandle, DesiredAccess, ObjectAttributes, ClientId);
}

NTSTATUS KernelImporter::MyNtSuspendThread(
	HANDLE ThreadHandle,
	ULONG PreviousSuspendCount)
{
	static const auto NtSuspendThread = reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PULONG)>(
		KernelModule::FindPatternInModule("ntoskrnl.exe", (BYTE*)"\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x50\x48\x8B\xDA\x4C", "xxxx?xxxxxxxxx")
		);

	if (!NtSuspendThread)
	{
		IMPORT_DBG("[%ws]::%hs:: Error: dData->NtSuspendThreadIndex\r\n", PROJECT_NAME, __FUNCTION__);
		return STATUS_ACCESS_DENIED;
	}

	NTSTATUS status = NtSuspendThread(ThreadHandle, &PreviousSuspendCount);

	return status;
}

NTSTATUS KernelImporter::MyNtResumeThread(
	HANDLE hThread,
	ULONG PreviousSuspendCount)
{
	
	static const auto NtResumeThread = reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PULONG)>(
		KernelModule::FindPatternInModule("ntoskrnl.exe", (BYTE*)"\x40\x53\x48\x83\xEC\x50\x48\x8B\xDA\x4C", "xxxxxxxxxx")
		);

	if (!NtResumeThread)
	{
		IMPORT_DBG("[%ws]::%hs:: Error: dData->NtResumeThreadIndex \r\n", PROJECT_NAME, __FUNCTION__);
		return STATUS_ACCESS_DENIED;
	}

	auto status = NtResumeThread(hThread, &PreviousSuspendCount);

	return status;
}

NTSTATUS KernelImporter::MyNtTerminateThread(
	HANDLE hThread, 
	NTSTATUS ExitStatus)
{

	static const auto NtTerminateThread = reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, NTSTATUS)>(
		KernelModule::FindPatternInModule("ntoskrnl.exe", (BYTE*)"\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x56\x48\x83\xEC\x40", "xxxx?xxxx?xxxxx")
		);

	if (!NtTerminateThread)
	{
		IMPORT_DBG("[%ws]::%hs:: Error: dData->NtSuspendThreadIndex\r\n", PROJECT_NAME, __FUNCTION__);
		return STATUS_ACCESS_DENIED;
	}

	auto status = NtTerminateThread(hThread, ExitStatus);

	return status;
}



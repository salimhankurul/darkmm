#pragma once

class KernelImporter
{
public:

	static NTSTATUS MyZwOpenThread(PHANDLE ThreadHandle,ACCESS_MASK DesiredAccess,POBJECT_ATTRIBUTES ObjectAttributes,PCLIENT_ID ClientId);
	static NTSTATUS MyNtTerminateThread(HANDLE hThreadID, NTSTATUS ExitStatus);
	static NTSTATUS MyNtSuspendThread(HANDLE hThread, ULONG PreviousSuspendCount);
	static NTSTATUS MyNtResumeThread(HANDLE hThreadID, ULONG PreviousSuspendCount);
	static NTSTATUS MyNtCreateThreadEx(HANDLE ProcessHandle, PVOID lpStartAddress, PVOID lpParameter, PHANDLE ThreadHandle);
	
	static ULONG BuildNo;
};

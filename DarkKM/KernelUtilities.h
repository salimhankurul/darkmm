#pragma once


class KernelUtilities
{
public:
	KernelUtilities();
	~KernelUtilities();

	static NTSTATUS DumpAllThreads();
	
	static NTSTATUS BBCheckProcessStatus(PEPROCESS pProcess);
	static NTSTATUS BBGetBuildNO(OUT PULONG pBuildNo);

	static void CloseHandle(const HANDLE& handle);
	static void Sleep(ULONGLONG milliseconds);
	static void Sleep100ns(ULONGLONG ns100seconds);
	static u64 Now();

	static BOOLEAN bDataCompare(const BYTE* pData, const BYTE* bMask, const char* szMask);
	static UINT64 FindPattern(UINT64 dwAddress, UINT32 dwLen, BYTE *bMask, char * szMask);

	
	static HANDLE CreateSystemThread(PKSTART_ROUTINE pRoutine, void* pArgument);
	static void WaitForSystemThread(HANDLE hMain);
	
};


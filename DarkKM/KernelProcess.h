#pragma once

struct Process
{
	Process()
    {
        RtlZeroMemory(this, sizeof(Process));
    }

    void* operator new(size_t size)
    {
        return ALLOCATE( sizeof(Process));
    }

    void operator delete(void* ptr)
    {
        if (reinterpret_cast<Process*>(ptr)->MmGameHandle)
            ObDereferenceObject(reinterpret_cast<Process*>(ptr)->MmGameHandle);

        if (reinterpret_cast<Process*>(ptr)->ZwGameHandle)
            KernelUtilities::CloseHandle(reinterpret_cast<Process*>(ptr)->ZwGameHandle);   

        FREE(ptr);
    }
 

	HANDLE ID;	
	uint32_t Size;
    DarkInteger Base;

    PEPROCESS MmGameHandle;
    HANDLE ZwGameHandle;
};

class KernelProcess
{
public:

	static PEPROCESS GetPEPProcess(const char* process_name);
	static PEPROCESS GetPEPProcess(HANDLE GameId);
	static PEPROCESS GetPEPProcess(const wchar_t* process_name);

	static HANDLE GetProcessHandle(const wchar_t* process_name);
	static HANDLE GetProcessHandle(HANDLE GameId);

    static NTSTATUS SuspendAllProcessesNamed(const wchar_t* process_name);
    static HANDLE GetProcessId(const WCHAR* processName);
    static WCHAR* GetProcessNameFromID(HANDLE ID);


	//static NTSTATUS CatchProcess(Process& pProcess);
	static NTSTATUS GetProcess(
		const wchar_t* ProcessName,
		Process& Game);
};

typedef struct _PEB_LDR_DATA
{
    ULONG Length;
    UCHAR Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    LIST_ENTRY HashLinks;
    ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;


typedef struct _PEB
{
    UCHAR InheritedAddressSpace;
    UCHAR ReadImageFileExecOptions;
    UCHAR BeingDebugged;
    UCHAR BitField;
    PVOID Mutant;
    PVOID ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    PVOID ProcessParameters;
    PVOID SubSystemData;
    PVOID ProcessHeap;
    PVOID FastPebLock;
    PVOID AtlThunkSListPtr;
    PVOID IFEOKey;
    PVOID CrossProcessFlags;
    PVOID KernelCallbackTable;
    ULONG SystemReserved;
    ULONG AtlThunkSListPtr32;
    PVOID ApiSetMap;
} PEB, * PPEB;

typedef struct _PEB_LDR_DATA32
{
    ULONG Length;
    UCHAR Initialized;
    ULONG SsHandle;
    LIST_ENTRY32 InLoadOrderModuleList;
    LIST_ENTRY32 InMemoryOrderModuleList;
    LIST_ENTRY32 InInitializationOrderModuleList;
} PEB_LDR_DATA32, * PPEB_LDR_DATA32;

typedef struct _LDR_DATA_TABLE_ENTRY32
{
    LIST_ENTRY32 InLoadOrderLinks;
    LIST_ENTRY32 InMemoryOrderLinks;
    LIST_ENTRY32 InInitializationOrderLinks;
    ULONG DllBase;
    ULONG EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING32 FullDllName;
    UNICODE_STRING32 BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    LIST_ENTRY32 HashLinks;
    ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY32, * PLDR_DATA_TABLE_ENTRY32;

typedef struct _PEB32
{
    UCHAR InheritedAddressSpace;
    UCHAR ReadImageFileExecOptions;
    UCHAR BeingDebugged;
    UCHAR BitField;
    ULONG Mutant;
    ULONG ImageBaseAddress;
    ULONG Ldr;
    ULONG ProcessParameters;
    ULONG SubSystemData;
    ULONG ProcessHeap;
    ULONG FastPebLock;
    ULONG AtlThunkSListPtr;
    ULONG IFEOKey;
    ULONG CrossProcessFlags;
    ULONG UserSharedInfoPtr;
    ULONG SystemReserved;
    ULONG AtlThunkSListPtr32;
    ULONG ApiSetMap;
} PEB32, * PPEB32;

typedef struct _WOW64_PROCESS
{
    PPEB32 Wow64;
} WOW64_PROCESS, * PWOW64_PROCESS;

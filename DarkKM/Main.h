#pragma once

// MDL - Memory Descriptor List 
// VAD - Virtual Address Descriptor 
// PEB - Process Environment Block
// IAT - Import Address Table
// IRQ - Interrupt Request Level()

#include <ntdef.h>
#include <ntifs.h>
#include <math.h>
#include <iostream.h>
#include <string.h>
#include <stdio.h>
#include <ntddk.h>                          
#include <wdm.h>                                                                  
#include <ntstrsafe.h>                             
#include <Ntddkbd.h>                                     
#include <Wdmsec.h>                                          
#include <Fltkernel.h>

#include <stdlib.h>
#include <winapifamily.h> 
#include <ntimage.h>
#include <stdarg.h>

#include <ntdddisk.h>
#include <scsi.h>
#include <intrin.h>

#include <windef.h>
#include <minwindef.h>

#pragma comment(lib,"ntstrsafe.lib")  

#pragma section(".text")

//Public Headers for Windows Kernel
//To develop Windows kernel, you need these headers :
//
//aux_klib.h
//buffring.h
//hwnclx.h
//ioaccess.h
//iointex.h
//ntddk.h
//ntddsfio.h
//ntddsysenv.h
//ntimage.h
//ntintsafe.h
//ntpoapi.h
//ntstrsafe.h
//pcivirt.h
//pep_x.h
//pepevents.h
//pepfx.h
//procgrp.h
//pwmutil.h
//vpci.h
//wdm.h
//wdmsec.h
//wmidata.h
//wmilib.h
//wmistr.h

#pragma region OtherStructures
typedef unsigned short	   wchar;
typedef unsigned char	   byte, BYTE;
typedef signed char        int8_t,i8;
typedef short              int16_ti,i16;
typedef int                int32_t,i32;
typedef long long          int64_t,i64;
typedef unsigned char      uint8_t,u8;
typedef unsigned short     uint16_t,u16;
typedef unsigned int       uint32_t,u32;
typedef unsigned long long uint64_t, u64;

typedef signed char        int_least8_t;
typedef short              int_least16_t;
typedef int                int_least32_t;
typedef long long          int_least64_t;
typedef unsigned char      uint_least8_t;
typedef unsigned short     uint_least16_t;
typedef unsigned int       uint_least32_t;
typedef unsigned long long uint_least64_t;

typedef signed char        int_fast8_t;
typedef int                int_fast16_t;
typedef int                int_fast32_t;
typedef long long          int_fast64_t;
typedef unsigned char      uint_fast8_t;
typedef unsigned int       uint_fast16_t;
typedef unsigned int       uint_fast32_t;
typedef unsigned long long uint_fast64_t;

typedef long long          intmax_t;
typedef unsigned long long uintmax_t;
typedef unsigned __int64 QWORD;


typedef struct _LARGE_UNICODE_STRING {
	ULONG  Length;
	ULONG  MaximumLength;
	//ULONG  bAnsi : 1;
	PWSTR  Buffer;
} LARGE_UNICODE_STRING;
typedef LARGE_UNICODE_STRING* PLARGE_UNICODE_STRING;

struct CALLBACK_HOOK {

	POB_PRE_OPERATION_CALLBACK PreProcessOriginal;
	POB_POST_OPERATION_CALLBACK PostProcessOriginal;

	POB_PRE_OPERATION_CALLBACK PreThreadOriginal;
	POB_POST_OPERATION_CALLBACK PostThreadOriginal;

	POB_PRE_OPERATION_CALLBACK MyPreProcess;
	POB_POST_OPERATION_CALLBACK MyPostProcess;

	POB_PRE_OPERATION_CALLBACK MyPreThread;
	POB_POST_OPERATION_CALLBACK MyPostThread;

};

typedef struct _SYSTEM_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[256];
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

typedef struct _SYSTEM_MODULE
{
	ULONG NumberOfModules;
	SYSTEM_MODULE_INFORMATION Modules[1];
} SYSTEM_MODULE, * PSYSTEM_MODULE;

typedef struct _UNLOADED_DRIVERS {
	UNICODE_STRING Name;
	PVOID StartAddress;
	PVOID EndAddress;
	LARGE_INTEGER CurrentTime;
} UNLOADED_DRIVERS, * PUNLOADED_DRIVERS;




typedef struct _SYSTEM_BASIC_INFORMATION
{
	ULONG Reserved;
	ULONG TimerResolution;
	ULONG PageSize;
	ULONG NumberOfPhysicalPages;
	ULONG LowestPhysicalPageNumber;
	ULONG HighestPhysicalPageNumber;
	ULONG AllocationGranularity;
	ULONG_PTR MinimumUserModeAddress;
	ULONG_PTR MaximumUserModeAddress;
	KAFFINITY ActiveProcessorsAffinityMask;
	CHAR NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION, * PSYSTEM_BASIC_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemNextEventIdInformation,
	SystemEventIdsInformation,
	SystemCrashDumpInformation,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemPlugPlayBusInformation,
	SystemDockInformation,
	SystemPowerInformation2,
	SystemProcessorSpeedInformation,
	SystemCurrentTimeZoneInformation,
	SystemLookasideInformation
} SYSTEM_INFORMATION_CLASS, * PSYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_THREAD_INFORMATION
{
	LARGE_INTEGER KernelTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER CreateTime;
	ULONG WaitTime;
	PVOID StartAddress;
	CLIENT_ID ClientId;
	KPRIORITY Priority;
	LONG BasePriority;
	ULONG ContextSwitches;
	ULONG ThreadState;
	KWAIT_REASON WaitReason;
}SYSTEM_THREAD_INFORMATION, * PSYSTEM_THREAD_INFORMATION;

typedef struct _THREAD_BASIC_INFORMATION
{
	NTSTATUS ExitStatus;
	PVOID TebBaseAddress;
	CLIENT_ID ClientId;
	ULONG_PTR AffinityMask;
	LONG Priority;
	LONG BasePriority;
} THREAD_BASIC_INFORMATION, * PTHREAD_BASIC_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFO
{
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	LARGE_INTEGER WorkingSetPrivateSize;
	ULONG HardFaultCount;
	ULONG NumberOfThreadsHighWatermark;
	ULONGLONG CycleTime;
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ImageName;
	KPRIORITY BasePriority;
	HANDLE UniqueProcessId;
	HANDLE InheritedFromUniqueProcessId;
	ULONG HandleCount;
	ULONG SessionId;
	ULONG_PTR UniqueProcessKey;
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize;
	ULONG PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
	LARGE_INTEGER ReadOperationCount;
	LARGE_INTEGER WriteOperationCount;
	LARGE_INTEGER OtherOperationCount;
	LARGE_INTEGER ReadTransferCount;
	LARGE_INTEGER WriteTransferCount;
	LARGE_INTEGER OtherTransferCount;
	SYSTEM_THREAD_INFORMATION Threads[1];
}SYSTEM_PROCESS_INFO, * PSYSTEM_PROCESS_INFO;

typedef enum _WinVer
{
	WINVER_7 = 0x0610,
	WINVER_7_SP1 = 0x0611,
	WINVER_8 = 0x0620,
	WINVER_81 = 0x0630,
	WINVER_10 = 0x0A00,
	WINVER_10_RS1 = 0x0A01, // Anniversary update
	WINVER_10_RS2 = 0x0A02, // Creators update
	WINVER_10_RS3 = 0x0A03, // Fall creators update
	WINVER_10_RS4 = 0x0A04, // Spring creators update
	WINVER_10_RS5 = 0x0A05, // October 2018 update
} WinVer;

typedef struct _INITIAL_TEB
{
	struct
	{
		PVOID OldStackBase;
		PVOID OldStackLimit;
	} OldInitialTeb;
	PVOID StackBase;
	PVOID StackLimit;
	PVOID StackAllocationBase;
} INITIAL_TEB, * PINITIAL_TEB;

typedef struct _SYSTEM_SERVICE_DESCRIPTOR_TABLE
{
	PULONG_PTR ServiceTableBase;
	PULONG ServiceCounterTableBase;
	ULONG_PTR NumberOfServices;
	PUCHAR ParamTableBase;
} SYSTEM_SERVICE_DESCRIPTOR_TABLE, * PSYSTEM_SERVICE_DESCRIPTOR_TABLE;


#pragma endregion

#pragma region IMPORTS

#define PROJECT_NAME L"DP"

#define IS_NT_SUCCESS(X) (X == STATUS_SUCCESS)

#define IsSafe(X) (X && HIWORD(X) != 0x0 && HIWORD(X) != 0xFFFF && LOWORD(X) != 0xFFFF) ? true : false
#define IsSafe64(X) (X) ? true : false

#define GTI_SECONDS(t)      (ULONGLONG(t)*10000000)
#define GTI_MILLISECONDS(t) (ULONGLONG(t)*10000)
#define GTI_MICROSECONDS(t) (ULONGLONG(t)*10)

// Character count wide char
#define CCHWC(X) sizeof(X) / 2

extern "C" uint64_t WOW64stdcall(void* pfn, uint64_t argc, uint64_t, uint64_t, ...);
extern "C" DWORD32 _x86call();
extern "C" DWORD64 _x86push(DWORD32 offset, DWORD32 val);

EXTERN_C_START
extern NTKERNELAPI
NTSTATUS
ObReferenceObjectByName(
	IN PUNICODE_STRING ObjectName,
	IN ULONG Attributes,
	IN PACCESS_STATE PassedAccessState,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_TYPE ObjectType,
	IN KPROCESSOR_MODE AccessMode,
	IN OUT PVOID ParseContext,
	OUT PVOID* Object
);
EXTERN_C_END

EXTERN_C
NTSYSAPI NTSTATUS NTAPI
ZwQuerySystemInformation(
	IN ULONG SystemInformationClass,
	IN PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength);

EXTERN_C
NTSYSAPI NTSTATUS NTAPI
ZwOpenProcess(
	OUT PHANDLE ProcessHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN PCLIENT_ID ClientId OPTIONAL
);


EXTERN_C
NTSYSAPI BOOLEAN
HalMakeBeep(
	IN  ULONG Frequency
);


EXTERN_C
NTSYSAPI NTSTATUS NTAPI
ZwTerminateProcess(
	IN HANDLE ProcessHandle,
	IN NTSTATUS ExitStatus
);

EXTERN_C
NTSYSAPI NTSTATUS NTAPI
ZwSuspendProcess(
	IN HANDLE ProcessHandle
);

EXTERN_C
NTSYSAPI NTSTATUS NTAPI
ZwProtectVirtualMemory(
	IN HANDLE ProcessHandle,
	IN PVOID* BaseAddress, /* THIS IS ACTUALLY AN IN_OUT */
	IN SIZE_T* NumberOfBytesToProtect,
	IN ULONG NewAccessProtection,
	OUT PULONG OldAccessProtection
);

EXTERN_C
NTSTATUS NTAPI
MmCopyVirtualMemory
(
	PEPROCESS SourceProcess,
	PVOID SourceAddress,
	PEPROCESS TargetProcess,
	PVOID TargetAddress,
	SIZE_T BufferSize,
	KPROCESSOR_MODE PreviousMode,
	PSIZE_T ReturnSize
);

EXTERN_C
NTSTATUS NTAPI
PsSuspendProcess
(
	PEPROCESS SourceProcess
);


EXTERN_C
NTSTATUS NTAPI
PsResumeProcess
(
	PEPROCESS SourceProcess
);

NTSYSAPI ULONG RtlRandomEx(
	PULONG Seed
);

EXTERN_C PPEB NTAPI PsGetProcessPeb(PEPROCESS Process);

EXTERN_C
PVOID
NTAPI
PsGetProcessWow64Process(IN PEPROCESS Process);

EXTERN_C LPSTR PsGetProcessImageFileName(PEPROCESS Process);

EXTERN_C
PVOID
NTAPI
RtlImageDirectoryEntryToData(
	PVOID ImageBase,
	BOOLEAN MappedAsImage,
	USHORT DirectoryEntry,
	PULONG Size
);

EXTERN_C
NTKERNELAPI
PVOID
NTAPI
PsGetThreadWin32Thread(__in PETHREAD Thread);

EXTERN_C
NTKERNELAPI
PVOID
NTAPI
PsSetThreadWin32Thread
(_Inout_ PETHREAD Thread,
	_In_opt_ PVOID Win32Thread,
	_In_opt_ PVOID OldWin32Thread);





#pragma endregion

// include Main Stuff
#include "KernelStringEncrypter.h"
#include "MemoryAllocator.h"
#include "KernelPassanger.hpp"
#include "KernelMath.hpp"
#include "MemoryManipulation.hpp"
#include "KernelLogger.h"


#include "KernelUtilities.h"
#include "KernelProcess.h"
#include "KernelRegistery.h"
#include "KernelModule.h"
#include "KernelThread.h"
#include "KernelFile.h"
#include "KernelGDI.h"
#include "KernelBeep.h"

#include "KernelImporter.h"
#include "VAD.h"

// Kernel Functions
#include "FreeMemoryFinder.h"
#include "DllMapper.h"
#include "NotifyRoutines.h"
#include "ObRegisterCallbacks.h"
#include "ProcessDumper.h"
#include "RegistrySpoofer.h"
#include "AntiCheatBypass.h"
#include "HWIDSpoofers.h"
#include "UnloadedCleaner.h"

#pragma region VSFUNCS

#define EXECUTE_EVERY(TIME,EXECUTE) \
		static u64 LastCheck = *((volatile u64*)(SharedSystemTime)); \
		if ((KernelUtilities::Now() - LastCheck) > TIME){ \
			KeQuerySystemTime(&LastCheck);\
			EXECUTE;\
		}

#pragma endregion
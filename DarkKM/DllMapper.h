#pragma once

class Process;

typedef struct LoaderParams86
{
	LoaderParams86()
	{
		RtlZeroMemory(this, sizeof(LoaderParams86));
	}
	void* operator new(size_t size)
	{
		return ALLOCATE( sizeof(LoaderParams86));
	}
	void operator delete(void* ptr)
	{
		FREE(ptr);
	}
	DWORD pImageBase;
	DWORD pNtHeaders;
	DWORD pBaseRelocation;
	DWORD pImportDescriptor;
	DWORD fLoadLibrary;
	DWORD fGetProcAddress;
	BOOLEAN Inited;
};


typedef struct LoaderParams64
{
	LoaderParams64()
	{
		RtlZeroMemory(this, sizeof(LoaderParams64));
	}
	void* operator new(size_t size)
	{
		return ALLOCATE( sizeof(LoaderParams64));
	}
	void operator delete(void* ptr)
	{
		FREE(ptr);
	}
	DWORD64 pImageBase;
	DWORD64 pNtHeaders;
	DWORD64 pBaseRelocation;
	DWORD64 pImportDescriptor;
	DWORD64 fLoadLibrary;
	DWORD64 fGetProcAddress;
	BOOLEAN Inited;
};

typedef enum _DLLMemoryType
{
	ZwAllocate,
	VadAllocate,
	FreeMemory
} DLLMemoryType;


class Mapper
{
public:

	static NTSTATUS Mapper::ManuelMapImage86(
		PEPROCESS MmGameHandle,
		KernelFile* pFile,
		DWORD LoadLibrary,
		DWORD GetProcAddress,
		DWORD& ImageAddress,
		DWORD& ParametersAddress,
		DWORD& LoaderAddress
	);

	static NTSTATUS Mapper::ManuelMapImage64(
		PEPROCESS MmGameHandle,
		KernelFile* pFile,
		LONGLONG LoadLibrary,
		LONGLONG GetProcAddress,
		LONGLONG& ImageAddress,
		LONGLONG& ParametersAddress,
		LONGLONG& LoaderAddress
	);

	static NTSTATUS Mapper::FixProtection(
		HANDLE ZwGameHandle,
		uint64_t TempAddress);

	static NTSTATUS Inject(
		const wchar_t* ProcessName,
		DLLMemoryType MemoryType,
		KernelFile* Dll,
		DWORD64 EmptyAddress, 
		DWORD SleepTime);
};

// Get Process 
// Get handles
// Get Memory
// Write DLl
// Call Dll
// Clean up



//#define PROJECT_APEKS_ENABLED 1
//#define x64_PROJECT 1
//
////#include "Images\TestDllx64_Dbgview.h"
//#include "Images\ApeksDLLImage.h"
//
//#define pBuffer rawData // Used by ManuelMapImage64
//#define ALLOCATE_OWN_MEMORY 0 // Will use ZwAllocateVirtualMemory if its true
//#define SLEEP_TIME 15000
//
//#define EXECUTABLE_NAME L"r5apex.exe"
//#define TARGET_IMAGE_NAME L"r5apex.exe"  //r5apex.exe AlisInjector(x64).exe r5apex.exe
//#define APEX_TARGET_IMAGE_EMPTY_ADDRESS 0x1890000 // 0x2220000 // Will use this if ALLOCATE_OWN_MEMORY is 0 ********[FoundEmptySpace]::R5Apex.exe-[0x7FF73E0D0000] - EmptyStart: 0x7FF73F9C30BE  EmptySize : 0x7F5A2 Protection 4
//
//#define APEX_TARGET_CALL_ADDRESS 0x1407049CC - 0x140000000  // MD5_PseudoRandom  E8 ? ? ? ? 48 81 C4 ? ? ? ? 41 5E 5E C3  - 3.0.0 - enable_three_weapons 
//#define APEX_TARGET_ORIGINAL_CALL 0x1408AC6D0 - 0x140000000

//***********************//
// 32 Bit Normal Injection -- Uses Allocates Memory - Uses CreateThread - CSGO
//***********************//

//#include "Images\CSGODLL.h" // CSGODLL
//
//#define x86_PROJECT 1
//#define pBuffer rawData // Used by ManuelMapImage64
//
//
//#define EXECUTABLE_NAME L"csgo.exe"
//#define TARGET_IMAGE_NAME L"client_panorama.dll"  // ReClassEx r5apex AlisInjector(x64) r5apex
//#define SLEEP_TIME 20000


//***********************//
// 32 Bit Normal Injection -- Uses Empty Address - Uses CreateThread - CSGO
//***********************//
//
//#include "Images\CSGODLL.h" // CSGODLL
//
//#define x86_PROJECT 1
//#define pBuffer rawData // Used by ManuelMapImage64
//
//
//#define EXECUTABLE_NAME L"csgo.exe"
//#define TARGET_IMAGE_NAME L"client_panorama.dll"  // ReClassEx r5apex AlisInjector(x64) r5apex
//#define SLEEP_TIME 20000
//
//#define PROJECT_CSGO_ENABLED 1
//#define CSGO_TARGET_IMAGE_EMPTY_ADDRESS 0x4D6F000  


//***********************//
// 32 Bit Normal Injection -- Uses Empty Address - Uses CreateThread - BF3
//***********************//

//#include "Images\Bf3DLL.h" // CSGODLL
//
//#define x86_PROJECT 1
//#define pBuffer rawData // Used by ManuelMapImage64
//
//#define EXECUTABLE_NAME L"bf3.exe"
//#define TARGET_IMAGE_NAME L"bf3.exe"  // ReClassEx r5apex AlisInjector(x64) r5apex
//#define SLEEP_TIME 20000
//
//#define PROJECT_BF3_ENABLED 1
//#define BF3_TARGET_IMAGE_EMPTY_ADDRESS 0x1E4F000  


//***********************//
// 64 Bit Normal Injection TEST -- Allocates Memory Uses CreateThread
//***********************//

//#include "Images\TestDll.h"
//#define x64_PROJECT 1
//
//#define pBuffer rawData // Used by ManuelMapImage64
//
//#define EXECUTABLE_NAME L"ReaClaass64s.exe"
//#define TARGET_IMAGE_NAME L"ReaClaass64s.exe"  // ReClassEx ReaClaass64s r5apex.exe AlisInjector(x64).exe r5apex.exe
//#define SLEEP_TIME 2000

//***********************//
// 32 Bit Normal Injection TEST -- Allocates Memory Uses CreateThread
//***********************//

//#include "Images\TestDllx86.h" //
//
//#define x86_PROJECT 1
//#define pBuffer rawData // Used by ManuelMapImage64
//
//#define EXECUTABLE_NAME L"ReClassEx.exe"
//#define TARGET_IMAGE_NAME L"ReClassEx.exe"  // ReClassEx r5apex AlisInjector(x64) r5apex
//#define SLEEP_TIME 2000

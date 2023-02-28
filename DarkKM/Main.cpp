#include "Main.h"

extern "C" int _fltused = 0;

#define PROTECTION_DRIVER_SUCCES_MSG L"1jgoFQXLkU"
//#pragma section(".customsection",read,write,execute)
//__declspec(allocate(".text")) char Test[26214400];

NTSTATUS DumpCallBacks()
{
	DEBUG_PRINT("[%ws]::%hs:: Waiting \r\n", PROJECT_NAME, __FUNCTION__);


	KernelUtilities::Sleep(120000);

	DEBUG_PRINT("[%ws]::%hs:: Dumping \r\n", PROJECT_NAME, __FUNCTION__);

	ObRegisterCallbacks::DumpAllProcessCallbacks();
	ObRegisterCallbacks::DumpAllThreadCallbacks();

	DEBUG_PRINT("[%ws]::%hs:: Dumped \r\n", PROJECT_NAME, __FUNCTION__);

	return STATUS_SUCCESS;
}

NTSTATUS DumpProcess()
{

	ProcessDumper::DumpAllProcessNamed(L"ModernWarfare.exe", L"\\SystemRoot\\aaDumped", 60000);

	//ProcessDumper::DumpAllProcessNamed(L"PUBGLite-Win64-Shipping.exe", L"\\SystemRoot\\aaDumped", 60000); 

	DEBUG_PRINT("[%ws]::%hs:: Dumped \r\n", PROJECT_NAME, __FUNCTION__);

	return STATUS_SUCCESS;
}

NTSTATUS SuspendProcess()
{
	DEBUG_PRINT("[%ws]::%hs:: Waiting \r\n", PROJECT_NAME, __FUNCTION__);

	KernelUtilities::Sleep(180000);

	DEBUG_PRINT("[%ws]::%hs:: Suspending \r\n", PROJECT_NAME, __FUNCTION__);

	KernelProcess::SuspendAllProcessesNamed(L"ModernWarfare.exe");
	KernelProcess::SuspendAllProcessesNamed(L"Battle.net.exe");

	//AntiCheatBypass::DisablePUBGLITE(L"PUBGLite-Win64-Shipping.exe", L"xhunter1.sys",L"380800");

	DEBUG_PRINT("[%ws]::%hs:: Suspended \r\n", PROJECT_NAME, __FUNCTION__);

	return STATUS_SUCCESS;
}


inline NTSTATUS Protector()
{
	DEBUG_PRINT("[%ws]::%hs:: Initing Started ! \r\n", PROJECT_NAME, __FUNCTION__);
	
	KernelUtilities::Sleep(2000);
	
	auto LastDriverName = new DK::DString<WCHAR>();

	KernelImporter::BuildNo = 0;
	auto status = KernelUtilities::BBGetBuildNO(&KernelImporter::BuildNo);

	if (!NT_SUCCESS(status))
		goto exit;

	DEBUG_PRINT("[%ws]::%hs:: Running On Build %d \r\n", PROJECT_NAME, __FUNCTION__, KernelImporter::BuildNo);

	status = UnloadedCleaner::ClearLastUnloadedDriver(LastDriverName);

	if (!NT_SUCCESS(status))
		goto exit;

	status = UnloadedCleaner::ClearPiDDBCacheTable(LastDriverName);

	if (!NT_SUCCESS(status))
		goto exit;

	status = RegistrySpoofer::SpoofHWIDRegistrys();

	if (!NT_SUCCESS(status))
		goto exit;

	//status = HWIDSpoofers::HookWmipRawSMBiosTableHandler();

	//if (!NT_SUCCESS(status))
	//	goto exit;

	status = HWIDSpoofers::StaticHDD_HWIDHook();

	if (!NT_SUCCESS(status))
		goto exit;

	//status = HWIDSpoofers::DynamicHDD_HWIDHook();

	//if (!NT_SUCCESS(status))
	//	goto exit;

exit:
	delete LastDriverName;
	DEBUG_PRINT("[%ws]::%hs:: Initing Finished ! Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
	return status;
}

NTSTATUS TEST_UM_GetModuleExport()
{	

	DEBUG_PRINT("[%ws]::%hs:: Started  \r\n", PROJECT_NAME, __FUNCTION__);
	
	//auto Process = KernelProcess::GetPEPProcess(L"ReaClaass64s.exe");
	auto Process = KernelProcess::GetPEPProcess(L"ReClassEx.exe");

	if (!Process)
	{
		DEBUG_PRINT("[%ws]::%hs:: Process Not Found !! \r\n", PROJECT_NAME, __FUNCTION__);
		return STATUS_NO_TOKEN;
	}
	
	PVOID isWow64 = PsGetProcessWow64Process(Process);

	bool IsX64 = isWow64 == 0x0 ? true : false;

	DEBUG_PRINT("[%ws]::%hs:: Process %X IsX64 %X\r\n", PROJECT_NAME, __FUNCTION__, Process, IsX64);

	KeAttachProcess(Process); 

	PVOID pGetProcAddress = KernelModule::GetUserModuleExport(L"KERNEL32.DLL", "GetProcAddress", Process); //KERNEL32.DLL
	PVOID pLoadLibrary = KernelModule::GetUserModuleExport(L"KERNEL32.DLL", "LoadLibraryA", Process);


	if (IsX64)
		DEBUG_PRINT("[%ws]::%hs:: pGetProcAddress %llX \r\n", PROJECT_NAME, __FUNCTION__, pGetProcAddress);
	else
		DEBUG_PRINT("[%ws]::%hs:: pGetProcAddress %X \r\n", PROJECT_NAME, __FUNCTION__, pGetProcAddress);

	if (IsX64)
		DEBUG_PRINT("[%ws]::%hs:: pLoadLibrary %llX \r\n", PROJECT_NAME, __FUNCTION__, pLoadLibrary);
	else
		DEBUG_PRINT("[%ws]::%hs:: pLoadLibrary %X \r\n", PROJECT_NAME, __FUNCTION__, pLoadLibrary);

	KeDetachProcess();
	ObDereferenceObject(Process);

	DEBUG_PRINT("[%ws]::%hs:: Finished  \r\n", PROJECT_NAME, __FUNCTION__);

	return STATUS_SUCCESS;
}

NTSTATUS TEST_KM_GetModuleExport()
{

	DEBUG_PRINT("[%ws]::%hs:: Started  \r\n", PROJECT_NAME, __FUNCTION__);

	auto Process = KernelProcess::GetPEPProcess(L"winlogon.exe");

	if (!Process)
	{
		DEBUG_PRINT("[%ws]::%hs:: Process Not Found !! \r\n", PROJECT_NAME, __FUNCTION__);
		return STATUS_NO_TOKEN;
	}

	PVOID isWow64 = PsGetProcessWow64Process(Process);

	bool IsX64 = isWow64 == 0x0 ? true : false;

	DEBUG_PRINT("[%ws]::%hs:: Process %X IsX64 %X\r\n", PROJECT_NAME, __FUNCTION__, Process, IsX64);

	KeAttachProcess(Process);

	auto DxgkGetDeviceState = KernelModule::GetKernelModuleExport("dxgkrnl.sys", "DxgkGetDeviceState");


	DEBUG_PRINT("[%ws]::%hs:: DxgkGetDeviceState %llX \r\n", PROJECT_NAME, __FUNCTION__, DxgkGetDeviceState);

	KeDetachProcess();
	ObDereferenceObject(Process);

	DEBUG_PRINT("[%ws]::%hs:: Finished  \r\n", PROJECT_NAME, __FUNCTION__);

	return STATUS_SUCCESS;
}

NTSTATUS TEST_KM_GetModuleWithSignature()
{

	DEBUG_PRINT("[%ws]::%hs:: Started  \r\n", PROJECT_NAME, __FUNCTION__);

	auto Process = KernelProcess::GetPEPProcess(L"winlogon.exe");

	if (!Process)
	{
		DEBUG_PRINT("[%ws]::%hs:: Process Not Found !! \r\n", PROJECT_NAME, __FUNCTION__);
		return STATUS_NO_TOKEN;
	}

	KeAttachProcess(Process);

	// DxgkGetDeviceState for win 10 - > 17763.1
	auto pDxgkGetDeviceState = KernelModule::FindPatternInModule("dxgkrnl.sys", (BYTE*)"\xE8\x00\x00\x00\x00\x48\x8B\x4F\x48\x48\x8D\x54\x24\x00\x44", (char*)"x????xxxxxxxx?x");

	DEBUG_PRINT("[%ws]::%hs:: pDxgkGetDeviceState %llX \r\n", PROJECT_NAME, __FUNCTION__, pDxgkGetDeviceState);

	if (pDxgkGetDeviceState) {
		
		auto DxgkGetDeviceState = KernelMemory::follow_call<uint8_t*>(pDxgkGetDeviceState);

		DEBUG_PRINT("[%ws]::%hs:: DxgkGetDeviceState %llX \r\n", PROJECT_NAME, __FUNCTION__, DxgkGetDeviceState);
	}

	KeDetachProcess();
	ObDereferenceObject(Process);

	DEBUG_PRINT("[%ws]::%hs:: Finished  \r\n", PROJECT_NAME, __FUNCTION__);

	return STATUS_SUCCESS;
}


extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	
	UNREFERENCED_PARAMETER(pDriverObject);
	UNREFERENCED_PARAMETER(pRegistryPath);

	MemoryAllocator::Init();

	PWORK_QUEUE_ITEM WorkItem = (PWORK_QUEUE_ITEM)ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));

	if (!WorkItem)
	{
		DEBUG_PRINT("[%ws]::%hs:: Erroor \r\n", PROJECT_NAME, __FUNCTION__);
		return STATUS_SUCCESS;
	}

	ExInitializeWorkItem(WorkItem, reinterpret_cast<PWORKER_THREAD_ROUTINE>(TEST_KM_GetModuleWithSignature), WorkItem);
	ExQueueWorkItem(WorkItem, DelayedWorkQueue);

	//auto hMain = KernelUtilities::CreateSystemThread(reinterpret_cast<PKSTART_ROUTINE>(TEST_KM_GetModuleWithSignature), NULL);
	//KernelUtilities::CloseHandle(hMain);;
	
	return STATUS_SUCCESS;
}


//#include "ObRegisterCallbacks.h"
//#include "Utilities.h"
//#include "ProtectionDriver.h"
//#include "Logger.h"
//
//#define PROTECTION_DRIVER_SUCCES_MSG L"1jgoFQXLkU"
//
//
//CALLBACK_HOOK ProtectionDriver::Protection_Hook;
//
//
//WCHAR ProtectionDriver::m_process_name[64];
//UNICODE_STRING ProtectionDriver::CallBackHookAltitute;
//
//PEPROCESS  ProtectionDriver::m_process = NULL;
//HANDLE     ProtectionDriver::m_processId = { 0 };
//
//ProtectionDriver::ProtectionDriver()
//{
//}
//
//
//ProtectionDriver::~ProtectionDriver()
//{
//}
//
//NTSTATUS ProtectionDriver::InitProtectedProcess()
//{
//	m_processId = KernelProcess::GetProcessId(m_process_name);
//
//	DEBUG_PRINT("[%ws]::%hs  m_process_name %ws m_processId %d \r\n", PROJECT_NAME, __FUNCTION__, m_process_name, m_processId);
//
//	NTSTATUS NTStatus;
//	NTStatus = PsLookupProcessByProcessId(m_processId, &m_process);
//
//	if (!NT_SUCCESS(NTStatus))
//		DEBUG_PRINT("[%ws]::%hs Error: To Init m_process %X \r\n", PROJECT_NAME, __FUNCTION__, NTStatus, m_process);
//	else
//		DEBUG_PRINT("[%ws]::%hs Updated m_process 0x%llX \r\n", PROJECT_NAME, __FUNCTION__, m_process);
//
//	return NTStatus;
//}
//
//void ProtectionDriver::UnHookProtection()
//{
//
//	DEBUG_PRINT("[%ws]::%hs Protected Process Closed - Restoring Callbacks... \r\n", PROJECT_NAME, __FUNCTION__);
//
//	NTSTATUS status = STATUS_SUCCESS;
//
//	status = ObRegisterCallbacks::ChangeObProcessCallbacks(CallBackHookAltitute.Buffer, false, &Protection_Hook);
//
//	if (!NT_SUCCESS(status))
//	{
//		DEBUG_PRINT("[%ws]::%hs:: Error: Restore Protection ChangeObProcessCallbacks. Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
//		return;
//	}
//
//	ObDereferenceObject(m_process);
//
//	DEBUG_PRINT("[%ws]::%hs Successfully Restored Protection Callback... \r\n", PROJECT_NAME, __FUNCTION__);
//}
//
//OB_PREOP_CALLBACK_STATUS ProtectionDriver::ProtectionPreProcessCallback(void* RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation)
//{
//	{	
//		// Initilize Protected Process
//		if (!m_processId)
//		{
//			if (!IS_NT_SUCCESS(InitProtectedProcess()))
//				goto Exit;
//		}
//
//		// This Means Process Was Closed
//		if (KernelUtilities::BBCheckProcessStatus(m_process) == 0)
//		{
//			UnHookProtection();
//			goto Exit;
//		}
//
//		if (OperationInformation->Object != m_process) // Only Block Protected Process
//			goto Exit;
//
//		if (OperationInformation->Object == PsGetCurrentProcess()) // Allow Driver To Access Our Shit
//			goto Exit;
//
//		if (PsGetCurrentProcessId() == m_processId) // Allow Launcher to Access It Self
//			goto Exit;
//
//		if (PsGetCurrentProcessId() == KernelProcess::GetProcessId(L"lsass.exe") || // Allow lsass to Access 
//			PsGetCurrentProcessId() == KernelProcess::GetProcessId(L"csrss.exe")) // Allow csrss to Access 
//			goto Exit;
//
//
//		if (OperationInformation->Operation == OB_OPERATION_HANDLE_CREATE)
//			OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = (SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION);
//		else
//			OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess = (SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION);
//
//
//		//DEBUG_PRINT("[%ws]::%hs Blocked  TdLauncherProcessId %d \r\n", PROJECT_NAME, __FUNCTION__, PsGetCurrentProcessId());
//
//		return OB_PREOP_SUCCESS;
//
//	}
//
//Exit:
//	return Protection_Hook.PreProcessOriginal(RegistrationContext, OperationInformation);
//}
//
//OB_PREOP_CALLBACK_STATUS ProtectionDriver::ProtectionPreThreadCallback(void* RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation)
//{
//	{
//		DEBUG_PRINT("[%ws]::%hs Blocked  TdLauncherProcessId %d \r\n", PROJECT_NAME, __FUNCTION__, PsGetCurrentProcessId());
//
//		// Initilize Protected Process
//		if (!m_processId)
//		{
//			if (!NT_SUCCESS(InitProtectedProcess()))
//				goto Exit;
//		}
//
//		// This Means Process Was Closed
//		if (KernelUtilities::BBCheckProcessStatus(m_process) == 0)
//		{
//			UnHookProtection();
//			goto Exit;
//		}
//
//		if (OperationInformation->Object != m_process) // Only Block Protected Process
//			goto Exit;
//
//		if (PsGetCurrentProcessId() == m_processId) // Allow Main Launcher to Access Our Shit
//			goto Exit;
//
//		if (OperationInformation->Object == PsGetCurrentProcess()) // Allow Driver To Access Our Shit
//			goto Exit;
//
//		if (PsGetCurrentProcessId() == KernelProcess::GetProcessId(L"lsass.exe") || // Allow lsass to Access 
//			PsGetCurrentProcessId() == KernelProcess::GetProcessId(L"csrss.exe")) // Allow csrss to Access 
//			goto Exit;
//
//
//		if (OperationInformation->Operation == OB_OPERATION_HANDLE_CREATE)
//			OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = (SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION);
//		else
//			OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess = (SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION);
//
//
//		//DEBUG_PRINT("[%ws]::%hs Blocked  TdLauncherProcessId %d \r\n", PROJECT_NAME, __FUNCTION__, PsGetCurrentProcessId());
//
//		return OB_PREOP_SUCCESS;
//
//	}
//Exit:
//	return Protection_Hook.PreThreadOriginal(RegistrationContext, OperationInformation);
//}
//
//NTSTATUS ProtectionDriver::InitCallBacks() 
//{
//	NTSTATUS status = STATUS_SUCCESS;
//	// ******************************************* 
//	// Read The Current Process Name
//	// ******************************************* 
//	
//	//UNICODE_STRING LauncherName;
//	////status = RegistryManager::ReciveSignalFromUserMode(&LauncherName);
//
//	//if (!NT_SUCCESS(status))
//	//{
//	//	DEBUG_PRINT("[%ws]::%hs:: Error: ReadRegistryString. Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
//	//	return status;
//	//}
//
//	//DEBUG_PRINT("[%ws]::%hs:: Current Process Name: %ls Length: %d MaxLenght: %d \r\n", PROJECT_NAME, __FUNCTION__, LauncherName.Buffer, LauncherName.Length, LauncherName.MaximumLength);
//	//
//	//if (wcsstr(LauncherName.Buffer, L"DontProtect")) 
//	//{
//	//	DEBUG_PRINT("[%ws]::%hs:: DontProtect.exe Disabling ProtectionDriver \r\n", PROJECT_NAME, __FUNCTION__);
//	//	//status = RegistryManager::SendSignalToUserMode(PROTECTION_DRIVER_SUCCES_MSG);
//	//	return status;
//	//}
//	//
//	/*RtlZeroMemory(m_process_name, sizeof(m_process_name));
//	RtlCopyMemory(m_process_name, LauncherName.Buffer, LauncherName.Length);*/
//
//	// ******************************************* 
//	// Set Protection Driver Callbacks
//	// ******************************************* 
//
//	Protection_Hook.MyPreProcess = ProtectionPreProcessCallback;
//	Protection_Hook.MyPreThread = ProtectionPreThreadCallback;
//
//	// 1809
//	if (ObRegisterCallbacks::GetObProcessCallBack(L"328010") != nullptr)
//		RtlInitUnicodeString(&CallBackHookAltitute, L"328010");
//
//	if (CallBackHookAltitute.Length == 0)
//	{
//		DEBUG_PRINT("[%ws]::%hs:: Error: Cant Find Any Altitute: %wZ \r\n", PROJECT_NAME, __FUNCTION__, &CallBackHookAltitute);
//		return STATUS_ACCESS_DENIED;
//	}
//	
//	DEBUG_PRINT("[%ws]::%hs:: CallBackHookAltitute: %wZ \r\n", PROJECT_NAME, __FUNCTION__, &CallBackHookAltitute);
//
//	status = ObRegisterCallbacks::ChangeObProcessCallbacks(CallBackHookAltitute.Buffer, true, &Protection_Hook);
//
//	if (!NT_SUCCESS(status))
//	{
//		DEBUG_PRINT("[%ws]::%hs:: Error: Protection ChangeObProcessCallbacks. Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
//		return status;
//	}
//
//	// ******************************************* 
//	// Send Launcher Signal That Callbacks were Succesfull
//	// ******************************************* 
//
//	//if (NT_SUCCESS(status))
//		//status = RegistryManager::SendSignalToUserMode(PROTECTION_DRIVER_SUCCES_MSG);
//
//	if (!NT_SUCCESS(status))
//	{
//		DEBUG_PRINT("\n[%ws]::%hs:: Error SetRegistryString. Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
//		return status;
//	}
//
//	return status;
//}
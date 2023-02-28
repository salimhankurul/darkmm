#include "Main.h"

ObRegisterCallbacks::ObRegisterCallbacks()
{
}


ObRegisterCallbacks::~ObRegisterCallbacks()
{
}


CALLBACK_HOOK ObRegisterCallbacks::Game_Hook;
WCHAR* ObRegisterCallbacks::m_altitude;

#define CALLBACKS_DEBUG //DEBUG_PRINT

CALLBACK_ENTRY_ITEM* ObRegisterCallbacks::GetObProcessCallBack(const WCHAR* Altitute)
{
	auto pProcessType = PsProcessType;

	if (pProcessType)
	{
		POBJECT_TYPE pObjectType = *pProcessType;
		if (MmIsAddressValid(reinterpret_cast<void*>(pObjectType)))
		{	
			UINT64 callbackListOffset = 0;

			auto status = GetCallBackListOffset(callbackListOffset);

			if (!NT_SUCCESS(status))
			{
				CALLBACKS_DEBUG("[%ws]::%hs:: Error: To Get GetCallBackListOffset Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
				return nullptr;
			}

			if (callbackListOffset && MmIsAddressValid(reinterpret_cast<void*>(reinterpret_cast<UINT64>(pObjectType) + callbackListOffset)))
			{
				LIST_ENTRY* callbackList = reinterpret_cast<LIST_ENTRY*>(reinterpret_cast<UINT64>(pObjectType) + callbackListOffset);

				if (!callbackList || !MmIsAddressValid(callbackList))
					return nullptr;

				if (callbackList->Flink && MmIsAddressValid(callbackList->Flink))
				{
					CALLBACK_ENTRY_ITEM* firstCallback = reinterpret_cast<CALLBACK_ENTRY_ITEM*>(callbackList->Flink);
					CALLBACK_ENTRY_ITEM* currentCallback = firstCallback;

					do
					{
						if (currentCallback && MmIsAddressValid(currentCallback) && MmIsAddressValid(currentCallback->CallbackEntry))
						{
							UNICODE_STRING altitudeString = currentCallback->CallbackEntry->Altitude;

							if (wcsstr(altitudeString.Buffer, Altitute))				
								if (currentCallback->PreOperation && MmIsAddressValid(currentCallback->PreOperation)) {
									//CALLBACKS_DEBUG("[%ws]::%hs:: Successfully To Find Altitute: %ws Belongs To: %s\r\n", PROJECT_NAME, __FUNCTION__, Altitute,KernelUtilities::GetKernelModuleAtAddress((uint64_t)currentCallback->PreOperation));
									return currentCallback;
								}					
						}
						currentCallback = reinterpret_cast<CALLBACK_ENTRY_ITEM*>(currentCallback->CallbackList.Flink);
					} while (currentCallback != firstCallback);

					CALLBACKS_DEBUG("[%ws]::%hs:: Error: To Find Altitute: %ws \r\n", PROJECT_NAME, __FUNCTION__, Altitute);

					return nullptr;
				}
			}
		}
	}

	return nullptr;
}

CALLBACK_ENTRY_ITEM* ObRegisterCallbacks::GetObThreadCallBack(const WCHAR* Altitute)
{
	auto pThreadType = PsThreadType;

	if (pThreadType)
	{
		POBJECT_TYPE pObjectType = *pThreadType;

		if (MmIsAddressValid(reinterpret_cast<void*>(pObjectType)))
		{
			NTSTATUS found = 0xC000008B5;
			UINT64 callbackListOffset = 0;

			auto status = GetCallBackListOffset(callbackListOffset);

			if (!IS_NT_SUCCESS(status))
			{
				CALLBACKS_DEBUG("[%ws]::%hs:: Error: To Get GetCallBackListOffset Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
				return nullptr;
			}

			if (callbackListOffset && MmIsAddressValid(reinterpret_cast<void*>(reinterpret_cast<UINT64>(pObjectType) + callbackListOffset)))
			{
				LIST_ENTRY* callbackList = reinterpret_cast<LIST_ENTRY*>(reinterpret_cast<UINT64>(pObjectType) + callbackListOffset);

				if (!callbackList || !MmIsAddressValid(callbackList))
					return nullptr;

				if (callbackList->Flink && MmIsAddressValid(callbackList->Flink))
				{
					CALLBACK_ENTRY_ITEM* firstCallback = reinterpret_cast<CALLBACK_ENTRY_ITEM*>(callbackList->Flink);
					CALLBACK_ENTRY_ITEM* currentCallback = firstCallback;

					do
					{
						if (currentCallback && MmIsAddressValid(currentCallback) && MmIsAddressValid(currentCallback->CallbackEntry))
						{
							UNICODE_STRING altitudeString = currentCallback->CallbackEntry->Altitude;

							if (wcsstr(altitudeString.Buffer, Altitute))
								if (currentCallback->PreOperation && MmIsAddressValid(currentCallback->PreOperation))
									return currentCallback;

								
							
						}

						currentCallback = reinterpret_cast<CALLBACK_ENTRY_ITEM*>(currentCallback->CallbackList.Flink);

					} while (currentCallback != firstCallback);

					CALLBACKS_DEBUG("[%ws]::%hs:: Error: To Find Altitute: %ws \r\n", PROJECT_NAME, __FUNCTION__, Altitute);
					return nullptr;
				}
			}
		}
	}
	return nullptr;
}

// ******************************************* 
// Main Functions
// ******************************************* 

NTSTATUS ObRegisterCallbacks::GetCallBackListOffset(UINT64& Offset)
{
	auto pProcessType = PsProcessType;

	if (pProcessType)
	{
		POBJECT_TYPE pObjectType = *pProcessType;

		if (pObjectType)
		{
			if (MmIsAddressValid(reinterpret_cast<void*>(pObjectType)))
			{
				for (int i = 0xF8; i > 0; i -= 8)
				{
					UINT64 first = *reinterpret_cast<UINT64*>(reinterpret_cast<UINT64>(pObjectType) + i);
					UINT64 second = *reinterpret_cast<UINT64*>(reinterpret_cast<UINT64>(pObjectType) + (i + 8));

					if (first && MmIsAddressValid(reinterpret_cast<void*>(first)) && second && MmIsAddressValid(reinterpret_cast<void*>(second)))
					{
						UINT64 test1First = *reinterpret_cast<UINT64*>(first + 0x0);
						UINT64 test1Second = *reinterpret_cast<UINT64*>(first + 0x8);

						if (test1First && MmIsAddressValid(reinterpret_cast<void*>(test1First)) && test1Second && MmIsAddressValid(reinterpret_cast<void*>(test1Second)))
						{
							UINT64 testObjectType = *reinterpret_cast<UINT64*>(first + 0x20);

							if (testObjectType == reinterpret_cast<UINT64>(pObjectType))
							{
								Offset = static_cast<UINT64>(i);
								return STATUS_SUCCESS;
							}								
						}
					}
				}
			} else
				return 0xC000008B1;
		}
		else 
			return 0xC000008B2;
	} 
	else 
		return 0xC000008B3;

	return 0xC000008B4;
}

NTSTATUS ObRegisterCallbacks::ChangeObProcessCallbacks(const WCHAR* Altitute, bool Hook, CALLBACK_HOOK* mycallback)
{

	if (!Altitute || !mycallback || !mycallback->MyPreProcess)
	{
		CALLBACKS_DEBUG("[%ws]::%hs:: Error: Wrong Params: %ws \r\n", PROJECT_NAME, __FUNCTION__, Altitute);
		return STATUS_ACCESS_DENIED;
	}

	CALLBACK_ENTRY_ITEM* ProcessCallbacks = GetObProcessCallBack(Altitute);
	
	if (ProcessCallbacks)
	{
		if (ProcessCallbacks->PreOperation && MmIsAddressValid(ProcessCallbacks->PreOperation))
		{
			CALLBACKS_DEBUG("[%ws]::%hs %ws Found -> %llX \r\n", PROJECT_NAME, __FUNCTION__, Altitute, ProcessCallbacks);
			CALLBACKS_DEBUG("[%ws]::%hs PreOperation: %llX -> %llX \r\n", PROJECT_NAME, __FUNCTION__, ProcessCallbacks->PreOperation, mycallback->MyPreProcess);
			CALLBACKS_DEBUG("[%ws]::%hs PostOperation: %llX -> %llX \r\n", PROJECT_NAME, __FUNCTION__, ProcessCallbacks->PostOperation, mycallback->MyPostProcess);

			if (Hook)
			{	
				mycallback->PreProcessOriginal = ProcessCallbacks->PreOperation;
				mycallback->PostProcessOriginal = ProcessCallbacks->PostOperation;

				if (mycallback->MyPreProcess)
					ProcessCallbacks->PreOperation = mycallback->MyPreProcess;

				if (mycallback->MyPostProcess)
					ProcessCallbacks->PostOperation = mycallback->MyPostProcess;

				CALLBACKS_DEBUG("[%ws]::%hs %ws Successfully ProcessCallback Hooked \r\n", PROJECT_NAME, __FUNCTION__, Altitute);
			}
			else
			{
	
				ProcessCallbacks->PreOperation = mycallback->PreProcessOriginal;
				ProcessCallbacks->PostOperation = mycallback->PostProcessOriginal;

				CALLBACKS_DEBUG("[%ws]::%hs %ws Successfully Restored \r\n", PROJECT_NAME, __FUNCTION__, Altitute);		
			}
				
			return STATUS_SUCCESS;
		}
	}
	
	CALLBACKS_DEBUG("[%ws]::%hs:: Cant find %ws callback is nullptr \r\n", PROJECT_NAME, __FUNCTION__, Altitute);

	return STATUS_ACCESS_DENIED;
}

NTSTATUS ObRegisterCallbacks::ChangeObThreadCallbacks(const WCHAR* Altitute, bool Hook, CALLBACK_HOOK* mycallback)
{

	if (!Altitute || !mycallback || !mycallback->MyPreThread)
	{
		CALLBACKS_DEBUG("[%ws]::%hs:: Error: Wrong Params: %ws \r\n", PROJECT_NAME, __FUNCTION__, Altitute);
		return STATUS_ACCESS_DENIED;
	}

	CALLBACK_ENTRY_ITEM* ThreadCallbacks = GetObThreadCallBack(Altitute);

	if (ThreadCallbacks)
	{
		if (ThreadCallbacks->PreOperation && MmIsAddressValid(ThreadCallbacks->PreOperation))
		{

			CALLBACKS_DEBUG("[%ws]::%hs %ws Found -> %llX \r\n", PROJECT_NAME, __FUNCTION__, Altitute, ThreadCallbacks);
			CALLBACKS_DEBUG("[%ws]::%hs PreOperation: %llX -> %llX \r\n", PROJECT_NAME, __FUNCTION__, ThreadCallbacks->PreOperation, mycallback->MyPreThread);
			CALLBACKS_DEBUG("[%ws]::%hs PostOperation: %llX -> %llX \r\n", PROJECT_NAME, __FUNCTION__, ThreadCallbacks->PostOperation, mycallback->MyPostThread);

			if (Hook)
			{
				mycallback->PreThreadOriginal = ThreadCallbacks->PreOperation;
				mycallback->PostThreadOriginal = ThreadCallbacks->PostOperation;

				if (mycallback->MyPreThread)
					ThreadCallbacks->PreOperation = mycallback->MyPreThread;

				if (mycallback->MyPostThread)
					ThreadCallbacks->PostOperation = mycallback->MyPostThread;


				CALLBACKS_DEBUG("[%ws]::%hs %ws Successfully ThreadCallback Hooked \r\n", PROJECT_NAME, __FUNCTION__, Altitute);
			}
			else
			{
	
				ThreadCallbacks->PreOperation = mycallback->PreThreadOriginal;
				ThreadCallbacks->PostOperation = mycallback->PostThreadOriginal;

				CALLBACKS_DEBUG("[%ws]::%hs %ws Successfully Restored \r\n", PROJECT_NAME, __FUNCTION__, Altitute);
			}

			return STATUS_SUCCESS;
		}
	}

	CALLBACKS_DEBUG("[%ws]::%hs:: Cant find %ws callback is nullptr \r\n", PROJECT_NAME, __FUNCTION__, Altitute);

	return STATUS_ACCESS_DENIED;
}


// ******************************************* 
// Debugging For ObCallbacks
// ******************************************* 

bool ObRegisterCallbacks::DumpAllProcessCallbacks()
{
	auto pProcessType = PsProcessType;

	if (pProcessType)
	{
		POBJECT_TYPE pObjectType = *pProcessType;
		if (MmIsAddressValid(reinterpret_cast<void*>(pObjectType)))
		{
			bool found = false;
			
			UINT64 callbackListOffset = 0;

			auto status = GetCallBackListOffset(callbackListOffset);

			if (!NT_SUCCESS(status))
			{
				CALLBACKS_DEBUG("\n[%ws]::%hs:: Error To Get GetCallBackListOffset Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
				return STATUS_NO_TOKEN;
			}
			if (callbackListOffset && MmIsAddressValid(reinterpret_cast<void*>(reinterpret_cast<UINT64>(pObjectType) + callbackListOffset)))
			{
				LIST_ENTRY* callbackList = reinterpret_cast<LIST_ENTRY*>(reinterpret_cast<UINT64>(pObjectType) + callbackListOffset);

				if (!callbackList || !MmIsAddressValid(callbackList))
					return false;

				if (callbackList->Flink && MmIsAddressValid(callbackList->Flink))
				{
					CALLBACK_ENTRY_ITEM* firstCallback = reinterpret_cast<CALLBACK_ENTRY_ITEM*>(callbackList->Flink);
					CALLBACK_ENTRY_ITEM* currentCallback = firstCallback;

					do
					{
						if (currentCallback && MmIsAddressValid(currentCallback) && MmIsAddressValid(currentCallback->CallbackEntry))
						{
							UNICODE_STRING altitudeString = currentCallback->CallbackEntry->Altitude;
							DEBUG_PRINT("[%ws]::%hs:: Altitude: %wZ\r\n", PROJECT_NAME, __FUNCTION__, &altitudeString);
							found = true;
						}

						currentCallback = reinterpret_cast<CALLBACK_ENTRY_ITEM*>(currentCallback->CallbackList.Flink);

					} while (currentCallback != firstCallback);
				}
			}
			return found;
		}
	}

	return false;
}

bool ObRegisterCallbacks::DumpAllThreadCallbacks()
{
	auto pThreadType = PsThreadType;

	if (pThreadType)
	{
		POBJECT_TYPE pObjectType = *pThreadType;

		if (MmIsAddressValid(reinterpret_cast<void*>(pObjectType)))
		{
			bool found = false;
			
			UINT64 callbackListOffset = 0;

			auto status = GetCallBackListOffset(callbackListOffset);

			if (!NT_SUCCESS(status))
			{
				CALLBACKS_DEBUG("\n[%ws]::%hs:: Error To Get GetCallBackListOffset Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, status);
				return STATUS_NO_TOKEN;
			}

			if (callbackListOffset && MmIsAddressValid(reinterpret_cast<void*>(reinterpret_cast<UINT64>(pObjectType) + callbackListOffset)))
			{
				LIST_ENTRY* callbackList = reinterpret_cast<LIST_ENTRY*>(reinterpret_cast<UINT64>(pObjectType) + callbackListOffset);

				if (!callbackList || !MmIsAddressValid(callbackList))
					return false;

				if (callbackList->Flink && MmIsAddressValid(callbackList->Flink))
				{
					CALLBACK_ENTRY_ITEM* firstCallback = reinterpret_cast<CALLBACK_ENTRY_ITEM*>(callbackList->Flink);
					CALLBACK_ENTRY_ITEM* currentCallback = firstCallback;

					do
					{
						if (currentCallback && MmIsAddressValid(currentCallback) && MmIsAddressValid(currentCallback->CallbackEntry))
						{
							UNICODE_STRING altitudeString = currentCallback->CallbackEntry->Altitude;
							DEBUG_PRINT("[%ws]::%hs:: Altitude: %wZ\r\n", PROJECT_NAME, __FUNCTION__, &altitudeString);
							found = true;
						}

						currentCallback = reinterpret_cast<CALLBACK_ENTRY_ITEM*>(currentCallback->CallbackList.Flink);

					} while (currentCallback != firstCallback);
				}
			}
			return found;
		}
	}
	return false;
}

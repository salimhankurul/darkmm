#include "Main.h"

#define MODULE_DEBUG(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, __VA_ARGS__)

NTSTATUS KernelModule::GetKernelModule(uint64_t ModuleAddress, KModule* pKModule)
{
    ULONG bytes = 0;

    NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, 0, bytes, &bytes);

    if (!bytes)
        return STATUS_ACCESS_DENIED;

    PSYSTEM_MODULE current_modules = (PSYSTEM_MODULE)ALLOCATE(bytes);  // 'ENON'

    if (!current_modules)
        return STATUS_ACCESS_DENIED;

    status = ZwQuerySystemInformation(SystemModuleInformation, current_modules, bytes, &bytes);

    if (!NT_SUCCESS(status))
    {
        MODULE_DEBUG("[%ws]::%hs:: NtQuerySystemInformation Error: 0x%x \r\n", PROJECT_NAME, __FUNCTION__, status);
        FREE(current_modules);
        return status;
    }

    status = STATUS_ACCESS_DENIED;

    for (auto i = 0u; i < current_modules->NumberOfModules; i++)
    {
        const auto current_module = &current_modules->Modules[i];

        if (!current_module)
            continue;

        uint64_t ImageStart = reinterpret_cast<uint64_t>(current_module->ImageBase);
        uint64_t ImageEnd = ImageStart + current_module->ImageSize;

        if (ModuleAddress >= ImageStart && ModuleAddress < ImageEnd)
        {
            const auto file_name = reinterpret_cast<const char*>(current_module->OffsetToFileName + current_module->FullPathName);

            MODULE_DEBUG("[%ws]::%hs:: Module Found for %llX \r\n", PROJECT_NAME, __FUNCTION__, ModuleAddress);
            MODULE_DEBUG("[%ws]::%hs::Base %llX Size %X \r\n", PROJECT_NAME, __FUNCTION__, ImageStart, current_module->ImageSize);
            MODULE_DEBUG("[%ws]::%hs:: Name %hs \r\n", PROJECT_NAME, __FUNCTION__, file_name);
            
            pKModule->Base.QuadPart = ImageStart;
            pKModule->Size.LowPart = current_module->ImageSize;
            pKModule->Name = new DK::DString<WCHAR>(L"%hs", file_name);
          
            status = STATUS_SUCCESS;
        }
    }

    FREE(current_modules);
    return status;
}

NTSTATUS KernelModule::GetKernelModule(const CHAR* ModuleName, KModule* pKModule)
{
  
    ULONG bytes = 0;

    NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, 0, bytes, &bytes);

    if (!bytes)
        return STATUS_ACCESS_DENIED;

    PSYSTEM_MODULE current_modules = (PSYSTEM_MODULE)ALLOCATE(bytes); // 'ENON'

    if (!current_modules)
        return STATUS_ACCESS_DENIED;

    status = ZwQuerySystemInformation(SystemModuleInformation, current_modules, bytes, &bytes);

    if (!NT_SUCCESS(status))
    {
        MODULE_DEBUG("[%ws]::%hs:: NtQuerySystemInformation Error: 0x%x \r\n", PROJECT_NAME, __FUNCTION__, status);
        FREE(current_modules);
        return STATUS_ACCESS_DENIED;
    }

    for (auto i = 0u; i < current_modules->NumberOfModules; i++)
    {
        const auto current_module = &current_modules->Modules[i];

        if (!current_module)
            continue;

        /* file_name_offset is the offset from full_path to the actual file's name, instead of file path */
        const auto file_name = reinterpret_cast<const char*>(current_module->OffsetToFileName + current_module->FullPathName);

        if (strcmp(file_name, ModuleName) != 0)
            continue;

        MODULE_DEBUG("[%ws]::%hs:: %hs Module Found at %llX \r\n", PROJECT_NAME, __FUNCTION__, ModuleName, current_module->ImageBase);

        pKModule->Base.QuadPart = reinterpret_cast<uint64_t>(current_module->ImageBase);
        pKModule->Size.LowPart = current_module->ImageSize;
        break;
    }

    FREE(current_modules);

    return STATUS_SUCCESS;
}

uint8_t* KernelModule::FindPatternInModule(const CHAR* ModuleName, BYTE* bMask, char* szMask)
{
    uint8_t* rvalue = 0;
    auto SMI = new KModule();
    auto Status = GetKernelModule(ModuleName, SMI);

    if (!NT_SUCCESS(Status))
        goto exit;
    
    MODULE_DEBUG("[%ws]::%hs:: Searching pattern in %hs [0x%llX//%X] \r\n", PROJECT_NAME, __FUNCTION__, 
        ModuleName, SMI->Base.QuadPart, SMI->Size.LowPart);

    rvalue = (uint8_t*)KernelUtilities::FindPattern(SMI->Base.QuadPart, SMI->Size.LowPart, bMask, szMask);

exit:
    if (!NT_SUCCESS(Status)) 
        MODULE_DEBUG("[%ws]::%hs:: Error To Get Module %hs \r\n", PROJECT_NAME, __FUNCTION__, ModuleName);
      
     delete SMI;
     return rvalue;
}

NTSTATUS KernelModule::GetUserModule(IN PEPROCESS pProcess, IN const wchar_t* ModuleName, KModule* pKModule)
{ 
    PVOID isWow64 = PsGetProcessWow64Process(pProcess);
  
        if (isWow64) // If project is x86
        {
            PPEB32 pPeb32 = (PPEB32)PsGetProcessWow64Process(pProcess);

            if (pPeb32 == NULL)
                return STATUS_FAILED_DRIVER_ENTRY;

            if (!pPeb32->Ldr)
                return STATUS_FAILED_DRIVER_ENTRY;

            // Search in InLoadOrderModuleList
            for (PLIST_ENTRY32 pListEntry = (PLIST_ENTRY32)((PPEB_LDR_DATA32)pPeb32->Ldr)->InLoadOrderModuleList.Flink;
                pListEntry != &((PPEB_LDR_DATA32)pPeb32->Ldr)->InLoadOrderModuleList;
                pListEntry = (PLIST_ENTRY32)pListEntry->Flink)
            {
                PLDR_DATA_TABLE_ENTRY32 pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);

                if (wcscmp((PWCH)pEntry->BaseDllName.Buffer, ModuleName) == 0)
                {
                    pKModule->Base.LowPart = (DWORD)pEntry->DllBase;
                    pKModule->Size.LowPart = pEntry->SizeOfImage;
                    pKModule->isWow64 = true;

                    return STATUS_SUCCESS;
                }

            }
        }
        else // if x64
        {
            PPEB pPeb = PsGetProcessPeb(pProcess);

            if (!pPeb)
                return STATUS_FAILED_DRIVER_ENTRY;

            if (!pPeb->Ldr)
                return STATUS_FAILED_DRIVER_ENTRY;

            // Search in InLoadOrderModuleList
            for (PLIST_ENTRY pListEntry = pPeb->Ldr->InLoadOrderModuleList.Flink;
                pListEntry != &pPeb->Ldr->InLoadOrderModuleList;
                pListEntry = pListEntry->Flink)
            {
                PLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

                MODULE_DEBUG("[%ws]::%hs:: Checking Module:->  %ws \r\n", PROJECT_NAME, __FUNCTION__, pEntry->BaseDllName.Buffer);

                if (wcscmp(pEntry->BaseDllName.Buffer, ModuleName) == 0)
                {
                    pKModule->Base.QuadPart = (DWORD64)pEntry->DllBase;
                    pKModule->Size.LowPart = pEntry->SizeOfImage;
                    pKModule->isWow64 = false;

                    return STATUS_SUCCESS;
                }
            }
        }

    return STATUS_FAILED_DRIVER_ENTRY;
}

void KernelModule::DumpUserModules(IN PEPROCESS pProcess)
{
    KModule rKModule;

    PVOID isWow64 = PsGetProcessWow64Process(pProcess);
    __try
    {
        if (isWow64) // If project is x86
        {
            PPEB32 pPeb32 = (PPEB32)PsGetProcessWow64Process(pProcess);

            if (pPeb32 == NULL)
                return ;

            if (!pPeb32->Ldr)
                return ;

            // Search in InLoadOrderModuleList
            for (PLIST_ENTRY32 pListEntry = (PLIST_ENTRY32)((PPEB_LDR_DATA32)pPeb32->Ldr)->InLoadOrderModuleList.Flink;
                pListEntry != &((PPEB_LDR_DATA32)pPeb32->Ldr)->InLoadOrderModuleList;
                pListEntry = (PLIST_ENTRY32)pListEntry->Flink)
            {
                PLDR_DATA_TABLE_ENTRY32 pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);

                MODULE_DEBUG("[%ws]::%hs:: %ws \r\n", PROJECT_NAME, __FUNCTION__, pEntry->BaseDllName.Buffer);
            }
        }
        else
        {
            PPEB pPeb = PsGetProcessPeb(pProcess);

            if (!pPeb)
                return ;

            if (!pPeb->Ldr)
                return ;

            // Search in InLoadOrderModuleList
            for (PLIST_ENTRY pListEntry = pPeb->Ldr->InLoadOrderModuleList.Flink;
                pListEntry != &pPeb->Ldr->InLoadOrderModuleList;
                pListEntry = pListEntry->Flink)
            {
                PLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

                MODULE_DEBUG("[%ws]::%hs:: %ws \r\n", PROJECT_NAME, __FUNCTION__, pEntry->BaseDllName.Buffer);
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        MODULE_DEBUG("[%ws]::%hs:: Exception, Code: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, GetExceptionCode());
        return ;
    }

    return ;
}

uint8_t* KernelModule::GetModuleExport(IN PVOID pBase, IN PCCHAR name_ord)
{
    PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)pBase;
    PIMAGE_NT_HEADERS32 pNtHdr32 = NULL;
    PIMAGE_NT_HEADERS64 pNtHdr64 = NULL;
    PIMAGE_EXPORT_DIRECTORY pExport = NULL;
    ULONG expSize = 0;
    ULONG_PTR pAddress = 0;

    ASSERT(pBase != NULL);
    
    if (pBase == NULL)
        return NULL;

    /// Not a PE file
    if (pDosHdr->e_magic != IMAGE_DOS_SIGNATURE)
        return NULL;

    pNtHdr32 = (PIMAGE_NT_HEADERS32)((PUCHAR)pBase + pDosHdr->e_lfanew);
    pNtHdr64 = (PIMAGE_NT_HEADERS64)((PUCHAR)pBase + pDosHdr->e_lfanew);

    // Not a PE file
    if (pNtHdr32->Signature != IMAGE_NT_SIGNATURE)
        return NULL;

    // 64 bit image
    if (pNtHdr32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    {
        pExport = (PIMAGE_EXPORT_DIRECTORY)(pNtHdr64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + (ULONG_PTR)pBase);
        expSize = pNtHdr64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
    }
    // 32 bit image
    else
    {
        pExport = (PIMAGE_EXPORT_DIRECTORY)(pNtHdr32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + (ULONG_PTR)pBase);
        expSize = pNtHdr32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
    }

    PUSHORT pAddressOfOrds = (PUSHORT)(pExport->AddressOfNameOrdinals + (ULONG_PTR)pBase);
    PULONG  pAddressOfNames = (PULONG)(pExport->AddressOfNames + (ULONG_PTR)pBase);
    PULONG  pAddressOfFuncs = (PULONG)(pExport->AddressOfFunctions + (ULONG_PTR)pBase);

    for (ULONG i = 0; i < pExport->NumberOfFunctions; ++i)
    {
        USHORT OrdIndex = 0xFFFF;
        PCHAR  pName = NULL;

        // Find by index
        if ((ULONG_PTR)name_ord <= 0xFFFF)
        {
            OrdIndex = (USHORT)i;
        }
        // Find by name
        else if ((ULONG_PTR)name_ord > 0xFFFF && i < pExport->NumberOfNames)
        {
            pName = (PCHAR)(pAddressOfNames[i] + (ULONG_PTR)pBase);
            OrdIndex = pAddressOfOrds[i];

            //MODULE_DEBUG("[%ws]::%hs:: pName -> %hs \n", PROJECT_NAME, __FUNCTION__, pName);
        }
        // Weird params
        else
            return NULL;
 
        if (((ULONG_PTR)name_ord <= 0xFFFF && (USHORT)((ULONG_PTR)name_ord) == OrdIndex + pExport->Base) ||
            ((ULONG_PTR)name_ord > 0xFFFF && strcmp(pName, name_ord) == 0))
        {
            pAddress = pAddressOfFuncs[OrdIndex] + (ULONG_PTR)pBase;

            // Check forwarded export


            break;
        }
    }

    return (uint8_t*)pAddress;
}

uint8_t* KernelModule::GetKernelModuleExport(const char* module_name, PCCHAR routine_name)
{
    auto lpModule = new KModule();

    auto Status = GetKernelModule(module_name, lpModule);

    if (!NT_SUCCESS(Status))
    {
        MODULE_DEBUG("[%ws]::%hs:: Error To Get Module %hs \r\n", PROJECT_NAME, __FUNCTION__, module_name);
        delete lpModule;
        return 0;
    }

    auto ImageBase = (PVOID)lpModule->Base.QuadPart;

    if (!ImageBase)
        return NULL;

    auto a = KernelModule::GetModuleExport(ImageBase, routine_name);
    delete lpModule;

    MODULE_DEBUG("[%ws]::%hs:: Routine named %hs Found at 0x%llX inside \r\n", PROJECT_NAME, __FUNCTION__, routine_name, a, module_name);

    return a;
}

uint8_t* KernelModule::GetUserModuleExport(const wchar_t* module_name, PCCHAR routine_name, PEPROCESS pPEPROCESS)
{
    auto lpModule = new KModule();

    auto Status = KernelModule::GetUserModule(pPEPROCESS, module_name, lpModule);

    if (!NT_SUCCESS(Status))
    {
        MODULE_DEBUG("[%ws]::%hs:: Error To Get Module %ws \r\n", PROJECT_NAME, __FUNCTION__, module_name);
        delete lpModule;
        return 0;
    }

    MODULE_DEBUG("[%ws]::%hs:: %ws Found -> 0x%llX isWow64 -> %i \r\n", PROJECT_NAME, __FUNCTION__, module_name, lpModule->Base.QuadPart, lpModule->isWow64);
   
    // If x64
    if (!lpModule->isWow64)
        return KernelModule::GetModuleExport((PVOID)lpModule->Base.QuadPart, routine_name);
    else
        return KernelModule::GetModuleExport((PVOID)lpModule->Base.LowPart, routine_name);
}

uint8_t* KernelModule::FindFreeMemoryInSytemModule(const CHAR* ModuleName, uint32_t DesiredSize)
{
    auto pModule = new KModule();

    auto Status = GetKernelModule(ModuleName, pModule);

    if (!NT_SUCCESS(Status))
    {
        MODULE_DEBUG("[%ws]::%hs:: Error To Get Module %hs \r\n", PROJECT_NAME, __FUNCTION__, ModuleName);
        delete pModule;
        return 0;
    }

    uint8_t* returnadd = 0;
    uint32_t CurrentEmptySpace = 0;

    for (uint64_t i = pModule->Base.QuadPart;
        i < pModule->Base.QuadPart + pModule->Size.LowPart;
        i++)
    {

        if (*reinterpret_cast<BYTE*>(i) == 0 || *reinterpret_cast<BYTE*>(i) == 0xCC)
            CurrentEmptySpace += 1;

        if (*reinterpret_cast<BYTE*>(i) != 0 && *reinterpret_cast<BYTE*>(i) != 0xCC)
        {
            if (CurrentEmptySpace >= DesiredSize) // Empty Memory End
            {
                // Rebase EmptyStart Address to Games Memory instead of our malloc address
                uint64_t EmptyStart = i - CurrentEmptySpace;

                MODULE_DEBUG("[%ws]::%hs:: %hs-[0x%llX] - EmptyStart: 0x%llX  EmptySize : 0x%X\r\n",
                    PROJECT_NAME, __FUNCTION__, ModuleName,
                    pModule->Base.QuadPart, EmptyStart, CurrentEmptySpace);

                returnadd = reinterpret_cast<uint8_t*>(EmptyStart);
                break;
            }

            CurrentEmptySpace = 0;
        }
    }

    if (!returnadd)
     MODULE_DEBUG("[%ws]::%hs:: Couldnt Find Empty Space\r\n", PROJECT_NAME, __FUNCTION__);

    delete pModule;
    return returnadd;
}

//NTSTATUS KernelUtilities::DumpAllKernelModuleThreads()
//{
//	NTSTATUS status = STATUS_SUCCESS;
//	PVOID pBuf = ALLOCATE( 1024 * 1024);
//	PSYSTEM_PROCESS_INFO pInfo = (PSYSTEM_PROCESS_INFO)pBuf;
//
//	if (!pInfo)
//		return STATUS_NO_MEMORY;
//
//	// Get the process thread list
//	status = ZwQuerySystemInformation(SystemProcessInformation, pInfo, 1024 * 1024, NULL);
//
//	if (!NT_SUCCESS(status))
//	{
//		FREE(pBuf);
//		return status;
//	}
//
//	// Find target thread
//	if (NT_SUCCESS(status))
//	{
//		while (pInfo->NextEntryOffset)
//		{
//
//			if (pInfo->UniqueProcessId == (HANDLE)0x4)
//			{
//				UTILS_DEBUG("[%ws]::%hs:: Current Process ID pInfo->ProcessId %d -> Number Of Threads %d \r\n", PROJECT_NAME, __FUNCTION__, pInfo->UniqueProcessId, pInfo->NumberOfThreads);
//
//				for (ULONG i = 0; i < pInfo->NumberOfThreads; i++)
//				{
//					// Skip current thread
//					if (pInfo->Threads[i].ClientId.UniqueThread == PsGetCurrentThreadId())
//					{
//						UTILS_DEBUG("[%ws]::%hs:: Current Threaad \r\n", PROJECT_NAME, __FUNCTION__);
//
//						PETHREAD peThread = NULL;
//						PKTHREAD pkThread = NULL;
//						HANDLE hThread = NULL;
//
//						CLIENT_ID clientId;
//						OBJECT_ATTRIBUTES oa;
//						InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);
//						clientId.UniqueProcess = pInfo->UniqueProcessId;
//						clientId.UniqueThread = pInfo->Threads[i].ClientId.UniqueThread;
//
//						/*if (ZwOpenThread)
//							status = ZwOpenThread(&hThread, THREAD_ALL_ACCESS, &oa, &clientId);*/
//
//						if (NT_SUCCESS(status))
//							UTILS_DEBUG("\n[%ws]::%hs:: hThread 0x%X\r\n", PROJECT_NAME, __FUNCTION__, hThread);
//
//						status = PsLookupThreadByThreadId(pInfo->Threads[i].ClientId.UniqueThread, &peThread);
//
//						if (NT_SUCCESS(status))
//							UTILS_DEBUG("\n[%ws]::%hs:: peThread 0x%X\r\n", PROJECT_NAME, __FUNCTION__, peThread);
//
//						status = ObReferenceObjectByHandle(hThread, FILE_ANY_ACCESS, NULL, KernelMode, (PVOID*)&pkThread, NULL);
//
//						if (NT_SUCCESS(status))
//							UTILS_DEBUG("\n[%ws]::%hs:: pkThread 0x%X\r\n", PROJECT_NAME, __FUNCTION__, pkThread);
//
//						THREAD_BASIC_INFORMATION info = { 0 };
//						ULONG bytes = 0;
//
//						//status = ZwQueryInformationThread(hThread, ThreadBasicInformation, &info, sizeof(info), &bytes);
//
//						if (NT_SUCCESS(status))
//							UTILS_DEBUG("\n[%ws]::%hs:: TebBaseAddress 0x%llX\r\n", PROJECT_NAME, __FUNCTION__, info.TebBaseAddress);
//
//
//
//
//						//PsSuspendThread(peThread, 0);
//
//						//PsTerminateSystemThread();
//
//						/*ULONG PreviousSuspendCount;
//
//						if (ZwSuspendThread)
//							ZwSuspendThread(hThread, &PreviousSuspendCount);*/
//
//					}
//
//					auto ModuleName = GetKernelModuleAtAddress(reinterpret_cast<uint64_t>(pInfo->Threads[i].StartAddress));
//
//					UTILS_DEBUG("[%ws]::%hs:: ****************Thread ID %d -> Starts At 0x%llX ModuleName %hs \r\n", PROJECT_NAME, __FUNCTION__, pInfo->Threads[i].ClientId.UniqueThread, pInfo->Threads[i].StartAddress, ModuleName);
//
//				}
//			}
//
//			pInfo = (PSYSTEM_PROCESS_INFO)((PUCHAR)pInfo + pInfo->NextEntryOffset);
//		}
//	}
//
//
//	if (pBuf)
//		FREE(pBuf);
//
//	return status;
//}
//
//NTSTATUS KernelUtilities::DumpAllKernelModules()
//{
//	ULONG bytes = 0;
//
//	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, 0, bytes, &bytes);
//
//	if (!bytes)
//	{
//		UTILS_DEBUG("\n[%ws]::%hs:: NtQuerySystemInformation Error: 0x%x \r\n", PROJECT_NAME, __FUNCTION__, status);
//		return STATUS_NO_TOKEN;
//	}
//
//	PSYSTEM_MODULE modules = (PSYSTEM_MODULE)ALLOCATE( bytes); // 'ENON'
//
//	status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);
//
//	if (!NT_SUCCESS(status))
//	{
//		UTILS_DEBUG("\n[%ws]::%hs:: NtQuerySystemInformation Error: 0x%x \r\n", PROJECT_NAME, __FUNCTION__, status);
//		return STATUS_NO_TOKEN;
//	}
//
//	PSYSTEM_MODULE_INFORMATION module = modules->Modules;
//
//	for (ULONG i = 0; i < modules->NumberOfModules; i++)
//	{
//		UTILS_DEBUG("\n[%ws]::%hs:: %hs \r\n", PROJECT_NAME, __FUNCTION__, module[i].FullPathName);
//	}
//
//	if (modules)
//		FREE(modules);
//
//	return STATUS_SUCCESS;
//}

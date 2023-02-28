#pragma once

class KernelModule
{
public:

	static uint8_t* FindFreeMemoryInSytemModule(const CHAR* ModuleName, uint32_t DesiredSize);
	static NTSTATUS GetKernelModule(const CHAR* ModuleName, KModule* pKModule);
	static NTSTATUS GetKernelModule(uint64_t ModuleAddress, KModule* pKModule);
	static uint8_t* FindPatternInModule(const CHAR* ModuleName, BYTE* bMask, char* szMask);

	static uint8_t* GetModuleExport(IN PVOID pBase, IN PCCHAR name_ord);
	static NTSTATUS KernelModule::GetUserModule(IN PEPROCESS pProcess, IN const wchar_t* ModuleName, KModule* pKModule);
	static void DumpUserModules(IN PEPROCESS pProcess);

	static uint8_t* GetKernelModuleExport(const char* module_name, PCCHAR routine_name);
	static uint8_t* GetUserModuleExport(const wchar_t* module_name, PCCHAR routine_name, PEPROCESS pPEPROCESS);
};


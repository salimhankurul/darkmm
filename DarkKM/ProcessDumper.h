#pragma once

class ProcessDumper
{
public:
	static NTSTATUS DumpProcess(wchar_t* ProcessName, wchar_t* TargetPath, uint32_t SleepTime);

	static NTSTATUS DumpAllProcessNamed(wchar_t* ProcessName, wchar_t* TargetPath, uint32_t SleepTime);
	static NTSTATUS DumpModule(PEPROCESS MmGameHandle, KModule Module, wchar_t* ModuleTargetPath);
};


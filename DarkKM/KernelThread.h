#pragma once
class KernelThread
{
public:
	static NTSTATUS SuspendAllThreadsBelogsTo(const WCHAR* ModuleName);
};


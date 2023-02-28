#pragma once



typedef struct KernelRegisteryData
{
	KernelRegisteryData()
	{
		RtlZeroMemory(this, sizeof(KernelRegisteryData));
	}
	void* operator new(size_t size)
	{
		return ALLOCATE( sizeof(KernelRegisteryData));
	}
	void operator delete(void* ptr)
	{
		FREE(ptr);
	}
	
	uint16_t Size;
	ULONG   Type;
	uint8_t Data[0x2048];
};


// Sample Usage
/*
#define PATH (L"\\registry\\machine\\SYSTEM\\CurrentControlSet\\Services\\ACPI")
#define KEY (L"Group")

KernelRegisteryData* pData = new KernelRegisteryData();
KernelRegistery::ReadRegistryData(PATH, KEY,*pData);
delete pData;

////////////////////////////////////

KernelRegistery::SetRegistryString(PATH, KEY, L"Deneme Aliveli");
*/

class KernelRegistery
{
public:
	static NTSTATUS ReadRegistryData(PCWSTR KeyPath, PCWSTR ValueName, KernelRegisteryData& Data);
	static NTSTATUS SetRegistryData(PCWSTR KeyPath, PCWSTR ValueName, KernelRegisteryData& Data);
	static NTSTATUS SetRegistryString(PCWSTR KeyPath, PCWSTR ValueName, wchar_t* String);
};


#include "Main.h"

#define REG_DEBUG(format, ...) //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, __VA_ARGS__)

NTSTATUS KernelRegistery::ReadRegistryData(PCWSTR KeyPath, PCWSTR ValueName, KernelRegisteryData& Data)
{
	
	OBJECT_ATTRIBUTES KeyAttributes;
	UNICODE_STRING CfgPath, ValueString;
	RtlInitUnicodeString(&CfgPath, KeyPath);
	RtlInitUnicodeString(&ValueString, ValueName);
	ULONG Temp;
	HANDLE Key = NULL;

	InitializeObjectAttributes(&KeyAttributes, &CfgPath, OBJ_CASE_INSENSITIVE, NULL, NULL);

	auto Status = ZwOpenKey(&Key, KEY_ALL_ACCESS, &KeyAttributes);

	if (Status != STATUS_SUCCESS) 
	{
		ZwClose(Key);
		return Status;
	}

	auto KeyValue = reinterpret_cast<PKEY_VALUE_PARTIAL_INFORMATION>(ALLOCATE(0x2048));

	Status = ZwQueryValueKey(Key,
		&ValueString,
		KeyValuePartialInformation,
		KeyValue,
		0x2048,
		&Temp);

	if (!NT_SUCCESS(Status))
	{
		FREE(KeyValue);
		ZwClose(Key);
		return Status;
	}

	Data.Size = KeyValue->DataLength;
	Data.Type = KeyValue->Type;
	RtlCopyMemory(Data.Data, KeyValue->Data, KeyValue->DataLength);
	 
	FREE(KeyValue);
	ZwClose(Key);
	return Status;
}

NTSTATUS KernelRegistery::SetRegistryData(PCWSTR KeyPath, PCWSTR ValueName, KernelRegisteryData& Data) // REG_SZ
{	
	OBJECT_ATTRIBUTES KeyAttributes;
	HANDLE Key = NULL;
	UNICODE_STRING CfgPath,ValueString;

	RtlInitUnicodeString(&CfgPath, KeyPath);
	RtlInitUnicodeString(&ValueString, ValueName);

	InitializeObjectAttributes(&KeyAttributes, &CfgPath, OBJ_CASE_INSENSITIVE, NULL, NULL);

	auto Status = ZwOpenKey(&Key, KEY_ALL_ACCESS, &KeyAttributes);

	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	Status = ZwSetValueKey(Key, &ValueString, 0, Data.Type, Data.Data, Data.Size);

	ZwClose(Key);
	return Status;
}

NTSTATUS KernelRegistery::SetRegistryString(PCWSTR KeyPath, PCWSTR ValueName, wchar_t* String) // REG_SZ
{
	OBJECT_ATTRIBUTES KeyAttributes;
	HANDLE Key = NULL;
	UNICODE_STRING CfgPath, ValueString;

	RtlInitUnicodeString(&CfgPath, KeyPath);
	RtlInitUnicodeString(&ValueString, ValueName);

	InitializeObjectAttributes(&KeyAttributes, &CfgPath, OBJ_CASE_INSENSITIVE, NULL, NULL);

	auto Status = ZwOpenKey(&Key, KEY_ALL_ACCESS, &KeyAttributes);

	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	Status = ZwSetValueKey(Key, &ValueString, 0, REG_SZ, String, wcslen(String) * 2);

	ZwClose(Key);
	return Status;
}


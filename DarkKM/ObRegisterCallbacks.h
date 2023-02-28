#pragma once

#pragma warning(disable: 4200)

typedef struct _CALLBACK_ENTRY {
	USHORT Version;
	USHORT OperationRegistrationCount;
	ULONG32 unk1;
	PVOID RegistrationContext;
	UNICODE_STRING Altitude;
} CALLBACK_ENTRY, *PCALLBACK_ENTRY;

typedef struct _CALLBACK_ENTRY_ITEM {
	LIST_ENTRY CallbackList;
	OB_OPERATION Operations;
	ULONG32 Active;
	CALLBACK_ENTRY* CallbackEntry;
	PVOID ObjectType;
	POB_PRE_OPERATION_CALLBACK PreOperation;
	POB_POST_OPERATION_CALLBACK PostOperation;
	DWORD64 unk1;
} CALLBACK_ENTRY_ITEM, *PCALLBACK_ENTRY_ITEM;

#pragma warning(default: 4200)

#define PROCESS_QUERY_LIMITED_INFORMATION      0x1000


class ObRegisterCallbacks
{
public:
	ObRegisterCallbacks();
	~ObRegisterCallbacks();

	static CALLBACK_ENTRY_ITEM* GetObProcessCallBack(const WCHAR* Altitute);
	static CALLBACK_ENTRY_ITEM* GetObThreadCallBack(const WCHAR* Altitute);

	static NTSTATUS ChangeObProcessCallbacks(const WCHAR* Altitute,  bool Hook, CALLBACK_HOOK* mycallback);
	static NTSTATUS ChangeObThreadCallbacks(const WCHAR* Altitute,  bool Hook, CALLBACK_HOOK* mycallback);
	
	static NTSTATUS GetCallBackListOffset(UINT64& Offset);
	static bool DumpAllThreadCallbacks();
	static bool DumpAllProcessCallbacks();

	static CALLBACK_HOOK Game_Hook;

	
	static WCHAR* m_altitude;
};


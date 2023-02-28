#include "Main.h"

NotifyRoutines::NotifyRoutines()
{
}


NotifyRoutines::~NotifyRoutines()
{
}

#define NOTIFY_ROUTINES_DEBUG DEBUG_PRINTE

PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutines::m_pOriginalImageRoutines[8];
PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutines::m_pOriginalThreadRoutines[64];

ULONG64 NotifyRoutines::FindPsCreateThreadNotifyRoutine()
{
	PULONG64 pFlags = 0;
	ULONG64	i = 0, pCheckArea = 0;
	UNICODE_STRING	functionName;
	RtlInitUnicodeString(&functionName, L"PsSetCreateThreadNotifyRoutine");

	auto pAddress = MmGetSystemRoutineAddress(&functionName);
	pCheckArea = reinterpret_cast<ULONG64>(MmGetSystemRoutineAddress(&functionName));

	if (!pCheckArea || !MmIsAddressValid(pAddress)) 
	{
		NOTIFY_ROUTINES_DEBUG("[Dark]: Error Cant Find %wZ %p %p \r\n", &functionName, pCheckArea, pCheckArea);
		return 0;
	}

	pCheckArea += 0x2;
	for (i = pCheckArea; i < pCheckArea + 0xFF; i++)
	{
		if (*reinterpret_cast<PUCHAR>(i) == 0x48 && *reinterpret_cast<PUCHAR>(i + 1) == 0x8D && *reinterpret_cast<PUCHAR>(i + 2) == 0x0D)
		{
			NOTIFY_ROUTINES_DEBUG("[Dark WIN10]: GetNotifyFlags Char ValueS %02x %02x %02x \r\n", *reinterpret_cast<PUCHAR>(i), *reinterpret_cast<PUCHAR>(i + 1), *reinterpret_cast<PUCHAR>(i + 2));

			NOTIFY_ROUTINES_DEBUG("[Dark WIN10]: GetNotifyFlags %p ->. %p \r\n", i, pCheckArea);
			LONG offset = 0;

			memcpy(&offset, reinterpret_cast<PUCHAR>(i + 3), 4);
			pFlags = reinterpret_cast<PULONG64>(offset + i + 0x7);

			break;
		}
	}

	NOTIFY_ROUTINES_DEBUG("[Dark]: Founded Flags for CreateThreadNotifyRoutine -> %02x \r\n", pFlags);
	return reinterpret_cast<ULONG64>(pFlags);
}

bool NotifyRoutines::RemoveCreateThreadNotifyRoutine()
{
	int i = 0;
	bool notifyWasRemoved = false;
	ULONG64	ulPsCreateThreadNotifyRoutine = 0;

	ulPsCreateThreadNotifyRoutine = FindPsCreateThreadNotifyRoutine();

	if (!ulPsCreateThreadNotifyRoutine || !MmIsAddressValid(reinterpret_cast<void*>(ulPsCreateThreadNotifyRoutine))) {
		NOTIFY_ROUTINES_DEBUG("[Dark] Error While : RemoveCreateThreadNotifyRoutine. \r\n");
		return false;
	}

	RtlZeroMemory(m_pOriginalThreadRoutines, 64 * sizeof(DWORD64));

	ULONG64 ulNotifyAddress = 0;
	ULONG64 ulMagicPtr = 0;

	for (i = 0; i < 64; i++)
	{
		ulMagicPtr = ulPsCreateThreadNotifyRoutine + (i * 8);

		if (!ulMagicPtr || !MmIsAddressValid(reinterpret_cast<void*>(ulMagicPtr)))
			continue;

		ulNotifyAddress = *reinterpret_cast<PULONG64>(ulMagicPtr);

		if (ulNotifyAddress && MmIsAddressValid(reinterpret_cast<void*>(ulNotifyAddress)))
		{
			PCREATE_THREAD_NOTIFY_ROUTINE routine;
			ulNotifyAddress = *reinterpret_cast<PULONG64>(ulNotifyAddress & 0xfffffffffffffff8);
			routine = reinterpret_cast<PCREATE_THREAD_NOTIFY_ROUTINE>(ulNotifyAddress);

			if (routine && MmIsAddressValid(routine))
			{
				NOTIFY_ROUTINES_DEBUG("[Dark] Successfuly Removing PsSetCreateThreadNotifyRoutine. %p \r\n", routine);
				m_pOriginalThreadRoutines[i] = routine;
				PsRemoveCreateThreadNotifyRoutine(routine);

				notifyWasRemoved = true;
			}
		}
	}

	if (!notifyWasRemoved)
		NOTIFY_ROUTINES_DEBUG("[Dark] Error There is No CreateThreadNotifyRoutine for removal \r\n");

	return notifyWasRemoved;
}

void NotifyRoutines::RestoreCreateThreadNotifyRoutine()
{
	for (int i = 0; i < 64; i++)
	{
		PCREATE_THREAD_NOTIFY_ROUTINE routine = m_pOriginalThreadRoutines[i];

		if (routine && MmIsAddressValid(routine)) {
			PsSetCreateThreadNotifyRoutine(routine);
			NOTIFY_ROUTINES_DEBUG("[Dark] Restored : PsSetCreateThreadNotifyRoutine. \r\n");
		}

	}
}

ULONG64 NotifyRoutines::FindPsLoadImageNotifyRoutine()
{
	PULONG64 pFlags = 0;
	ULONG64 i = 0, pCheckArea = 0;
	UNICODE_STRING	functionName;
	RtlInitUnicodeString(&functionName, L"PsSetLoadImageNotifyRoutine");

	auto pAddress = MmGetSystemRoutineAddress(&functionName);
	pCheckArea = reinterpret_cast<ULONG64>(pAddress);

	if (!pCheckArea || !MmIsAddressValid(pAddress)) {
		NOTIFY_ROUTINES_DEBUG("[Dark]: Error Cant Find %wZ %p %p \r\n", &functionName, pCheckArea, pCheckArea);
		return 0;
	}

	for (i = pCheckArea; i < pCheckArea + 0xFF; i++)
	{
		if (*reinterpret_cast<PUCHAR>(i) == 0x48 && *reinterpret_cast<PUCHAR>(i + 1) == 0x8D && *reinterpret_cast<PUCHAR>(i + 2) == 0x0D)
		{
			NOTIFY_ROUTINES_DEBUG("[Dark WIN10]: GetNotifyFlags Char ValueS %02x %02x %02x \r\n", *reinterpret_cast<PUCHAR>(i), *reinterpret_cast<PUCHAR>(i + 1), *reinterpret_cast<PUCHAR>(i + 2));

			NOTIFY_ROUTINES_DEBUG("[Dark WIN10]: GetNotifyFlags %p ->. %p \r\n", i, pCheckArea);
			LONG offset = 0;

			memcpy(&offset, reinterpret_cast<PUCHAR>(i + 3), 4);
			pFlags = reinterpret_cast<PULONG64>(offset + i + 0x7);

			break;
		}
	}

	NOTIFY_ROUTINES_DEBUG("[Dark]: Founded Flags for LoadImageNotifyRoutine -> %02x \r\n", pFlags);
	return reinterpret_cast<ULONG64>(pFlags);
}

bool NotifyRoutines::RemoveLoadImageNotifyRoutine()
{
	int i = 0;
	bool notifyWasRemoved = false;
	ULONG64	ulPsLoadImageNotifyRoutine = 0;

	ulPsLoadImageNotifyRoutine = FindPsLoadImageNotifyRoutine();

	if (!ulPsLoadImageNotifyRoutine || !MmIsAddressValid(reinterpret_cast<void*>(ulPsLoadImageNotifyRoutine))) {
		NOTIFY_ROUTINES_DEBUG("[Dark] Error While : RemoveLoadImageNotifyRoutine. \r\n");
		return false;
	}

	RtlZeroMemory(m_pOriginalImageRoutines, 8 * sizeof(DWORD64));

	ULONG64 ulNotifyAddress = 0;
	ULONG64 ulMagicPtr = 0;

	for (i = 0; i < 8; i++)
	{
		ulMagicPtr = ulPsLoadImageNotifyRoutine + (i * 8);

		if (!ulMagicPtr || !MmIsAddressValid(reinterpret_cast<void*>(ulMagicPtr)))
			continue;

		ulNotifyAddress = *reinterpret_cast<PULONG64>(ulMagicPtr);

		if (ulNotifyAddress && MmIsAddressValid(reinterpret_cast<void*>(ulNotifyAddress)))
		{
			PLOAD_IMAGE_NOTIFY_ROUTINE routine;

			ulNotifyAddress = *reinterpret_cast<PULONG64>(ulNotifyAddress & 0xfffffffffffffff8);
			routine = reinterpret_cast<PLOAD_IMAGE_NOTIFY_ROUTINE>(ulNotifyAddress);
			if (routine && MmIsAddressValid(routine))
			{
				NOTIFY_ROUTINES_DEBUG("[Dark] Successfuly Removing PsRemoveLoadImageNotifyRoutine. %p \r\n", routine);
				m_pOriginalImageRoutines[i] = routine;
				PsRemoveLoadImageNotifyRoutine(routine);

				notifyWasRemoved = true;
			}
		}
	}

	if (!notifyWasRemoved)
		NOTIFY_ROUTINES_DEBUG("[Dark] Error There is No  LoadImageNotifyRoutine for removal \r\n");

	return notifyWasRemoved;
}

void NotifyRoutines::RestoreLoadImageNotifyRoutine()
{
	for (int i = 0; i < 8; i++)
	{
		PLOAD_IMAGE_NOTIFY_ROUTINE routine = m_pOriginalImageRoutines[i];

		if (routine && MmIsAddressValid(routine)) {
			PsSetLoadImageNotifyRoutine(routine);
			NOTIFY_ROUTINES_DEBUG("[Dark] Restored : PsSetLoadImageNotifyRoutine. \r\n");
		}

	}
}

#pragma once


class NotifyRoutines
{
public:
	NotifyRoutines();
	~NotifyRoutines();

	static ULONG64 FindPsCreateThreadNotifyRoutine();
	static bool RemoveCreateThreadNotifyRoutine();
	static void RestoreCreateThreadNotifyRoutine();
	static ULONG64 FindPsLoadImageNotifyRoutine();
	static bool RemoveLoadImageNotifyRoutine();
	static void RestoreLoadImageNotifyRoutine();

	static PLOAD_IMAGE_NOTIFY_ROUTINE m_pOriginalImageRoutines[8];
	static PCREATE_THREAD_NOTIFY_ROUTINE m_pOriginalThreadRoutines[64];
};


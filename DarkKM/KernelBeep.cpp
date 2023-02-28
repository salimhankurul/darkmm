#include "Main.h"
#define FILE_DEVICE_BEEP                0x00000001

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
    )

#define IOCTL_BEEP_SET \
    CTL_CODE(FILE_DEVICE_BEEP,0,METHOD_BUFFERED,FILE_ANY_ACCESS)


typedef struct _BEEP_SET_PARAMETERS {
	ULONG Frequency;
	ULONG Duration;
} BEEP_SET_PARAMETERS, * PBEEP_SET_PARAMETERS;

BOOL KernelBeep::Beep(DWORD dwFreq, DWORD dwDuration)
{
	//return false;
	HANDLE hBeep;
	UNICODE_STRING BeepDevice;
	OBJECT_ATTRIBUTES ObjectAttributes;
	IO_STATUS_BLOCK IoStatusBlock;
	BEEP_SET_PARAMETERS BeepSetParameters;
	NTSTATUS Status;

	//      print(L"Beep Params OK\n");
		/* open the device */
	RtlInitUnicodeString(&BeepDevice,
		L"\\Device\\Beep");

	InitializeObjectAttributes(&ObjectAttributes,
		&BeepDevice,
		(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
		NULL,
		NULL);

	Status = ZwCreateFile(&hBeep,
		FILE_READ_DATA | FILE_WRITE_DATA,
		&ObjectAttributes,
		&IoStatusBlock,
		NULL,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN_IF,
		0,
		NULL,
		0);

	if (!NT_SUCCESS(Status))
		return false;

	//          print(L"Createfile worked\n");
	/* Set beep data */
	BeepSetParameters.Frequency = dwFreq;
	BeepSetParameters.Duration = dwDuration;

	if (!NT_SUCCESS(Status))
		return false;

	Status = ZwDeviceIoControlFile(hBeep,
		NULL,
		NULL,
		NULL,
		&IoStatusBlock,
		IOCTL_BEEP_SET,
		&BeepSetParameters,
		sizeof(BEEP_SET_PARAMETERS),
		NULL,
		0);

	if (!NT_SUCCESS(Status))
		return FALSE;

	/* do an alertable wait if necessary */
	if (NT_SUCCESS(Status) &&
		(dwFreq != 0x0 || dwDuration != 0x0) && dwDuration != (DWORD)-1)
	{
		KernelUtilities::Sleep(dwDuration);
	}

	ZwClose(hBeep);

	return TRUE;
}

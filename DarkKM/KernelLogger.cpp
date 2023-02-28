#include "Main.h"

#define KernelLogger_PRINT(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, __VA_ARGS__)

NTSTATUS KernelLogger::LogToFile(PCSTR msg)
{
	static KernelFile* pFile = new KernelFile(L"\\SystemRoot\\aaDebug.txt");
		
	KernelLogger_PRINT("Log::%hs \r\n", msg);
	return 0;
	NTSTATUS Status = STATUS_SUCCESS;
	
	Status = pFile->OpenFile();
	
	if (!NT_SUCCESS(Status))
	{
		if (Status == STATUS_SHARING_VIOLATION)
			KernelLogger_PRINT("[%ws]::%hs File Was Already accessed \r\n", PROJECT_NAME, __FUNCTION__);
		
		KernelLogger_PRINT("[%ws]::%hs Error To OpenFile -> Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, Status);
		pFile->CloseFile();
		return Status;
	}

	Status = pFile->GetFileSize();

	if (!NT_SUCCESS(Status))
	{
		KernelLogger_PRINT("[%ws]::%hs Error To GetFileSize -> Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, Status);
		pFile->CloseFile();
		return Status;
	}

	size_t  Size = strlen(msg);
	
	LARGE_INTEGER byteOffset;
	RtlZeroMemory(&byteOffset,sizeof(byteOffset));

	byteOffset.QuadPart = pFile->size;

	Status = ZwWriteFile(pFile->FileHandle, NULL, NULL, NULL, &pFile->ioStatusBlock, (PVOID)msg, Size, &byteOffset, NULL);

	pFile->CloseFile();

	return 0;
}

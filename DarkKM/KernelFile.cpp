#include "Main.h"

// Has to be own
#define FILE_DEBUG(format, ...) //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, __VA_ARGS__)

NTSTATUS KernelFile::CloseFile()
{
	if (!this->HandleOpened)
		return STATUS_NO_TOKEN;

	this->HandleOpened = false;
	ZwClose(this->FileHandle);
		
	return STATUS_SUCCESS;
}
NTSTATUS KernelFile::OpenFile() 
{
	
	OBJECT_ATTRIBUTES ObjectAttributes;
	NTSTATUS Status;
	FILE_OBJECT* FileObj;

	InitializeObjectAttributes(
		&ObjectAttributes,
		&this->path,
		(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
		NULL,
		NULL
	);

	Status = ZwCreateFile(
		&this->FileHandle,
		FILE_ALL_ACCESS,
		&ObjectAttributes, &this->ioStatusBlock,
		NULL, FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0
	);

	// File Does not exist
	if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
	{
		// Create A new file
		Status = ZwCreateFile(
			&this->FileHandle,
			FILE_ALL_ACCESS,
			&ObjectAttributes, &this->ioStatusBlock,
			NULL, FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_READ,
			FILE_CREATE,
			FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0
		);

		if (!NT_SUCCESS(Status))
		{
			FILE_DEBUG("[%ws]::%hs  Error ZwCreateFile FILE_CREATE Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, Status);
			return Status;
		}

		FILE_DEBUG("[%ws]::%hs  Created File \r\n", PROJECT_NAME, __FUNCTION__);
	}
	else if (Status == STATUS_SHARING_VIOLATION)
	{
		FILE_DEBUG("[%ws]::%hs Error STATUS_SHARING_VIOLATION \r\n", PROJECT_NAME, __FUNCTION__);
		return Status;
	}
	else if (!NT_SUCCESS(Status))
	{
		FILE_DEBUG("[%ws]::%hs  Error ZwCreateFile FILE_OPEN Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, Status);
		return Status;
	}

	FILE_DEBUG("[%ws]::%hs  File Opened \r\n", PROJECT_NAME, __FUNCTION__);
	this->HandleOpened = true;
	return Status;
}

NTSTATUS KernelFile::GetFileSize()
{
	FILE_DEBUG("[%ws]::%hs Directory: %wZ \r\n", PROJECT_NAME, __FUNCTION__, &this->path);

	if (!this->HandleOpened)
		return STATUS_NO_TOKEN;

	IO_STATUS_BLOCK nwioStatusBlock;

	PFILE_STANDARD_INFORMATION info = (PFILE_STANDARD_INFORMATION)ALLOCATE(sizeof(FILE_STANDARD_INFORMATION)); //;

	auto Status = ZwQueryInformationFile(this->FileHandle, &nwioStatusBlock, info, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);

	if (!NT_SUCCESS(Status))
	{
		FILE_DEBUG("[%ws]::%hs  Error ZwQueryInformationFile Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, Status);
		FREE(info);
		return Status;
	}

	FILE_DEBUG("[%ws]::%hs File Size: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, info->EndOfFile.QuadPart); //LowPart

	this->size = info->EndOfFile.LowPart;
	
	FREE(info);

	return Status;
}

NTSTATUS KernelFile::ReadFile()
{
	FILE_DEBUG("[%ws]::%hs Reading File Directory: \"%wZ\" \r\n", PROJECT_NAME, __FUNCTION__, &this->path);

	if (!this->HandleOpened)
		return STATUS_NO_TOKEN;

	OBJECT_ATTRIBUTES ObjectAttributes;
	NTSTATUS Status;
	
	FILE_OBJECT* FileObj;

	IO_STATUS_BLOCK nwioStatusBlock;
	
	PFILE_STANDARD_INFORMATION info = (PFILE_STANDARD_INFORMATION)ALLOCATE( sizeof(FILE_STANDARD_INFORMATION));

	Status = ZwQueryInformationFile(FileHandle, &nwioStatusBlock, info, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);

	if (!NT_SUCCESS(Status))
	{
		FILE_DEBUG("[%ws]::%hs  Error ZwQueryInformationFile Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, Status);
		FREE(info);
		return Status;
	}
	
	FILE_DEBUG("[%ws]::%hs File Size is : 0x%X \r\n", PROJECT_NAME, __FUNCTION__, info->EndOfFile.LowPart); //LowPart

	this->Allocate(info->EndOfFile.LowPart);
	
	FREE(info);
	
	Status = ZwReadFile(FileHandle, NULL, NULL, NULL, &ioStatusBlock, this->data, this->size, NULL, NULL);
	
	if (!NT_SUCCESS(Status))
	{
		FILE_DEBUG("[%ws]::%hs Error To ZwReadFile -> Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, Status);
		return Status;
	}

	FILE_DEBUG("[%ws]::%hs File successfully read \r\n", PROJECT_NAME, __FUNCTION__); //LowPart

	return Status;
}

NTSTATUS KernelFile::WriteFile()
{
	NTSTATUS Status;

	if (!this->HandleOpened)
		return STATUS_NO_TOKEN;

	if (!this->Allocated ||!this->data ||!this->size)
		return STATUS_NO_TOKEN;

	Status = ZwWriteFile(FileHandle, NULL, NULL, NULL, &ioStatusBlock,
		this->data, this->size, NULL, NULL);

	if (!NT_SUCCESS(Status))
	{
		FILE_DEBUG("[%ws]::%hs Error To ZwWriteFile -> Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__, Status);
		return Status;
	}
	
	return Status;
}

NTSTATUS KernelFile::FileIsx64(bool& x64)
{
	NTSTATUS status;

	if (!this->HandleOpened)
		return STATUS_NO_TOKEN;

	PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(this->data);

	PIMAGE_NT_HEADERS pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>((reinterpret_cast<LPBYTE>(this->data) + pDosHeader->e_lfanew));

	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		status = STATUS_NO_TOKEN;

	if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		status = STATUS_NO_TOKEN;

	if (pNtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
	{
		x64 = false;
		return STATUS_SUCCESS;
	}
	else if (pNtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
	{
		x64 = true;
		return STATUS_SUCCESS;
	}

	return STATUS_NO_TOKEN;
}
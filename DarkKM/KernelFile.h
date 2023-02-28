#pragma once

struct KernelFile
{

	KernelFile(const wchar_t* Source)
	{
		RtlZeroMemory(this, sizeof(KernelFile));
		RtlInitUnicodeString(&this->path, Source);
	}

	void* operator new(size_t size)
	{
		return ALLOCATE( sizeof(KernelFile));
	}

	void operator delete(void* ptr)
	{
		reinterpret_cast<KernelFile*>(ptr)->FreeFile();
		FREE(ptr);
	}

	inline void Allocate(UINT32 Size)
	{
		this->size = Size;
		this->data = ExAllocatePool(PagedPool, Size);
		this->Allocated = true;
	}

	inline void FreeFile()
	{	
		this->CloseFile();
		if (this->Allocated && this->data)
		{		
			ExFreePool(this->data);
			this->Allocated = false;
			this->data = 0;
			this->size = 0;
		}
	}

	NTSTATUS GetFileSize();
	NTSTATUS ReadFile();
	NTSTATUS WriteFile();
	NTSTATUS FileIsx64(bool& x64);

	NTSTATUS OpenFile();
	NTSTATUS CloseFile();

	bool Allocated;
	bool HandleOpened;
	
	PVOID data;
	UINT32 size;

	HANDLE FileHandle;
	IO_STATUS_BLOCK ioStatusBlock;

	UNICODE_STRING path;
};




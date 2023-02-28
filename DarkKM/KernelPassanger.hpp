#pragma once


namespace DK
{
	template<typename T>
	class Vector
	{
	public:
		u16                         Size;
		u16                         Capacity;
		T*							Data;

		typedef T                   value_type;
		typedef value_type* iterator;
		typedef const value_type* const_iterator;

		Vector() { Size = NULL, Capacity = NULL, Data = NULL; }
		
		void* operator new(size_t size)
		{
			return ALLOCATE(sizeof(Vector<T>));
		}

		void operator delete(void* ptr)
		{
			reinterpret_cast<Vector*>(ptr)->free();
			FREE(ptr);
		}

		inline value_type& at(uintptr_t i) { return this->Data[i]; }
		inline value_type& operator[](int i) { return Data[i]; }
		inline const value_type& operator[](int i) const { return Data[i]; }

		inline void                 clear() { if (Data) { this->free();} }
		inline iterator             begin() { return Data; }
		inline const_iterator       begin() const { return Data; }
		inline iterator             end() { return Data + Size; }
		inline const_iterator       end() const { return Data + Size; }
		inline value_type& front() { IM_ASSERT(Size > 0); return Data[0]; }
		inline const value_type& front() const { return Data[0]; }
		inline value_type& back() { IM_ASSERT(Size > 0); return Data[Size - 1]; }
		inline const value_type& back() const { return Data[Size - 1]; }

		inline void free()
		{
			if (this->Data)
				FREE(this->Data);

			this->Size = 0;
			this->Data = 0;
			this->Capacity = 0;
		}

		inline void Reserve(uint32_t NewSize)
		{
			T* temp = (T*)ALLOCATE(NewSize * sizeof(T));

			// Automaticly move old data
			if (this->Data && NewSize >= this->Size)
			{
				RtlCopyMemory(temp, this->Data, this->Size * sizeof(T));
				FREE(this->Data);
			}

			this->Data = temp;
		}

		inline void CopyFrom(Vector<T>* From)
		{
			// Clear Old or Copying an empty array
			if (this->Data || !From->Size)
				this->free();

			// If not Copying From Empty Array
			if (From->Size)
			{
				this->Reserve(From->Size);
				this->Size = From->Size;
				this->Capacity = From->Capacity;

				if (From->Data && this->Data)
					RtlCopyMemory(this->Data, From->Data, From->Size * sizeof(T));
			}

		}

		inline void AddItem(const T& v)
		{

			// Filled already, lets just give it space for 100 more objects
			if (Size >= Capacity || !Data)
			{
				Capacity += 50;
				this->Reserve(Capacity);
			}

			// Add Item
			Data[Size] = v;

			// Increase Size
			Size += 1;
		}
	};

#define KerneasdasdaslLogger_PRINT(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, __VA_ARGS__)


	template<typename T>
	class StaticString
	{
	public:
		static void Format(T* Buffer,size_t maxcch, const T* format, ...)
		{
			bool WideChar = sizeof(T) == 2;
			va_list argptr;
			va_start(argptr, format);

			if (WideChar)
				RtlStringCchVPrintfW(reinterpret_cast<NTSTRSAFE_PWSTR>(Buffer), maxcch, reinterpret_cast<NTSTRSAFE_PCWSTR>(format), argptr);
			else
				RtlStringCchVPrintfA(reinterpret_cast<NTSTRSAFE_PSTR>(Buffer), maxcch, reinterpret_cast<NTSTRSAFE_PCSTR>(format), argptr);

			va_end(argptr);
		}
	};

	// new DString<CHAR>("Test")
	// new DString<WCHAR>(L"Test")
	template<typename T>
	class DString
	{
	public:
		bool							WideChar;
		uint32_t                        Length;
		T* Buffer;

		void* operator new(size_t size)
		{
			return ALLOCATE(sizeof(DString<T>));
		}

		void operator delete(void* ptr)
		{
			// Freebuffer
			auto pBuffer = reinterpret_cast<DString<T>*>(ptr)->Buffer;

			if (pBuffer)
				FREE(pBuffer);

			// Free yourself
			FREE(ptr);
		}

		DString()
		{
			this->Length = 0;
			this->Buffer = 0;
		}

		DString(size_t size)
		{
			this->Length = 0;
			this->Buffer = (T*)ALLOCATE(size * sizeof(T));
		}

		template<class ... Types>
		DString(const T* txt, Types ... args)
		{
			this->WideChar = sizeof(T) == 2;
			this->Format(txt, (args)...);
		}


		inline void SetByText(const T* Text)
		{
			this->Length = this->WideChar ? wcslen(reinterpret_cast<const WCHAR*>(Text)) : strlen(reinterpret_cast<const CHAR*>(Text));
			this->Buffer = (T*)ALLOCATE(this->Length * sizeof(T));
			RtlCopyMemory(this->Buffer, Text, this->Length * sizeof(T));
			this->Buffer[this->Length] = 0x0;
		}

		inline void Format(const T* format, ...)
		{
			auto TempBuffer = (T*)ALLOCATE(512 * sizeof(T));

			va_list argptr;
			va_start(argptr, format);

			
			if (this->WideChar)
				RtlStringCchVPrintfW(reinterpret_cast<NTSTRSAFE_PWSTR>(TempBuffer), 512, reinterpret_cast<NTSTRSAFE_PCWSTR>(format), argptr);
			else
				RtlStringCchVPrintfA(reinterpret_cast<NTSTRSAFE_PSTR>(TempBuffer), 512, reinterpret_cast<NTSTRSAFE_PCSTR>(format), argptr);

			va_end(argptr);

			this->SetByText(TempBuffer);

			FREE(TempBuffer);
		}



		inline void SetByUnicodeString(const PUNICODE_STRING us)
		{
			this->Length = us->Length / 2;
			this->Buffer = (T*)ALLOCATE(this->Length * sizeof(WCHAR));
			RtlCopyMemory(this->Buffer, us->Buffer, this->Length * sizeof(WCHAR));
			this->Buffer[this->Length] = 0x0;
		}
	};

	// new AString("Test")
	class AString
	{
	public:
		u32								Length;
		char*							Buffer;

		void* operator new(size_t size)
		{
			return ALLOCATE(sizeof(AString));
		}

		void operator delete(void* ptr)
		{
			// Freebuffer
			auto pBuffer = reinterpret_cast<AString*>(ptr)->Buffer;

			if (pBuffer)
				FREE(pBuffer);

			// Free yourself
			FREE(ptr);
		}

		typedef char T;

		AString(size_t size)
		{
			this->Length = 0;
			this->Buffer = (T*)ALLOCATE(size * sizeof(T));
		}

		template<class ... Types>
		AString(const T* txt, Types ... args)
		{
			this->Format(txt, (args)...);
		}
	
		inline void SetByText(const T* Text)
		{
			this->Length = strlen(reinterpret_cast<const CHAR*>(Text));
			this->Buffer = (T*)ALLOCATE(this->Length * sizeof(T));
			RtlCopyMemory(this->Buffer, Text, this->Length * sizeof(T));
			this->Buffer[this->Length] = 0x0;
		}

		inline void Format(const T* format, ...)
		{
			auto TempBuffer = (T*)ALLOCATE(512 * sizeof(T));
			va_list argptr;
			va_start(argptr, format);	
			RtlStringCchVPrintfA(reinterpret_cast<NTSTRSAFE_PSTR>(TempBuffer), 512, reinterpret_cast<NTSTRSAFE_PCSTR>(format), argptr);
			va_end(argptr);
			this->SetByText(TempBuffer);
			FREE(TempBuffer);
		}
	};

	// new WString(L"Test")
	class WString
	{
	public:
		u32								Length;
		wchar_t*						Buffer;

		void* operator new(size_t size)
		{
			return ALLOCATE(sizeof(WString));
		}

		void operator delete(void* ptr)
		{
			// Freebuffer
			auto pBuffer = reinterpret_cast<WString*>(ptr)->Buffer;

			if (pBuffer)
				FREE(pBuffer);

			// Free yourself
			FREE(ptr);
		}

		typedef wchar_t T;

		WString()
		{
			this->Length = 0;
			this->Buffer = 0;
		}

		template<class ... Types>
		WString(const T* txt, Types ... args)
		{
			this->Format(txt, (args)...);
		}

		inline void SetByText(const T* Text)
		{
			this->Length = wcslen(reinterpret_cast<const WCHAR*>(Text));
			this->Buffer = (T*)ALLOCATE(this->Length * sizeof(T));
			RtlCopyMemory(this->Buffer, Text, this->Length * sizeof(T));
			this->Buffer[this->Length] = 0x0;
		}

		inline void Format(const T* format, ...)
		{
			auto TempBuffer = (T*)ALLOCATE(512 * sizeof(T));
			va_list argptr;
			va_start(argptr, format);	
			RtlStringCchVPrintfW(reinterpret_cast<NTSTRSAFE_PWSTR>(TempBuffer), 512, reinterpret_cast<NTSTRSAFE_PCWSTR>(format), argptr);
			va_end(argptr);
			this->SetByText(TempBuffer);
			FREE(TempBuffer);
		}

		inline void SetByUnicodeString(const PUNICODE_STRING us)
		{
			this->Length = us->Length / 2;
			this->Buffer = (T*)ALLOCATE(this->Length * sizeof(WCHAR));
			RtlCopyMemory(this->Buffer, us->Buffer, this->Length * sizeof(WCHAR));
			this->Buffer[this->Length] = 0x0;
		}
	};

};

#pragma region Structures

 struct Cor
{
	 Cor()
	 {
		 RtlZeroMemory(this, sizeof(Cor));
	 }

	 Cor(LONG xx, LONG yy)
	 {
		 x = xx;
		 y = yy;
	 }

	LONG  x;
	LONG  y;
};

struct DarkBox
{
	DarkBox()
	{
		RtlZeroMemory(this, sizeof(DarkBox));
	}
	Cor TopLeft;
	Cor BottomRight;
};

struct DarkRECT
{
	DarkRECT()
	{
		RtlZeroMemory(this, sizeof(DarkRECT));
	}

	void* operator new(size_t size)
	{
		return ALLOCATE(sizeof(DarkRECT));
	}

	void operator delete(void* ptr)
	{
		FREE(ptr);
	}

	LONG    left;
	LONG    top;
	LONG    right;
	LONG    bottom;
};

union DarkInteger
{
	DarkInteger()
	{
		RtlZeroMemory(this, sizeof(DarkInteger));
	}

	struct {
		ULONG LowPart;
		LONG HighPart;
	};
	LONGLONG QuadPart;
};

struct KModule
{
	KModule()
	{
		RtlZeroMemory(this, sizeof(KModule));
	}

	void* operator new(size_t size)
	{
		return ALLOCATE(sizeof(KModule));
	}

	void operator delete(void* ptr)
	{
		FREE(ptr);
	}

	DarkInteger Base;
	DarkInteger Size;
	DK::DString<WCHAR>* Name;
	bool isWow64;
};

#pragma endregion
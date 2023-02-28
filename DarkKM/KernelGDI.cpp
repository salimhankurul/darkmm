 #include "Main.h"

#pragma region DEFS

//https://github.com/RanchoIce/44Con2018/blob/master/44Con-Gaining%20Remote%20System%20Subverting%20The%20DirectX%20Kernel.pdf

#define PATCOPY             (DWORD)0x00F00021
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
//////////////////////////////////////

#define CSGO_ONLINE 1
#define PUBG_ONLINE 0
#define DEBUG_ONLINE 0
// Hook
uint8_t* CallAddress = NULL;
uint8_t* OriginalCall = NULL;

//#pragma optimize("", off)
//using fPsGetCurrentProcess = int64_t(__fastcall*)(void*, int);
//int64_t __fastcall BlaBla(void* a, int b)
//{
//	auto pReturnValue = reinterpret_cast<fPsGetCurrentProcess>(0xFFFFFFFFFFFFFBB)(a, b);
//	auto pMyCall = reinterpret_cast<fPsGetCurrentProcess>(0xFFFFFFFFFFFFFAA)(a, b);
//	return pReturnValue;
//}
//#pragma optimize("", on)

byte shellCode[] =
{
	0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x48, 0x83, 0xEC, 0x38, 0x8B, 0x54, 0x24,
	0x48, 0x48, 0x8B, 0x4C, 0x24, 0x40, 0x48, 0xB8, 0xBB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F,
	0xFF, 0xD0, 0x48, 0x89, 0x44, 0x24, 0x20, 0x8B, 0x54, 0x24, 0x48, 0x48, 0x8B, 0x4C, 0x24, 0x40,
	0x48, 0xB8, 0xAA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0xFF, 0xD0, 0x48, 0x89, 0x44, 0x24,
	0x28, 0x48, 0x8B, 0x44, 0x24, 0x20, 0x48, 0x83, 0xC4, 0x38, 0xC3
};

#pragma endregion

#pragma region GDIRenderer

namespace GDIRenderer
{
	class FPSCounter
	{
	public:

		FPSCounter()
		{
			AcceptedFrames = 0;
			RejectedFrames = 0;
			frameTime = 0;
			KeQuerySystemTime(&tframeStart);
			KeQuerySystemTime(&tframeEnd);

			MinFrameTime = 0;
		}

		FPSCounter(uint16_t Limit)
		{
			AcceptedFrames = 0;
			RejectedFrames = 0;
			frameTime = 0;
			KeQuerySystemTime(&tframeStart);
			KeQuerySystemTime(&tframeEnd);

			MinFrameTime = Limit;
		}

		void* operator new(size_t size)
		{
			return ALLOCATE(sizeof(FPSCounter));
		}

		void operator delete(void* ptr)
		{
			FREE(ptr);
		}

		bool AcceptedFramesCounter()
		{
			static uint16_t counter = 0;

			EXECUTE_EVERY(GTI_SECONDS(1), {
				// Set Overlay FPS
				AcceptedFrames = counter;
				// Reset Counter
				counter = 0;
				return true;
			})

			counter++;
			return true;
		}

		bool RejectedFramesCounter()
		{
			static uint16_t counter = 0;

			EXECUTE_EVERY(GTI_SECONDS(1), {
				// Set Overlay FPS
				RejectedFrames = counter;
				// Reset Counter
				counter = 0;
				return false;
			})

			counter++;
			return false;
		}

		void FrameStart()
		{
			KeQuerySystemTime(&tframeStart);
		}

		void FrameEnd()
		{
			KeQuerySystemTime(&tframeEnd);
			frameTime = (tframeEnd.QuadPart - tframeStart.QuadPart);
		}

		bool Tick()
		{

			if (!MinFrameTime) // No Frame Limit
				return AcceptedFramesCounter();

			EXECUTE_EVERY(GTI_MILLISECONDS(MinFrameTime), {
				return AcceptedFramesCounter();
			})

			return RejectedFramesCounter();
		}

		uint16_t AcceptedFrames;
		uint16_t RejectedFrames;

		uint16_t MinFrameTime;


		LARGE_INTEGER tframeStart;
		LARGE_INTEGER tframeEnd;
		uint64_t frameTime; // macrosecond
	};

	// This will turn float to 2 integer



	template<class T>
	class RenderText
	{
	public:

		template<class ... Args>
		static void Draw(HDC pHDC, int x, int y, const T* format, Args... args)
		{
			static wchar_t dbgstr[960];
			RtlZeroMemory(dbgstr, sizeof(dbgstr));
			DK::StaticString<wchar>::Format(dbgstr, CCHWC(dbgstr), format, args...);
			dxgkrnl_hook::GreExtTextOutWInternal(pHDC, x, y, NULL, NULL, dbgstr, wcsnlen_s(dbgstr, CCHWC(dbgstr)), NULL, NULL);
		}
	};

	class KernelFloatToInt
	{
	public:
		KernelFloatToInt(float a, float decimalcount)
		{
			First = 0, Second = 0;

			if (a == 0.f)
				return;

			First = static_cast<i32>(a);

			a = a - static_cast<float>(First);

			a *= KernelMath::pow(10.f, decimalcount);

			Second = static_cast<i32>(a);
		}
		i32 First;
		i32 Second;
	};

	void RectBox(HDC pHDC, DarkBox r, int size, HBRUSH Color)
	{
		auto left = r.TopLeft.x;
		auto top = r.TopLeft.y;
		auto right = r.BottomRight.x;
		auto bottom = r.BottomRight.y;

		auto height = KernelMath::fabs(bottom - top);
		auto width = KernelMath::fabs(right - left);

		if (left)
		{
			auto prevhbr = dxgkrnl_hook::GreSelectBrush(pHDC, Color);

			dxgkrnl_hook::NtGdiPatBlt(pHDC, left, top, size, height, PATCOPY); // sol 1
			dxgkrnl_hook::NtGdiPatBlt(pHDC, right, top, size, height, PATCOPY); // sol 1

			dxgkrnl_hook::NtGdiPatBlt(pHDC, left, top, width, size, PATCOPY); // 
			dxgkrnl_hook::NtGdiPatBlt(pHDC, left, bottom - size, width, size, PATCOPY);

			if (prevhbr)
				dxgkrnl_hook::GreSelectBrush(pHDC, prevhbr);
		}
	};

	void FilledBox(HDC pHDC, DarkBox r, HBRUSH Color)
	{
		auto left = r.TopLeft.x;
		auto top = r.TopLeft.y;
		auto right = r.BottomRight.x;
		auto bottom = r.BottomRight.y;

		if (left)
		{
			auto prevhbr = dxgkrnl_hook::GreSelectBrush(pHDC, Color);

			dxgkrnl_hook::NtGdiPatBlt(pHDC, left, top, right - left, bottom - top, PATCOPY);

			if (prevhbr)
				dxgkrnl_hook::GreSelectBrush(pHDC, prevhbr);
		}
	};

	auto DrawPlayerBox(HDC pHDC, DarkBox r, int size, HBRUSH Color, int optimization_level = 0)
	{
		auto left = r.TopLeft.x;
		auto top = r.TopLeft.y;
		auto right = r.BottomRight.x;
		auto bottom = r.BottomRight.y;

		auto height = KernelMath::fabs(bottom - top);
		auto width = KernelMath::fabs(right - left);

		// Box is to little for complicated box
		bool forcesimplebox = height < 20 ? 1 : 0;

		if (left)
		{
			if (forcesimplebox || optimization_level == 1)
				RectBox(pHDC, r, 1, Color);
			else if (optimization_level == 2)
			{
				return;
			}
			else
			{
				auto prevhbr = dxgkrnl_hook::GreSelectBrush(pHDC, Color);

				dxgkrnl_hook::NtGdiPatBlt(pHDC, left, top, size, height / 4, PATCOPY); // sol 1
				dxgkrnl_hook::NtGdiPatBlt(pHDC, left, top + (height * 3 / 4), size, height / 4, PATCOPY); // sol 2

				dxgkrnl_hook::NtGdiPatBlt(pHDC, right, top, size, height / 4, PATCOPY); // sol 1
				dxgkrnl_hook::NtGdiPatBlt(pHDC, right, top + (height * 3 / 4), size, height / 4, PATCOPY); // sol 1

				dxgkrnl_hook::NtGdiPatBlt(pHDC, left, top, width, size, PATCOPY); // 
				dxgkrnl_hook::NtGdiPatBlt(pHDC, left, bottom - size, width, size, PATCOPY);


				if (prevhbr)
					dxgkrnl_hook::GreSelectBrush(pHDC, prevhbr);
			}
		}
	};

	auto DrawHealtBar(HDC pHDC, DarkBox r, float Health, int size, HBRUSH Color, HBRUSH OutlineColor)
	{
		auto left = r.TopLeft.x;
		auto top = r.TopLeft.y;
		auto right = r.BottomRight.x;
		auto bottom = r.BottomRight.y;

		auto height = KernelMath::fabs(bottom - top);
		auto width = KernelMath::fabs(right - left);

		float hbd = KernelMath::fabs(height * 0.05f); //healtbardistance

		// min height for hbd
		if (hbd < 7.f)
			hbd = 7.f;

		if (hbd > 12.f)
			hbd = 12.f;

		DarkBox hbb; //HealthBarBox
		hbb.TopLeft = Cor(left, top - hbd);
		hbb.BottomRight = Cor(right + size, top - size);

		// min height for hbb
		if (hbb.BottomRight.y - hbb.TopLeft.y < 5)
			hbb.TopLeft.y = hbb.BottomRight.y + 5;

		// max height for hbb
		if (hbb.BottomRight.y - hbb.TopLeft.y > 10)
			hbb.TopLeft.y = hbb.BottomRight.y + 10;

		if (left)
		{

			float barh = (width * Health / 100) + size;

			auto prevhbr = dxgkrnl_hook::GreSelectBrush(pHDC, Color);

			dxgkrnl_hook::NtGdiPatBlt(pHDC,
				hbb.TopLeft.x,
				hbb.TopLeft.y,
				barh,
				hbb.BottomRight.y - hbb.TopLeft.y, PATCOPY);

			if (prevhbr)
				dxgkrnl_hook::GreSelectBrush(pHDC, prevhbr);

			RectBox(pHDC, hbb, 1, OutlineColor); // Outside
		}
	};

}

using namespace GDIRenderer;

#pragma endregion

#pragma region MAIN

NTSTATUS dxgkrnl_hook::Unhook()
{
	DWORD CallBuffer = static_cast<DWORD>(OriginalCall - CallAddress - 0x5);
	KernelMemory::BruteForceWrite(CallAddress + 1, &CallBuffer, sizeof(CallBuffer));

	// Wait Game thread to finish
	KernelUtilities::Sleep(250);
	KernelBeep::Beep(1245, 150);

	DEBUG_PRINT("[%ws]::%hs:: submit_command_hook UnInitialized \r\n", PROJECT_NAME, __FUNCTION__);
	return 0;
}

bool dxgkrnl_hook::AlreadHooked(u8* Address)
{
	return *reinterpret_cast<u8*>(Address) == 0x89 ? true : false;
}

NTSTATUS dxgkrnl_hook::getForegroundWindowName(LARGE_UNICODE_STRING& WndName) {
	static KGUARDED_MUTEX lock;
	static bool lockinit = false;
	NTSTATUS status = STATUS_SUCCESS;

	if (!lockinit)
	{
		lockinit = true;
		KeInitializeGuardedMutex(&lock);
	}

	KeAcquireGuardedMutex(&lock);

	auto hwnd = dxgkrnl_hook::NtUserGetForegroundWindow();

	if (!hwnd)
	{
		status = STATUS_NO_TOKEN;
		goto end;
	}

	auto wnd = dxgkrnl_hook::ValidateHwnd(hwnd);

	if (!wnd)
	{
		status = STATUS_NO_TOKEN;
		goto end;
	}

	memset(&WndName, 0, sizeof(LARGE_UNICODE_STRING));
	dxgkrnl_hook::ProtectedLargeUnicodeStringWNDstrName(&wnd->strName, &WndName);

end:
	KeReleaseGuardedMutex(&lock);
	return status;
}


NTSTATUS dxgkrnl_hook::setTargetWindow(wchar_t* name)
{

	static KGUARDED_MUTEX lock;
	static bool lockinit = false;

	if (!lockinit)
	{
		lockinit = true;
		KeInitializeGuardedMutex(&lock);
	}

	KeAcquireGuardedMutex(&lock);

	NTSTATUS status = STATUS_SUCCESS;
	
	if (!dxgkrnl_hook::TargetWindow)
	{
	
		static LARGE_UNICODE_STRING WndName;
		status = dxgkrnl_hook::getForegroundWindowName(WndName);

		if (status != STATUS_SUCCESS) goto end;

		if (WndName.Length)
		{
			if (!wcscmp(WndName.Buffer, name))
			{
				auto hwnd = dxgkrnl_hook::NtUserGetForegroundWindow();
				dxgkrnl_hook::TargetWindow = hwnd;
				DEBUG_PRINT("[%ws]::%hs Target Window Found \"%ws\"  \r\n", PROJECT_NAME, __FUNCTION__, WndName.Buffer);
				memset(&WndName, 0, sizeof(LARGE_UNICODE_STRING));
			}
		/*	else {
				DEBUG_PRINT("[%ws]::%hs Target Window Found \"%ws\"  \r\n", PROJECT_NAME, __FUNCTION__, WndName.Buffer);
			}*/

		}
	}

	if (!dxgkrnl_hook::TargetWindow)
		status = STATUS_NO_TOKEN;
	
end:
	KeReleaseGuardedMutex(&lock);
	return status;
}

NTSTATUS dxgkrnl_hook::setDWM()
{
	static KGUARDED_MUTEX lock;
	static bool lockinit = false;

	if (!lockinit)
	{
		lockinit = true;
		KeInitializeGuardedMutex(&lock);
	}

	KeAcquireGuardedMutex(&lock);

	if (!dxgkrnl_hook::PEDWM)
	{
		static auto dwms = skCrypt("dwm");
		const auto current_process = IoGetCurrentProcess();
		const auto process_name = PsGetProcessImageFileName(current_process);

		dwms.decrypt();
		bool CurrentProcessIsDWM = strstr(process_name, dwms);
		dwms.encrypt();

		if (CurrentProcessIsDWM)
		{
			dxgkrnl_hook::PEDWM = IoGetCurrentProcess();
			DEBUG_PRINT("[%ws]::%hs:: PEDWM Successfully Inited \r\n", PROJECT_NAME, __FUNCTION__);

			auto status = STATUS_SUCCESS;

			static auto win32kbase_sys_s = skCrypt("win32kbase.sys");
			static auto win32kfull_sys_s = skCrypt("win32kfull.sys");

			//win32kbase
			static auto NtUserGetDCs = skCrypt("NtUserGetDC");
			static auto NtUserReleaseDCs = skCrypt("NtUserReleaseDC");
			static auto NtUserGetAsyncKeyStates = skCrypt("NtUserGetAsyncKeyState");
			static auto GetDispInfos = skCrypt("GetDispInfo");
			static auto GreSelectBrushs = skCrypt("GreSelectBrush");
			static auto ValidateHwnds = skCrypt("ValidateHwnd");

			//win32kfull
			static auto NtGdiPatBlts = skCrypt("NtGdiPatBlt");
			static auto NtGdiCreateSolidBrushs = skCrypt("NtGdiCreateSolidBrush");
			static auto NtGdiCreateCompatibleDCs = skCrypt("NtGdiCreateCompatibleDC");
			static auto NtUserGetForegroundWindows = skCrypt("NtUserGetForegroundWindow");
			static auto NtUserWindowFromDCs = skCrypt("NtUserWindowFromDC");
			static auto NtUserSetLayeredWindowAttributess = skCrypt("NtUserSetLayeredWindowAttributes");
			static auto NtUserGetWindowDCs = skCrypt("NtUserGetWindowDC");


			win32kbase_sys_s.decrypt(); win32kfull_sys_s.decrypt(); NtUserGetDCs.decrypt(); NtUserReleaseDCs.decrypt();
			NtUserGetAsyncKeyStates.decrypt(); GetDispInfos.decrypt(); NtGdiPatBlts.decrypt(); ValidateHwnds.decrypt();
			GreSelectBrushs.decrypt(); NtGdiCreateSolidBrushs.decrypt(); NtGdiCreateCompatibleDCs.decrypt(); NtUserGetForegroundWindows.decrypt();
			NtUserWindowFromDCs.decrypt(); NtUserSetLayeredWindowAttributess.decrypt(); NtUserGetWindowDCs.decrypt();

			dxgkrnl_hook::NtGdiCreateSolidBrush = reinterpret_cast<HBRUSH(*)(COLORREF, HBRUSH)>(
				KernelModule::GetKernelModuleExport(win32kfull_sys_s, NtGdiCreateSolidBrushs)
				);

			dxgkrnl_hook::GreSelectBrush = reinterpret_cast<HBRUSH(*)(HDC, HBRUSH)>(
				KernelModule::GetKernelModuleExport(win32kbase_sys_s, GreSelectBrushs)
				);

			dxgkrnl_hook::NtUserGetDC = reinterpret_cast<HDC(*)(HWND)>(
				KernelModule::GetKernelModuleExport(win32kbase_sys_s, NtUserGetDCs)
				);

			dxgkrnl_hook::NtUserReleaseDC = reinterpret_cast<int(*)(HWND, HDC)>(
				KernelModule::GetKernelModuleExport(win32kbase_sys_s, NtUserReleaseDCs)
				);

			dxgkrnl_hook::NtUserGetAsyncKeyState = reinterpret_cast<SHORT(*)(int)>(
				KernelModule::GetKernelModuleExport(win32kbase_sys_s, NtUserGetAsyncKeyStates)
				);

			dxgkrnl_hook::GetDispInfo = reinterpret_cast<DISPLAYINFO * (*)()>(
				KernelModule::GetKernelModuleExport(win32kbase_sys_s, GetDispInfos)
				);

			dxgkrnl_hook::NtGdiPatBlt = reinterpret_cast<int(*)(HDC, INT, INT, INT, INT, DWORD)>(
				KernelModule::GetKernelModuleExport(win32kfull_sys_s, NtGdiPatBlts)
				);

			dxgkrnl_hook::NtGdiCreateCompatibleDC = reinterpret_cast<HDC(*)(HDC)>(
				KernelModule::GetKernelModuleExport(win32kbase_sys_s, NtGdiCreateCompatibleDCs)
				);

			dxgkrnl_hook::NtUserGetForegroundWindow = reinterpret_cast<HWND(*)()>(
				KernelModule::GetKernelModuleExport(win32kfull_sys_s, NtUserGetForegroundWindows)
				);

			dxgkrnl_hook::NtUserWindowFromDC = reinterpret_cast<HWND(*)(HDC)>(
				KernelModule::GetKernelModuleExport(win32kfull_sys_s, NtUserWindowFromDCs)
				);

			dxgkrnl_hook::NtUserSetLayeredWindowAttributes = reinterpret_cast<BOOL(*)(HWND, COLORREF, BYTE, DWORD)>(
				KernelModule::GetKernelModuleExport(win32kfull_sys_s, NtUserSetLayeredWindowAttributess)
				);

			dxgkrnl_hook::NtUserGetWindowDC = reinterpret_cast<HDC(*)(HWND)>(
				KernelModule::GetKernelModuleExport(win32kfull_sys_s, NtUserGetWindowDCs)
				);

			dxgkrnl_hook::ValidateHwnd = reinterpret_cast<WND * (*)(HWND)>(
				KernelModule::GetKernelModuleExport(win32kbase_sys_s, ValidateHwnds)
				);

			NtGdiCreateSolidBrushs.encrypt();
			GreSelectBrushs.encrypt();
			NtUserGetDCs.encrypt();
			NtUserReleaseDCs.encrypt();
			NtUserGetAsyncKeyStates.encrypt();
			GetDispInfos.encrypt();
			NtGdiPatBlts.encrypt();
			NtGdiCreateCompatibleDCs.encrypt();
			NtUserGetForegroundWindows.encrypt();
			NtUserWindowFromDCs.encrypt();
			NtUserSetLayeredWindowAttributess.encrypt();
			NtUserGetWindowDCs.encrypt();
			ValidateHwnds.encrypt();

			auto BuildNo = KernelImporter::BuildNo;

			if (BuildNo == 18362 || BuildNo == 18363 || BuildNo == 19041 || BuildNo == 19042)
			{

				auto SafePatternToCall = [](const char* ModuleName, const char* Pattern, const char* Sig) -> uint8_t*
				{
					auto a = KernelModule::FindPatternInModule(ModuleName, (BYTE*)Pattern, (char*)Sig);

					if (a)
						return KernelMemory::follow_call<uint8_t*>(a);
					else
						return nullptr;
				};
				//GetPrimaryMonitorRect
				dxgkrnl_hook::GetMonitorRect = reinterpret_cast<DarkRECT * (*)(DarkRECT*, QWORD)>(
					SafePatternToCall(win32kfull_sys_s,
						"\xE8\x00\x00\x00\x00\xEB\x5F\x48\x8B\x03", "x????xxxxx"));

				dxgkrnl_hook::GreExtTextOutWInternal = reinterpret_cast<int(*)(HDC, unsigned int, unsigned int,
					unsigned int, unsigned __int16*, LPWSTR, INT, int*, void*)>(
						SafePatternToCall(win32kfull_sys_s,
							"\xE8\x00\x00\x00\x00\x8B\x05\x00\x00\x00\x00\x45\x33\xC9\x41",
							"x????xx????xxxx"));

				dxgkrnl_hook::GreSetBkMode = reinterpret_cast<int(*)(HDC, int)>(
					SafePatternToCall(win32kfull_sys_s,
						"\xE8\x00\x00\x00\x00\xBA\x00\x00\x00\x00\x48\x8B\xCB\x8B",
						"x????x????xxxx"));

				dxgkrnl_hook::GreSetBkColor = reinterpret_cast<int(*)(HDC, int)>(
					SafePatternToCall(win32kfull_sys_s,
						"\xE8\x00\x00\x00\x00\x8B\xD8\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x08\x8B",
						"x????xxxxx????xxxx"));

				dxgkrnl_hook::GreSetTextColor = reinterpret_cast<int(*)(HDC, int)>(
					SafePatternToCall(win32kfull_sys_s,
						"\xE8\x00\x00\x00\x00\x44\x8B\xF8\x44\x89",
						"x????xxxxx"));

				dxgkrnl_hook::ProtectedLargeUnicodeStringWNDstrName = reinterpret_cast<PLARGE_UNICODE_STRING(*)(PLARGE_UNICODE_STRING, PLARGE_UNICODE_STRING)>(
					KernelModule::FindPatternInModule(win32kfull_sys_s,
						(BYTE*)"\x48\x8B\x01\x4C\x8B\xDA\x48",
						"xxxxxxx"));

				win32kbase_sys_s.encrypt(); win32kfull_sys_s.encrypt();
			}
			else
			{

				DEBUG_PRINT("[%ws]::%hs:: Windows Version Error \r\n", PROJECT_NAME, __FUNCTION__);
				KeReleaseGuardedMutex(&lock);
				dxgkrnl_hook::Unhook();
				return STATUS_NO_TOKEN;
			}

			if (!dxgkrnl_hook::GetMonitorRect || !dxgkrnl_hook::GreExtTextOutWInternal || !dxgkrnl_hook::GreSetBkMode ||
				!dxgkrnl_hook::GreSetBkColor || !dxgkrnl_hook::GreSetTextColor || !dxgkrnl_hook::ProtectedLargeUnicodeStringWNDstrName)
			{

				DEBUG_PRINT("[%ws]::%hs Error Patterns \r\n", PROJECT_NAME, __FUNCTION__);
				KeReleaseGuardedMutex(&lock);
				dxgkrnl_hook::Unhook();
				return STATUS_NO_TOKEN;
			}

			if (!dxgkrnl_hook::NtUserGetDC || !dxgkrnl_hook::NtGdiCreateCompatibleDC || !dxgkrnl_hook::NtUserGetForegroundWindow ||
				!dxgkrnl_hook::NtUserGetWindowDC || !dxgkrnl_hook::NtUserWindowFromDC || !dxgkrnl_hook::NtUserSetLayeredWindowAttributes ||
				!dxgkrnl_hook::NtUserReleaseDC || !dxgkrnl_hook::NtGdiCreateSolidBrush || !dxgkrnl_hook::GreSelectBrush ||
				!dxgkrnl_hook::GreSetBkMode || !dxgkrnl_hook::GreSetBkColor || !dxgkrnl_hook::GreSetTextColor ||
				!dxgkrnl_hook::NtGdiPatBlt || !dxgkrnl_hook::GreExtTextOutWInternal || !dxgkrnl_hook::NtUserGetAsyncKeyState ||
				!dxgkrnl_hook::GetMonitorRect || !dxgkrnl_hook::GetDispInfo || !dxgkrnl_hook::ValidateHwnd
				)
			{
				DEBUG_PRINT("[%ws]::%hs Error Exports \r\n", PROJECT_NAME, __FUNCTION__);
				KeReleaseGuardedMutex(&lock);
				dxgkrnl_hook::Unhook();
				return STATUS_NO_TOKEN;
			}


			DEBUG_PRINT("[%ws]::%hs:: NtUserGetDC: %llX \r\n", PROJECT_NAME, __FUNCTION__, dxgkrnl_hook::NtUserGetDC);
			DEBUG_PRINT("[%ws]::%hs:: NtUserReleaseDC: %llX \r\n", PROJECT_NAME, __FUNCTION__, dxgkrnl_hook::NtUserReleaseDC);
			DEBUG_PRINT("[%ws]::%hs:: NtGdiPatBlt: %llX \r\n", PROJECT_NAME, __FUNCTION__, dxgkrnl_hook::NtGdiPatBlt);
			DEBUG_PRINT("[%ws]::%hs:: GreExtTextOutWInternal: %llX \r\n", PROJECT_NAME, __FUNCTION__, dxgkrnl_hook::GreExtTextOutWInternal);
			DEBUG_PRINT("[%ws]::%hs:: NtUserGetAsyncKeyState: %llX \r\n", PROJECT_NAME, __FUNCTION__, dxgkrnl_hook::NtUserGetAsyncKeyState);
			DEBUG_PRINT("[%ws]::%hs:: GetMonitorRect: %llX \r\n", PROJECT_NAME, __FUNCTION__, dxgkrnl_hook::GetMonitorRect);
			DEBUG_PRINT("[%ws]::%hs:: GetDispInfo: %p \r\n", PROJECT_NAME, __FUNCTION__, dxgkrnl_hook::GetDispInfo);
			DEBUG_PRINT("[%ws]::%hs:: ntGdiCreateSolidBrush: %llX \r\n", PROJECT_NAME, __FUNCTION__, dxgkrnl_hook::NtGdiCreateSolidBrush);
			DEBUG_PRINT("[%ws]::%hs:: GreSelectBrush: %p \r\n", PROJECT_NAME, __FUNCTION__, dxgkrnl_hook::GreSelectBrush);
			DEBUG_PRINT("[%ws]::%hs:: ProtectedLargeUnicodeStringWNDstrName: %p \r\n", PROJECT_NAME, __FUNCTION__, dxgkrnl_hook::ProtectedLargeUnicodeStringWNDstrName);

			//GetPrimaryMonitorRect
			auto Monitor = dxgkrnl_hook::GetDispInfo()->pMonitorFirst;
			MonitorRect = *dxgkrnl_hook::GetMonitorRect(&MonitorRect, Monitor);

			DEBUG_PRINT("[%ws]::%hs:: Monitor Initialized [%ix%i] - %i/%i/%i/%i \r\n", PROJECT_NAME, __FUNCTION__,
				MonitorRect.right, MonitorRect.bottom, MonitorRect.top,
				MonitorRect.left, MonitorRect.right, MonitorRect.bottom);

			DEBUG_PRINT("[%ws]::%hs:: GDI Functions Initing Complete \r\n", PROJECT_NAME, __FUNCTION__);

		}
	}

	KeReleaseGuardedMutex(&lock);

	KernelBeep::Beep(1568, 150);
	KernelBeep::Beep(1568, 150);

	return STATUS_SUCCESS;
}

NTSTATUS dxgkrnl_hook::setTarget(char* name)
{
	static KGUARDED_MUTEX lock;
	static bool lockinit = false;

	if (!lockinit)
	{
		lockinit = true;
		KeInitializeGuardedMutex(&lock);
	}

	KeAcquireGuardedMutex(&lock);

	if (!dxgkrnl_hook::PETarget)
	{
		const auto current_process = IoGetCurrentProcess();
		const auto process_name = PsGetProcessImageFileName(current_process);

		bool CurrentProcessIsDWM = strstr(process_name, name);

		if (CurrentProcessIsDWM)
		{
			dxgkrnl_hook::PETarget = IoGetCurrentProcess();
			DEBUG_PRINT("[%ws]::%hs:: PETarget Successfully Inited \r\n", PROJECT_NAME, __FUNCTION__);
		}
			
	}

	KeReleaseGuardedMutex(&lock);
	return STATUS_SUCCESS;
}

NTSTATUS dxgkrnl_hook::Hook()
{
	KeInitializeGuardedMutex(&dxgkrnl_hook::RenderListLock); 
	KeInitializeGuardedMutex(&dxgkrnl_hook::GameClosingLock); 
	auto BuildNo = KernelImporter::BuildNo;

	static auto winlogon_exe_s = skCrypt(L"winlogon.exe");
	static auto dxgkrnl_sys_s = skCrypt("dxgkrnl.sys");
	static auto DxgkSubmitCommand_s = skCrypt("DxgkGetDeviceState");

	winlogon_exe_s.decrypt();
	dxgkrnl_sys_s.decrypt();
	DxgkSubmitCommand_s.decrypt();

	auto Process = KernelProcess::GetPEPProcess(winlogon_exe_s);

	if (!Process)
		return STATUS_NO_TOKEN;


	KeAttachProcess(Process);

	auto DxgkGetDeviceState = KernelModule::GetKernelModuleExport(dxgkrnl_sys_s, DxgkSubmitCommand_s);

	if (!DxgkGetDeviceState)
		goto exit;

	auto DxgkGetDeviceStateInternal = KernelMemory::follow_call<uint8_t*>(DxgkGetDeviceState + 0x1F);

	DEBUG_PRINT("[%ws]::%hs:: DxgkGetDeviceState: %p \r\n", PROJECT_NAME, __FUNCTION__, DxgkGetDeviceState);
	DEBUG_PRINT("[%ws]::%hs:: DxgkGetDeviceStateInternal: %p \r\n", PROJECT_NAME, __FUNCTION__, DxgkGetDeviceStateInternal);

	auto verfix = 0;


	if (BuildNo == 18362 || BuildNo == 18363)
		verfix = 0x51;
	else if (BuildNo == 19041 || BuildNo == 19042)
		verfix = 0x50;
	else
	{
		DEBUG_PRINT("[%ws]::%hs:: Non Supported Version %i \r\n", PROJECT_NAME, __FUNCTION__, BuildNo);
		goto exit;
	}
	

	CallAddress = DxgkGetDeviceStateInternal + verfix;
	OriginalCall = KernelMemory::follow_call<uint8_t*>(CallAddress);
	
	if (dxgkrnl_hook::AlreadHooked(OriginalCall))
	{
		DEBUG_PRINT("[%ws]::%hs:: Already Hooked  \r\n", PROJECT_NAME, __FUNCTION__);
		goto exit;
	}
		
	auto ashellCode = KernelModule::FindFreeMemoryInSytemModule(dxgkrnl_sys_s, 0x1000);

	DEBUG_PRINT("[%ws]::%hs:: CallAddress: %p \r\n", PROJECT_NAME, __FUNCTION__, CallAddress);
	DEBUG_PRINT("[%ws]::%hs:: OriginalCall: %p \r\n", PROJECT_NAME, __FUNCTION__, OriginalCall);
	DEBUG_PRINT("[%ws]::%hs:: ShellCode: %p \r\n", PROJECT_NAME, __FUNCTION__, ashellCode);

	if (ashellCode)
	{

		*reinterpret_cast<DWORD64*>(shellCode + 22 + 2) = reinterpret_cast<DWORD64>(OriginalCall);
		*reinterpret_cast<DWORD64*>(shellCode + 48 + 2) = reinterpret_cast<DWORD64>(MyHookedFunc);
		//*reinterpret_cast<DWORD64*>(shellCode + 48 + 2) = reinterpret_cast<DWORD64>(DebugMyHookedFunc);
		//
		KernelMemory::BruteForceWrite(ashellCode, shellCode, sizeof(shellCode));
		DWORD CallBuffer = static_cast<DWORD>(ashellCode - CallAddress - 0x5);
		KernelMemory::BruteForceWrite(CallAddress + 1, &CallBuffer, sizeof(CallBuffer));
	}

exit:
	winlogon_exe_s.clear();
	dxgkrnl_sys_s.clear();
	DxgkSubmitCommand_s.clear();

	KeDetachProcess();
	ObDereferenceObject(Process);

	return STATUS_SUCCESS;
}

#pragma endregion

#if (CSGO_ONLINE)

#define CSGO_DEBUG DEBUG_PRINT
#include "KernelCheats/csgo.h"

// We will Draw when any other process than game it selfs calls our hook. Since we dont want game to drop fps
void DrawingThread(DK::Vector<DrawInfo>* DrawList, FPSCounter* gameFPS, FPSCounter* overlayFPS)
{
	static const auto pHDC = dxgkrnl_hook::NtUserGetDC(0);

	static const auto RedBrush = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(255, 0, 0), NULL);
	static const auto GreenBrush = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(0, 255, 0), NULL);

	static const auto VisibleBrush = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(206, 95, 10), NULL);
	static const auto NotVisibleBrush = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(245, 36, 20), NULL);
	static const auto TeamMateBrush = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(0, 0, 255), NULL);

	static const auto AimbotBrush = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(203, 66, 244), NULL);
	static const auto AimingBrush = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(64, 224, 208), NULL);

	auto RectBox = [](RECT r, HBRUSH Color)
	{
		auto prevhbr = dxgkrnl_hook::GreSelectBrush(pHDC, Color);

		dxgkrnl_hook::NtGdiPatBlt(pHDC, r.left, r.top, 1, r.bottom - r.top, PATCOPY);
		dxgkrnl_hook::NtGdiPatBlt(pHDC, r.right - 1, r.top, 1, r.bottom - r.top, PATCOPY);
		dxgkrnl_hook::NtGdiPatBlt(pHDC, r.left, r.top, r.right - r.left, 1, PATCOPY);
		dxgkrnl_hook::NtGdiPatBlt(pHDC, r.left, r.bottom - 1, r.right - r.left, 1, PATCOPY);

		if (prevhbr)
			dxgkrnl_hook::GreSelectBrush(pHDC, prevhbr);
	};

	auto FilledBox = [](RECT r, HBRUSH Color)
	{
		auto prevhbr = dxgkrnl_hook::GreSelectBrush(pHDC, Color);

		dxgkrnl_hook::NtGdiPatBlt(pHDC, r.left, r.top, r.right - r.left, r.bottom - r.top, PATCOPY);

		if (prevhbr)
			dxgkrnl_hook::GreSelectBrush(pHDC, prevhbr);
	};

	auto getBrush = [](BrushType type) -> HBRUSH {
		switch (type)
		{
		case VISIBLE:
			return VisibleBrush;
			break;
		case NOTVISIBLE:
			return NotVisibleBrush;
			break;
		case TEAMMATE:
			return TeamMateBrush;
			break;
		case AIMBOTTARGET:
			return AimbotBrush;
			break;
		case AIMING:
			return AimingBrush;
			break;
		default:
			break;
		}
		return NotVisibleBrush;
	};

	for (auto& DrawInfo : *DrawList)
	{
		const auto brush = getBrush(DrawInfo.brush);

		RectBox(DrawInfo.PlayerBox, brush);

		if (DrawInfo.PlayerDistance <= SETTINGS_ESP_DIST)
		{
			FilledBox(DrawInfo.HealthBarRect, RedBrush);
			FilledBox(DrawInfo.HealthBarRect2, GreenBrush);

			RenderText<wchar>::Draw(pHDC, DrawInfo.PlayerBox.left, DrawInfo.PlayerBox.bottom + 20, L"%hs %i %u", DrawInfo.PlayerName, DrawInfo.PlayerDistance, DrawInfo.WeaponID);
		}
	}

	if (CSGO::Singleton())
	{
		dxgkrnl_hook::GreSetBkColor(pHDC, RGB(78, 78, 78));
		dxgkrnl_hook::GreSetTextColor(pHDC, RGB(222, 222, 222));
		RenderText<wchar>::Draw(pHDC, 20, 20, L"[%i Rects][MemChunks: %i][Weapon %X-%i-%X][Dmg %i][State %i]",
			DrawList->Size, MemoryAllocator::CurrentAllocatedChunks,
			CSGO::Singleton()->LocalWeapon, CSGO::Singleton()->LocalWeaponDefIndex, CSGO::Singleton()->LastWeaponData,
			CSGO::Singleton()->LastDamageCalculated,
			CSGO::Singleton()->LastClientState);

		RenderText<wchar>::Draw(pHDC, 20, 40, L"OFPS: [%03u/%03u][%05llu]",
			overlayFPS->AcceptedFrames, overlayFPS->RejectedFrames, overlayFPS->frameTime);

		RenderText<wchar>::Draw(pHDC, 20, 60, L"GFPS: [%03u/%03u][%05llu]",
			gameFPS->AcceptedFrames, gameFPS->RejectedFrames, gameFPS->frameTime);
			
	}
}

u64 __fastcall dxgkrnl_hook::MyHookedFunc(void* data, int b)
{
	static DK::Vector<DrawInfo>* RenderList = new DK::Vector<DrawInfo>();
	static CSGO* Game = nullptr;
	static auto overlayfpscounter = new FPSCounter(15);
	static auto gamefpscounter = new FPSCounter();

	/// *****************************
	/// Init Desktop Window Manager
	/// *****************************

	if (!dxgkrnl_hook::PEDWM)
		dxgkrnl_hook::setDWM();

	/// *****************************
	/// Init Target
	/// *****************************

	if (dxgkrnl_hook::PEDWM && !dxgkrnl_hook::PETarget)
		dxgkrnl_hook::setTarget("csgo");

	/// *****************************
	/// Init Target Window
	/// *****************************

	if (dxgkrnl_hook::PEDWM && dxgkrnl_hook::PETarget)
		if (!dxgkrnl_hook::TargetWindow && IoGetCurrentProcess() == dxgkrnl_hook::PEDWM)
			dxgkrnl_hook::setTargetWindow(L"Counter-Strike: Global Offensive - Direct3D 9");


	/// *****************************
	/// Init DONE
	/// *****************************

	if (dxgkrnl_hook::NtUserGetAsyncKeyState(0x2E) & 1 || KernelUtilities::BBCheckProcessStatus(dxgkrnl_hook::PETarget) == 0) //PAGE UP key
	{
		dxgkrnl_hook::Unhook();
		return STATUS_NO_TOKEN;
	}

	/// *****************************
	/// BEFORE RENDER
	/// *****************************

	// Dont Draw Anything when game window is'nt up
	if (!dxgkrnl_hook::TargetWindow || dxgkrnl_hook::NtUserGetForegroundWindow() != dxgkrnl_hook::TargetWindow) {
		return STATUS_NO_TOKEN;
	}
		
	/// *****************************
	/// RENDER
	/// *****************************
	

	// When Game is Calling DxgkSubmitCommand
	if (dxgkrnl_hook::PETarget)
	{
		if (dxgkrnl_hook::PETarget == IoGetCurrentProcess())
		{
			// Create new Game Instance
			if (!Game)
				Game = new CSGO();
			else
			{
				if (!Game->Inited)
					Game->Init(&dxgkrnl_hook::MonitorRect);
				else
				{
					auto TempGameList = new DK::Vector<DrawInfo>();

					gamefpscounter->Tick();

					// Lets Read From Game
					gamefpscounter->FrameStart();
					Game->Tick(TempGameList);
					gamefpscounter->FrameEnd();
					
					// Set RenderList -Also Clears Old Buffer	
					KeAcquireGuardedMutex(&dxgkrnl_hook::RenderListLock);
					RenderList->CopyFrom(TempGameList);
					KeReleaseGuardedMutex(&dxgkrnl_hook::RenderListLock);

					// Clear Memory
					delete TempGameList;
				}
			}
			return 6565113516;
		}
	}

	// When DWM is Calling DxgkSubmitCommand

	if (dxgkrnl_hook::PEDWM == IoGetCurrentProcess())
	{
		if (!overlayfpscounter->Tick())
			return 6565113515;

		auto TempRenderList = new DK::Vector<DrawInfo>();

		// Set TempRenderList
		KeAcquireGuardedMutex(&dxgkrnl_hook::RenderListLock);
		TempRenderList->CopyFrom(RenderList);	
		KeReleaseGuardedMutex(&dxgkrnl_hook::RenderListLock);

		// Render Shit
		overlayfpscounter->FrameStart();
		DrawingThread(TempRenderList, gamefpscounter, overlayfpscounter);
		overlayfpscounter->FrameEnd();

		// Clear Memory
		delete TempRenderList;
	}


	return 6565113515;
}

#endif

#if (PUBG_ONLINE)

#define PUBGL_DEBUG DEBUG_PRINT
#include "KernelCheats/pubglite.hpp"

void DrawingThread(Vector<DrawInfo>* DrawList, FPSCounter* overlayfpscounter, FPSCounter* gamefpscounter, PUBGLite* Game)
{
	static const auto pHDC = dxgkrnl_hook::NtUserGetDC(0);

	// Player
	static const auto HealtBarBrushOutline = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(21, 85, 109), NULL);
	static const auto HealtBarBrush = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(255, 140, 0), NULL);

	static const auto VisibleBrush = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(0, 255, 0), NULL);
	static const auto NonVisibleBrush = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(255, 255, 0), NULL);
	static const auto TeamMateBrush = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(0, 0, 255), NULL);

	static const auto AimbotBrush = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(203, 66, 244), NULL);
	static const auto AimingBrush = dxgkrnl_hook::NtGdiCreateSolidBrush(RGB(64, 224, 208), NULL);
	
	bool GameInited = Game ? Game->Ingame : false;
	auto PlayerCount = 0;

	if (DrawList)
	{
			
		for (auto& DrawInfo : *DrawList)
		{
			if (DrawInfo.IsPlayer)
				PlayerCount++;
		};

		bool optimization_level = 0;
		
		if (PlayerCount > 10)
			optimization_level = 1;
		else if (PlayerCount > 20)
			optimization_level = 2;
		
		for (auto& DrawInfo : *DrawList)
		{
			if (optimization_level == 0 && (DrawInfo.IsVehicle || DrawInfo.IsLootBox || DrawInfo.IsItem))
			{
				if (DrawInfo.IsVehicle)
				{
					dxgkrnl_hook::GreSetBkColor(pHDC, RGB(40, 40, 40));
					dxgkrnl_hook::GreSetTextColor(pHDC, RGB(255, 255, 0));
				}
				if (DrawInfo.IsLootBox)
				{
					dxgkrnl_hook::GreSetBkColor(pHDC, RGB(40, 40, 40));
					dxgkrnl_hook::GreSetTextColor(pHDC, RGB(40, 40, 255));
				}
				if (DrawInfo.IsItem)
				{
					dxgkrnl_hook::GreSetBkColor(pHDC, RGB(40, 40, 40));
					dxgkrnl_hook::GreSetTextColor(pHDC, RGB(255, 178, 51));
				}
				
				dxgkrnl_hook::GreExtTextOutWInternal(pHDC, DrawInfo.TextPos.x, DrawInfo.TextPos.y, 0, 0, DrawInfo.Text, wcslen(DrawInfo.Text), 0, 0);
			}
			else if (DrawInfo.IsPlayer)
			{

				if (!DrawInfo.IsDead && !DrawInfo.IsOnVehicle)
				{
					auto brush = DrawInfo.TeamMate ? TeamMateBrush : (DrawInfo.Visible ? VisibleBrush : NonVisibleBrush);

					if (DrawInfo.AimbotTarget)
					{
						if (DrawInfo.Aiming)
							brush = AimingBrush;
						else
							brush = AimbotBrush;
					}

					DrawPlayerBox(pHDC, DrawInfo.PlayerBox, 2, brush, optimization_level);

					if (optimization_level == 0)
						DrawHealtBar(pHDC, DrawInfo.PlayerBox, DrawInfo.PlayerHealth, 2, HealtBarBrush, HealtBarBrushOutline);			
				}

				dxgkrnl_hook::GreSetBkColor(pHDC, RGB(40, 40, 40));
				dxgkrnl_hook::GreSetTextColor(pHDC, RGB(255, 51, 133));

				dxgkrnl_hook::GreExtTextOutWInternal(pHDC, DrawInfo.TextPos.x, DrawInfo.TextPos.y, 0, 0, DrawInfo.Text, wcsnlen_s(DrawInfo.Text,CCHWC(DrawInfo.Text)), 0, 0);
				
				if (DrawInfo.OnGround)
					dxgkrnl_hook::GreExtTextOutWInternal(pHDC, DrawInfo.TextPos.x, DrawInfo.TextPos.y + 20, 0, 0, L"Fallen", wcsnlen_s(L"Fallen",10), 0, 0);

			}
		}
	}	

	if (true)
	{
		dxgkrnl_hook::GreSetBkColor(pHDC, RGB(78, 78, 78));
		dxgkrnl_hook::GreSetTextColor(pHDC, RGB(222, 222, 222));

		auto Time = KernelLogger::GetTime();

		RenderText<wchar>::Draw(pHDC, 20, 20, L"%hs  FPS: [%03u/%03u][%05llu/%05llu]  Mem: %03i  Players: %02i  Ingame: %i",
			Time->Buffer, overlayfpscounter->AcceptedFrames, overlayfpscounter->RejectedFrames,
			overlayfpscounter->frameTime, gamefpscounter->frameTime,
			MemoryAllocator::CurrentAllocatedChunks, PlayerCount, GameInited);

		delete Time;	
	}

	if (Game)
	{			
		//KernelFloatToInt angles_x(Game->DebugRotator.Pitch, 5);
		//auto heatlhs = new DK::DString<WCHAR>(L"%i.%i %hhx", Health.First, Health.Second, Game->DebugByte);
				
		RenderText<wchar>::Draw(pHDC, 20, 40, L"SpecCount: %u", Game->SpecCount);
		
		if (dxgkrnl_hook::NtUserGetAsyncKeyState(0x2D) < 0) //VK_INSERT
		{			
				RenderText<wchar>::Draw(pHDC, 20, 80, L"Level %llX ", Game->cULevel);
				RenderText<wchar>::Draw(pHDC, 20, 100, L"AArray %llX ", Game->cActorsArray);		
				RenderText<wchar>::Draw(pHDC, 20, 120, L"GameInst %llX ", Game->cGameInst);
				RenderText<wchar>::Draw(pHDC, 20, 140, L"LP %llX ", Game->cLocalPlayer);
				RenderText<wchar>::Draw(pHDC, 20, 160, L"LPC %llX ", Game->cLPController);
				RenderText<wchar>::Draw(pHDC, 20, 180, L"Pawn %llX ", Game->cLocalPawn);
				RenderText<wchar>::Draw(pHDC, 20, 200, L"Root %llX ", Game->cLocalRoot);
				RenderText<wchar>::Draw(pHDC, 20, 220, L"Mesh %llX ", Game->cLocalMesh);
				RenderText<wchar>::Draw(pHDC, 20, 240, L"Actors %llX ", Game->cActors);

				RenderText<wchar>::Draw(pHDC, 20, 260, L"Base %llX ", Game->GameBase);
				RenderText<wchar>::Draw(pHDC, 20, 280, L"GNames %llX ", Game->pGNames->BaseAddress);	
				RenderText<wchar>::Draw(pHDC, 20, 300, L"World %llX ", Game->cUWorld);
				RenderText<wchar>::Draw(pHDC, 20, 320, L"LastError %u ", Game->LastError);
				RenderText<wchar>::Draw(pHDC, 20, 340, L"PID %u ", Game->GameProcessID);
		}
		
	}
		
}

u64 __fastcall dxgkrnl_hook::MyHookedFunc(void* data, int b)
{
	/// *****************************
	/// Init Desktop Window Manager
	/// *****************************
	
	if (!dxgkrnl_hook::PEDWM)
		dxgkrnl_hook::setDWM();

	/// *****************************
	/// Init Target
	/// *****************************

	if (dxgkrnl_hook::PEDWM && !dxgkrnl_hook::PETarget)
		dxgkrnl_hook::setTarget("PUBGLite"); 

	/// *****************************
	/// Init Target Window
	/// *****************************
	
	if (dxgkrnl_hook::PEDWM && dxgkrnl_hook::PETarget)
		if (!dxgkrnl_hook::TargetWindow && IoGetCurrentProcess() == dxgkrnl_hook::PEDWM)
				dxgkrnl_hook::setTargetWindow("PUBG LITE ");

	// Initing is not done
	if (!dxgkrnl_hook::TargetWindow)
		return STATUS_NO_TOKEN;

	/// *****************************
	/// Run
	/// *****************************

	if (dxgkrnl_hook::NtUserGetAsyncKeyState(0x21) & 1 || KernelUtilities::BBCheckProcessStatus(dxgkrnl_hook::PETarget) == 0) //PAGE UP key
	{
		dxgkrnl_hook::Unhook();
		return STATUS_NO_TOKEN;
	}

	// Dont Draw Anything when game window is'nt up
	if (dxgkrnl_hook::NtUserGetForegroundWindow() != dxgkrnl_hook::TargetWindow)
		return STATUS_NO_TOKEN;
		
	static PUBGLite* Game = nullptr;
	static LARGE_INTEGER LastTimeRendered;
	static auto RenderList = new Vector<DrawInfo>();
	static auto RenderStatus = true;
	static auto overlayfpscounter = new FPSCounter(5);
	static auto gamefpscounter = new FPSCounter();

	// When Game is Calling DxgkSubmitCommand
	if (dxgkrnl_hook::PETarget == IoGetCurrentProcess()) //VK_LCONTROL
	{
		if (!Game)
			Game = new PUBGLite();
		else
		{
			if (!Game->GameBase)
				Game->Init(&dxgkrnl_hook::MonitorRect);
			else
			{			
				auto TempGameList = new Vector<DrawInfo>();

				// Lets Read From Game
				gamefpscounter->FrameStart();
				Game->Tick(*TempGameList);
				gamefpscounter->FrameEnd();

	
				// Set RenderList -Also Clears Old Buffer	
				KeAcquireGuardedMutex(&dxgkrnl_hook::RenderListLock);
				RenderList->CopyFrom(TempGameList);
				KeReleaseGuardedMutex(&dxgkrnl_hook::RenderListLock);		
								
				// Clear 
				delete TempGameList;
				
			}
		}
		return 6565113516;
	}

	// When DWM is Calling DxgkSubmitCommand
	if (dxgkrnl_hook::PEDWM == IoGetCurrentProcess())
	{
		if (!overlayfpscounter->Tick())
			return 1;

		KeAcquireGuardedMutex(&dxgkrnl_hook::RenderListLock);
		
		auto TempRenderList = RenderList->Size == 0 ? nullptr : new Vector<DrawInfo>();
	
		if (TempRenderList)
		{
			TempRenderList->CopyFrom(RenderList);
			//RenderList->clear(); //we should clear it
		}
			
		KeReleaseGuardedMutex(&dxgkrnl_hook::RenderListLock); // Release it soon as possible
		
		overlayfpscounter->FrameStart();
		DrawingThread(TempRenderList, overlayfpscounter, gamefpscounter, Game);
		overlayfpscounter->FrameEnd();

		if (TempRenderList)
			delete TempRenderList;

		return 1;
	}

	return 0;
}

#endif 

#if (DEBUG_ONLINE)

u64 __fastcall dxgkrnl_hook::MyHookedFunc(void* data, int b)
{
	/// *****************************
		/// Init Desktop Window Manager
		/// *****************************

	if (!dxgkrnl_hook::PEDWM)
		dxgkrnl_hook::IsDWM();

	if (dxgkrnl_hook::PEDWM == IoGetCurrentProcess())
	{
	
		if (dxgkrnl_hook::NtUserGetAsyncKeyState(0x2E)) //VK_DELETE
		{
			DEBUG_PRINT("[%ws]::%hs:: submit_command_hook UnInitialized \r\n", PROJECT_NAME, __FUNCTION__);
			DWORD CallBuffer = static_cast<DWORD>(OriginalCall - CallAddress - 0x5);
			KernelMemory::BruteForceWrite(CallAddress + 1, &CallBuffer, sizeof(CallBuffer));
			KernelBeep::Beep(1245, 150);
			return 6565113517;
		}

		if (dxgkrnl_hook::NtUserGetAsyncKeyState(0x22)) //VK_DELETE
		{
			KernelBeep::Beep(TONE_3, 150);
			return 6565113517;
		}

		
		static const auto pHDC = dxgkrnl_hook::NtUserGetDC(0);
		dxgkrnl_hook::NtGdiPatBlt(pHDC, 50, 50, 100, 100, PATCOPY);

		auto Time = KernelLogger::GetTime();
		auto info = new DK::DString<WCHAR>(L"Time:%hs", Time->Buffer);
		dxgkrnl_hook::GreExtTextOutWInternal(pHDC, 10, 10, NULL, NULL, info->Buffer, info->Length, NULL, NULL);

		delete info;
		delete Time;
						
	}


	//static const auto pHDC = dxgkrnl_hook::NtUserGetDC(0);
	//dxgkrnl_hook::NtGdiPatBlt(pHDC, 50, 50, 100, 100, PATCOPY);
	
	return 6565113517;

	if (!dxgkrnl_hook::PEDWM)
	{
		const auto current_process = IoGetCurrentProcess();
		const auto process_name = PsGetProcessImageFileName(current_process);

		if (!dxgkrnl_hook::PEDWM && strstr(process_name, "dwm"))
		{
			DEBUG_PRINT("[%ws]::%hs:: PEDWM Inited \r\n", PROJECT_NAME, __FUNCTION__);
			dxgkrnl_hook::PEDWM = current_process;
		}
	}

	//if (dxgkrnl_hook::PEDWM == IoGetCurrentProcess())
	//{
		static const auto pHDC = dxgkrnl_hook::NtUserGetDC(0);
		const auto hwnd = dxgkrnl_hook::NtUserGetForegroundWindow();
		//const auto pHDC = dxgkrnl_hook::NtUserGetDC(hwnd);

		auto Time = KernelLogger::GetTime();

		auto info = new DK::DString<WCHAR>(L"Time:%hs", Time->Buffer);

	/*	static auto NtGdiRectangle = reinterpret_cast<bool(*)(_In_ HDC hdc,
			_In_ INT left,
			_In_ INT top,
			_In_ INT right,
			_In_ INT bottom)>(
		KernelModule::GetKernelModuleExport("win32kfull.sys", "NtGdiRectangle")
		);

		static auto NtGdiRoundRect = reinterpret_cast<bool(*)(_In_ HDC hdc,
			_In_ INT left,
			_In_ INT top,
			_In_ INT right,
			_In_ INT bottom,
			_In_ INT width,
			_In_ INT height)>(
				KernelModule::GetKernelModuleExport("win32kfull.sys", "NtGdiRoundRect")
				);

		static auto NtGdiEllipse = reinterpret_cast<bool(*)(_In_ HDC hdc,
			_In_ INT left,
			_In_ INT top,
			_In_ INT right,
			_In_ INT bottom)>(
				KernelModule::GetKernelModuleExport("win32kfull.sys", "NtGdiEllipse")
				);

		static auto NtGdiPolyPolyDraw = reinterpret_cast<int(*)(HDC, POINT*, ULONG*, INT, INT)>
					(KernelModule::GetKernelModuleExport("win32kbase.sys", "NtGdiPolyPolyDraw"));

		static auto GreGetStockObject = reinterpret_cast<HBRUSH(*)(HDC,  INT)>
			(KernelModule::GetKernelModuleExport("win32kbase.sys", "GreGetStockObject"));*/

		/*if (NtGdiRectangle)
			NtGdiRectangle(pHDC, 40, 40, 50, 50);

		if (NtGdiRoundRect)
			NtGdiRoundRect(pHDC, 140, 140, 50, 50,50,50);*/
		
	//	auto prevhbr = dxgkrnl_hook::GreSelectBrush(pHDC, GreGetStockObject(pHDC,5));

		//if (NtGdiEllipse)
		//	NtGdiEllipse(pHDC, 40, 40, 50, 50);

		//if (prevhbr)
		//	dxgkrnl_hook::GreSelectBrush(pHDC, prevhbr);

	//	ULONG Count3[2] = { 0, 1 };
		//static ULONG count = 2;
	//	static POINT points[2] = { {100,100}, {200,200} };

	//	if (NtGdiPolyPolyDraw)
	//	 NtGdiPolyPolyDraw(pHDC, (PPOINT)points, (PULONG)Count3, 1, 2);

		dxgkrnl_hook::GreExtTextOutWInternal(pHDC, 10, 10, NULL, NULL, info->Buffer, info->Length, NULL, NULL);

		//dxgkrnl_hook::GreSetBkMode(pHDC, a);

		delete info;
		delete Time;

		dxgkrnl_hook::NtUserReleaseDC(hwnd, pHDC);
	//}
	
	
	
}

NTSTATUS dxgkrnl_hook::DebugHook()
{

	auto Process = KernelProcess::GetPEPProcess(L"winlogon.exe");

	if (!Process)
		return STATUS_NO_TOKEN;

	KeAttachProcess(Process);

	
#if (0)
	auto func = KernelModule::GetKernelModuleExport("dxgkrnl.sys", "DxgkPresent");

	if (!func)
		return STATUS_NO_TOKEN;

	CallAddress = func + 0xF5;
	OriginalCall = KernelMemory::follow_call<uint8_t*>(CallAddress);

#elif (0)

	auto func = KernelModule::GetKernelModuleExport("dxgkrnl.sys", "DxgkPresent");

	if (!func)
		return STATUS_NO_TOKEN;

	auto PushProfilerEntry = KernelMemory::follow_call<uint8_t*>(func + 0xF5);
	CallAddress = PushProfilerEntry + 0x22;
	OriginalCall = KernelMemory::follow_call<uint8_t*>(CallAddress);

#elif (1)

	auto DxgkGetDeviceState = KernelModule::GetKernelModuleExport("dxgkrnl.sys", "DxgkGetDeviceState");
	auto DxgkGetDeviceStateInternal = KernelMemory::follow_call<uint8_t*>(DxgkGetDeviceState + 0x1F);

	auto verfix = 0;

	auto BuildNo = KernelImporter::BuildNo;

	if (BuildNo == 18362 || BuildNo == 18363)
		verfix = 0x51;
	else if (BuildNo == 19041 || BuildNo == 19042)
		verfix = 0x50;

	CallAddress = DxgkGetDeviceStateInternal + verfix;
	OriginalCall = KernelMemory::follow_call<uint8_t*>(CallAddress);


#elif (0)

	auto DdiSubmitCommandVirtualADAPTER_RENDER = KernelModule::FindPatternInModule("dxgkrnl.sys",
		(BYTE*)"\x4C\x8B\xDC\x57",
		"xxxx");

	CallAddress = DdiSubmitCommandVirtualADAPTER_RENDER + 0x64;
	OriginalCall = KernelMemory::follow_call<uint8_t*>(CallAddress);


#elif (0)
	// Work on bahadir
	auto DxgkSubmitCommandToHwQueue = KernelModule::GetKernelModuleExport("dxgkrnl.sys", "DxgkSubmitCommandToHwQueue");
	auto DxgkSubmitCommandToHwQueueInternal = KernelMemory::follow_call<uint8_t*>(DxgkSubmitCommandToHwQueue + 0x6);

	CallAddress = DxgkSubmitCommandToHwQueueInternal + 0x85;
	OriginalCall = KernelMemory::follow_call<uint8_t*>(CallAddress);


#elif (0)
	auto func = KernelModule::GetKernelModuleExport("dxgkrnl.sys", "DxgkRender");

	if (!func)
		return STATUS_NO_TOKEN;

	auto DxgkRender = 0;
	auto BuildNo = KernelImporter::BuildNo;

	if (BuildNo == 18362 || BuildNo == 18363)
		DxgkRender = 0x5E;
	else if (BuildNo == 19041 || BuildNo == 19042)
		DxgkRender = 0x5A;
	else
		return STATUS_NO_TOKEN;

	CallAddress = func + DxgkRender;
	OriginalCall = KernelMemory::follow_call<uint8_t*>(CallAddress);

#elif (0)
	auto func = KernelModule::GetKernelModuleExport("dxgkrnl.sys", "DxgkSubmitCommand");

	if (!func)
		return STATUS_NO_TOKEN;

	auto BuildNo = KernelImporter::BuildNo;

	auto dxgkInternal = 0;

	if (BuildNo == 18362 || BuildNo == 18363)
		dxgkInternal = 0x55;
	else if (BuildNo == 19041 || BuildNo == 19042)
		dxgkInternal = 0x58;
	else
		return STATUS_NO_TOKEN;

	auto DxgkSubmitCommandInternal = KernelMemory::follow_call<uint8_t*>(func + dxgkInternal);

	CallAddress = DxgkSubmitCommandInternal + 0x72;
	OriginalCall = KernelMemory::follow_call<uint8_t*>(CallAddress);
#endif

	auto ashellCode = KernelModule::FindFreeMemoryInSytemModule("dxgkrnl.sys", 0x1000);

	DEBUG_PRINT("[%ws]::%hs:: CallAddress: %p \r\n", PROJECT_NAME, __FUNCTION__, CallAddress);
	DEBUG_PRINT("[%ws]::%hs:: OriginalCall: %p \r\n", PROJECT_NAME, __FUNCTION__, OriginalCall);
	DEBUG_PRINT("[%ws]::%hs:: ShellCode: %p \r\n", PROJECT_NAME, __FUNCTION__, ashellCode);

	if (ashellCode)
	{

		*reinterpret_cast<DWORD64*>(shellCode + 22 + 2) = reinterpret_cast<DWORD64>(OriginalCall);
		*reinterpret_cast<DWORD64*>(shellCode + 48 + 2) = reinterpret_cast<DWORD64>(DebugMyHookedFunc);

		KernelMemory::BruteForceWrite(ashellCode, shellCode, sizeof(shellCode));

		DWORD CallBuffer = static_cast<DWORD>(ashellCode - CallAddress - 0x5);
		KernelMemory::BruteForceWrite(CallAddress + 1, &CallBuffer, sizeof(CallBuffer));
	}


	KeDetachProcess();
	ObDereferenceObject(Process);

	return STATUS_SUCCESS;
}
#endif






























/*


EXECUTE_EVERY(GTI_SECONDS(2),
				{
					DEBUG_PRINT("[%ws]::%hs Readed Window name \"%ws\" %i -> Wanted %i %i  \r\n", PROJECT_NAME, __FUNCTION__,
					WndName.Buffer, WndName.Length, !wcscmp(WndName.Buffer, L"PUBG LITE "), Ourwnd);
				});

if (dxgkrnl_hook::NtUserInternalGetWindowText)
	{
		static wchar Name[MAX_PATH];

		memset(Name, 0, MAX_PATH);
		auto hwnd = dxgkrnl_hook::NtUserGetForegroundWindow();
		dxgkrnl_hook::NtUserInternalGetWindowText(hwnd, (u8*)Name, MAX_PATH);

		static u64 LastCheck = *((volatile u64*)(SharedSystemTime));

		if ((KernelUtilities::Now() - LastCheck) > GTI_SECONDS(1))
		{
			KeQuerySystemTime(&LastCheck);
			DEBUG_PRINT("[%ws]::%hs Waiting for %ws ... \r\n", PROJECT_NAME, __FUNCTION__, Name);
		}
	}*/

	/*
	const auto current_process = IoGetCurrentProcess();
	const auto process_name = PsGetProcessImageFileName(current_process);

	if (strstr(process_name, "AlisInjector64"))
	{

		DEBUG_PRINT("[%ws]::%hs:: MyDxgkSubmitCommandInternal 0x%llX \r\n", PROJECT_NAME, __FUNCTION__, &MyDxgkSubmitCommandInternal);

		static auto Address = KernelModule::GetUserModuleExport(L"AlisInjector64.exe", "?Toplama@@YAHHH@Z", IoGetCurrentProcess());

		DEBUG_PRINT("[%ws]::%hs::AlisInjector64 Toplama %llX \r\n", PROJECT_NAME, __FUNCTION__, Address);

		auto MyFunc = reinterpret_cast<KernelMemory::ShellCode<int32_t, int32_t, int32_t>>(Address);
		auto Test = KernelMemory::CallUserModeFunction(MyFunc,25, 10);

		DEBUG_PRINT("[%ws]::%hs:: Toplama(25,10) %i \r\n", PROJECT_NAME, __FUNCTION__, Test);
	}

	if (strstr(process_name, "AlisInjector86"))
	{

		DEBUG_PRINT("[%ws]::%hs:: MyDxgkSubmitCommandInternal 0x%llX \r\n", PROJECT_NAME, __FUNCTION__, &MyDxgkSubmitCommandInternal);

		static auto Address = (uint32_t)KernelModule::GetUserModuleExport(L"AlisInjector86.exe", "?Function@@YGXXZ", IoGetCurrentProcess());
		static auto ToplamaAddress = (uint32_t)KernelModule::GetUserModuleExport(L"AlisInjector86.exe", "?Toplama@@YGHHH@Z", IoGetCurrentProcess());

		DEBUG_PRINT("[%ws]::%hs::AlisInjector86 Function %X \r\n", PROJECT_NAME, __FUNCTION__, Address);
		DEBUG_PRINT("[%ws]::%hs::AlisInjector86 ToplamaAddress %X \r\n", PROJECT_NAME, __FUNCTION__, ToplamaAddress);

		KernelMemory::CallUserModeFunctionx86(ToplamaAddress);

		DEBUG_PRINT("[%ws]::%hs:: Address Called \r\n", PROJECT_NAME, __FUNCTION__);
	}*/

	//static const auto pHDC = dxgkrnl_hook::NtUserGetDC(0);

	//static auto NtGdiPolyPolyDraw = reinterpret_cast<int(*)(HDC, POINT*, ULONG*, INT, INT)>
	//	(KernelModule::GetKernelModuleExport("win32kbase.sys", "NtGdiPolyPolyDraw"));

	//static auto GrePolyPolyline = reinterpret_cast<int(*)(HDC, POINT*, ULONG*, INT, INT)>
	//	(KernelMemory::resolve_lea<uint8_t*>(
	//		KernelModule::FindPatternInModule("win32kbase.sys",
	//		(BYTE*)"\x48\xFF\x15\x00\x00\x00\x00\x0F\x1F\x44\x00\x00\xE9\x00\x00\x00\x00\x33\xDB\xE9",
	//			"xxx????xxxx?x????xxx")));

	//static const auto GreMoveTo = reinterpret_cast<int(*)(HDC, int, int, POINT*)>(
	//	KernelModule::FindPatternInModule("win32kfull.sys", (BYTE*)"\xE8\x00\x00\x00\x00\x4C\x8B\x55\xE0", "x????xxxx") - 0x31
	//	);

	//static const auto NtGdiLineTo = reinterpret_cast<int(*)(HDC, INT, INT)>(
	//	KernelModule::GetKernelModuleExport("win32kfull.sys", "NtGdiLineTo")
	//	);

	//static const auto NtGdiMoveTo = reinterpret_cast<int(*)(HDC, INT, INT, POINT*)>(
	//	KernelModule::GetKernelModuleExport("win32kfull.sys", "NtGdiMoveTo")
	//	);

	//static const auto NtUserGetForegroundWindow = reinterpret_cast<HWND(*)(void)>(
	//	KernelModule::GetKernelModuleExport("win32kfull.sys", "NtUserGetForegroundWindow")
	//	);

	//static auto NtGdiCreateCompatibleDC = reinterpret_cast<HDC(*)(HDC)>(
	//	KernelModule::GetKernelModuleExport("win32kfull.sys", "NtGdiCreateCompatibleDC")
	//	);

	//static auto NtUserReleaseDC = reinterpret_cast<HDC(*)(HWND,HDC)>(
	//	KernelModule::GetKernelModuleExport("win32kbase.sys", "NtUserReleaseDC")
	//	);


//static auto NtUserSetWindowLongPtr = reinterpret_cast<int(*)(HWND, int, ULONG_PTR, BOOL)>(
//	KernelModule::GetKernelModuleExport("win32kfull.sys", "NtUserSetWindowLongPtr")
//	);
//
//NtUserSetWindowLongPtr(hwnd,/*GWL_EXSTYLE*/-20, 0x00080000/*WS_EX_LAYERED*/, 1);


	//if (NtGdiLineTo && NtGdiMoveTo)
	//{

	//	auto WDN = NtUserGetForegroundWindow();

	//	auto pHDC2 = dxgkrnl_hook::NtUserGetDC(WDN);

	//	auto HDC3 = NtGdiCreateCompatibleDC(0);

	//	DEBUG_PRINT("[%ws]::%hs::1 %X \r\n", PROJECT_NAME, __FUNCTION__, NtGdiCreateCompatibleDC(pHDC));
	//	DEBUG_PRINT("[%ws]::%hs::2 %X \r\n", PROJECT_NAME, __FUNCTION__, NtGdiCreateCompatibleDC(0));
	//	DEBUG_PRINT("[%ws]::%hs::3 %X \r\n", PROJECT_NAME, __FUNCTION__, pHDC); 
	//	DEBUG_PRINT("[%ws]::%hs::4 %X \r\n", PROJECT_NAME, __FUNCTION__, pHDC2);

	//	if (HDC3 && NtGdiPolyPolyDraw)
	//	{
	//		HDC hDC = dxgkrnl_hook::NtUserGetDC(NULL);
	//		NtGdiMoveTo(hDC, 0, 0, NULL);
	//		NtGdiLineTo(hDC, 200, 20);
	//		NtUserReleaseDC(NULL, hDC);

	//	
	//		/*POINT pts[2] = {};
	//		pts[0].x = 10;
	//		pts[1].x = 200;
	//		pts[0].y = pts[1].y = 50;

	//		Polyline(pHDC2, pts, 2);*/
	//	/*	static ULONG Count1 = 1;

	//		static POINT Points[2] = { {10,100}, {1000,800} };

	//		auto bout = NtGdiPolyPolyDraw(pHDC2, Points, &Count1, 1, 2);*/

	//		auto infoo = new DK::DString<WCHAR>(L"[%i] Drawing %i", MemoryAllocator::CurrentAllocatedChunks, 0);
	//		dxgkrnl_hook::GreExtTextOutWInternal(pHDC, 20, 60, 0, 0, infoo->Buffer, infoo->Length, 0, 0);
	//		delete infoo;

	//	}
	//}

#pragma endregion

//////////////////////////////////////

/*static const auto NtGdiSaveDC = reinterpret_cast<HDC(*)(HDC)>(
		KernelModule::GetKernelModuleExport("win32kfull.sys", "NtGdiSaveDC")
		);

	static const auto NtGdiRestoreDC = reinterpret_cast<HDC(*)(HDC, HDC)>(
		KernelModule::GetKernelModuleExport("win32kfull.sys", "NtGdiRestoreDC")
		);	
		
	static const auto GreMoveTo = reinterpret_cast<int(*)(HDC,int , int ,void*)>(
		KernelModule::FindPatternInModule("win32kfull.sys", (BYTE*)"\xE8\x00\x00\x00\x00\x4C\x8B\x55\xE0", "x????xxxx") - 0x31
		);

	static const auto NtGdiLineTo = reinterpret_cast<int(*)(HDC, INT, INT)>(
		KernelModule::GetKernelModuleExport("win32kfull.sys", "NtGdiLineTo")
		);	
		


	// Get it one time also no need to release
	//static const auto pHDC = dxgkrnl_hook::NtUserGetDC(0);
	//auto a = dxgkrnl_hook::GreSetBkMode(pHDC, 1);
	//dxgkrnl_hook::GreSetBkMode(pHDC, a);
	const auto wnd = dxgkrnl_hook::NtUserGetForegroundWindow();
	const auto pHDC = dxgkrnl_hook::NtUserGetWindowDC(wnd);

	//auto wnd = dxgkrnl_hook::NtUserWindowFromDC(pHDC);
	//dxgkrnl_hook::NtUserSetLayeredWindowAttributes(wnd, 0, 1.0f, 2); // LWA_ALPHA
	//dxgkrnl_hook::NtUserSetLayeredWindowAttributes(wnd, 0, RGB(0, 0, 0), 1); // LWA_COLORKEY
		dxgkrnl_hook::NtUserReleaseDC(pHDC);
		*/



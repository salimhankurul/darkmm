#pragma once

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct _MARGINS
{
	int cxLeftWidth;      // width of left border that retains its size
	int cxRightWidth;     // width of right border that retains its size
	int cyTopHeight;      // height of top border that retains its size
	int cyBottomHeight;   // height of bottom border that retains its size
} MARGINS, * PMARGINS;
typedef unsigned short RTL_ATOM, * PRTL_ATOM;

typedef struct _WND
{
	char junk[0xB8];
    LARGE_UNICODE_STRING strName; //Found By ProtectedLargeUnicodeStringWNDstrName
} WND, * PWND;

typedef struct _DISPLAYINFO
{
    char junk[0x60]; //GetPrimaryMonitorRect in win32kfull
    u64 pMonitorPrimary; 
    u64 pMonitorFirst;
} DISPLAYINFO, * PDISPLAYINFO;


namespace dxgkrnl_hook
{
	
	static auto NtUserGetDC = reinterpret_cast<HDC(*)(HWND)>(0);

	static auto NtGdiCreateCompatibleDC = reinterpret_cast<HDC(*)(HDC)>(0);
	
	static auto NtUserGetForegroundWindow = reinterpret_cast<HWND(*)()>(0);

	static auto NtUserGetWindowDC = reinterpret_cast<HDC(*)(HWND)>(0);  

	static auto NtUserWindowFromDC = reinterpret_cast<HWND(*)(HDC)>(0);

	static auto NtUserSetLayeredWindowAttributes = reinterpret_cast<BOOL(*)(HWND, COLORREF, BYTE, DWORD)>(0);

	static auto NtUserReleaseDC = reinterpret_cast<int(*)(HWND,HDC)>(0);

	static auto NtGdiCreateSolidBrush = reinterpret_cast<HBRUSH(*)(COLORREF, HBRUSH)>(0);

	static auto GreSelectBrush = reinterpret_cast<HBRUSH(*)(HDC, HBRUSH)>(0);

	static auto GreSetBkMode = reinterpret_cast<int(*)(HDC, int)>(0);
	
	static auto GreSetBkColor = reinterpret_cast<int(*)(HDC, int)>(0);
	
	static auto GreSetTextColor = reinterpret_cast<int(*)(HDC, int)>(0);

	static auto NtGdiPatBlt = reinterpret_cast<int(__stdcall *)(HDC, INT, INT, INT, INT, DWORD)>(0);

	static auto GreExtTextOutWInternal = reinterpret_cast<int(*)(HDC, unsigned int, unsigned int, unsigned int, unsigned __int16*, LPWSTR, INT, int*, void*)>(0);

	static auto NtUserGetAsyncKeyState = reinterpret_cast<SHORT(*)(int)>(0);

	static auto GetMonitorRect = reinterpret_cast<DarkRECT * (*)(DarkRECT*, QWORD)>(0);

	static auto GetDispInfo = reinterpret_cast<DISPLAYINFO*(*)()>(0);
	
	//static auto DwmExtendFrameIntoClientArea = reinterpret_cast<DWORD(*)(HWND, MARGINS*)>(0);

	static auto ProtectedLargeUnicodeStringWNDstrName = reinterpret_cast<PLARGE_UNICODE_STRING(*)(PLARGE_UNICODE_STRING, PLARGE_UNICODE_STRING)>(0);

	static auto ValidateHwnd = reinterpret_cast<WND*(*)(HWND)>(0);

	static DarkRECT MonitorRect;
	static PEPROCESS PETarget = NULL, PEDWM = NULL;
	static HWND TargetWindow = NULL;

	static KGUARDED_MUTEX RenderListLock;
	static KGUARDED_MUTEX GameClosingLock;

	NTSTATUS setDWM();
	NTSTATUS setTarget(char* name);
	NTSTATUS setTargetWindow(wchar_t* name);
	NTSTATUS getForegroundWindowName(LARGE_UNICODE_STRING& WndName);
	
	NTSTATUS Hook();
	NTSTATUS Unhook();

	u64 __fastcall MyHookedFunc(void* data, int b);

	
	NTSTATUS DebugHook();
	u64 __fastcall DebugMyHookedFunc(void* data, int b);

	bool AlreadHooked(u8* Address);

	
}
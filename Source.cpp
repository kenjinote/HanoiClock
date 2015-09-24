#pragma comment(lib,"dwmapi")
#include <windows.h>
#define _USE_MATH_DEFINES
#include <math.h>

TCHAR szClassName[] = TEXT("Window");

__int64 SystemTimeToUnixTime(const LPSYSTEMTIME pst)
{
	const __int64 WINDOWS_TICK = 10000000LL;
	const __int64 SEC_TO_UNIX_EPOCH = 11644473600LL;
	FILETIME ft;
	if (!SystemTimeToFileTime(pst, &ft)) return -1;
	ULARGE_INTEGER uli;
	uli.LowPart = ft.dwLowDateTime;
	uli.HighPart = ft.dwHighDateTime;
	return uli.QuadPart / WINDOWS_TICK - SEC_TO_UNIX_EPOCH;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HFONT hFont;
	static __int64 n;
	static char a[3][64];
	switch (msg)
	{
	case WM_CREATE:
		hFont = CreateFont(32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("Consolas"));
		SetTimer(hWnd, 0x1234, 1000, 0);
		break;
	case WM_TIMER:
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			n = SystemTimeToUnixTime(&st);
			{
				ZeroMemory(a, 3 * 64);
				int count[3] = { 0 };
				int pos = 0;
				bool prevbit = 0;
				for (int i = 63; i >= 0; --i)
				{
					const bool currentbit = (n >> i) & 1;
					if (prevbit != currentbit)
					{
						pos = (pos + 2 - (i + currentbit) % 2) % 3;
					}
					a[pos][count[pos]++] = i + 1;
					prevbit = currentbit;
				}
			}
		}
		InvalidateRect(hWnd, 0, 1);
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			const HDC hdc = BeginPaint(hWnd, &ps);
			if (n)
			{
				RECT rect;
				GetClientRect(hWnd, &rect);
				for (int i = 0; i < 3; ++i)
				{
					for (int j = 0; j < 64 && a[i][j] > 0; ++j)
					{
						Rectangle(hdc,
							(int)(i * rect.right / 3 + (rect.right / 3)*(1 - (a[i][j] + 8) / 72.0) / 2),
							rect.bottom - j * 3,
							(int)(i * rect.right / 3 + (rect.right / 3)*(1 + (a[i][j] + 8) / 72.0) / 2),
							rect.bottom - j * 3 + 3);
					}
				}
				HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
				TCHAR szText[1024];
				wsprintf(szText, TEXT("%I64d"), n);
				DrawText(hdc, szText, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 0x1234);
		DeleteObject(hFont);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("Hanoi Clock"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
		);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

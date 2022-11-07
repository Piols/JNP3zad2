#include "WinMain.h"
#include "D2DApp.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	static FLOAT arg = 0.0f;
	switch (Msg) {
	case WM_CREATE:
		SetTimer(hwnd, 501, 10, nullptr);
		InitDirect2D(hwnd);
		return 0;
	case WM_DESTROY:
		DestroyDirect2D();
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		arg += 0.02f;
		InvalidateRect(hwnd, nullptr, true);
		return 0;
	case WM_PAINT:
		OnPaint(hwnd, arg, lParam);
		ValidateRect(hwnd, nullptr);
		return 0;
	case WM_SIZE:
		DestroyRenderTarget();
		return 0;
	}
	return DefWindowProc(hwnd, Msg, wParam, lParam);
}

INT WINAPI wWinMain(_In_ [[maybe_unused]] HINSTANCE instance,
	_In_opt_ [[maybe_unused]] HINSTANCE prev_instance,
	_In_ [[maybe_unused]] PWSTR cmd_line,
	_In_ [[maybe_unused]] INT cmd_show) {

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = instance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(instance, IDC_HAND);
	wcex.hbrBackground = nullptr, //static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = TEXT("Template Window Class");
	wcex.hIconSm = NULL;

	RegisterClassEx(&wcex);

	HWND hwnd = CreateWindowEx(
		0, // Optional window styles.
		wcex.lpszClassName, // Window class
		TEXT("Fat Mouse"), // Window text
		WS_OVERLAPPEDWINDOW, // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL, // Parent window
		NULL, // Menu
		wcex.hInstance, // Instance handle
		NULL // Additional application data
	);

	if (hwnd == NULL) {
		return 1;
	}

	ShowWindow(hwnd, cmd_show);

	MSG msg = {};
	while (BOOL rv = GetMessage(&msg, NULL, 0, 0) != 0) {
		if (rv < 0) {
			DestroyWindow(hwnd);
			return 1;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(hwnd);
	return 0;
}

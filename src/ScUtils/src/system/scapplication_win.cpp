#include "scutils/system/scapplication.h"

#include <Windows.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include <assert.h>
#include <iostream>

#pragma comment(lib, "user32.lib")

class ScApplicationPrivate
{
	SC_DECLARE_PUBLIC(ScApplication)
public:
	ScApplicationPrivate(ScApplication *q) : q_ptr(q) { }

	ScApplication *q_ptr;
	static HWND hwnd;
};

HWND ScApplicationPrivate::hwnd = nullptr;


// 窗口过程函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
		HWND consoleHd = GetConsoleWindow();
		if (nullptr != consoleHd)
			PostMessage(consoleHd, WM_CLOSE, 0, 0);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

// 控制台控制处理函数
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
		if (ScApplicationPrivate::hwnd != NULL)
		{
			PostMessage(ScApplicationPrivate::hwnd, WM_CLOSE, 0, 0);
			while (IsWindow(ScApplicationPrivate::hwnd))
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		return TRUE;
	default:
		return FALSE;
	}
}

ScApplication::ScApplication(int argc, char **argv)
	: ScApplication(new ScApplicationPrivate(this))
{
	SC_D(ScApplication);
	assert(nullptr == d->hwnd && "只能创建一个应用程序实例！");
	if (nullptr != d->hwnd)
		return;

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	if (!SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE))
	{
		std::cerr << "Failed to set console control handler." << std::endl;
		return;
	}

	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = "SimpleWindowClass";
	RegisterClass(&wc);
	d->hwnd = CreateWindowA(wc.lpszClassName, "ScApplication", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, wc.hInstance, NULL);
	assert(nullptr != d->hwnd);

	FILE *stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);
}

ScApplication::~ScApplication()
{
	HWND consoleHd = GetConsoleWindow();
	if (nullptr != consoleHd)
	{
		PostMessage(consoleHd, WM_CLOSE, 0, 0);
		FreeConsole();
	}
	fclose(stdout);
	delete d_ptr;
}

std::string ScApplication::applicationDirPath()
{
	char appath[MAX_PATH]{ };
	GetModuleFileNameA(NULL, appath, MAX_PATH);
	std::string fileName(appath);
	std::replace(fileName.begin(), fileName.end(), '\\', '/');
	return fileName.substr(0, fileName.find_last_of('/'));
}

int ScApplication::exec()
{
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return static_cast<int>(msg.wParam);
}

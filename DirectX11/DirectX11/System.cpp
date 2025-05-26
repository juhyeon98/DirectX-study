#include "System.h"

System::System()
{
	mInput = 0;
	mApplication = 0;
}

System::System(const System& other)
{
}

System::~System()
{
}

bool System::Initialize()
{
	int screenWidth = 0, screenHeight = 0;

	InitializeWindows(screenWidth, screenHeight);

	mInput = new Input;
	mInput->Initialize();

	mApplication = new Application;
	if (!mApplication->Initialize(screenWidth, screenHeight, mHwnd))
	{
		return false;
	}
	return true;
}

void System::Shutdown()
{
	if (mApplication)
	{
		mApplication->Shutdown();
		delete mApplication;
		mApplication = 0;
	}

	if (mInput)
	{
		delete mInput;
		mInput = 0;
	}

	ShutdownWindows();
}

void System::Run()
{
	MSG msg;
	bool done = false;

	ZeroMemory(&msg, sizeof(MSG));
	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else if (!Frame())
		{
			done = true;
		}
	}
}

bool System::Frame()
{
	if (mInput->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	if (!mApplication->Frame())
	{
		return false;
	}
	return true;
}

LRESULT CALLBACK System::MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (msg)
	{
	case WM_KEYDOWN:
		mInput->KeyDown((unsigned int)wParam);
		break;
	case WM_KEYUP:
		mInput->KeyUp((unsigned int)wParam);
		break;
	default:
		result = DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}

	return result;
}

void System::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	ApplicationHandle = this;
	mhInstance = GetModuleHandle(NULL);
	mApplicationName = L"Engine";

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhInstance;
	wc.hIcon = LoadIcon(NULL, IDC_ARROW);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = mApplicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (FULL_SCREEN)
	{
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		posX = posY = 0;
	}
	else
	{
		screenWidth = 800;
		screenHeight = 600;
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	mHwnd = CreateWindowEx(
		WS_EX_APPWINDOW, mApplicationName, mApplicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight,
		NULL, NULL, mhInstance, NULL
	);
	ShowWindow(mHwnd, SW_SHOW);
	SetForegroundWindow(mHwnd);
	SetFocus(mHwnd);
	ShowCursor(false);
}

void System::ShutdownWindows()
{
	ShowCursor(true);

	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(mHwnd);
	mHwnd = NULL;

	UnregisterClass(mApplicationName, mhInstance);
	mhInstance = NULL;

	ApplicationHandle = NULL;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (msg)
	{
	case WM_DESTROY: /* fall through */
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		result = ApplicationHandle->MessageHandler(hwnd, msg, wParam, lParam);
	}
	return result;
}
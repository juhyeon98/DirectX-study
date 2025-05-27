#include "Application.h"

Application::Application()
{
	mDirect3D = 0;
}

Application::Application(const Application& other)
{
}

Application::~Application()
{
}

bool Application::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	mDirect3D = new D3D;
	if (!mDirect3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLE, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))
	{
		// 대화 상자 생성
		MessageBox(hwnd, L"Could not initialize Direct 3D", L"Error", MB_OK);
		return false;
	}
	return true;
}

void Application::Shutdown()
{
	if (mDirect3D)
	{
		mDirect3D->Shutdown();
		delete mDirect3D;
		mDirect3D = 0;
	}
}

bool Application::Frame()
{
	if (!Render())
	{
		return false;
	}
	return true;
}

bool Application::Render()
{
	mDirect3D->BegineScene(0.5f, 0.5f, 0.5f, 1.0f);
	mDirect3D->EndScene();
	return true;
}
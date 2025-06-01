/*
* ������Ʈ ���ù�
* 1. �ַ�� Ž������ ������Ʈ �Ӽ��� ����
* 2. ��Ŀ�� ����
* 3. ���� �Է¿��� �߰� ���Ӽ��� d3d11.lib; dxgi.lib; d3dcompiler.lib; �߰�
* 4. �ý����� ���� �ý��ۿ��� `/SUBSYSTEM:WINDOWS` �� ����
*/

#include <d3d11.h>
#include <DirectXMath.h> // change d3dx11.h
#include <WICTextureLoader.h> // change d3dx10.h

#include <Windows.h>
#include <windowsx.h>

#include <iostream>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "user32.lib")

IDXGISwapChain* swapchain;
ID3D11Device* dev;
ID3D11DeviceContext* devcon;

void InitD3D(HWND hWnd);
void CleanD3D(void);

LRESULT CALLBACK WindowProc(HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPervInstance,
					LPSTR lpCmdLine,
					int nCmdShow )
{
	HWND hWnd;

	// Window class ����
	// Window class �� Ŀ���� �������ϰ� �̺�Ʈ ó���� �ϱ� ���� ���ø�(��Ÿ������)
	WNDCLASSEX wc;
	::ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = TEXT("WindowClass1");


	RECT wr = { 0,0,500,400 };
	// Ŭ���̾�Ʈ ������ �������� ��ü ������ ũ��� ��ȯ
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	// Window class ���
	::RegisterClassEx(&wc);
	hWnd = CreateWindowEx(NULL,
						TEXT("WindowClass1"),
						TEXT("TEST"),
						WS_OVERLAPPEDWINDOW,
						300,
						300,
						wr.right - wr.left,
						wr.bottom - wr.top,
						NULL,
						NULL,
						hInstance,
						NULL );
	::ShowWindow(hWnd, nCmdShow);

	MSG msg = { 0, };
	// Windows �̺�Ʈ �� �޽��� ó��
	while (TRUE)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				break;
			}
		}
		else
		{
			// Game
			InitD3D(hWnd);
		}
	}

	return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_DESTROY:
		{
			CleanD3D();
			::PostQuitMessage(0); // WM_QUIT ����
			return 0;
		}break;
	}
	::DefWindowProc(hWnd, message, wParam, lParam);
}

// DXGI
/*
* DirectX Graphic Infrastructure
* - DirectX�� �Ϻΰ� �ƴ϶�, DirectX�� �ϵ������ �������̽� ����
* - Application -> Direct3D 11 -> DXGI -> Hardware
*/

// Swap chain
/*
* - GPU�� �޸𸮿� �̹��� ���۸� ������ ����.
* - ���� ȭ�鿡 ������ �� ������ �̹��� ���۸� ����ͷ� ����
* - �̹��� ���� ä�� -> ����Ϳ� ���� -> ����� ��� -> �ݺ�
* - ������, ������ ����� ����� ������ �ʴٴ� ��. �ƹ��� ���� 60 ~ 100Hz
* - �׷��� ����� ���ʿ��� ���� �̹����� ������, �Ʒ����� ���ο� �̹����� ������ �̻��� ������ �߻�
* 
* - �̸� �����ϱ� ���� DXGI�� ������ ����� ����
* - ����Ϳ� ���� ������ �ϴ� ���� �ƴ�, �� ���۶�� ���� ���ۿ� ä��� ���
* - ����Ʈ ���۴� ���� ����Ϳ��� ��µǴ� ����
* - DirectX�� �� ���ۿ� �̹����� ä���, DXGI�� ����Ʈ ���۸� �� ������ �������� ä��
* - ������ �̷��� �ص� ȭ���� ������ �� ����
* 
* - ������ DXGI�� �� ���۸� ��� �����ͷ� ������ �ְ�, �� ������ ���� ������ Ƽ��� ����
* - ����� �ֻ����� ���߾� ������ ���� ���İ��� �ٲپ� Ƽ� ������ �����ϴ� ��
*/

// ������ ����������
/*
* 1. Input assembler : ������ �Ϸ��� 3D �� ������ GPU �޸𸮿� ����, ������ �� ������ �غ�
* 2. Rasterizer : �� ���� �̹����� � �ȼ��� �׷����� ������ �������� ����
* 3. Output merger : ���� �� �̹����� �ϳ��� �̹����� �����ϰ� �� ���ۿ� ���ε�
*/

// �׷��� ��ü ���� ���
/*
* 1. ��(point) ��� : �������� ���, �� �������� ��ġ ���� ������ ����.
* 2. ��(line) ��� : ���� �ΰ��� �̷�� ���п� ���� ���. 3D �׸���, ��������Ʈ � ����
* 3. ��(line) strips : ��� ������ �������� ����� ��, �� �������� ����. ���̾� ������ �̹���, ����뿡 ����
* 4. �ﰢ�� ��� : �� ���� ������ �׷��� �̿��� �ϳ��� �ﰢ���� ����. �� �ﰢ������ ���.
* 5. �ﰢ�� strips : ���� ����� �Ϸ��� �ﰢ���� �����ϴ� ���������� ����. 3D �׷��Ƚ��� ���� ���
*/

void InitD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC scd;

	::ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 4;
	scd.Windowed = TRUE;

	::D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&swapchain,
		&dev,
		NULL,
		&devcon);
}

void CleanD3D(void)
{
	swapchain->Release();
	dev->Release();
	devcon->Release();
}
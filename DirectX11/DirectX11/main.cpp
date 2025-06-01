/*
* ������Ʈ ���ù�
* 1. �ַ�� Ž������ ������Ʈ �Ӽ��� ����
* 2. ��Ŀ�� ����
* 3. ���� �Է¿��� �߰� ���Ӽ��� d3d11.lib; dxgi.lib; d3dcompiler.lib; �߰�
* 4. �ý����� ���� �ý��ۿ��� `/SUBSYSTEM:WINDOWS` �� ����
*/

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h> // change d3dx11.h
#include <WICTextureLoader.h> // change d3dx10.h

#include <Windows.h>
#include <windowsx.h>

#include <iostream>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "user32.lib")

IDXGISwapChain* swapchain; // ���� ü��
ID3D11Device* dev; // GPU ��ġ - COM���� ����
ID3D11DeviceContext* devcon; // GPU�� ������ ������ ����. ������ ��� ����
ID3D11RenderTargetView* backbuffer; // �� ���ۿ� �������� �̹���

ID3D11VertexShader* pVs;
ID3D11PixelShader* pPs;

void InitD3D(HWND hWnd);
void RenderFrame(void);
void CleanD3D(void);

void InitPipeLine(void);

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
	//wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = TEXT("WindowClass1");


	RECT wr = { 0,0,SCREEN_WIDTH,SCREEN_HEIGHT };
	// Ŭ���̾�Ʈ ������ �������� ��ü ������ ũ��� ��ȯ
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	// Window class ���
	::RegisterClassEx(&wc);
	hWnd = CreateWindowEx(NULL,
						TEXT("WindowClass1"),
						TEXT("TEST"),
						//WS_OVERLAPPEDWINDOW,
						WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
						300,
						300,
						wr.right - wr.left,
						wr.bottom - wr.top,
						NULL,
						NULL,
						hInstance,
						NULL );
	::ShowWindow(hWnd, nCmdShow);

	InitD3D(hWnd);
	
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
			RenderFrame();
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
	// ���� ü�� ��Ÿ������
	DXGI_SWAP_CHAIN_DESC scd;

	::ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferCount = 1; // �� ���� �ϳ�
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32��Ʈ RGB
	scd.BufferDesc.Width = SCREEN_WIDTH;
	scd.BufferDesc.Height = SCREEN_HEIGHT;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 4; // ��Ƽ ���� ��
	scd.Windowed = TRUE; // â ��� or ��ü ȭ�� ���
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// scd�� �������� ��ġ ���ؽ�Ʈ�� ���� ü�� ����
	::D3D11CreateDeviceAndSwapChain(NULL, // GPU ��͸� ã�µ� DXGI�� �˾Ƽ� ó��
		D3D_DRIVER_TYPE_HARDWARE, // ����̹� ����
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

	// ȭ�鿡 �׸� �̹����� 2D�̹���
	ID3D11Texture2D* pBackBuffer;
	// ���� ü�ο��� �� ���۸� ã�� ID3D11Texture2D ��ü�� ����
	// �� ���۴� ���� 0���ſ� ����
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// ���� Ÿ�� ��ü�� ����
	// ���� Ÿ���� ����ۿ� �ø��� �� �̹����� �����ϴ� ���۸� �ϳ� �Ҵ��ϰڴٴ� �ǹ�
	dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	pBackBuffer->Release();

	// ���� Ÿ���� �� ���۷� ����
	devcon->OMSetRenderTargets(1, &backbuffer, NULL);

	// ����Ʈ ����
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0; // ���� ��� X���� 0
	viewport.TopLeftY = 0; // ���� ��� Y���� 0
	viewport.Width = SCREEN_WIDTH; // �ʺ� 800
	viewport.Height = SCREEN_HEIGHT; // �ʺ� 600
	devcon->RSSetViewports(1, &viewport);
}

void RenderFrame(void)
{
	DirectX::XMFLOAT4 color(0.0f, 0.2f, 0.4f, 1.0f); // D3DXCOLOR�� ���� �������� ����
	// ������ �������� ä���.
	devcon->ClearRenderTargetView(backbuffer, &color.x);
	// �� ���ۿ� ����Ʈ ���� ��ȯ
	swapchain->Present(0, 0);
}
void CleanD3D(void)
{
	swapchain->SetFullscreenState(FALSE, NULL);

	pVs->Release();
	pPs->Release();

	swapchain->Release();
	backbuffer->Release();
	dev->Release();
	devcon->Release();
}

void InitPipeLine(void)
{
	// ���̴� �ΰ��� ������ ������
	ID3D10Blob* vs, * ps;
	ID3DBlob* errorBlob;

	// D3DX11CompileFromFile -> D3DCompileFromFile
	D3DCompileFromFile(L"shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, &vs, &errorBlob);
	D3DCompileFromFile(L"shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, &ps, &errorBlob);

	// �����ϵ� ���̴��� COM ��ü�� ����
	// COM ��ü�� �̿��� ���̴� ��ä ����
	dev->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &pVs);
	dev->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, &pPs);

	devcon->VSSetShader(pVs, 0, 0);
	devcon->PSSetShader(pPs, 0, 0);
}
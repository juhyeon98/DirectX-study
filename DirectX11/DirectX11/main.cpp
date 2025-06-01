/*
* 프로젝트 세팅법
* 1. 솔루션 탐색기의 프로젝트 속성에 진입
* 2. 링커에 진입
* 3. 먼저 입력에서 추가 종속성에 d3d11.lib; dxgi.lib; d3dcompiler.lib; 추가
* 4. 시스템의 서브 시스템에서 `/SUBSYSTEM:WINDOWS` 로 변경
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

IDXGISwapChain* swapchain; // 스왑 체인
ID3D11Device* dev; // GPU 장치 - COM으로 연결
ID3D11DeviceContext* devcon; // GPU와 렌더링 파이프 관리. 렌더링 방식 결정
ID3D11RenderTargetView* backbuffer; // 백 버퍼에 렌더링할 이미지

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

	// Window class 설정
	// Window class 는 커널이 렌더링하고 이벤트 처리를 하기 위한 템플릿(메타데이터)
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
	// 클라이언트 영역을 기준으로 전체 윈도우 크기로 변환
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	// Window class 등록
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
	// Windows 이벤트 및 메시지 처리
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
			::PostQuitMessage(0); // WM_QUIT 전송
			return 0;
		}break;
	}
	::DefWindowProc(hWnd, message, wParam, lParam);
}

// DXGI
/*
* DirectX Graphic Infrastructure
* - DirectX의 일부가 아니라, DirectX와 하드웨어의 인터페이스 역할
* - Application -> Direct3D 11 -> DXGI -> Hardware
*/

// Swap chain
/*
* - GPU는 메모리에 이미지 버퍼를 가지고 있음.
* - 실제 화면에 렌더링 할 때에는 이미지 버퍼를 모니터로 전송
* - 이미지 버퍼 채움 -> 모니터에 전송 -> 모니터 출력 -> 반복
* - 하지만, 문제는 모니터 출력이 빠르지 않다는 것. 아무리 빨라도 60 ~ 100Hz
* - 그래서 모니터 위쪽에는 이전 이미지가 나오고, 아래쪽은 새로운 이미지가 나오는 이상한 현상이 발생
* 
* - 이를 방지하기 위해 DXGI가 스와핑 기능을 구현
* - 모니터에 직접 렌더링 하는 것이 아닌, 백 버퍼라는 보조 버퍼에 채우는 방식
* - 프론트 버퍼는 현재 모니터에서 출력되는 버퍼
* - DirectX가 백 버퍼에 이미지를 채우면, DXGI가 프론트 버퍼를 백 버퍼의 내용으로 채움
* - 하지만 이렇게 해도 화면이 찢어질 수 있음
* 
* - 때문에 DXGI는 각 버퍼를 모두 포인터로 가지고 있고, 이 포인터 값만 변경해 티어링을 예방
* - 모니터 주사율에 맞추어 포인터 값만 순식간에 바꾸어 티어링 현상을 예방하는 것
*/

// 그래픽 객체 구성 요소
/*
* 1. 점(point) 목록 : 꼭짓점의 목록, 각 꼭짓점의 위치 값을 가지고 있음.
* 2. 선(line) 목록 : 정점 두개가 이루는 선분에 대한 목록. 3D 그리드, 웨이포인트 등에 유용
* 3. 선(line) strips : 모든 정점이 선분으로 연결될 때, 각 정점들의 모음. 와이어 프레임 이미지, 디버깅에 유용
* 4. 삼각형 목록 : 세 개의 꼭짓점 그룹을 이용해 하나의 삼각형을 만듬. 이 삼각형들의 목록.
* 5. 삼각형 strips : 서로 연결된 일련의 삼각형을 생성하는 꼭짓점들의 모음. 3D 그래픽스에 자주 사용
*/

void InitD3D(HWND hWnd)
{
	// 스왑 체인 메타데이터
	DXGI_SWAP_CHAIN_DESC scd;

	::ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferCount = 1; // 백 버퍼 하나
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32비트 RGB
	scd.BufferDesc.Width = SCREEN_WIDTH;
	scd.BufferDesc.Height = SCREEN_HEIGHT;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 4; // 멀티 샘플 수
	scd.Windowed = TRUE; // 창 모드 or 전체 화면 모드
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// scd를 바탕으로 장치 컨텍스트와 스왑 체인 생성
	::D3D11CreateDeviceAndSwapChain(NULL, // GPU 어뎁터를 찾는데 DXGI가 알아서 처리
		D3D_DRIVER_TYPE_HARDWARE, // 드라이버 유형
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

	// 화면에 그릴 이미지는 2D이미지
	ID3D11Texture2D* pBackBuffer;
	// 스왑 체인에서 백 버퍼를 찾아 ID3D11Texture2D 객체를 생성
	// 백 버퍼는 보통 0번쩨에 있음
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// 렌더 타겟 객체를 생성
	// 렌더 타겟은 백버퍼에 올리기 전 이미지를 생성하는 버퍼를 하나 할당하겠다는 의미
	dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	pBackBuffer->Release();

	// 렌더 타겟을 백 버퍼로 설정
	devcon->OMSetRenderTargets(1, &backbuffer, NULL);

	// 뷰포트 설정
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0; // 좌측 상단 X값을 0
	viewport.TopLeftY = 0; // 좌측 상단 Y값을 0
	viewport.Width = SCREEN_WIDTH; // 너비를 800
	viewport.Height = SCREEN_HEIGHT; // 너비를 600
	devcon->RSSetViewports(1, &viewport);
}

void RenderFrame(void)
{
	DirectX::XMFLOAT4 color(0.0f, 0.2f, 0.4f, 1.0f); // D3DXCOLOR는 이제 지원하지 않음
	// 지정한 색상으로 채운다.
	devcon->ClearRenderTargetView(backbuffer, &color.x);
	// 백 버퍼와 프론트 버퍼 전환
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
	// 셰이더 두개를 가져와 컴파일
	ID3D10Blob* vs, * ps;
	ID3DBlob* errorBlob;

	// D3DX11CompileFromFile -> D3DCompileFromFile
	D3DCompileFromFile(L"shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, &vs, &errorBlob);
	D3DCompileFromFile(L"shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, &ps, &errorBlob);

	// 컴파일된 셰이더는 COM 객체에 저장
	// COM 객체를 이용해 셰이더 객채 생성
	dev->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &pVs);
	dev->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, &pPs);

	devcon->VSSetShader(pVs, 0, 0);
	devcon->PSSetShader(pPs, 0, 0);
}
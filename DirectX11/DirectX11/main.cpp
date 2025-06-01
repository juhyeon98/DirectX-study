/*
* 프로젝트 세팅법
* 1. 솔루션 탐색기의 프로젝트 속성에 진입
* 2. 링커에 진입
* 3. 먼저 입력에서 추가 종속성에 d3d11.lib; dxgi.lib; d3dcompiler.lib; 추가
* 4. 시스템의 서브 시스템에서 `/SUBSYSTEM:WINDOWS` 로 변경
*/

#include <Windows.h>
#include <iostream>

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
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = TEXT("WindowClass1");


	RECT wr = { 0,0,500,400 };
	// 클라이언트 영역을 기준으로 전체 윈도우 크기로 변환
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	// Window class 등록
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
			::PostQuitMessage(0); // WM_QUIT 전송
			return 0;
		}break;
	}
	::DefWindowProc(hWnd, message, wParam, lParam);
}
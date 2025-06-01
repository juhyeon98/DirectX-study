/*
* ������Ʈ ���ù�
* 1. �ַ�� Ž������ ������Ʈ �Ӽ��� ����
* 2. ��Ŀ�� ����
* 3. ���� �Է¿��� �߰� ���Ӽ��� d3d11.lib; dxgi.lib; d3dcompiler.lib; �߰�
* 4. �ý����� ���� �ý��ۿ��� `/SUBSYSTEM:WINDOWS` �� ����
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
			::PostQuitMessage(0); // WM_QUIT ����
			return 0;
		}break;
	}
	::DefWindowProc(hWnd, message, wParam, lParam);
}
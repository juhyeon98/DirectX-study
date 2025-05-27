/*
* https://rastertek.com/dx11win10tut02.html ���⿡���� ���α׷� ����
* 
* WinMain : entry point
* ������ System
*		������ Input : �Է� ó�� ���
*		������ Application : ������ ���
*/

#pragma
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

/*
* Win32���� �� ������ �ʴ� �͵��� ���ܽ��� ���� �ӵ��� ���̴� ������ �Ѵ�.
*/
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Input.h"
#include "Application.h"

class System
{
public:
	System();
	System(const System&);
	~System();

	bool Initialize();
	void Shutdown();
	void Run();

	/*
	* Unix�� ������, �ñ׳��� ó���ϴ� �Լ�
	* ������, Unix�� �ñ׳��� ����Ʈ���� ���ͷ�Ʈ�� �ݸ�, Windows�� Message�� ����Ʈ����/�ϵ���� ���ͷ�Ʈ�� ������ �д�.
	* �׸��� Windows�� ������ Ŀ�� ������ ������, Ŀ���� ��� �������� Message ť�� �����Ѵ�.
	* Windows�� message driven �ý������� ��� ���ͷ�Ʈ�� message�� ó���Ѵ�.
	* 
	* �⺻���� ���� ������
	* 1. Ŀ�ο��� â(HWND)������ �̺�Ʈ�� �߻��ϴ� ���� ����
	* 2. �ݹ� �Լ����� � �̺�Ʈ(UINT)������ ������(WPARAM, LPARAM)�� ����
	* �̶�, WPARAM�� LPARAM�� ���� ���� ������(word)�� ū ������(long)��, �ܼ��� �����ʹ� WPARAM���� �����ϰ� �ڼ��� �����ʹ� LPARAM���� �����Ѵ�.
	* ex) ���콺�� � Ű�� �Է� �Ǿ���? : WPARAM���� ����. ���콺�� �Է� ��ġ��? : LPARAM
	* 
	* �Լ��� �ڼ��� ����
	* - LRESUTL : long*
	* - CALLBACK : __stdcall. ��ȣ���ڰ� ������ �����ϴµ�, WinAPI������ �ַ� __stdcall�� ���
	* - HWND : �ڵ�. Windows������ Unix�� fd���� ������ �Ѵ�. ��, fd�� ���Ͽ� �������̶��, �ڵ��� �� �� ���� ������ Ŀ�� ��ü�� �����Ѵ�.
	* - WPARAM : unsigned int*
	* - LPARAM : long*
	* �����, �����͵��� ��� __int64�� ��õǾ��ִ�.
	*/
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR mApplicationName;
	HINSTANCE mhInstance;
	HWND mHwnd;

	Input* mInput;
	Application* mApplication;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static System* ApplicationHandle = 0;

#endif
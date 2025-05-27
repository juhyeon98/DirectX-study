/*
* https://rastertek.com/dx11win10tut02.html 여기에서의 프로그램 구조
* 
* WinMain : entry point
* └── System
*		├── Input : 입력 처리 담당
*		└── Application : 렌더링 담당
*/

#pragma
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

/*
* Win32에서 잘 사용되지 않는 것들을 제외시켜 빌드 속도를 높이는 역할을 한다.
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
	* Unix로 따지면, 시그널을 처리하는 함수
	* 하지만, Unix의 시그널은 소프트웨어 인터럽트인 반면, Windows의 Message는 소프트웨어/하드웨어 인터럽트로 범위가 넓다.
	* 그리고 Windows는 별도의 커널 스택이 없으며, 커널이 모든 스레드의 Message 큐를 관리한다.
	* Windows는 message driven 시스템으로 모든 인터럽트를 message로 처리한다.
	* 
	* 기본적인 동작 원리는
	* 1. 커널에서 창(HWND)에서의 이벤트가 발생하는 것을 감지
	* 2. 콜백 함수에게 어떤 이벤트(UINT)인지와 데이터(WPARAM, LPARAM)을 전송
	* 이때, WPARAM과 LPARAM은 각각 작은 데이터(word)와 큰 데이터(long)로, 단순한 데이터는 WPARAM으로 전송하고 자세한 데이터는 LPARAM으로 전송한다.
	* ex) 마우스의 어떤 키가 입력 되었나? : WPARAM으로 전송. 마우스의 입력 위치는? : LPARAM
	* 
	* 함수를 자세히 뜯어보면
	* - LRESUTL : long*
	* - CALLBACK : __stdcall. 피호출자가 스택을 정리하는데, WinAPI에서는 주로 __stdcall을 사용
	* - HWND : 핸들. Windows에서는 Unix의 fd같은 역할을 한다. 단, fd가 파일에 한정적이라면, 핸들은 좀 더 넓은 범위의 커널 객체를 지정한다.
	* - WPARAM : unsigned int*
	* - LPARAM : long*
	* 참고로, 포인터들은 모두 __int64로 명시되어있다.
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
/*
* Application은 렌더링을 해주는 역할로 실제 렌더링은 D3D가 해준다.
* WinMain : entry point
* └── System
*		├── Input : 입력 처리 담당
*		└── Application : 렌더링 담당
*				└──D3D : Direct 3D - 실제 렌더링
*/

#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <Windows.h>
#include "D3D.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLE = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;

class Application
{
public:
	Application();
	Application(const Application&);
	~Application();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();
private:
	bool Render();
private:
	D3D* mDirect3D;
};

#endif
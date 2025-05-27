/*
* Application�� �������� ���ִ� ���ҷ� ���� �������� D3D�� ���ش�.
* WinMain : entry point
* ������ System
*		������ Input : �Է� ó�� ���
*		������ Application : ������ ���
*				������D3D : Direct 3D - ���� ������
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
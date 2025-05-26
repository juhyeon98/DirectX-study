/*
* 프로젝트 세팅법
* 1. 솔루션 탐색기의 프로젝트 속성에 진입
* 2. 링커에 진입
* 3. 먼저 입력에서 추가 종속성에 d3d11.lib; dxgi.lib; d3dcompiler.lib; 추가
* 4. 시스템의 서브 시스템에서 `/SUBSYSTEM:WINDOWS` 로 변경
*/

#include "System.h"

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdShow)
{
	System* system;
	bool result;

	system = new System;

	result = system->Initialize();
	if (result)
	{
		system->Run();
	}

	system->Shutdown();
	delete system;
	system = 0;
	return 0;
}
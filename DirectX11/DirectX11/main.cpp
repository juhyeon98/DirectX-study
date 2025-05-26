/*
* ������Ʈ ���ù�
* 1. �ַ�� Ž������ ������Ʈ �Ӽ��� ����
* 2. ��Ŀ�� ����
* 3. ���� �Է¿��� �߰� ���Ӽ��� d3d11.lib; dxgi.lib; d3dcompiler.lib; �߰�
* 4. �ý����� ���� �ý��ۿ��� `/SUBSYSTEM:WINDOWS` �� ����
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
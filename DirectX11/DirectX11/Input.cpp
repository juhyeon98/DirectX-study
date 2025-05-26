#include "Input.h"

Input::Input()
{
}

Input::Input(const Input* other)
{
}

Input::~Input()
{
}

void Input::Initialize()
{
	for (int i = 0; i < 256; i++)
	{
		mKeys[i] = false;
	}
}

void Input::KeyDown(unsigned int input)
{
	mKeys[input] = true;
}

void Input::KeyUp(unsigned int input)
{
	mKeys[input] = false;
}

bool Input::IsKeyDown(unsigned int key)
{
	return mKeys[key];
}
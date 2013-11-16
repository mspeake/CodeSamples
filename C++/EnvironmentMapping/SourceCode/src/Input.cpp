/********************************************************************
**Input.cpp
**
**-Implements the Input class, used for detecting mouse position and
** states of keys(pressed, released, etc.)
********************************************************************/

#include "SystemModules.h"

//-----Input Class Functions-----//
Input::Input()
{
	CurrentKeyState = new unsigned char[256];
	PreviousKeyState = new unsigned char[256];
	
	memset(CurrentKeyState, 0, sizeof(unsigned char) * 256);
	memset(PreviousKeyState, 0, sizeof(unsigned char) * 256);

	CursorPosX	= 0;
	CursorPosY = 0;
	CursorDeltaX = 0;
	CursorDeltaY = 0;
}

Input::~Input()
{
	if(CurrentKeyState)
		delete [] CurrentKeyState;
	if(PreviousKeyState)
		delete [] PreviousKeyState;

	CurrentKeyState = NULL;
	PreviousKeyState = NULL;
}

void Input::Update(void)
{
	memcpy(PreviousKeyState, CurrentKeyState, sizeof(unsigned char) * 256);
	GetKeyboardState(CurrentKeyState);

	POINT Coord;
	GetCursorPos(&Coord);
	ScreenToClient(GetForegroundWindow(), &Coord);

	CursorDeltaX = Coord.x - CursorPosX;
	CursorDeltaY = Coord.y - CursorPosY;
	CursorPosX = Coord.x;
	CursorPosY = Coord.y;
}

unsigned char Input::GetCurrentKeyState(unsigned char key) const
{
	return CurrentKeyState[key];
}

unsigned char Input::GetPreviousKeyState(unsigned char key) const
{
	return PreviousKeyState[key];
}

bool Input::CheckPrevious(unsigned char key) const
{
	return (PreviousKeyState[key] & 0x80) != 0;
}

bool Input::CheckPressed(unsigned char key) const
{
	return (CurrentKeyState[key] & 0x80) != 0;
}

bool Input::CheckReleased(unsigned char key) const
{
	return (PreviousKeyState[key] & (~CurrentKeyState[key]) & 0x80) != 0;
}

bool Input::CheckTriggered(unsigned char key) const
{
	return (CurrentKeyState[key] & (~PreviousKeyState[key]) & 0x80) != 0;
}

signed long	Input::GetCursorPosX(void) const
{
	return CursorPosX;
}

signed long	Input::GetCursorPosY(void) const
{
	return CursorPosY;
}

signed long	Input::GetCursorDeltaX(void) const
{
	return CursorDeltaX;
}

signed long	Input::GetCursorDeltaY(void) const
{
	return CursorDeltaY;
}
//-----Input Class Functions-----//
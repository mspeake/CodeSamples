#ifndef INPUT_H
#define INPUT_H

/********************************************************************
**Input.h
**
**-Defines the Input class, used for detecting keyboard and mouse
** interactions
********************************************************************/

#include <Windows.h>

class Input
{
private:
	unsigned char* CurrentKeyState;
	unsigned char* PreviousKeyState;

	signed long	CursorPosX, CursorPosY;
	signed long	CursorDeltaX, CursorDeltaY;

public:
	Input();
	~Input();
	
	void Update(void);

	unsigned char GetCurrentKeyState(unsigned char key) const;
	unsigned char GetPreviousKeyState(unsigned char key) const;
	
	bool CheckPrevious(unsigned char key) const;
	bool CheckPressed(unsigned char key) const;
	bool CheckReleased(unsigned char key) const;
	bool CheckTriggered(unsigned char key) const;
	
	signed long	GetCursorPosX(void) const;
	signed long	GetCursorPosY(void) const;

	signed long	GetCursorDeltaX(void) const;
	signed long	GetCursorDeltaY(void) const;
};

#endif
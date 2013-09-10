/*--------------------------------------------------
Defines some helper functions
--------------------------------------------------*/
#ifndef __UTILS_H__
#define __UTILS_H__

#include <math.h>
#include "Geometry.h"
#define PI 3.14159

void RotateX(Vector &p, float a);
void RotateY(Vector &p, float a);
void RotateZ(Vector &p, float a);
void Translate(Vector &p, float _x, float _y, float _z);

void DrawAxis(void);
void DrawLine(const Point& p1, const Point& p2);

float DegToRad(float a);

class Input
{
public:
	static Input*	Instance	();
	static void		Free		();
	
	void			Update		();

	unsigned char				KeyStateCurr	(unsigned char key) const	{	return mpKeyStateCurr[key];	}
	unsigned char				KeyStatePrev	(unsigned char key) const	{	return mpKeyStatePrev[key];	}
	
	bool			CheckPrevious	(unsigned char key) const	{	return (mpKeyStatePrev[key] & 0x80) != 0;	}
	bool			CheckPressed	(unsigned char key) const	{	return (mpKeyStateCurr[key] & 0x80) != 0;	}
	bool			CheckReleased	(unsigned char key) const	{	return (mpKeyStatePrev[key] & (~mpKeyStateCurr[key]) & 0x80) != 0;	}
	bool			CheckTriggered	(unsigned char key) const	{	return (mpKeyStateCurr[key] & (~mpKeyStatePrev[key]) & 0x80) != 0;	}
	
	// return the current cursor (mouse) position
	signed long				CursorPosX		() const		{	return mCursorPosX;		}
	signed long				CursorPosY		() const		{	return mCursorPosY;		}

	// return the cursor (mouse) displacement between the last 2 updates
	signed long				CursorDeltaX	() const		{	return mCursorDeltaX;	}
	signed long				CursorDeltaY	() const		{	return mCursorDeltaY;	}

private:
	// variables to keep track the keyboard status
	unsigned char*				mpKeyStateCurr;
	unsigned char*				mpKeyStatePrev;

	// variables to to keep track the mouse position
	signed long				mCursorPosX, 
					mCursorPosY;
	signed long				mCursorDeltaX, 
					mCursorDeltaY;
					
					Input		();
					Input		(const Input& rhs);
	Input&			operator=	(const Input& rhs);

					~Input		();
};

extern Input*	spInput;

#endif
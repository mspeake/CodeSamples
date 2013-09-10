/*--------------------------------------------------
Implements the input class, written by Sun Fam
--------------------------------------------------*/
#include <Windows.h>
#include "Utils.h"
#include <GL/gl.h>
#include <string.h>
#include "Geometry.h"

Input* Input::Instance()
{
	if (spInput)
		return spInput;

	// create an instance of the input class
	spInput = new Input;
	
	return spInput;
}

// ---------------------------------------------------------------------------

void Input::Free()
{
	if (spInput)
	{
		delete spInput;
		spInput = 0;
	}
}

// ---------------------------------------------------------------------------

Input::Input()
{
	mpKeyStateCurr = new unsigned char [256];
	mpKeyStatePrev = new unsigned char [256];
	
	memset(mpKeyStateCurr, 0, sizeof(unsigned char) * 256);
	memset(mpKeyStatePrev, 0, sizeof(unsigned char) * 256);

	mCursorPosX		= 0;
	mCursorPosY		= 0;
	mCursorDeltaX	= 0;
	mCursorDeltaY	= 0;
}

// ---------------------------------------------------------------------------

Input::~Input()
{
	if (mpKeyStateCurr)
		delete [] mpKeyStateCurr;
	if (mpKeyStatePrev)
		delete [] mpKeyStatePrev;

	mpKeyStateCurr = 0;
	mpKeyStatePrev = 0;
}

// ---------------------------------------------------------------------------

void Input::Update()
{
	// backup the current to previous
	memcpy(mpKeyStatePrev, mpKeyStateCurr, sizeof(unsigned char) * 256);

	// get current keyboard state
	GetKeyboardState(mpKeyStateCurr);

	POINT coord;

	// get the cursor position and adjust it to the client area
	GetCursorPos	(&coord);
	ScreenToClient	(GetForegroundWindow(), &coord);

	mCursorDeltaX	= coord.x - mCursorPosX;
	mCursorDeltaY	= coord.y - mCursorPosY;
	mCursorPosX		= coord.x;
	mCursorPosY		= coord.y;
}


void RotateX(Vector &p, float a)
{
	Vector p2;
	p2.x = p.x;
	p2.y = (cos(a)*p.y) + (-sin(a)*p.z);
	p2.z = (sin(a)*p.y) + (cos(a)*p.z);

	p = p2;
}

void RotateY(Vector &p, float a)
{
	Vector p2;

	p2.x = (cos(a)*p.x) + (sin(a)*p.z);
	p2.y = p.y;
	p2.z = (-sin(a)*p.x) + (cos(a)*p.z);

	p = p2;
}

void RotateZ(Vector &p, float a)
{
	Vector p2;

	p2.x = (cos(a)*p.x) + (-sin(a)*p.y);
	p2.y = (sin(a)*p.x) + (cos(a)*p.y);
	p2.z = p.z;

	p = p2;
}

void Translate(Vector &p, float _x, float _y, float _z)
{
	p.x += _x;
	p.y += _y;
	p.z += _z;
}

void DrawAxis(void)
{
	float end = 2000.0f;
	glEnableClientState(GL_VERTEX_ARRAY);

	glColor3f(1.0f, 0.0f, 0.0f);
	DrawLine(Point(end, 0.0f, 0.0f), Point(-end, 0.0f, 0.0f));

	glColor3f(0.0f, 1.0f, 0.0f);
	DrawLine(Point(0.0f, end, 0.0f), Point(0.0f, -end, 0.0f));

	glColor3f(0.0f, 0.0f, 1.0f);
	DrawLine(Point(0.0f, 0.0f, end), Point(0.0f, 0.0f, -end));

	glDisableClientState(GL_VERTEX_ARRAY);
}

void DrawLine(const Point& p1, const Point& p2)
{
	float line[6] = {p1.x, p1.y, p1.z,
					 p2.x, p2.y, p2.z};
	glVertexPointer(3, GL_FLOAT, 0, line);
	glDrawArrays(GL_LINES, 0, 2);
}

void Camera::AlphaClamp(float alpha)
{	
	if(alpha < (-PI/2.0))
		Alpha = -PI/2.0;
	else if(alpha > (PI/2.0))
		Alpha = PI/2.0;
	else
		Alpha = alpha;		
}

void Camera::BetaClamp(float beta)
{	
	if(beta < (-PI))
		Beta += (PI * 2.0);
	else if(beta > PI)
		Beta -= (PI * 2.0);
	else
		Beta = beta;
}


float DegToRad(float a)
{
	return (a / 180.0f) * PI;
}
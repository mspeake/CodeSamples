/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Utils.cpp
Purpose: implementation of the input class utilities
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 5/22/2012
End Header --------------------------------------------------------*/

#include <Windows.h>
#include "Utils.h"
#include <GL/gl.h>

#include <string.h>

#include "Geometry.h"

#define MAX_PT_ROWS 25
std::vector< std::vector<int> > PascalsTriangle;

//-----curve functions-----
void CalculatePascalsTriangle(void)
{
	std::vector<int> r0, r1;
	r0.push_back(1);
	PascalsTriangle.push_back(r0);

	r1.push_back(1);
	r1.push_back(1);
	PascalsTriangle.push_back(r1);

	for(int i = 1; i < MAX_PT_ROWS; i++)
	{
		std::vector<int> new_row;
		new_row.push_back(1);
		for(int j = 1; j < PascalsTriangle[i].size(); j++)
		{
			int number = PascalsTriangle[i][j-1] + PascalsTriangle[i][j];
			new_row.push_back(number);
		}
		new_row.push_back(1);
		PascalsTriangle.push_back(new_row);
	}
}

int nCi(int n, int i) //n Choose i
{
	if(n < MAX_PT_ROWS)
		return PascalsTriangle[n][i];
	else
		return 0;
}

float power(float lhs, int rhs)
{
	if(rhs == 0)
		return 1.0f;
	else
	{
		float val = 1.0f;
		for(int i = 0; i < rhs; i++)
		{
			val *= lhs;
		}
		return val;
	}
}

Point CalcBBPoint(float t, std::vector<Point> &points, int point_count) //bernstein-bezier curve
{
	Point sum;

	for(int i = 0; i < point_count; i++)
	{
		int d = point_count-1;
		int dci = nCi(d, i);
		Point p = points[i];
		float lhs = power(1-t, d-i);
		float rhs = power(t, i);
		p.x *= (dci) * lhs * rhs;
		p.z *= (dci) * lhs * rhs; //this curve is along the XZ plane rather than XY

		sum.x += p.x;
		sum.z += p.z;
	}

	float dY = points[point_count-1].y - points[0].y; //linearly interpolate y
	sum.y = points[0].y + (dY * t);

	return sum;
}
//-----curve functions-----

Vector3f::Vector3f( float _r, float _g, float _b )
{
	r = _r;
	g = _g;
	b = _b;
	
}

Vector3f::Vector3f( float _x )
{
	Vector4f( _x, _x, _x, 1.0f );
}

Vector3f::Vector3f( Vector3f &src )
{
	this->x = src.x;
	this->y = src.y;
	this->z = src.z;
}


Vector3f::Vector3f( const Vector3f &src )
{
	this->x = src.x;
	this->y = src.y;
	this->z = src.z;
}

float *Vector3f::to_float()
{
	return v;
}

////////////////////////////////////////////////////////////////////

Vector4f::Vector4f( float _r, float _g, float _b, float _a )
{
	v[0] = _r;
	v[1] = _g;
	v[2] = _b;
	v[3] = _a;
}

Vector4f::Vector4f( float _x )
{
	Vector4f( _x, _x, _x, 1.0f );
}

Vector4f::Vector4f( Vector4f &src )
{
	this->x = src.x;
	this->y = src.y;
	this->z = src.z;
	this->w = src.w;
}


Vector4f::Vector4f( const Vector4f &src )
{
	this->x = src.x;
	this->y = src.y;
	this->z = src.z;
	this->w = src.w;
}


float *Vector4f::to_float()
{
	return v;
}

///////////////////////////////////////////////////////////////////
Input* Input::Instance()
{
	if (spInput)
		return spInput;

	// create an instance of the input class
	spInput = new Input;
	//assert(spInput);
	
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
	
	//assert(mpKeyStateCurr && mpKeyStatePrev);
	
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
	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_NORMAL_ARRAY);

	float end = 2000.0f;

	glColor3f(1.0f, 0.0f, 0.0f);
	float line[6] = {end, 0.0f, 0.0f,
				     -end, 0.0f, 0.0f};
	glVertexPointer(3, GL_FLOAT, 0, line);
	glDrawArrays(GL_LINES, 0, 2);


	glColor3f(0.0f, 1.0f, 0.0f);
	float line2[6] = {0.0f, end, 0.0f,
				     0.0f, -end, 0.0f};
	glVertexPointer(3, GL_FLOAT, 0, line2);
	glDrawArrays(GL_LINES, 0, 2);


	glColor3f(0.0f, 0.0f, 1.0f);
	float line3[6] = {0.0f, 0.0f, end,
				     0.0f, 0.0f, -end};
	glVertexPointer(3, GL_FLOAT, 0, line3);
	glDrawArrays(GL_LINES, 0, 2);


	/*glColor3f(0.0f, 1.0f, 0.0f);
	float lineX[6] = {5.0f, end, 0.0f,
				      5.0f, -end, 0.0f};
	glVertexPointer(3, GL_FLOAT, 0, lineX);
	glDrawArrays(GL_LINES, 0, 2);*/

	glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_NORMAL_ARRAY);
}

void DrawLine(const Point& p1, const Point& p2)
{
	glEnableClientState(GL_VERTEX_ARRAY);

	float line[6] = {p1.x, p1.y, p1.z,
					 p2.x, p2.y, p2.z};
	glVertexPointer(3, GL_FLOAT, 0, line);
	glDrawArrays(GL_LINES, 0, 2);

	glDisableClientState(GL_VERTEX_ARRAY);
}

void Camera::AlphaClamp(float alpha)
{	
	float max = DegToRad(179.0f / 2.0f);
	//need to max this at < 180 degrees because staring straight down or up breaks things
	if(alpha < -max) //PI/2.0f
		Alpha = -max;
	else if(alpha > max)
		Alpha = max;
	else
		Alpha = alpha;		
}

void Camera::BetaClamp(float beta)
{	
	if(beta < (-PI * 2.0f))
		Beta += (PI * 2.0f);
	else if(beta > PI * 2.0f)
		Beta -= (PI * 2.0f);
	else
		Beta = beta;
}


float DegToRad(float a)
{
	return (a / 180.0f) * PI;
}

float RadToDeg(float a)
{
	return (a / PI) * 180.0f;
}

float ClampTo360(float a)
{
	if(a < -360.0f)
		return a + 360.0f;
	if(a > 360.0f)
		return a - 360.0f;

	return a;
}

void ZeroClamp(float &f)
{
	if(f > 0.0f && f < 0.000009f)
	{
		f = 0.0f;
	}

	else if(f < 0.0f && f > -0.000009f)
	{
		f = 0.0f;
	}
}
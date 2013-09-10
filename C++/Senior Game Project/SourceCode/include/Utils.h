/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Utils.h
Purpose: utilities for the input class
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 5/22/2012
End Header --------------------------------------------------------*/

#ifndef __UTILS_H__
#define __UTILS_H__

#include <math.h>
#include "Geometry.h"
#define PI 3.14159

//  Generic vector class to describe 3-channel vectors
class Vector3f
{
public:
	
	union {
		
		struct {
			union {
				float r;
				float x;
			};
			
			union {
				float g;
				float y;
			};
			
			union {
				float b;
				float z;
			};
		};
		
		float v[3];
	};
	
	Vector3f( float _r = 0.0f, float _g = 0.0f, float _b = 0.0f );
	Vector3f( float _a );
	Vector3f( Vector3f &src );
	Vector3f( const Vector3f &src );	
	
	float *to_float();
	
};


// Generic vector class to describe 4-channel vectors
class Vector4f 
{
public:

	union {
		
		struct {
			
			union {
				float r;
				float x;
			};
			
			union {
				float g;
				float y;
			};
			
			union {
				float b;
				float z;
			};
			
			union {
				float a;
				float w;
			};
		};
		
		float v[4];
	};
	
	Vector4f( float _r = 0.0f, float _g = 0.0f, float _b = 0.0f, float _a = 0.0f );
	Vector4f( float _a );
	Vector4f( Vector4f &src );
	Vector4f( const Vector4f &src );
	
	float *to_float();
};

typedef Vector4f  Color4f ;

void RotateX(Vector &p, float a);
void RotateY(Vector &p, float a);
void RotateZ(Vector &p, float a);
void Translate(Vector &p, float _x, float _y, float _z);

void DrawAxis(void);
void DrawLine(const Point& p1, const Point& p2);

float DegToRad(float a);
float RadToDeg(float a);

float ClampTo360(float a);

void ZeroClamp(float &f);

Point CalcBBPoint(float t, std::vector<Point> &points, int point_count);
float power(float lhs, int rhs);
int nCi(int n, int i);
void CalculatePascalsTriangle(void);

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
extern std::vector< std::vector<int> > PascalsTriangle;

#endif
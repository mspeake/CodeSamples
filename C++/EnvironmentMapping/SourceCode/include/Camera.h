/********************************************************************
**Camera.h
**
**-Defines the Camera class, and which keys should cause it to move
** in which manner
********************************************************************/

#ifndef CAMERA_H
#define CAMERA_H

#include "MathLib.h"
#include "Graphics.h"
#include "Input.h"
#include "FrameTimer.h"

//some definitions for camera movement
#define CAMERA_MOVEMENT_ROTATE VK_LBUTTON
#define CAMERA_MOVEMENT_FORWARD 'W'
#define CAMERA_MOVEMENT_BACKWARD 'S'
#define CAMERA_MOVEMENT_RIGHT 'D'
#define CAMERA_MOVEMENT_LEFT 'A'
#define CAMERA_MOVEMENT_UP VK_SPACE
#define CAMERA_MOVEMENT_DOWN 'X'

class Camera
{
private:
	void ProcessInput(void);
	float Alpha, Beta;
	float Mat[16]; //the opengl function that loads the camera matrix uses arrays

public:
	Point Pos, Tar;
	Vector Up, Dir;

	Camera(void);

	void Reset(void);
	void Update(void);
	void LoadMatrix(void);

	void AlphaClamp(const float alpha);
	void BetaClamp(const float beta);
};

#endif
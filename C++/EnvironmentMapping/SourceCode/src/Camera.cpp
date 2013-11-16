/********************************************************************
**Camera.cpp
**
**-Implements the camera class
********************************************************************/

#include "SystemModules.h"

//-----Camera Class Functions-----//
Camera::Camera(void)
{
	Pos = Point(0.0f, 0.0f, 100.0f);
	Tar = Point(0.0f, 0.0f, 0.0f);
	Up = Vector(0.0f, 1.0f, 0.0f);
	Dir = Vector(0.0f, 0.0f, -1.0f);
	Alpha = 0.0f; 
	Beta = 0.0f;
	memset(Mat, 0.0f, 16 * sizeof(float));
}

void Camera::Reset(void)
{
	Alpha = 0.0f;
	Beta  = 0.0f;
	this->Update();
}

void Camera::ProcessInput(void)
{
	float Movement = 100.0f * System::Framerate->GetFrameTime();

	if(System::InputHandler->CheckPressed(CAMERA_MOVEMENT_ROTATE))
	{
		Alpha += System::InputHandler->GetCursorDeltaY() * 0.01f;
		Beta  -= System::InputHandler->GetCursorDeltaX() * 0.01f;
	}

	if(System::InputHandler->CheckPressed(CAMERA_MOVEMENT_FORWARD))
		Pos += (Dir * Movement);
	
	if(System::InputHandler->CheckPressed(CAMERA_MOVEMENT_BACKWARD))
		Pos -= (Dir * Movement);

	Vector Right = Dir.Cross(Up);
	Right.NormalizeMe();

	if(System::InputHandler->CheckPressed(CAMERA_MOVEMENT_RIGHT))
		Pos += (Right * Movement);

	if(System::InputHandler->CheckPressed(CAMERA_MOVEMENT_LEFT))
		Pos -= (Right * Movement);

	if(System::InputHandler->CheckPressed(CAMERA_MOVEMENT_UP))
		Pos += (Up * Movement);

	if(System::InputHandler->CheckPressed(CAMERA_MOVEMENT_DOWN))
		Pos -= (Up * Movement);
}

void Camera::Update(void)
{
	float CosA = cos(Alpha), SinA = sin(Alpha), CosB = cos(Beta), SinB = sin(Beta);

	//use the current Alpha and Beta values to determine Tar and Dir
	Tar = Pos + Vector(-CosA * SinB, -SinA, -CosA * CosB);
	Dir = Tar - Pos;

	//process the input for camera movement
	this->ProcessInput();

	//use the new Alpha and Beta values to process camera changes
	this->AlphaClamp(Alpha);
	this->BetaClamp(Beta);

	CosA = cos(Alpha);
	SinA = sin(Alpha);
	CosB = cos(Beta);
	SinB = sin(Beta);

	Point UpTar = Pos + Vector(SinA * -SinB, CosA, SinA * -CosB);

	Up = UpTar - Pos;
	Tar = Pos + Vector(-CosA * SinB, -SinA, -CosA * CosB);
	Dir = Tar - Pos;
}

void Camera::LoadMatrix(void)
{
	gluLookAt(Pos.X, Pos.Y, Pos.Z, Tar.X, Tar.Y, Tar.Z, Up.X, Up.Y, Up.Z);
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)(Mat));
}

void Camera::AlphaClamp(const float alpha)
{
	if(alpha < float(-PI / 2.0))
		Alpha = float(-PI / 2.0);
	else if(alpha > float(PI / 2.0))
		Alpha = float(PI / 2.0);
	else
		Alpha = alpha;
}

void Camera::BetaClamp(const float beta)
{
	if(beta < float(-PI))
		Beta += float(PI * 2.0);
	else if(beta > float(PI))
		Beta -= float(PI * 2.0);
	else
		Beta = beta;
}
//-----Camera Class Functions-----//
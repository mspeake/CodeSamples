/********************************************************************
**WindowManager.cpp
**
**-Implements the WindowManager and WindowObj class. This enables
** the user to create windows through glut and control their various
** aspects such as size
********************************************************************/

#include "WindowManager.h"

bool WindowManager::Initialized = false;

namespace WindowData
{
	WindowManager* WindowsController = NULL;
}

//-----WindowObj Class Functions-----//
WindowObj::WindowObj(const std::string n, const int w, const int h, const int px, const int py) : Name(n)
{
	WindowID = glutCreateWindow(n.c_str());
	Width = w;
	Height = h;
	PosX = px;
	PosY = py;
	glutShowWindow();
	Callback::Reshape(Width, Height);
}

int WindowObj::GetWindowID(void) const
{
	return WindowID;
}

int WindowObj::GetWidth(void) const
{
	return Width;
}

int WindowObj::GetHeight(void) const
{
	return Height;
}

void WindowObj::SetWidth(const int w)
{
	if(w > 0)
		Width = w;
}

void WindowObj::SetHeight(const int h)
{
	if(h > 0)
		Height = h;
}
//-----WindowObj Class Functions-----//

//-----WindowManager Class Functions-----//
WindowManager::WindowManager()
{
	MainWindow = -1;
}

WindowManager::~WindowManager()
{
	std::map<int, WindowObj*>::iterator It = WindowList.begin();

	while(It != WindowList.end())
		delete It->second;

	WindowList.clear();
}

int WindowManager::AddWindow(const std::string n, const int w, const int h, const int px, const int py)
{
	WindowObj* WO = NULL;
	if(Initialized == false)
	{
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
		glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
		glutInitWindowPosition(WINDOW_POS_X, WINDOW_POS_Y);
		
		WO = new WindowObj(n, w, h, px, py);

		glutDisplayFunc(Callback::Display);
		glutIdleFunc(Callback::Idle);
		glutReshapeFunc(Callback::Reshape);

		MainWindow = WO->GetWindowID();
		Initialized = true;
	}
	else
		WO = new WindowObj(n, w, h, px, py);

	int WindowID = WO->GetWindowID();
	WindowList[WindowID] = WO;
	return WindowID;
}

int WindowManager::GetWindowID(void) const
{
	return MainWindow;
}

int WindowManager::GetWidth(void)
{
	if(MainWindow > 0)
		return WindowList[MainWindow]->GetWidth();

	return 0;
}

int WindowManager::GetHeight(void)
{
	if(MainWindow > 0)
		return WindowList[MainWindow]->GetHeight();

	return 0;
}

void WindowManager::SetWidth(const int w)
{
	if(MainWindow > 0)
		WindowList[MainWindow]->SetWidth(w);
}

void WindowManager::SetHeight(const int h)
{
	if(MainWindow > 0)
		WindowList[MainWindow]->SetHeight(h);
}
//-----WindowManager Class Functions-----//
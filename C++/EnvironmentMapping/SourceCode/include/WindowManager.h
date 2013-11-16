/********************************************************************
**WindowManager.h
**
**-Defines the WindowManager and WindowObj classes, as well as the
** various values used to define it, such as default size, near/far
** values, and clear color
********************************************************************/

#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <Windows.h>
#include <string>

#include "Callbacks.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_POS_X 0
#define WINDOW_POS_Y 0

#define FOV_Y 45.0
#define NEAR_VAL 1.0
#define FAR_VAL 1000.0

#define CLEAR_COLOR_R 0.1f
#define CLEAR_COLOR_G 0.1f
#define CLEAR_COLOR_B 0.1f

class WindowObj
{
private:
	int WindowID;
	int Width, Height;
	int PosX, PosY;
	const std::string Name;

public:
	WindowObj(const std::string n, 
		      const int w = WINDOW_WIDTH, const int h = WINDOW_HEIGHT, 
		      const int px = WINDOW_POS_X, const int py = WINDOW_POS_Y);

	int GetWindowID(void) const;
	int GetWidth(void) const;
	int GetHeight(void) const;
	void SetWidth(const int w);
	void SetHeight(const int h);
};

class WindowManager
{
private:
	std::map<int, WindowObj*> WindowList;
	int MainWindow;
	static bool Initialized;

public:
	WindowManager();
	~WindowManager();

	int AddWindow(const std::string name, 
		          const int w = WINDOW_WIDTH, const int h = WINDOW_HEIGHT, 
		          const int px = WINDOW_POS_X, const int py = WINDOW_POS_Y);
	int GetWindowID(void) const;
	int GetWidth(void);
	int GetHeight(void);
	void SetWidth(const int w);
	void SetHeight(const int h);
};

namespace WindowData
{
	extern WindowManager* WindowsController;
}

#endif
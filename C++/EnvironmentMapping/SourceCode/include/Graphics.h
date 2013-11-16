/********************************************************************
**Graphics.h
**
**-Primarily used to make sure all opengl headers are always declared
** in the correct order, also defines various graphics related
** utility functions
********************************************************************/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "glew.h"
#include <GL/gl.h> //CANNOT be included before glew.h
#include <GL/glext.h>
#include <GL/glut.h>
#include <GL/glui.h>

namespace GraphicsUtils
{
	void DrawLine(const Point& p1, const Point& p2);
	void DrawSquare(GLuint tex, Point pos, float width, float height);
}

#endif
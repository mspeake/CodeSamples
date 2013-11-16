/********************************************************************
**Callbacks.h
**
**-Defines the callback functions used by glut, and the functions
** that define rendering to various targets(screen, framebuffer)
********************************************************************/

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "FrameBuffer.h"

namespace Callback
{
	//glut callback functions
	void Display(void);
	void Idle(void);
	void Reshape(int w, int h);
}

namespace RenderFunctions
{
	//-----Setup Functions-----//
	void StandardSetup(void);
	void ScreenRenderSetup(void);
	void FrameBufferRenderSetup(const GLuint& buffer);
	//-----Setup Functions-----//

	//-----Cleanup Functions-----//
	void ScreenRenderCleanup(void);
	void FrameBufferRenderCleanup(const GLuint& tex);
	//-----Cleanup Functions-----//

	//-----Targetted Rendering Functions-----//
	void RenderToScreen(void(*draw)(void));
	void RenderToFrameBuffer(void(*draw)(void), FrameBuffer* fb);
	//-----Targetted Rendering Functions-----//

	//-----Scene Rendering Functions-----//
	void GenerateEnvMap(const Point& pos);
	void DrawEnvMapObject(void);
	void DrawEnvMap(void);
	void DrawScene(void);
	//-----Scene Rendering Functions-----//
}

#endif
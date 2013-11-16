/********************************************************************
**FrameBuffer.h
**
**-Defines the FrameBuffer class, used for rendering to alternative
** targets than the screen, such as to a texture
********************************************************************/

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>

#include "ImageLoader.h"

#define FRAMEBUFFER_SIZE_X 512
#define FRAMEBUFFER_SIZE_Y 512

#define FRAMEBUFFER_FOV_Y 90.0

class FrameBuffer
{
private:
	static void GenerateFrameBufferTexture(GLuint& tex);
	static void GenerateDepthBuffer(GLuint& id);
	static void GenerateFrameBuffer(GLuint& tex, GLuint& fb, GLuint& db);

public:
	GLuint Tex, BufferID, DepthBuffer;

	FrameBuffer();
};

namespace FrameBufferObjects
{
	extern std::vector<FrameBuffer*> List;

	void CreateAllFrameBuffers(void);
}

#endif
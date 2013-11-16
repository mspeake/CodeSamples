/********************************************************************
**FrameBuffer.cpp
**
**-Implements the FrameBuffer class used for rendering to targets
** other than the screen
********************************************************************/

#include "FrameBuffer.h"

//-----FrameBuffer Class Functions-----//
void FrameBuffer::GenerateFrameBufferTexture(GLuint& tex)
{
	Image::GenTexture(tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FRAMEBUFFER_SIZE_X, FRAMEBUFFER_SIZE_Y,
				 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
}

void FrameBuffer::GenerateDepthBuffer(GLuint& id)
{
	Image::GenTexture(id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, FRAMEBUFFER_SIZE_X, FRAMEBUFFER_SIZE_Y, 
		         0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
}

void FrameBuffer::GenerateFrameBuffer(GLuint& tex, GLuint& fb, GLuint& db)
{
	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER, fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, db, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
}

FrameBuffer::FrameBuffer()
{
	FrameBuffer::GenerateFrameBufferTexture(Tex);
	FrameBuffer::GenerateDepthBuffer(DepthBuffer);
	FrameBuffer::GenerateFrameBuffer(Tex, BufferID, DepthBuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
}
//-----FrameBuffer Class Functions-----//

namespace FrameBufferObjects
{
	std::vector<FrameBuffer*> List;

	void CreateAllFrameBuffers(void)
	{
		for(unsigned i = 0; i < 6; i++)
		{
			FrameBuffer* FBO = new FrameBuffer();
			List.push_back(FBO);
		}
	}
}
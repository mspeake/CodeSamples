/********************************************************************
**GraphicsUtils.cpp
**
**-Implements random graphics related utility functions
********************************************************************/

#include "Shaders.h"

namespace GraphicsUtils
{
	void DrawLine(const Point& p1, const Point& p2)
	{
		glEnableClientState(GL_VERTEX_ARRAY);

		float line[6] = {p1.X, p1.Y, p1.Z,
						 p2.X, p2.Y, p2.Z};
		glVertexPointer(3, GL_FLOAT, 0, line);
		glDrawArrays(GL_LINES, 0, 2);

		glDisableClientState(GL_VERTEX_ARRAY);
	}

	void DrawSquare(GLuint tex, Point pos, float width, float height)
	{
		Point BL, BR, TR, TL;
		BL = Point(pos.X - (width/2.0f), pos.Y - (height/2.0f), 0.0f);
		BR = Point(pos.X + (width/2.0f), pos.Y - (height/2.0f), 0.0f);
		TR = Point(pos.X + (width/2.0f), pos.Y + (height/2.0f), 0.0f);
		TL = Point(pos.X - (width/2.0f), pos.Y + (height/2.0f), 0.0f);

		// draw in immediate mode
		Shaders::BindTexture(0, tex);
		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);

		glNormal3f(0.0f, 0.0f, 1.0f);

		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(BL.X, BL.Y, BL.Z);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(BR.X, BR.Y, BR.Z);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(TR.X, TR.Y, TR.Z);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(TL.X, TL.Y, TL.Z);

		glEnd();
		glDisable(GL_TEXTURE_2D);
	}

}
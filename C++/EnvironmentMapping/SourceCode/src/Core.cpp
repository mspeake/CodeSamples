/*--------------------------------------------------
Implements the major systems functions
--------------------------------------------------*/

#include "Shaders.h"  //cant include glew before gl
#include "Graphics.h"
#include <Windows.h>
#include "Core.h"

int wireframe_bool = 0;
int wind_width = SP_WINDOW_WIDTH, wind_height = SP_WINDOW_HEIGHT;
double start_time, end_time, frame_time = 0.0;
float rotAngleDelta = 5.0f;	// angle in degrees
float rotAngle = 0.0f;		// angle in degrees

GLuint LeftPic, RightPic, TopPic, BottomPic, FrontPic, BackPic;
GLuint NormalMapID;

GLuint fb1, fb2, fb3, fb4, fb5, fb6; //framebuffers for the 6 principle directions

Color clearColor(0.0, 0.0, 0.0, 1.0);

std::map<int, unsigned int> TexList, NormMapList, HeightMapList;

extern GLUI *glui;
extern int main_window;

void DrawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(clearColor.R, clearColor.G, clearColor.B, clearColor.A);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glLoadIdentity();

	glMatrixMode		(GL_MODELVIEW);
	glLoadIdentity		();
	sCamera.LoadMatrix();

	DrawEnv();
	DrawEnvMap();

	glPushMatrix();
	EnvironmentObj *obj = NULL;
	switch(shape_num)
	{
	case 0:
		{
			obj = new EnvironmentObj(SPHERE, 12, 20.0f, 20.0f, 20.0f, 3);
			break;
		}
	case 1:
		{
			obj = new EnvironmentObj(CUBE, 3, 20.0f, 20.0f, 20.0f, 3);
			break;
		}
	case 2:
		{
			obj = new EnvironmentObj(PLANE, 3, 20.0f, 20.0f, 20.0f, 3);
			break;
		}
	}

	EnvMapShader.RunShaders();
	PassEnvMapData();

	obj->Draw();
	EnvMapShader.StopShaders();
	glPopMatrix();
	delete obj;

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glutSwapBuffers();
	return;
}

void DrawEnv(void)
{
	MyShader.RunShaders();

	/*Draw all the objects*/
	for(unsigned i = 0; i < EnvObj.size(); i++)
	{
		if(EnvObj[i]->GetModelType() != PLANE)
		{
			glPushMatrix();
			if(i % 2 == 0)
				glTranslatef(cos(rotAngle) * 15.0f, sin(rotAngle) * 15.0f, cos(rotAngle) * 15.0f);
			else
				glTranslatef(sin(rotAngle) * 15.0f, cos(rotAngle) * 15.0f, sin(rotAngle) * 15.0f);

			glRotatef(rotAngle * 15.0f, 1.0f, 1.0f, 1.0f);
			EnvObj[i]->Draw();
			glPopMatrix();
		}
		else
			EnvObj[i]->Draw();
	}

	MyShader.StopShaders();
}

// Callback handlers for GLUT

/*
 *	Function	: display
 *	Arguments	: none
 *	Description	:
 *					The display callback for the application. This is where we display the content on the screen. 
 */
void display( void )
{
	GenerateEnvMap();
	reshape(wind_width, wind_height);
	DrawScene();
	//DrawEnv();
	//DrawSkyBox();
	GetTimeHelper(&end_time);
	frame_time = end_time - start_time;
	return;
}

/*
 *	Function	: reshape
 *	Arguments	: 
 *					int w, int h : new width and height values for the window 
 *	Description	:
 *					The reshape callback for the application. This is where we implement 
 *					window and viewport initialization and projection matrix. 
 */
void reshape( int w, int h )
{
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;
	
	float ratio = 1.0 * w / h;
	
	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);
	
	// Reset Matrix
	glLoadIdentity();
	
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);
	
	// Set the correct perspective.
	gluPerspective(FOV_Y,ratio,NEAR_VAL,FAR_VAL);
	
	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
	
	gluLookAt(0.0, 0.0, 5.0,
			  0.0, 0.0, 0.0,
			  0.0, 1.0, 0.0);
	
	wind_width = w;
	wind_height = h;
	return;
}

/*
 *	Function	: idle
 *	Arguments	: none
 *	Description	:
 *					The idle callback for the application. This is where we implement any updates to the state of
 *					the objects being rendered. 
 */
void idle( void )
{
	GetTimeHelper(&start_time);
	if ( glutGetWindow() != main_window )
		glutSetWindow(main_window);

	rotAngle += rotAngleDelta * frame_time;

	if(spInput->CheckTriggered(VK_ESCAPE))
	{
		EnvObj.clear();
		exit(0);
	}

	if( rotAngle > 360.0f )
		rotAngle = rotAngle - 360.0f;
	
	spInput->Update();
	sCamera.Update();

	glutPostRedisplay();
	
	return;
}


void FrameBufferGenTexture(unsigned &Pic)
{
	glGenTextures(1, &Pic);
	glBindTexture(GL_TEXTURE_2D, Pic);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512,
				 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
}

void GenDepthBuffer(unsigned &Pic)
{
	glGenTextures(1, &Pic);
	glBindTexture(GL_TEXTURE_2D, Pic);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 512, 512, 
		         0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
}

void GenFrameBuffer(unsigned &Pic, unsigned &FBuffer, unsigned &DBuffer)
{
	glGenFramebuffersEXT(1, &FBuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER, FBuffer);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DBuffer, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Pic, 0);
}

void CreateFrameBuffer(void)
{
	GLuint db1;

	GLenum ret = glGetError();
	
	//the textures to draw on
	FrameBufferGenTexture(LeftPic);
	FrameBufferGenTexture(RightPic);
	FrameBufferGenTexture(TopPic);
	FrameBufferGenTexture(BottomPic);
	FrameBufferGenTexture(FrontPic);
	FrameBufferGenTexture(BackPic);

	//generate a depth buffer
	GenDepthBuffer(db1);

	//generate frame buffers for the positive and negative principle axes
	GenFrameBuffer(LeftPic, fb1, db1);
	GenFrameBuffer(RightPic, fb2, db1);
	GenFrameBuffer(TopPic, fb3, db1);
	GenFrameBuffer(BottomPic, fb4, db1);
	GenFrameBuffer(FrontPic, fb5, db1);
	GenFrameBuffer(BackPic, fb6, db1);

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
	ret = glGetError();
}

void RenderToFrameBuffer(Point Pos, Vector Dir, Vector Up, unsigned Pic, unsigned &FBuffer)
{
	glBindFramebufferEXT(GL_FRAMEBUFFER, FBuffer);
	glPushAttrib(GL_VIEWPORT_BIT); 
	glViewport(0, 0, 512, 512);
	gluPerspective(90,1.0,1,1000);

	sCamera.Pos = Pos;
	sCamera.Up = Up;
	sCamera.Tar = Pos + Dir;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	sCamera.LoadMatrix();
	
	DrawEnv();

	glPopMatrix();
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
	
	glBindTexture(GL_TEXTURE_2D, Pic);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GenerateEnvMap(void)
{
	Point LastPos = sCamera.Pos;
	Vector LastUp = sCamera.Up;
	Vector LastTar = sCamera.Tar;
	Point ModelPosition(0.0f, 0.0f, 0.0f);
	if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		Vector DirV(-1.0f, 0.0f, 0.0f), UpV(0.0f, 1.0f, 0.0f);
		RenderToFrameBuffer(ModelPosition, DirV, UpV, LeftPic, fb1);

		DirV = Vector(1.0f, 0.0f, 0.0f);
		RenderToFrameBuffer(ModelPosition, DirV, UpV, RightPic, fb2);

		DirV = Vector(0.0f, 1.0f, 0.0f);
		UpV = Vector(0.0f, 0.0f, 1.0f);
		RenderToFrameBuffer(ModelPosition, DirV, UpV, TopPic, fb3);

		DirV = Vector(0.0f, -1.0f, 0.0f);
		RenderToFrameBuffer(ModelPosition, DirV, UpV, BottomPic, fb4);

		DirV = Vector(0.0f, 0.0f, 1.0f);
		UpV = Vector(0.0f, 1.0f, 0.0f);
		RenderToFrameBuffer(ModelPosition, DirV, UpV, FrontPic, fb5);

		DirV = Vector(0.0f, 0.0f, -1.0f);
		RenderToFrameBuffer(ModelPosition, DirV, UpV, BackPic, fb6);
	}
	sCamera.Pos = LastPos;
	sCamera.Tar = LastTar;
	sCamera.Up = LastUp;
}

void DrawSquare(unsigned Pic, Vector Pos, float Width, float Height)
{
	Vector BL, BR, TR, TL;

	// draw in immediate mode
	BindTexture(0, Pic);
	glEnable(GL_TEXTURE_2D);

	BL = Vector(Pos.x - (Width/2.0f), Pos.y - (Height/2.0f), 0.0f);
	BR = Vector(Pos.x + (Width/2.0f), Pos.y - (Height/2.0f), 0.0f);
	TR = Vector(Pos.x + (Width/2.0f), Pos.y + (Height/2.0f), 0.0f);
	TL = Vector(Pos.x - (Width/2.0f), Pos.y + (Height/2.0f), 0.0f);

	
	glBegin(GL_QUADS);

	glNormal3f(0.0f, 0.0f, 1.0f);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(BL.x, BL.y, BL.z);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(BR.x, BR.y, BR.z);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(TR.x, TR.y, TR.z);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(TL.x, TL.y, TL.z);

	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void DrawEnvMap(void)
{
	GLuint Left, Right, Top, Bottom, Front, Back;

	Left = LeftPic;
	Right = RightPic;
	Top = TopPic;
	Bottom = BottomPic;
	Front = FrontPic;
	Back = BackPic;

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, wind_width, 0, wind_height, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	float QuadWidth = 0.0f, Spacing = 0.0f;
	if(wind_width < 800)
		QuadWidth = wind_width /6.0f;
	else
	{
		Spacing = 10.0f;
		QuadWidth = 100.0f;
	}

	Vector BL, BR, TR, TL;
	// draw in immediate mode
	//-------------------------------//
	Vector Pos(60.0f, 60.0f, 0.0f);
	DrawSquare(Left, Pos, 100.0f, 100.0f);

	Pos.x += Spacing + QuadWidth;
	DrawSquare(Right, Pos, 100.0f, 100.0f);

	Pos.x += Spacing + QuadWidth;
	DrawSquare(Top, Pos, 100.0f, 100.0f);

	Pos.x += Spacing + QuadWidth;
	DrawSquare(Bottom, Pos, 100.0f, 100.0f);

	Pos.x += Spacing + QuadWidth;
	DrawSquare(Front, Pos, 100.0f, 100.0f);

	Pos.x += Spacing + QuadWidth;
	DrawSquare(Back, Pos, 100.0f, 100.0f);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}


void Camera::LoadMatrix(void)
{
	gluLookAt(Pos.x, Pos.y, Pos.z, Tar.x, Tar.y, Tar.z, Up.x, Up.y, Up.z);
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)(Mat));
}

void Camera::Update(void)
{
	Input* pInput = Input::Instance();

	float cosa = cos(Alpha), sina = sin(Alpha), cosb = cos(Beta), sinb = sin(Beta);
	float radius = 1.0f;

	Tar.x = Pos.x + (-cosa * sinb * radius);
	Tar.y = Pos.y + (-sina * radius);
	Tar.z = Pos.z + (-cosa * cosb * radius);

	Dir.x = Tar.x - Pos.x;
	Dir.y = Tar.y - Pos.y;
	Dir.z = Tar.z - Pos.z;

	float movement = 100.0f * frame_time;

	if (pInput->CheckPressed(VK_LBUTTON))
	{
		Alpha += pInput->CursorDeltaY() * 0.01f;
		Beta  -= pInput->CursorDeltaX() * 0.01f;
	}

	if (pInput->CheckPressed('W'))
		Pos += (Dir * movement);
	
	if (pInput->CheckPressed('S'))
		Pos -= (Dir * movement);

	if(pInput->CheckPressed('D'))
	{
		Vector right = Dir.Cross(Up);
		right.NormalizeMe();
		Pos += (right * movement);
	}
	if(pInput->CheckPressed('A'))
	{
		Vector right = Dir.Cross(Up);
		right.NormalizeMe();
		Pos -= (right * movement);
	}
	if(pInput->CheckPressed(VK_SPACE))
		Pos += (Up * movement);
	if(pInput->CheckPressed('X'))
		Pos -= (Up * movement);

	AlphaClamp(Alpha);
	BetaClamp(Beta);

	cosa = cos(Alpha);
	sina = sin(Alpha);
	cosb = cos(Beta);
	sinb = sin(Beta);

	Point UpTar(Pos.x + (sina * -sinb * radius), 
		        Pos.y + (cosa * radius), 
				Pos.z + (sina * -cosb * radius)); //find up the same way we find dir, just different directions

	Up.x =	UpTar.x - Pos.x;
	Up.y =	UpTar.y - Pos.y;
	Up.z =	UpTar.z - Pos.z;

	Tar.x = Pos.x + (-cosa * sinb * radius);
	Tar.y = Pos.y + (-sina * radius);
	Tar.z = Pos.z + (-cosa * cosb * radius);

	Dir.x = Tar.x - Pos.x;
	Dir.y = Tar.y - Pos.y;
	Dir.z = Tar.z - Pos.z;
}

void Shape::DrawSelf(bool wire)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, VA);
	glNormalPointer(GL_FLOAT, 0, VN);
	glTexCoordPointer(3, GL_FLOAT, 0, TCA);
	
	if((MyShader.Program != -1) && (MyShader.Running == true)) //pass the tangent and bitangent arrays to the shader
	{
		GLuint T_loc, B_loc;
		T_loc = glGetAttribLocation(MyShader.Program, "tangent");
		if(T_loc > 0)
		{
			glEnableVertexAttribArray(T_loc);
			glVertexAttribPointer(T_loc, 3, GL_FLOAT, false, 0, TanA);
		}
		B_loc = glGetAttribLocation(MyShader.Program, "bitangent");
		if(B_loc > 0)
		{
			glEnableVertexAttribArray(B_loc);
			glVertexAttribPointer(B_loc, 3, GL_FLOAT, false, 0, BitA);
		}
	}

	if((EnvMapShader.Program != -1) && (EnvMapShader.Running == true))
	{
		GLuint T_loc, B_loc;
		T_loc = glGetAttribLocation(EnvMapShader.Program, "tangent");
		if(T_loc > 0)
		{
			glEnableVertexAttribArray(T_loc);
			glVertexAttribPointer(T_loc, 3, GL_FLOAT, false, 0, TanA);
		}
		B_loc = glGetAttribLocation(EnvMapShader.Program, "bitangent");
		if(B_loc > 0)
		{
			glEnableVertexAttribArray(B_loc);
			glVertexAttribPointer(B_loc, 3, GL_FLOAT, false, 0, BitA);
		}
	}

	if(wireframe_bool || wire)
	{
		bool point_render = false;

		if(point_render)
		{
			for(unsigned i = 0; i < vert_count; i++)
			{
				Point p, mid(0.0f, 0.0f, 0.0f);
				p.x = VA[i*3 + 0], p.y = VA[i*3 + 1], p.z = VA[i*3 + 2];
				DrawLine(p, mid);
			}
		}
		else
		{
			for(unsigned i = 0; i < face_count; i++)
			{
				Point p1, p2, p3;
				Face f;
				f.p1 = FA[i*3 + 0], f.p2 = FA[i*3 + 1], f.p3 = FA[i*3 + 2];

				p1.x = VA[3*f.p1+0], p1.y = VA[3*f.p1+1], p1.z = VA[3*f.p1+2];
				p2.x = VA[3*f.p2+0], p2.y = VA[3*f.p2+1], p2.z = VA[3*f.p2+2];
				p3.x = VA[3*f.p3+0], p3.y = VA[3*f.p3+1], p3.z = VA[3*f.p3+2];

				DrawLine(p1, p2);
				DrawLine(p2, p3);
				DrawLine(p3, p1);
			}
		}
	}
	else
		glDrawElements(GL_TRIANGLES, face_count * 3, GL_UNSIGNED_SHORT, FA);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void AABB::DrawSelf(void) const
{
	float min_x = C.x - E.x, max_x = C.x + E.x,
		  min_y = C.y - E.y, max_y = C.y + E.y,
		  min_z = C.z - E.z, max_z = C.z + E.z;

	Point FTL(min_x, max_y, max_z), FBL(min_x, min_y, max_z),
		  FBR(max_x, min_y, max_z), FTR(max_x, max_y, max_z),
		  BTL(min_x, max_y, min_z), BBL(min_x, min_y, min_z),
		  BBR(max_x, min_y, min_z), BTR(max_x, max_y, min_z);

	glEnableClientState(GL_VERTEX_ARRAY);

	DrawLine(FTL, FBL);
	DrawLine(FTL, FTR);
	DrawLine(FTL, BTL);
	DrawLine(BBL, BTL);
	DrawLine(BBL, FBL);
	DrawLine(BBL, BBR);
	DrawLine(BTR, BTL);
	DrawLine(BTR, FTR);
	DrawLine(BTR, BBR);
	DrawLine(FBR, FBL);
	DrawLine(FBR, FTR);
	DrawLine(FBR, BBR);

	glDisableClientState(GL_VERTEX_ARRAY);
}

void Triangle::DrawSelf(void) const
{
	GLfloat tri[] = {P.x, P.y, P.z,
					Q.x, Q.y, Q.z,
					R.x, R.y, R.z};

	glColor3f(col.R, col.G, col.B);

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, tri);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableClientState(GL_VERTEX_ARRAY);
}


void GetTimeHelper(double *t)
{
	static bool				firstTime = true;
	static LARGE_INTEGER	f;
	LARGE_INTEGER			c;
	
	// if this is the 1st call to the function, 
	// query the performance counter frequency
	if (firstTime)
	{
		QueryPerformanceFrequency(&f);
		firstTime = false;
	}
	
	if (f.QuadPart)
	{
		QueryPerformanceCounter(&c);

		double r0, r1;

		r0	= c.QuadPart / f.QuadPart;
		r1	= (c.QuadPart % f.QuadPart) / (double)(f.QuadPart);
		*t	= r0 + r1;
	}
	else
	{
		*t = timeGetTime() * 0.001;
	}
}

void GameObj::Draw(void)
{
	glPushMatrix();

	glTranslatef(Pos.x, Pos.y, Pos.z);
	glRotatef(Graph->angle, Graph->axis.x, Graph->axis.y, Graph->axis.z);
	//scale is baked in upon creation to keep normal mapping happy
	glColor3f(Graph->color.R, Graph->color.G, Graph->color.B);

	int TexNum = Graph->texture;

	if((MyShader.Program != -1) && (MyShader.Running == true))
	{
		BindShaderTexture(0, MyShader.Program, "Texture", TexList[TexNum]);
		BindShaderTexture(1, MyShader.Program, "NormalMap", NormMapList[TexNum]);
		BindShaderTexture(2, MyShader.Program, "HeightMap", HeightMapList[TexNum]);

		PassShaderData();
	}

	Graph->model->DrawSelf();

	glPopMatrix();
}

void LoadTexture(std::string file, int index)
{
	Image *test_img = Image::Load(file.c_str());

	GLuint TexNum;
	
	glGenTextures(1, &TexNum);
	glBindTexture(GL_TEXTURE_2D, TexNum);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, (test_img->BPP() == 32) ? GL_RGBA : GL_RGB, test_img->SizeX(), test_img->SizeY(), 
		         0, (test_img->BPP() == 32) ? GL_BGRA : GL_BGR, GL_UNSIGNED_BYTE, test_img->Data());

	Image::Free(test_img);
	TexList[index] = TexNum;

	Image *test_nm = Image::Load(file.c_str());
	test_nm->NormalMap();
	
	glGenTextures(1, &TexNum);
	glBindTexture(GL_TEXTURE_2D, TexNum);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, (test_nm->BPP() == 32) ? GL_RGBA : GL_RGB, test_nm->SizeX(), test_nm->SizeY(), 
		         0, (test_nm->BPP() == 32) ? GL_BGRA : GL_BGR, GL_UNSIGNED_BYTE, test_nm->Data());

	Image::Free(test_nm);
	NormMapList[index] = TexNum;

	Image *test_hm = Image::Load(file.c_str());
	test_hm->HeightMap();
	
	glGenTextures(1, &TexNum);
	glBindTexture(GL_TEXTURE_2D, TexNum);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, (test_hm->BPP() == 32) ? GL_RGBA : GL_RGB, test_hm->SizeX(), test_hm->SizeY(), 
		         0, (test_hm->BPP() == 32) ? GL_BGRA : GL_BGR, GL_UNSIGNED_BYTE, test_hm->Data());

	Image::Free(test_hm);
	HeightMapList[index] = TexNum;
}

void LoadTextures(void)
{
	LoadTexture("data\\textures\\white_lines.tga", WHITE_LINES);
	LoadTexture("data\\textures\\Top.tga", TOP_TEX);
	LoadTexture("data\\textures\\test.tga", BOTTOM_TEX);
	LoadTexture("data\\textures\\Left.tga", LEFT_TEX);
	LoadTexture("data\\textures\\Right.tga", RIGHT_TEX);
	LoadTexture("data\\textures\\Front.tga", FRONT_TEX);
	LoadTexture("data\\textures\\Back.tga", BACK_TEX);
	NormalMapID = NormMapList[WHITE_LINES];
}

Point WorldSpaceCursor(void)
{
	POINT CursorPos;
	GetCursorPos(&CursorPos); //screen space coordinates, now convert back to NDC
	ScreenToClient(GetForegroundWindow(), &CursorPos);

	Point C = sCamera.Pos;
	Vector v = sCamera.Dir, u = sCamera.Up;
	v.NormalizeMe();
	u.NormalizeMe();
	Vector r = v.Cross(u); //camera's right vector
	r.NormalizeMe();

	float aspect_ratio = float(wind_width) / float(wind_height);

	Point near_center = C + (v * NEAR_VAL);

	float Vh = wind_height, Vw = wind_width;

	float xQ = float(2.0f * CursorPos.x) / Vw - 1.0f,
		  yQ = float(2.0f * (Vh - CursorPos.y)) / Vh - 1.0f;

	//hooray NDC space, now to camera space

	float xR = xQ * tan(DegToRad(float(FOV_Y)) / 2.0f) * aspect_ratio,
		  yR = yQ * tan(DegToRad(float(FOV_Y)) / 2.0f); //aww yeah camera space
	
	Point WSC = near_center + (r * xR) + (u * yR); //world space coordinates
	return WSC;
}

void BindTexture(unsigned TexNum, unsigned Texture)
{
	glActiveTexture	(GL_TEXTURE0 + TexNum);
	glBindTexture	(GL_TEXTURE_2D, Texture);
	glMatrixMode	(GL_TEXTURE);
	glLoadIdentity	();
	glMatrixMode	(GL_MODELVIEW);
}

void BindShaderTexture(unsigned TexNum, unsigned ShaderProgram, char *Name, unsigned Texture)
{
	GLint tex = glGetUniformLocation(ShaderProgram, Name);
	glUniform1i(tex, TexNum);
	BindTexture(TexNum, Texture);
}
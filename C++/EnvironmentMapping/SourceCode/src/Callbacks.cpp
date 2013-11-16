/********************************************************************
**Callbacks.cpp
**
**-Implements the callback functions used by glut, and functions
** used in rendering scenes, either directly to screen or to a
** texture using frame buffers
********************************************************************/

#include "Callbacks.h"
#include "WindowManager.h"
#include "SystemModules.h"
#include "Objects.h"

namespace RenderFunctions
{
	//-----Setup Functions-----//
	void StandardSetup(void)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		System::PrimaryCamera->LoadMatrix();
	}

	void ScreenRenderSetup(void)
	{
		glClearColor(CLEAR_COLOR_R, CLEAR_COLOR_G, CLEAR_COLOR_B, 1.0f);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		StandardSetup();
	}

	void FrameBufferRenderSetup(const GLuint& buffer)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER, buffer);
		glPushAttrib(GL_VIEWPORT_BIT); 
		glViewport(0, 0, FRAMEBUFFER_SIZE_X, FRAMEBUFFER_SIZE_Y);
		gluPerspective(FRAMEBUFFER_FOV_Y, 1.0, NEAR_VAL, FAR_VAL);
		StandardSetup();
	}
	//-----Setup Functions-----//

	//-----Cleanup Functions-----//
	void ScreenRenderCleanup(void)
	{
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	void FrameBufferRenderCleanup(const GLuint& tex)
	{
		glPopMatrix();
		glPopAttrib();
		glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	//-----Cleanup Functions-----//

	//-----Targetted Rendering Functions-----//
	void RenderToScreen(void(*draw)(void))
	{
		ScreenRenderSetup();
		draw();
		ScreenRenderCleanup();
	}

	void RenderToFrameBuffer(void(*draw)(void), FrameBuffer* fb)
	{
		FrameBufferRenderSetup(fb->BufferID);
		draw();
		FrameBufferRenderCleanup(fb->Tex);
	}
	//-----Targetted Rendering Functions-----//

	//-----Scene Rendering Functions-----//
	void GenerateEnvMap(const Point& pos)
	{
		Camera Copy = Camera(*System::PrimaryCamera); //save the camera's current data

		if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		{
			System::PrimaryCamera->Dir = Vector(-1.0f, 0.0f, 0.0f);
			System::PrimaryCamera->Up = Vector(0.0f, 1.0f, 0.0f);
			System::PrimaryCamera->Pos = pos;
			System::PrimaryCamera->Tar = System::PrimaryCamera->Pos + System::PrimaryCamera->Dir;
			RenderFunctions::RenderToFrameBuffer(DrawScene, FrameBufferObjects::List[0]); //Left

			System::PrimaryCamera->Dir = Vector(1.0f, 0.0f, 0.0f);
			System::PrimaryCamera->Tar = System::PrimaryCamera->Pos + System::PrimaryCamera->Dir;
			RenderFunctions::RenderToFrameBuffer(DrawScene, FrameBufferObjects::List[1]); //Right

			System::PrimaryCamera->Dir = Vector(0.0f, 1.0f, 0.0f);
			System::PrimaryCamera->Up = Vector(0.0f, 0.0f, 1.0f);
			System::PrimaryCamera->Tar = System::PrimaryCamera->Pos + System::PrimaryCamera->Dir;
			RenderFunctions::RenderToFrameBuffer(DrawScene, FrameBufferObjects::List[2]); //Top

			System::PrimaryCamera->Dir = Vector(0.0f, -1.0f, 0.0f);
			System::PrimaryCamera->Tar = System::PrimaryCamera->Pos + System::PrimaryCamera->Dir;
			RenderFunctions::RenderToFrameBuffer(DrawScene, FrameBufferObjects::List[3]); //Bottom

			System::PrimaryCamera->Dir = Vector(0.0f, 0.0f, 1.0f);
			System::PrimaryCamera->Up = Vector(0.0f, 1.0f, 0.0f);
			System::PrimaryCamera->Tar = System::PrimaryCamera->Pos + System::PrimaryCamera->Dir;
			RenderFunctions::RenderToFrameBuffer(DrawScene, FrameBufferObjects::List[4]); //Front

			System::PrimaryCamera->Dir = Vector(0.0f, 0.0f, -1.0f);
			System::PrimaryCamera->Tar = System::PrimaryCamera->Pos + System::PrimaryCamera->Dir;
			RenderFunctions::RenderToFrameBuffer(DrawScene, FrameBufferObjects::List[5]); //Back
		}

		*System::PrimaryCamera = Copy;
	}

	void DrawEnvMapObject(void)
	{
		Shape* Obj = new Shape(ProjectData::CenterShape, 8);
		float Sc = 30.0f;
		Vector ObjScale(Sc, Sc, Sc);
		Obj->ReScaleVertices(ObjScale);

		ShaderManager* SM = Shaders::ShaderController;
		std::string ShaderName = "EnvMap";

		SM->RunShader(ShaderName);

		SM->PassTexture(ShaderName, "TexLeft", FrameBufferObjects::List[0]->Tex, 1);
		SM->PassTexture(ShaderName, "TexRight", FrameBufferObjects::List[1]->Tex, 2);
		SM->PassTexture(ShaderName, "TexTop", FrameBufferObjects::List[2]->Tex, 3);
		SM->PassTexture(ShaderName, "TexBottom", FrameBufferObjects::List[3]->Tex, 4);
		SM->PassTexture(ShaderName, "TexFront", FrameBufferObjects::List[4]->Tex, 5);
		SM->PassTexture(ShaderName, "TexBack", FrameBufferObjects::List[5]->Tex, 6);
		SM->PassTexture(ShaderName, "NormalMap", Textures::NormMapList["WHITE_LINES"], 7);
		
		SM->PassUniform3f(ShaderName, "CameraPos", Vector(System::PrimaryCamera->Pos));
		SM->PassUniform1i(ShaderName, "Option", ProjectData::UseReflection);
		SM->PassUniform1i(ShaderName, "NormFlag", ProjectData::UseNormalMapping);

		Obj->DrawSelf();

		SM->StopShader();

		delete Obj;

		DrawScene();
		DrawEnvMap();
	}

	void DrawEnvMap(void)
	{
		GLuint Left, Right, Top, Bottom, Front, Back;

		Left = FrameBufferObjects::List[0]->Tex;
		Right = FrameBufferObjects::List[1]->Tex;
		Top = FrameBufferObjects::List[2]->Tex;
		Bottom = FrameBufferObjects::List[3]->Tex;
		Front = FrameBufferObjects::List[4]->Tex;
		Back = FrameBufferObjects::List[5]->Tex;

		float WindWidth = WindowData::WindowsController->GetWidth();
		float WindHeight = WindowData::WindowsController->GetHeight();

		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, WindWidth, 0, WindHeight, 0, 1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		float QuadWidth = 0.0f, Spacing = 0.0f;
		if(WindWidth < 800)
			QuadWidth = WindWidth /6.0f;
		else
		{
			Spacing = 10.0f;
			QuadWidth = 100.0f;
		}

		Vector BL, BR, TR, TL;

		// draw in immediate mode
		//-------------------------------//
		GLuint EnvMap[6] = {Left, Right, Top, Bottom, Front, Back};
		Point Pos(60.0f, 60.0f, 0.0f);
		Vector Shift(Spacing + QuadWidth, 0.0f, 0.0f);

		for(unsigned i = 0; i < 6; i++)
		{
			GraphicsUtils::DrawSquare(EnvMap[i], Pos, 100.0f, 100.0f);
			Pos += Shift;
		}
		
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
	}

	void DrawScene(void)
	{
		//-----Pass shader data
		ShaderManager* SM = Shaders::ShaderController;
		SM->RunShader("Lighting");
		SM->PassUniform3f("Lighting", "LightPosition", System::PrimaryCamera->Pos);
		SM->PassUniform3f("Lighting", "CameraPosition", System::PrimaryCamera->Pos);
		SM->PassUniform4f("Lighting", "lightDiffuseColor", Color(0.8f, 0.8f, 0.8f, 1.0f));
		SM->PassUniform4f("Lighting", "lightSpecColor", Color(1.0f, 1.0f, 1.0f, 1.0f));
		//-----

		glColor3f(1.0f, 1.0f, 1.0f);
		std::list<Object*>::iterator It = ObjectReader::ObjectsList.begin();
		while(It != ObjectReader::ObjectsList.end())
		{
			Object* Obj = *It;
			float RotationSpeed = 10.0f * System::Framerate->GetFrameTime();
			if(Obj->GetShapeType() != PLANE)
			{
				Obj->Angle += RotationSpeed;
				Obj->Axis = Vector(1.0f, 0.0f, 1.0f);
			}
			SM->PassTexture("Lighting", "Texture", Textures::TexList[Obj->Texture.c_str()], 0);
			SM->PassTexture("Lighting", "NormalMap", Textures::NormMapList[Obj->Texture.c_str()], 1);
			Obj->DrawSelf();
			It++;
		}

		SM->StopShader();
	}
	//-----Scene Rendering Functions-----//
}

namespace Callback
{
	void Display(void)
	{
		int WindWidth = WindowData::WindowsController->GetWidth();
		int WindHeight = WindowData::WindowsController->GetHeight();
		Reshape(WindWidth, WindHeight);
		RenderFunctions::GenerateEnvMap(Point());
		RenderFunctions::RenderToScreen(RenderFunctions::DrawEnvMapObject);
		glutSwapBuffers();
		System::Framerate->EndTimer();
	}

	void Idle(void)
	{
		int MainWindow = WindowData::WindowsController->GetWindowID();
		System::Framerate->StartTimer();
		if(glutGetWindow() != MainWindow)
			glutSetWindow(MainWindow);

		if(System::InputHandler->CheckTriggered(VK_ESCAPE))
			exit(0);

		if(System::InputHandler->CheckTriggered(CHANGE_CENTER_SHAPE))
		{
			ProjectData::CenterShape++;
			if(ProjectData::CenterShape > TORUS)
				ProjectData::CenterShape = PLANE;
		}

		if(System::InputHandler->CheckTriggered(CHANGE_NORMAL_MAPPING))
			ProjectData::UseNormalMapping = !ProjectData::UseNormalMapping;

		if(System::InputHandler->CheckTriggered(CHANGE_REFLECTION_OPTION))
			ProjectData::UseReflection = !ProjectData::UseReflection;

		System::InputHandler->Update();
		System::PrimaryCamera->Update();

		glutPostRedisplay();
	}

	void Reshape(int w, int h)
	{
		// Prevent a divide by zero, when window is too short
		// (you cant make a window of zero width).
		if(h == 0)
			h = 1;
		
		float Ratio = float(w) / float(h);
		
		// Use the Projection Matrix
		glMatrixMode(GL_PROJECTION);
		
		// Reset Matrix
		glLoadIdentity();
		
		// Set the viewport to be the entire window
		glViewport(0, 0, w, h);
		
		// Set the correct perspective.
		gluPerspective(FOV_Y, Ratio, NEAR_VAL, FAR_VAL);
		
		// Get Back to the Modelview
		glMatrixMode(GL_MODELVIEW);
		
		gluLookAt(0.0, 0.0, 5.0,
				  0.0, 0.0, 0.0,
				  0.0, 1.0, 0.0);
		
		WindowData::WindowsController->SetWidth(w);
		WindowData::WindowsController->SetHeight(h);
	}
}
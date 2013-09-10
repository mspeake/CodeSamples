/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: ShaderEditor.cpp
Purpose: implementation of the ShaderEditor gui
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 5/22/2012
End Header --------------------------------------------------------*/

#include "Shaders.h"  //cant include glew before gl
#include "Graphics.h"
#include <Windows.h>
#include "ShaderEditor.h"
#include "GameStateManager.h"
#include "Sound.h"

// ShaderEditor functionality 

// Global variables

float CamDist = 200.0f;

extern GLUI			*glui;

// typical color values
Color4f  clearColor(0.0, 0.0, 0.0, 1.0);

int wireframe_bool = 0, bvh_bool = 1;

extern int main_window;

int wind_width = SE_WINDOW_WIDTH, wind_height = SE_WINDOW_HEIGHT;
bool Full_Screen = false;

double start_time, end_time, frame_time = 0.0;
double SplashTimer = 0.0;

GameObj* selected_obj = NULL;

std::map<int, unsigned int> TexList, NormMapList, HeightMapList;


// Callback handlers for GLUT

/*
 *	Function	: display
 *	Arguments	: none
 *	Description	:
 *					The display callback for the application. This is where we display the content on the screen. 
 */
void display( void )
{
	reshape(wind_width, wind_height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glLoadIdentity();

	glMatrixMode		(GL_MODELVIEW);
	glLoadIdentity		();
	sCamera.LoadMatrix();
	
	for(unsigned i = 0; i < GSM.States.size(); i++)
		GSM.States[i]->Draw(i);

	DrawCursor();

	glutSwapBuffers();

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
	unsigned Current = GSM.GetCurrentState();
	unsigned Next = GSM.GetNextState();

	if(Current != Next && Current != GST_CONFIRM)
		GSM.AddState(Next);

	unsigned index = GSM.States.size() - 1;

	GetTimeHelper(&start_time);

	if(glutGetWindow() != main_window)
		glutSetWindow(main_window);

	GSM.States[index]->Update();

	glutPostRedisplay();

	return;
}

/*
 *	Function	: keyboard
 *	Arguments	:
 *					unsigned char key : the ASCII code for the key pressed
 *					int x, int y : the mouse position coordinates when the key was pressed
 *	Description	:
 *					The display callback for the application. This is where we display the content on the screen. 
 */
void keyboard( unsigned char key, int x, int y )
{
	return;
}

/*
 *	Function	: idle
 *	Arguments	: none
 *	Description	:
 *					The idle callback for the application. This is where we implement any updates to the state of
 *					the objects being rendered. 
 */


void DrawCursor(void)
{
	Point wsc = WorldSpaceCursor();
	Vector cursor_vec = wsc - sCamera.Pos;
	cursor_vec.NormalizeMe();
	Point end = wsc + (cursor_vec * FAR_VAL);
	Ray cursor_ray(wsc, cursor_vec);

	float DistFromNear = 5.1f;
	Point wsc2 = wsc + (cursor_vec * DistFromNear);
	glColor3f(1.0f, 1.0f, 0.0f);
	glPushMatrix();
	glTranslatef(wsc2.x, wsc2.y, wsc2.z);
	glutSolidSphere(0.05f, 10, 10);
	glPopMatrix();
}

//-----Main Menu functions-----//
void Main_Menu_Load(void)
{
	LL.LoadStuff("all_scenes\\Main_Menu_Data.txt");
}

void Main_Menu_Initialize(void)
{
	//resets key states so that a command to return here doesn't chain into an exit game command
	spInput->Update();
	sCamera.Reset();

	bool playing = false;
	MusicChan->isPlaying(&playing);
	if(!playing && !MuteMusic)
		Play_Sound(MENU_MUSIC);
}

void Main_Menu_Update(void)
{
	if(spInput->CheckTriggered('P'))
	{
		GSM.ExitToState(GST_LEVEL_1);
		return;
	}

	if(spInput->CheckTriggered(VK_ESCAPE))
	{
		//GSM.ExitState();
		GSM.AddState(GST_CONFIRM);
		GSM.SetNextState(GST_QUIT);
		return;
	}

	POINT CursorPos;
	GetCursorPos(&CursorPos);
	ScreenToClient(GetForegroundWindow(), &CursorPos);

	Point TitlePos(float(wind_width) / 2.0f, HudObj[0]->Scale.y / 2.0f, 0.0f);
	HudObj[0]->Pos = TitlePos;

	unsigned ButtonCount = HudObj.size() - 1,
		     PaddingCount = ButtonCount - 1;
	float ButtonWidth = HudObj[1]->Scale.x, Padding = 30.0f, ButtonHeight = HudObj[1]->Scale.y;
	float TotalWidth = (3 * ButtonWidth) + (2 * Padding);
	float FirstButtonX = (float(wind_width) / 2.0f) - (TotalWidth / 2.0f) + (ButtonWidth / 2.0f);
	float ButtonY = float(wind_height) / 2.0f;
	for(unsigned i = 1; i <= ButtonCount-2; i++)
	{
		Point ButtonPos(FirstButtonX + ((i-1) * (ButtonWidth + Padding)), ButtonY, 0.0f);
		HudObj[i]->Pos = ButtonPos;
	}

	ButtonY += ButtonHeight + Padding;
	TotalWidth = (2 * ButtonWidth) + Padding;
	FirstButtonX = (float(wind_width) / 2.0f) - (TotalWidth / 2.0f) + (ButtonWidth / 2.0f);
	for(unsigned i = 1; i <= 2; i++)
	{
		Point ButtonPos(FirstButtonX + ((i-1) * (ButtonWidth + Padding)), ButtonY, 0.0f);
		HudObj[i+(ButtonCount-2)]->Pos = ButtonPos;
	}

	for(unsigned i = 0; i < HudObj.size(); i++)
	{
		Button *b = HudObj[i];
		if(b->ButtonType == BT_NON_INTERACTIVE)
			continue;

		float Top = b->Pos.y + (b->Scale.y / 2.0f),
			  Bottom = b->Pos.y - (b->Scale.y / 2.0f),
			  Right = b->Pos.x + (b->Scale.x / 2.0f),
			  Left = b->Pos.x - (b->Scale.x / 2.0f);

		//if the cursor is over a button, change it's color
		if((CursorPos.x <= Right && CursorPos.x >= Left) && (CursorPos.y <= Top && CursorPos.y >= Bottom))
		{
			b->color = Color(0.0f, 0.4f, 0.4f);
			if(spInput->CheckTriggered(VK_LBUTTON))
				b->InteractWith();
		}
		else
			b->color = Color(1.0f, 1.0f, 1.0f);
	}

	spInput->Update();
}

void Main_Menu_Draw(unsigned index)
{
	for(unsigned i = 0; i < HudObj.size(); i++)
		HudObj[i]->Draw();
}

void Main_Menu_Unload(void)
{
	HudObj.clear();
	MusicChan->stop();
}
//-----Main Menu functions-----//

//-----Level 1 functions-----//
void Level_1_Load(void)
{
	Player = new PlayerObj(SPHERE, 15, 10, 10, 10, WHITE_LINES);
	Player->SetPos(Point(100.0f, 5.0f, 150.0f));
	
	Player->AddOrb(ET_EARTH);
	Player->AddOrb(ET_FIRE);
	Player->AddOrb(ET_WIND);
	Player->AddOrb(ET_WATER);

	LL.LoadStuff("all_scenes\\environment1.txt");
	LL.LoadEnemyData("all_scenes\\Level_1_Enemy_Data.txt");
	LL.LoadPaths("all_scenes\\Level_1_Patrol_Paths.txt");
}

void Level_1_Initialize(void)
{
	//reset keys to prevent command chaining
	spInput->Update();
	sCamera.Reset();
	sCamera.CalculateFrustum();

	bool playing = false;
	MusicChan->isPlaying(&playing);
	if(!playing && !MuteMusic)
		Play_Sound(LEVEL_MUSIC);
}

void Level_1_Update(void)
{
	float increment = 20.0f * frame_time;
	float player_speed = 50.0f * frame_time;

	unsigned index = GSM.States.size() - 1;
	//culling
	//unsigned cull_count = 0;
	for(unsigned i = GSM.States[index]->EnvFirst; i <= GSM.States[index]->EnvLast; i++)
	{
		Vector box_scale = EnvObj[i]->GetScale() / 2.0f;
		AABB obj_box(EnvObj[i]->GetPos(), box_scale);
		int Cull = sCamera.FrustumCull(EnvObj[i]);
		EnvObj[i]->cull = Cull;
		//cull_count += Cull;
	}
	for(unsigned i = 0; i < NpcObj.size(); i++)
	{
		Vector box_scale = NpcObj[i]->GetScale() / 2.0f;
		AABB obj_box(NpcObj[i]->GetPos(), box_scale);
		int Cull = sCamera.FrustumCull(NpcObj[i]);
		NpcObj[i]->cull = Cull;
		//cull_count += Cull;
	}

	if(spInput->CheckTriggered('P'))
	{
		GSM.SwitchToState(GST_PAUSE);
		return;
	}

	if(spInput->CheckTriggered(VK_ESCAPE))
	{
		//GSM.ExitToState(GST_MAIN_MENU);
		GSM.SwitchToState(GST_PAUSE);
		return;
	}

	if(spInput->CheckTriggered(VK_LBUTTON) && !spInput->CheckPressed(VK_RBUTTON))
		ObjectSelection();

	if(spInput->CheckPressed(VK_RBUTTON)) //rotate the player and camera
	{
		Player->SetAngle((sCamera.Beta) / PI * 180.0f);
		Plane pl(sCamera.Pos, Vector(0.0f, 1.0f, 0.0f));
		Point copy = sCamera.Pos + sCamera.Dir;
		copy.Project(pl);
		Vector pdir = copy - sCamera.Pos;
		pdir.NormalizeMe();
		Player->SetDir(pdir);
		if(spInput->CheckPressed(VK_LBUTTON)) //move the player forward
		{
			Point new_pos = Player->GetPos() + (Player->GetDir() * player_speed);
			Player->SetPos(new_pos);
		}
	}

	if(spInput->CheckPressed('W'))
	{
		Point new_pos = Player->GetPos() + (Player->GetDir() * player_speed);
		Player->SetPos(new_pos);
	}
	if(spInput->CheckPressed('S'))
	{
		Point new_pos = Player->GetPos() - (Player->GetDir() * player_speed);
		Player->SetPos(new_pos);
	}

	if(spInput->CheckPressed('D'))
	{
		sCamera.Beta -= 180.0f * 0.01f * frame_time;
		Player->SetAngle((sCamera.Beta) / PI * 180.0f);
		Plane pl(sCamera.Pos, Vector(0.0f, 1.0f, 0.0f));
		Point copy = sCamera.Pos + sCamera.Dir;
		copy.Project(pl);
		Vector pdir = copy - sCamera.Pos;
		pdir.NormalizeMe();
		Player->SetDir(pdir);
	}
	if(spInput->CheckPressed('A'))
	{
		sCamera.Beta += 180.0f * 0.01f * frame_time;
		Player->SetAngle((sCamera.Beta) / PI * 180.0f);
		Plane pl(sCamera.Pos, Vector(0.0f, 1.0f, 0.0f));
		Point copy = sCamera.Pos + sCamera.Dir;
		copy.Project(pl);
		Vector pdir = copy - sCamera.Pos;
		pdir.NormalizeMe();
		Player->SetDir(pdir);
	}

	if(spInput->CheckPressed(VK_SHIFT))
	{
		if(spInput->CheckTriggered('1'))
			Player->ChargeOrb(0);
		if(spInput->CheckTriggered('2'))
			Player->ChargeOrb(1);
		if(spInput->CheckTriggered('3'))
			Player->ChargeOrb(2);
		if(spInput->CheckTriggered('4'))
			Player->ChargeOrb(3);

		if(spInput->CheckTriggered('R'))
			Player->ShootChargingOrbs();
	}
	else
	{
		if(spInput->CheckTriggered('1'))
			Player->ShootOrb(0);
		if(spInput->CheckTriggered('2'))
			Player->ShootOrb(1);
		if(spInput->CheckTriggered('3'))
			Player->ShootOrb(2);
		if(spInput->CheckTriggered('4'))
			Player->ShootOrb(3);

		if(spInput->CheckTriggered('R'))
			Player->ShootIdleOrb();
	}

	if(spInput->CheckTriggered(VK_TAB))
	{
		Point PlayerPos = Player->GetPos();
		float ClosestDist;
		GameObj *NearestEnemy = NULL;
		if(selected_obj != NULL)
		{
			ClosestDist = (selected_obj->GetPos() - PlayerPos).Magnitude();
			NearestEnemy = selected_obj;
		}
		else
			ClosestDist = (NpcObj[0]->GetPos() - PlayerPos).Magnitude();
		
		for(unsigned i = 0; i < NpcObj.size(); i++)
		{
			Point NpcPos = NpcObj[i]->GetPos();
			Vector RayVec = NpcPos - PlayerPos;
			float Dist = (NpcPos - PlayerPos).Magnitude();
			if(Dist <= ClosestDist)
			{
				float LastDist = ClosestDist;
				ClosestDist = Dist;
				GameObj *LastTar = NearestEnemy;
				NearestEnemy = NpcObj[i]; //this enemy is closer, now check for line of sight
				for(unsigned j = 0; j < EnvObj.size(); j++)
				{
					if(EnvObj[j]->cull)
						continue;

					if(EnvObj[j]->GetModelType() == PLANE)
						continue;

					//RayVec.NormalizeMe();
					Ray R(PlayerPos, RayVec);

					Intersection NpcInter, EnvInter;
					Vector NpcScale = EnvObj[j]->GetScale();
					NpcScale /= 2.0f;

					AABB EnvBox(EnvObj[j]->GetPos(), NpcScale);
					Sphere EnemySphere(NpcPos, 1.0f); //dont need to do accurate intersection on enemy

					if(R.Intersect(EnvBox, EnvInter))
					{
						if(R.Intersect(EnemySphere, NpcInter)) 
						{
							if(NpcInter.T > EnvInter.T) //not in line of sight, revert to previous target
							{
								NearestEnemy = LastTar;
								ClosestDist = LastDist;
								break;
							}
						}
					}
				}
			}
		}

		if(NearestEnemy != NULL)
			selected_obj = NearestEnemy;
	}

	//update the camera/input stuff here
	spInput->Update();
	sCamera.Update();
	Player->Update();

	if(NpcObj.empty())
	{
		GSM.ExitToState(GST_WIN);
		return;
	}

	if(Player->GetHealth() == 0.0f)
	{
		GSM.ExitToState(GST_LOSE);
		return;
	}

	for(unsigned i = 0; i < NpcObj.size(); i++)
		NpcObj[i]->Update();
}

void Level_1_Draw(unsigned index)
{
	Point wsc = WorldSpaceCursor();
	Vector cursor_vec = wsc - sCamera.Pos;
	cursor_vec.NormalizeMe();
	Point end = wsc + (cursor_vec * FAR_VAL);
	Ray cursor_ray(wsc, cursor_vec);

	MultiLightShader.RunShaders();

	//-----Draw multi light shaded objects-----//
	int First = GSM.States[index]->EnvFirst, Last = GSM.States[index]->EnvLast;
	if(First > -1 && Last > -1)
	{
		for(unsigned i = First; i <= Last; i++)
		{
			if(EnvObj[i]->cull)
				continue;

			EnvObj[i]->Draw();
		}
	}

	Player->Draw();
	Player->DrawOrbs(true);
	//-----end multi light shaded object draw-----//

	MultiLightShader.StopShaders();

	SingleLightShader.RunShaders();

	//-----Draw single light shaded objects-----//
	std::vector<GameObj*>::iterator it;
	First = GSM.States[index]->NpcFirst, Last = GSM.States[index]->NpcLast;

	if(First > -1 && Last > -1)
	{
		for(it = NpcObj.begin(); it != NpcObj.end(); it++)
		{
			EnemyObj *enemy = reinterpret_cast<EnemyObj*>(*it);

			if(enemy->GetCurrentState() == DYING)
			{
				it = NpcObj.erase(it);
				selected_obj = NULL;
				break;
			}

			if((*it)->cull)
				continue;

			(*it)->Draw();
		}
	}
	//-----end single light shaded object draw-----//

	SingleLightShader.StopShaders();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	//-----Draw selected enemy life bar-----//
	if(selected_obj != NULL)
	{
		EnemyObj *EO = reinterpret_cast<EnemyObj*>(selected_obj);
		if(!EO->cull)
			EO->DrawLifeBar();
	}

	int PlayerState = Player->GetCurrentState();

	if(PlayerState == INVULNERABLE)
	{
		AABB box(Player->GetPos(), Player->GetScale() / 2.0f);
		glColor3f(1.0f, 0.0f, 1.0f);
		box.DrawSelf();
	}
	if(PlayerState == DYING)
	{
		AABB box(Player->GetPos(), Player->GetScale() / 2.0f);
		glColor3f(1.0f, 1.0f, 0.0f);
		box.DrawSelf();
	}

	Player->DrawLifeBar();

	return;	
}

void Level_1_Unload(void)
{
	EnvObj.clear();
	NpcObj.clear();

	delete Player;
	Player = NULL;
	MusicChan->stop();
}
//-----Level 1 functions-----//

//-----Pause functions-----//
void Pause_Load(void)
{
	LL.LoadStuff("all_scenes\\Pause_Menu_Data.txt");
}

void Pause_Initialize(void)
{
	//reset keys to prevent command chaining
	spInput->Update();
	MusicChan->setPaused(true);
	SoundChan->setPaused(true);
}

void Pause_Update(void)
{
	if(spInput->CheckTriggered('P') || spInput->CheckTriggered(VK_ESCAPE))
	{
		Play_Sound(MENU_BACK);
		GSM.ExitState();
		//reset keys to prevent command chaining
		spInput->Update();
		return;
	}

	POINT CursorPos;
	GetCursorPos(&CursorPos);
	ScreenToClient(GetForegroundWindow(), &CursorPos);

	unsigned ButtonCount = HudObj.size() - 1,
		     PaddingCount = ButtonCount - 1;
	float ButtonHeight = HudObj[1]->Scale.y, Padding = 30.0f;
	float TotalHeight = (ButtonCount * ButtonHeight) + (PaddingCount * Padding);
	float FirstButtonY = (float(wind_height) / 2.0f) - (TotalHeight / 2.0f) + (ButtonHeight / 2.0f);
	for(unsigned i = 0; i <= ButtonCount; i++)
	{
		Point ButtonPos(float(wind_width) / 2.0f, FirstButtonY + i * (ButtonHeight + Padding), 0.0f);
		HudObj[i]->Pos = ButtonPos;
	}

	for(unsigned i = 0; i < HudObj.size(); i++)
	{
		Button *b = HudObj[i];
		float Top = b->Pos.y + (b->Scale.y / 2.0f),
			  Bottom = b->Pos.y - (b->Scale.y / 2.0f),
			  Right = b->Pos.x + (b->Scale.x / 2.0f),
			  Left = b->Pos.x - (b->Scale.x / 2.0f);

		//if the cursor is over a button, change it's color
		if((CursorPos.x <= Right && CursorPos.x >= Left) && (CursorPos.y <= Top && CursorPos.y >= Bottom))
		{
			b->color = Color(0.0f, 0.4f, 0.4f);
			if(spInput->CheckTriggered(VK_LBUTTON))
				b->InteractWith();
		}
		else
			b->color = Color(1.0f, 1.0f, 1.0f);
	}

	spInput->Update();
}

void Pause_Draw(unsigned index)
{
	for(unsigned i = 0; i < HudObj.size(); i++)
	{
		HudObj[i]->Draw();
	}
}

void Pause_Unload(void)
{
	HudObj.clear();
	if(!MuteMusic)
		MusicChan->setPaused(false);
	if(!MuteSFX)
		SoundChan->setPaused(false);
}
//-----Pause functions-----//

//-----Splash Screen functions-----//
void Splash_Load(void)
{
	LL.LoadStuff("all_scenes\\Splash_Screen_Data.txt");
}

void Splash_Initialize(void)
{
	//reset keys to prevent command chaining
	spInput->Update();
	SplashTimer = 0.0;
}

void Splash_Update(void)
{
	if(spInput->CheckTriggered(VK_LBUTTON))
	{
		GSM.ExitToState(GST_MAIN_MENU);
		return;
	}

	SplashTimer += frame_time;

	if(SplashTimer >= 2.0)
	{
		GSM.ExitToState(GST_MAIN_MENU);
		return;
	}

	Point SplashPos(float(wind_width) / 2.0f, float(wind_height) / 2.0f, 0.0f);
	HudObj[0]->Pos = SplashPos;
	Vector SplashScale(float(wind_width), float(wind_height), 0.1f);
	HudObj[0]->Scale = SplashScale;
	spInput->Update();
}

void Splash_Draw(unsigned index)
{
	for(unsigned i = 0; i < HudObj.size(); i++)
	{
		HudObj[i]->Draw();
	}
}

void Splash_Unload(void)
{
	HudObj.clear();
}
//-----Splash Screen functions-----//

//-----Credits Screen functions-----//
void Credits_Load(void)
{
	LL.LoadStuff("all_scenes\\Credits_Screen_Data.txt");
}

void Credits_Initialize(void)
{
	//reset keys to prevent command chaining
	spInput->Update();
}

void Credits_Update(void)
{
	if(spInput->CheckTriggered(VK_ESCAPE))
	{
		GSM.ExitState();
		//reset keys to prevent command chaining
		spInput->Update();
		return;
	}

	Point CreditsPos(float(wind_width) / 2.0f, float(wind_height) / 2.0f, 0.0f);
	unsigned Index = HudObj.size() - 1;
	HudObj[Index]->Pos = CreditsPos;
	Vector CreditsScale(float(wind_width), float(wind_height), 0.1f);
	HudObj[Index]->Scale = CreditsScale;
	HudObj[Index]->color = Color(1.0f, 1.0f, 1.0f);
	spInput->Update();
}

void Credits_Draw(unsigned index)
{
	for(unsigned i = 0; i < HudObj.size(); i++)
	{
		HudObj[i]->Draw();
	}
}

void Credits_Unload(void)
{
	Play_Sound(MENU_BACK);
	HudObj.pop_back();
}
//-----Credits Screen functions-----//

//-----Confirm Action functions-----//
void Confirm_Load(void)
{
	LL.LoadStuff("all_scenes\\Confirm_Action_Data.txt");
}

void Confirm_Initialize(void)
{
	//reset keys to prevent command chaining
	spInput->Update();
}

void Confirm_Update(void)
{
	POINT CursorPos;
	GetCursorPos(&CursorPos);
	ScreenToClient(GetForegroundWindow(), &CursorPos);

	unsigned LastIndex = HudObj.size() - 1;
	float ButtonWidth = HudObj[LastIndex]->Scale.x, Padding = 50.0f, ButtonTopPadding = 5.0f;
	float ButtonY = ButtonTopPadding + HudObj[LastIndex - 2]->Scale.y + (ButtonWidth / 2.0f); //square buttons, so width = height

	Point ButtonPos(float(wind_width) / 2.0f + (Padding / 2.0f) + ButtonWidth, ButtonY, 0.0f);
	HudObj[LastIndex]->Pos = ButtonPos;

	ButtonPos = Point(float(wind_width) / 2.0f - (Padding / 2.0f) - ButtonWidth, ButtonY, 0.0f);
	HudObj[LastIndex - 1]->Pos = ButtonPos;

	ButtonPos = Point(float(wind_width) / 2.0f, HudObj[LastIndex - 2]->Scale.y / 2.0f, 0.0f);
	HudObj[LastIndex - 2]->Pos = ButtonPos;

	for(unsigned i = 0; i < 2; i++)
	{
		unsigned Index = LastIndex - i;
		Button *b = HudObj[Index];
		float Top = b->Pos.y + (b->Scale.y / 2.0f),
			  Bottom = b->Pos.y - (b->Scale.y / 2.0f),
			  Right = b->Pos.x + (b->Scale.x / 2.0f),
			  Left = b->Pos.x - (b->Scale.x / 2.0f);

		//if the cursor is over a button, change it's color
		if((CursorPos.x <= Right && CursorPos.x >= Left) && (CursorPos.y <= Top && CursorPos.y >= Bottom))
		{
			b->color = Color(0.0f, 0.4f, 0.4f);
			if(spInput->CheckTriggered(VK_LBUTTON))
			{
				b->InteractWith();
				spInput->Update();
				return;
			}
		}
		else
			b->color = Color(1.0f, 1.0f, 1.0f);
	}

	spInput->Update();
}

void Confirm_Draw(unsigned index)
{
	for(unsigned i = 0; i < HudObj.size(); i++)
	{
		HudObj[i]->Draw();
	}
}

void Confirm_Unload(void)
{
	unsigned HudCount = 3;
	for(unsigned i = 0; i < HudCount; i++)
		HudObj.pop_back();
}
//-----Confirm Action functions-----//

//-----How to play functions-----//
void Instructions_Load(void)
{
	LL.LoadStuff("all_scenes\\Instructions_Screen_Data.txt");
}

void Instructions_Initialize(void)
{
	//reset keys to prevent command chaining
	spInput->Update();
}

void Instructions_Update(void)
{
	if(spInput->CheckTriggered(VK_ESCAPE))
	{
		GSM.ExitState();
		//reset keys to prevent command chaining
		spInput->Update();
		return;
	}

	Point InstructionsPos(float(wind_width) / 2.0f, float(wind_height) / 2.0f, 0.0f);
	unsigned Index = HudObj.size() - 1;
	HudObj[Index]->Pos = InstructionsPos;
	Vector InstructionsScale(float(wind_width), float(wind_height), 0.1f);
	HudObj[Index]->Scale = InstructionsScale;
	HudObj[Index]->color = Color(1.0f, 1.0f, 1.0f);
	spInput->Update();
}

void Instructions_Draw(unsigned index)
{
	for(unsigned i = 0; i < HudObj.size(); i++)
	{
		HudObj[i]->Draw();
	}
}

void Instructions_Unload(void)
{
	HudObj.pop_back();
	Play_Sound(MENU_BACK);
}
//-----How to play functions-----//

//-----Options menu functions-----//
void Options_Load(void)
{
	LL.LoadStuff("all_scenes\\Options_Menu_Data.txt");
}

void Options_Initialize(void)
{
	//reset keys to prevent command chaining
	spInput->Update();
}

void Options_Update(void)
{
	if(spInput->CheckTriggered(VK_ESCAPE))
	{
		GSM.ExitState();
		//reset keys to prevent command chaining
		spInput->Update();
		return;
	}

	POINT CursorPos;
	GetCursorPos(&CursorPos);
	ScreenToClient(GetForegroundWindow(), &CursorPos);

	//6 buttons + menu backdrop
	unsigned RowCount = 3;
	unsigned LastIndex = HudObj.size() - 1;
	float OptionWidth = HudObj[LastIndex - 5]->Scale.x, ButtonWidth = HudObj[LastIndex]->Scale.x;
	float Padding = 30.0f, OptionX = (float(wind_width) - (OptionWidth + Padding)) / 2.0f;
	float ButtonX = (float(wind_width) + (ButtonWidth + Padding)) / 2.0f, 
		  TotalHeight = (RowCount * ButtonWidth)+ ((RowCount - 1) * Padding), Bottom = (float(wind_height) + TotalHeight) / 2.0f;
	for(unsigned i = 0; i < RowCount; i++)
	{
		float Y = Bottom - (ButtonWidth / 2.0f) - ((ButtonWidth + Padding) * i);
		Point OptionPos(OptionX, Y, 0.1f), ButtonPos(ButtonX, Y, 0.1f);
		HudObj[LastIndex - i]->Pos = ButtonPos;
		HudObj[LastIndex - (i+3)]->Pos = OptionPos;
	}

	Vector OptionsScale(float(wind_width), float(wind_height), 0.1f);
	Point OptionsPos(float(wind_width) / 2.0f, float(wind_height) / 2.0f, 0.0f);
	HudObj[LastIndex - 6]->Pos = OptionsPos;
	HudObj[LastIndex - 6]->Scale = OptionsScale;
	HudObj[LastIndex - 6]->color = Color(1.0f, 1.0f, 1.0f);

	//everything is now positioned happy, lets make the buttons do things
	for(unsigned i = 0; i < RowCount; i++)
	{
		unsigned Index = LastIndex - i;
		Button *b = HudObj[Index];
		float Top = b->Pos.y + (b->Scale.y / 2.0f),
			  Bottom = b->Pos.y - (b->Scale.y / 2.0f),
			  Right = b->Pos.x + (b->Scale.x / 2.0f),
			  Left = b->Pos.x - (b->Scale.x / 2.0f);

		switch(i)
		{
		case 0: //Sound FX button
			{
				if(MuteSFX)
					b->Tex = OFF_BUTTON;
				else
					b->Tex = ON_BUTTON;

				break;
			}
		case 1: //Music button
			{
				if(MuteMusic)
					b->Tex = OFF_BUTTON;
				else
					b->Tex = ON_BUTTON;

				break;
			}
		case 2: //Fullscreen button
			{
				if(Full_Screen)
					b->Tex = ON_BUTTON;
				else
					b->Tex = OFF_BUTTON;

				break;
			}
		}

		//if the cursor is over a button, change it's color
		if((CursorPos.x <= Right && CursorPos.x >= Left) && (CursorPos.y <= Top && CursorPos.y >= Bottom))
		{
			b->color = Color(0.0f, 0.4f, 0.4f);
			if(spInput->CheckTriggered(VK_LBUTTON))
			{
				switch(i)
				{
				case 0: //Sound FX button
					{
						MuteSFX = !MuteSFX;
						if(MuteSFX)
							b->Tex = OFF_BUTTON;
						else
							b->Tex = ON_BUTTON;

						break;
					}
				case 1: //Music button
					{
						MuteMusic = !MuteMusic;
						if(MuteMusic)
						{
							b->Tex = OFF_BUTTON;
							MusicChan->setPaused(true);
						}
						else
						{
							b->Tex = ON_BUTTON;
							unsigned PrevGameState = GSM.States.size() - 2;
							if(GSM.States[PrevGameState]->GetStateType() != GST_PAUSE)
								MusicChan->setPaused(false);
						}

						break;
					}
				case 2: //Fullscreen button
					{
						if(Full_Screen)
							b->Tex = OFF_BUTTON;
						else
							b->Tex = ON_BUTTON;

						ToggleFullScreen();

						break;
					}
				}

				Play_Sound(MENU_SELECT);
				spInput->Update();
				return;
			}
		}
		else
			b->color = Color(1.0f, 1.0f, 1.0f);
	}
	spInput->Update();
}

void Options_Draw(unsigned index)
{
	for(unsigned i = 0; i < HudObj.size(); i++)
	{
		HudObj[i]->Draw();
	}
}

void Options_Unload(void)
{
	unsigned ButtonCount = 7;

	for(unsigned i = 0; i < ButtonCount; i++)
		HudObj.pop_back();

	Play_Sound(MENU_BACK);
}
//-----Options menu functions-----//

//-----Win Screen functions-----//
void Win_Load(void)
{
	LL.LoadStuff("all_scenes\\Win_Screen_Data.txt");
}

void Win_Initialize(void)
{
	//reset keys to prevent command chaining
	spInput->Update();

	bool playing = false;
	MusicChan->isPlaying(&playing);
	if(!playing && !MuteMusic)
		Play_Sound(WIN_MUSIC);
}

void Win_Update(void)
{
	if(spInput->CheckTriggered(VK_ESCAPE))
	{
		GSM.ExitToState(GST_MAIN_MENU);
		return;
	}

	unsigned Index = HudObj.size() - 1;
	Point WinPos(float(wind_width) / 2.0f, float(wind_height) / 2.0f, 0.0f);
	HudObj[Index]->Pos = WinPos;
	Vector WinScale(float(wind_width), float(wind_height), 0.1f);
	HudObj[Index]->Scale = WinScale;
	spInput->Update();
}

void Win_Draw(unsigned index)
{
	for(unsigned i = 0; i < HudObj.size(); i++)
	{
		HudObj[i]->Draw();
	}
}

void Win_Unload(void)
{
	HudObj.clear();
	MusicChan->stop();
}
//-----Win Screen functions-----//

//-----Lose Screen functions-----//
void Lose_Load(void)
{
	LL.LoadStuff("all_scenes\\Lose_Screen_Data.txt");
}

void Lose_Initialize(void)
{
	spInput->Update();
	Play_Sound(LOSE_SOUND);
}

void Lose_Update(void)
{
	if(spInput->CheckTriggered(VK_ESCAPE))
	{
		GSM.ExitToState(GST_MAIN_MENU);
		return;
	}

	if(spInput->CheckTriggered('P'))
	{
		GSM.ExitToState(GST_LEVEL_1);
		return;
	}

	unsigned Index = HudObj.size() - 1;
	Point WinPos(float(wind_width) / 2.0f, float(wind_height) / 2.0f, 0.0f);
	HudObj[Index]->Pos = WinPos;
	Vector WinScale(float(wind_width), float(wind_height), 0.1f);
	HudObj[Index]->Scale = WinScale;
	spInput->Update();
}

void Lose_Draw(unsigned index)
{
	for(unsigned i = 0; i < HudObj.size(); i++)
	{
		HudObj[i]->Draw();
	}
}

void Lose_Unload(void)
{
	HudObj.clear();
	MusicChan->stop();
}
//-----Lose Screen functions-----//

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

	Dir.x = (-cosa * sinb * Distance);
	Dir.y = (-sina * Distance);
	Dir.z = (-cosa * cosb * Distance);
	Dir.NormalizeMe();

	if(Player == NULL)
		Tar = Point();
	else
		Tar = Player->GetPos();
	Pos = Tar - (Dir * Distance);

	float movement = 200.0f * frame_time;

	if((pInput->CheckPressed(VK_RBUTTON)) || 
	   (pInput->CheckPressed(VK_LBUTTON)))
	{
		Alpha += pInput->CursorDeltaY() * 0.01f;
		Beta  -= pInput->CursorDeltaX() * 0.01f;
	}

	AlphaClamp(Alpha);
	BetaClamp(Beta);

	cosa = cos(Alpha);
	sina = sin(Alpha);
	cosb = cos(Beta);
	sinb = sin(Beta);

	Vector right_vec = Vector(0.0f, -1.0f, 0.0f).Cross(Dir); //awesome way to find the up vector of a vector
	Up = right_vec.Cross(Dir);
	Up.NormalizeMe();

	Dir = Tar - Pos;
	Dir.NormalizeMe();

	CalculateFrustum();
}

int Camera::FrustumCull(const GameObj* obj)
{
	Matrix4x4 trans, rot, sc, tr, transform;
	trans.Translate(obj->GetPos().x, obj->GetPos().y, obj->GetPos().z);
	rot.Rotate(obj->GetAxis(), obj->GetAngle());
	sc.Scale(obj->GetScale().x, obj->GetScale().y, obj->GetScale().z);
	tr = trans * rot;
	transform = tr * sc;
	for(unsigned i = 0; i < obj->Graph->model->vert_count; i++)
	{
		bool outside = false;
		Point p(obj->Graph->model->VA[i*3 + 0], obj->Graph->model->VA[i*3 + 1], obj->Graph->model->VA[i*3 + 2]);
		Point pt = transform * p;
		for(unsigned j = 0; j < Frustum.size(); j++)
		{
			float dist = (pt - Frustum[j].P).Dot(Frustum[j].N); //all these Ns will be normalized already
			if(dist > 0.0f)
			{
				outside = true;
				break;
			}
		}
		if(!outside) // a point inside all the planes has been found, do not cull this object
			return 0;
	}

	return 1; //every single point is outside the frustum, cull this object
}

void Shape::DrawSelf(bool wire)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, VA);
	glNormalPointer(GL_FLOAT, 0, VN);
	glTexCoordPointer(3, GL_FLOAT, 0, TCA);
	
	if(MultiLightShader.Program != -1) //pass the tangent and bitangent arrays to the shader
	{
		GLuint T_loc, B_loc;
		T_loc = glGetAttribLocation(MultiLightShader.Program, "tangent");
		if(T_loc > 0)
		{
			glEnableVertexAttribArray(T_loc);
			glVertexAttribPointer(T_loc, 3, GL_FLOAT, false, 0, TanA);
		}
		B_loc = glGetAttribLocation(MultiLightShader.Program, "bitangent");
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
				Vector p, mid(0.0f, 0.0f, 0.0);
				p.x = VA[i*3 + 0], p.y = VA[i*3 + 1], p.z = VA[i*3 + 2];

				float line[6] = {p.x, p.y, p.z,
							     mid.x, mid.y, mid.z};
				glVertexPointer(3, GL_FLOAT, 0, line);

				glDrawArrays(GL_LINES, 0, 2);
			}
		}
		else
		{
			for(unsigned i = 0; i < face_count; i++)
			{
				Vector p1, p2, p3;
				Face f;
				f.p1 = FA[i*3 + 0], f.p2 = FA[i*3 + 1], f.p3 = FA[i*3 + 2];

				p1.x = VA[3*f.p1+0], p1.y = VA[3*f.p1+1], p1.z = VA[3*f.p1+2];
				p2.x = VA[3*f.p2+0], p2.y = VA[3*f.p2+1], p2.z = VA[3*f.p2+2];
				p3.x = VA[3*f.p3+0], p3.y = VA[3*f.p3+1], p3.z = VA[3*f.p3+2];
				float line1[6] = {p1.x, p1.y, p1.z,
								 p2.x, p2.y, p2.z};
				glVertexPointer(3, GL_FLOAT, 0, line1);

				glDrawArrays(GL_LINES, 0, 2);

				float line2[6] = {p2.x, p2.y, p2.z,
								  p3.x, p3.y, p3.z};
				glVertexPointer(3, GL_FLOAT, 0, line2);
				glDrawArrays(GL_LINES, 0, 2);

				float line3[6] = {p3.x, p3.y, p3.z,
								  p1.x, p1.y, p1.z};
				glVertexPointer(3, GL_FLOAT, 0, line3);
				glDrawArrays(GL_LINES, 0, 2);
			}
		}
	}
	else
		glDrawElements(GL_TRIANGLES, face_count * 3, GL_UNSIGNED_SHORT, FA);

	if(vert_norms_bool)
	{
		for(unsigned i = 0; i < vert_count; i++)
		{
			Vector N;
			Point mid, p;
			p.x = VA[i*3 + 0], p.y = VA[i*3 + 1], p.z = VA[i*3 + 2];
			N.x = VN[i*3 + 0], N.y = VN[i*3 + 1], N.z = VN[i*3 + 2];
			N.NormalizeMe();
			//mid.x = p.x + VN[i*3 + 0], mid.y = p.y + VN[i*3 + 1], mid.z = p.z + VN[i*3 + 2];
			mid = p + (N * 10.0f);

			float line[6] = {p.x, p.y, p.z,
							 mid.x, mid.y, mid.z};
			glVertexPointer(3, GL_FLOAT, 0, line);

			glDrawArrays(GL_LINES, 0, 2);
		}
	}

	if(face_norms_bool)
	{
		for(unsigned i = 0; i < face_count; i++)
		{
			Point p, mid, p1, p2, p3;
			
			Vector N;
			Face f;
			f.p1 = FA[i*3 + 0], f.p2 = FA[i*3 + 1], f.p3 = FA[i*3 + 2];

			//face normal = average of the normals of its 3 points
			N.x = (VN[f.p1*3 + 0] + VN[f.p2*3 + 0] + VN[f.p3*3 + 0]) / 3.0f;
			N.y = (VN[f.p1*3 + 1] + VN[f.p2*3 + 1] + VN[f.p3*3 + 1]) / 3.0f;
			N.z = (VN[f.p1*3 + 2] + VN[f.p2*3 + 2] + VN[f.p3*3 + 2]) / 3.0f;

			p1.x = VA[3*f.p1+0], p1.y = VA[3*f.p1+1], p1.z = VA[3*f.p1+2];
			p2.x = VA[3*f.p2+0], p2.y = VA[3*f.p2+1], p2.z = VA[3*f.p2+2];
			p3.x = VA[3*f.p3+0], p3.y = VA[3*f.p3+1], p3.z = VA[3*f.p3+2];

			//draw the face normal starting from the middle of the face
			mid.x = (p1.x + p2.x + p3.x) / 3.0f;
			mid.y = (p1.y + p2.y + p3.y) / 3.0f;
			mid.z = (p1.z + p2.z + p3.z) / 3.0f;

			p = mid + (N * 10.0f);

			float line[6] = {p.x, p.y, p.z,
							 mid.x, mid.y, mid.z};
			glVertexPointer(3, GL_FLOAT, 0, line);

			glDrawArrays(GL_LINES, 0, 2);
		}
	}
	

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void AABB::DrawSelf(void)
{
	float min_x = C.x - E.x, max_x = C.x + E.x,
		  min_y = C.y - E.y, max_y = C.y + E.y,
		  min_z = C.z - E.z, max_z = C.z + E.z;

	Point FTL(min_x, max_y, max_z), FBL(min_x, min_y, max_z),
		  FBR(max_x, min_y, max_z), FTR(max_x, max_y, max_z),
		  BTL(min_x, max_y, min_z), BBL(min_x, min_y, min_z),
		  BBR(max_x, min_y, min_z), BTR(max_x, max_y, min_z);

	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_NORMAL_ARRAY);

	float line1[6] = {FTL.x, FTL.y, FTL.z,
					  FBL.x, FBL.y, FBL.z};
	glVertexPointer(3, GL_FLOAT, 0, line1);
	glDrawArrays(GL_LINES, 0, 2);

	float line2[6] = {FTL.x, FTL.y, FTL.z,
					  FTR.x, FTR.y, FTR.z};
	glVertexPointer(3, GL_FLOAT, 0, line2);
	glDrawArrays(GL_LINES, 0, 2);

	float line3[6] = {FTL.x, FTL.y, FTL.z,
					  BTL.x, BTL.y, BTL.z};
	glVertexPointer(3, GL_FLOAT, 0, line3);
	glDrawArrays(GL_LINES, 0, 2);

	float line4[6] = {BBL.x, BBL.y, BBL.z,
					  BTL.x, BTL.y, BTL.z};
	glVertexPointer(3, GL_FLOAT, 0, line4);
	glDrawArrays(GL_LINES, 0, 2);

	float line5[6] = {BBL.x, BBL.y, BBL.z,
					  FBL.x, FBL.y, FBL.z};
	glVertexPointer(3, GL_FLOAT, 0, line5);
	glDrawArrays(GL_LINES, 0, 2);

	float line6[6] = {BBL.x, BBL.y, BBL.z,
					  BBR.x, BBR.y, BBR.z};
	glVertexPointer(3, GL_FLOAT, 0, line6);
	glDrawArrays(GL_LINES, 0, 2);

	float line7[6] = {BTR.x, BTR.y, BTR.z,
					  BTL.x, BTL.y, BTL.z};
	glVertexPointer(3, GL_FLOAT, 0, line7);
	glDrawArrays(GL_LINES, 0, 2);

	float line8[6] = {BTR.x, BTR.y, BTR.z,
					  FTR.x, FTR.y, FTR.z};
	glVertexPointer(3, GL_FLOAT, 0, line8);
	glDrawArrays(GL_LINES, 0, 2);

	float line9[6] = {BTR.x, BTR.y, BTR.z,
					  BBR.x, BBR.y, BBR.z};
	glVertexPointer(3, GL_FLOAT, 0, line9);
	glDrawArrays(GL_LINES, 0, 2);

	float lineA[6] = {FBR.x, FBR.y, FBR.z,
					  FBL.x, FBL.y, FBL.z};
	glVertexPointer(3, GL_FLOAT, 0, lineA);
	glDrawArrays(GL_LINES, 0, 2);

	float lineB[6] = {FBR.x, FBR.y, FBR.z,
					  FTR.x, FTR.y, FTR.z};
	glVertexPointer(3, GL_FLOAT, 0, lineB);
	glDrawArrays(GL_LINES, 0, 2);

	float lineC[6] = {FBR.x, FBR.y, FBR.z,
					  BBR.x, BBR.y, BBR.z};
	glVertexPointer(3, GL_FLOAT, 0, lineC);
	glDrawArrays(GL_LINES, 0, 2);

	glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_NORMAL_ARRAY);
}

void Triangle::DrawSelf(void)
{
	GLfloat tri[] = {P.x, P.y, P.z,
					Q.x, Q.y, Q.z,
					R.x, R.y, R.z};
	/*Vector N = Vector(Q - P).Cross(R - P);
	N.NormalizeMe();
	GLfloat nrm[] = {N.x, N.y, N.z,
		             N.x, N.y, N.z,
					 N.x, N.y, N.z};*/

	glColor3f(col.R, col.G, col.B);

	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, tri);
	//glNormalPointer(GL_FLOAT, 0, nrm);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_NORMAL_ARRAY);
}

void MPR::DrawPortals(void)
{
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, SE_WINDOW_WIDTH, 0, SE_WINDOW_HEIGHT, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	float scale_val = 1.0f, max_z = Largest(P1.z, P2.z, P3.z);
	if(C.z > max_z)
		max_z = C.z;

	Point O(0.0f, 0.0f, 0.0f);
	Point O_C((wind_width/2.0f), (wind_height/5.0f), 0.0f);
	Vector dir(0.0f, 0.0f, -1.0f), up(0.0f, 1.0f, 0.0f);
	Point CamPos(O.x, O.y, max_z + 100.0f); //setting the projection camera to stare at the origin
	Matrix4x4 wtoc, ctop;

	float Wh = 2.0f * tan(FOV_Y / 2.0f) * NEAR_VAL; //find the dimensions of the projection window
	float AR = wind_width / wind_height;

	wtoc.WtoC(CamPos, dir, up);
	ctop.CtoP(NEAR_VAL, 150, Wh, Wh * AR);
	Point P1_P, P2_P, P3_P, C_P;

	O = wtoc * O;
	O = ctop * O;
	Point tr = Point(O_C.x - O.x, O_C.y - O.y, O_C.z - O.z); //want the origin centered at a fixed point in screen space
	O = O_C;

	C_P = wtoc * C;
	C_P = ctop * C_P;
	C_P.x += tr.x, C_P.y += tr.y;

	P1_P = wtoc * P1;
	P1_P = ctop * P1_P;
	P1_P.x += tr.x, P1_P.y += tr.y;

	P2_P = wtoc * P2;
	P2_P = ctop * P2_P;
	P2_P.x += tr.x, P2_P.y += tr.y;

	P3_P = wtoc * P3;
	P3_P = ctop * P3_P;
	P3_P.x += tr.x, P3_P.y += tr.y;

	//---------------------------------//
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin		(GL_LINES);

	glNormal3f	(0.0f, 0.0f, 1.0f);
	glVertex3f	(P1_P.x, P1_P.y, 0.0f);
	glVertex3f	(P2_P.x, P2_P.y, 0.0f);
	glEnd		();
	//----------------------------------//
	glBegin		(GL_LINES);

	glNormal3f	(0.0f, 0.0f, 1.0f);
	glVertex3f	(P2_P.x, P2_P.y, 0.0f);
	glVertex3f	(P3_P.x, P3_P.y, 0.0f);
	glEnd		();
	//----------------------------------//
	glBegin		(GL_LINES);

	glNormal3f	(0.0f, 0.0f, 1.0f);
	glVertex3f	(P3_P.x, P3_P.y, 0.0f);
	glVertex3f	(P1_P.x, P1_P.y, 0.0f);
	glEnd		();
	//----------------------------------//
	glColor3f(0.5f, 1.0f, 0.5f);
	glBegin		(GL_LINES);

	glNormal3f	(0.0f, 0.0f, 1.0f);
	glVertex3f	(C_P.x, C_P.y, 0.0f);
	glVertex3f	(P1_P.x, P1_P.y, 0.0f);
	glEnd		();
	//----------------------------------//
	glLoadIdentity	();
	glMatrixMode	(GL_MODELVIEW);
	glBegin		(GL_LINES);

	glNormal3f	(0.0f, 0.0f, 1.0f);
	glVertex3f	(C_P.x, C_P.y, 0.0f);
	glVertex3f	(P2_P.x, P2_P.y, 0.0f);
	glEnd		();
	//----------------------------------*/
	glLoadIdentity	();
	glMatrixMode	(GL_MODELVIEW);
	glBegin		(GL_LINES);

	glNormal3f	(0.0f, 0.0f, 1.0f);
	glVertex3f	(C_P.x, C_P.y, 0.0f);
	glVertex3f	(P3_P.x, P3_P.y, 0.0f);
	glEnd		();
	//----------------------------------*/
	glColor3f(0.5f, 0.0f, 1.0f);
	glBegin		(GL_LINES);

	glNormal3f	(0.0f, 0.0f, 1.0f);
	glVertex3f	(O.x, O.y, 0.0f);
	glVertex3f	(C_P.x, C_P.y, 0.0f);
	glEnd		();
	//----------------------------------//

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	/**/
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void DrawSquare(void)
{
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, SE_WINDOW_WIDTH, 0, SE_WINDOW_HEIGHT, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	//
	glEnable(GL_TEXTURE_2D);
	Vector3f BL, BR, TR, TL;
	
	glActiveTexture	(GL_TEXTURE0);
	glBindTexture	(GL_TEXTURE_2D, TexList[METAL_ROOF]);
	glMatrixMode	(GL_TEXTURE);
	glLoadIdentity	();
	glMatrixMode	(GL_MODELVIEW);

	BL = Vector3f(10.0f, 10.0f, 0.0f);
	BR = Vector3f(110.0f, 10.0f, 0.0f);
	TR = Vector3f(110.0f, 110.0f, 0.0f);
	TL = Vector3f(10.0f, 110.0f, 0.0f);

	glBegin		(GL_QUADS);

	glNormal3f	(0.0f, 0.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f	(BL.x, BL.y, BL.z);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f	(BR.x, BR.y, BR.z);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f	(TR.x, TR.y, TR.z);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f	(TL.x, TL.y, TL.z);

	glEnd		();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	//
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
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
		r1	= (c.QuadPart % f.QuadPart) / (f64)(f.QuadPart);
		*t	= r0 + r1;
	}
	else
	{
		*t = timeGetTime() * 0.001;
	}
}

void GameObj::Draw(void)
{
	int TexNum = Graph->texture;

	if(MultiLightShader.Program != -1 && MultiLightShader.IsRunning == true)
	{
		GLint tex0 = glGetUniformLocation(MultiLightShader.Program, "Texture");
		glUniform1i(tex0, 0);
		glActiveTexture	(GL_TEXTURE0);
		glBindTexture	(GL_TEXTURE_2D, TexList[TexNum]);
		glMatrixMode	(GL_TEXTURE);
		glLoadIdentity	();
		glMatrixMode	(GL_MODELVIEW);

		GLint tex1 = glGetUniformLocation(MultiLightShader.Program, "NormalMap");
		glUniform1i(tex1, 1);
		glActiveTexture	(GL_TEXTURE1);
		glBindTexture	(GL_TEXTURE_2D, NormMapList[TexNum]);

		GLint tex2 = glGetUniformLocation(MultiLightShader.Program, "HeightMap");
		glUniform1i(tex2, 2);
		glActiveTexture	(GL_TEXTURE2);
		glBindTexture	(GL_TEXTURE_2D, HeightMapList[TexNum]);

		MultiLightShaderData();
	}

	if(SingleLightShader.Program != -1 && SingleLightShader.IsRunning == true)
	{
		GLint tex0 = glGetUniformLocation(SingleLightShader.Program, "Texture");
		glUniform1i(tex0, 0);
		glActiveTexture	(GL_TEXTURE0);
		glBindTexture	(GL_TEXTURE_2D, TexList[TexNum]);
		glMatrixMode	(GL_TEXTURE);
		glLoadIdentity	();
		glMatrixMode	(GL_MODELVIEW);

		GLint tex1 = glGetUniformLocation(SingleLightShader.Program, "NormalMap");
		glUniform1i(tex1, 1);
		glActiveTexture	(GL_TEXTURE1);
		glBindTexture	(GL_TEXTURE_2D, NormMapList[TexNum]);

		GLint tex2 = glGetUniformLocation(SingleLightShader.Program, "HeightMap");
		glUniform1i(tex2, 2);
		glActiveTexture	(GL_TEXTURE2);
		glBindTexture	(GL_TEXTURE_2D, HeightMapList[TexNum]);

		SingleLightShaderData();
	}

	glPushMatrix();

	glTranslatef(Pos.x, Pos.y, Pos.z);
	glRotatef(Graph->angle, Graph->axis.x, Graph->axis.y, Graph->axis.z);
	glScalef(Graph->scale.x, Graph->scale.y, Graph->scale.z);
	glColor3f(Graph->color.R, Graph->color.G, Graph->color.B);

	Graph->model->DrawSelf();

	glPopMatrix();
}

void GameObj::DrawHud(void)
{
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, wind_width, wind_height, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	float Width = GetScale().x, Height = GetScale().y;
	float Z_Pos = 0.0f;
	Point TL(Pos.x - (Width / 2.0f), Pos.y - (Height / 2.0f), Z_Pos), 
		  BL(Pos.x - (Width / 2.0f), Pos.y + (Height / 2.0f), Z_Pos), 
		  TR(Pos.x + (Width / 2.0f), Pos.y - (Height / 2.0f), Z_Pos), 
		  BR(Pos.x + (Width / 2.0f), Pos.y + (Height / 2.0f), Z_Pos);

	//-----immediate mode quad-----//
	TextureShader.RunShaders();

	glActiveTexture	(GL_TEXTURE0);
	glBindTexture	(GL_TEXTURE_2D, TexList[Graph->texture]);
	glMatrixMode	(GL_TEXTURE);
	glLoadIdentity	();
	glMatrixMode	(GL_MODELVIEW);
	GLint tex0 = glGetUniformLocation(TextureShader.Program, "Texture");
	glUniform1i(tex0, 0);

	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin		(GL_QUADS);

	glNormal3f	(0.0f, 0.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f	(BL.x, BL.y, BL.z);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f	(BR.x, BR.y, BR.z);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f	(TR.x, TR.y, TR.z);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f	(TL.x, TL.y, TL.z);

	glEnd		();
	TextureShader.StopShaders();
	//-----immediate mode quad-----//

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void Button::Draw(void)
{
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, wind_width, wind_height, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	float Width = Scale.x, Height = Scale.y;
	float Z_Pos = 0.0f;
	Point TL(Pos.x - (Width / 2.0f), Pos.y - (Height / 2.0f), Z_Pos), 
		  BL(Pos.x - (Width / 2.0f), Pos.y + (Height / 2.0f), Z_Pos), 
		  TR(Pos.x + (Width / 2.0f), Pos.y - (Height / 2.0f), Z_Pos), 
		  BR(Pos.x + (Width / 2.0f), Pos.y + (Height / 2.0f), Z_Pos);

	//-----immediate mode quad-----//
	TextureShader.RunShaders();

	glActiveTexture	(GL_TEXTURE0);
	glBindTexture	(GL_TEXTURE_2D, TexList[Tex]);
	glMatrixMode	(GL_TEXTURE);
	glLoadIdentity	();
	glMatrixMode	(GL_MODELVIEW);
	GLint tex0 = glGetUniformLocation(TextureShader.Program, "Texture");
	glUniform1i(tex0, 0);

	glColor3f(color.R, color.G, color.B);

	glBegin		(GL_QUADS);

	glNormal3f	(0.0f, 0.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f	(BL.x, BL.y, BL.z);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f	(BR.x, BR.y, BR.z);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f	(TR.x, TR.y, TR.z);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f	(TL.x, TL.y, TL.z);

	glEnd		();
	TextureShader.StopShaders();
	//-----immediate mode quad-----//

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void Orb::DrawThisOrb(void) const
{
	glPushMatrix();

	Point o_pos = GetPos();
	float o_angle = GetAngle();
	Vector o_axis = GetAxis(), o_scale = GetScale();
	Color o_color = GetColor();
	glTranslatef(o_pos.x, o_pos.y, o_pos.z);
	glRotatef(o_angle, o_axis.x, o_axis.y, o_axis.z);
	glScalef(o_scale.x, o_scale.y, o_scale.z);
	glColor3f(o_color.R, o_color.G, o_color.B);

	Graph->model->DrawSelf();

	glPopMatrix();
}


void PlayerObj::DrawLifeBar(void) const
{
	
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, wind_width, wind_height, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	

	Point BackBar_BotLeft, BackBar_BotRight, BackBar_TopRight, BackBar_TopLeft;
	Point HealthBar_BotLeft, HealthBar_BotRight, HealthBar_TopRight, HealthBar_TopLeft;
	float BackBarWidth = 200.0f, BackBarHeight = 50.0f;
	float InnerPadding = 10.0f, LeftSidePadding = 25.0f, TopSidePadding = 15.0f;
	float HealthBarWidth = BackBarWidth - (InnerPadding * 2.0f),
		  HealthBarHeight = BackBarHeight - (InnerPadding * 2.0f);
	// draw in immediate mode
	//-------------------------------//
	TextureShader.RunShaders();

	glActiveTexture	(GL_TEXTURE0);
	glBindTexture	(GL_TEXTURE_2D, TexList[METAL_ROOF]);
	glMatrixMode	(GL_TEXTURE);
	glLoadIdentity	();
	glMatrixMode	(GL_MODELVIEW);
	GLint tex0 = glGetUniformLocation(TextureShader.Program, "Texture");
	glUniform1i(tex0, 0);

	Point BarCenter(LeftSidePadding + (BackBarWidth / 2.0f), TopSidePadding + (BackBarHeight / 2.0f), 0.0f);
	float HealthPercentage = Health / 100.0f;
	HealthBarWidth *= HealthPercentage;
	Color HealthColor;
	if(HealthPercentage > 0.75f)
		HealthColor = Color(0.0f, 1.0f, 0.0f);
	else if(HealthPercentage <= 0.75f && HealthPercentage > 0.50f)
		HealthColor = Color(1.0f, 1.0f, 0.0f);
	else if(HealthPercentage <= 0.50f && HealthPercentage > 0.25f)
		HealthColor = Color(1.0f, 0.5f, 0.0f);
	else
		HealthColor = Color(1.0f, 0.0f, 0.0f);

	BackBar_TopLeft = Point(BarCenter.x - (BackBarWidth / 2.0f), BarCenter.y - (BackBarHeight / 2.0f), BarCenter.z);
	BackBar_BotLeft = Point(BarCenter.x - (BackBarWidth / 2.0f), BarCenter.y + (BackBarHeight / 2.0f), BarCenter.z);
	BackBar_TopRight = Point(BarCenter.x + (BackBarWidth / 2.0f), BarCenter.y - (BackBarHeight / 2.0f), BarCenter.z);
	BackBar_BotRight = Point(BarCenter.x + (BackBarWidth / 2.0f), BarCenter.y + (BackBarHeight / 2.0f), BarCenter.z);

	glColor3f(1.0f, 1.0f, 1.0f);

	
	glBegin		(GL_QUADS);

	glNormal3f	(0.0f, 0.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f	(BackBar_BotLeft.x, BackBar_BotLeft.y, BackBar_BotLeft.z);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f	(BackBar_BotRight.x, BackBar_BotRight.y, BackBar_BotRight.z);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f	(BackBar_TopRight.x, BackBar_TopRight.y, BackBar_TopRight.z);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f	(BackBar_TopLeft.x, BackBar_TopLeft.y, BackBar_TopLeft.z);

	glEnd		();

	TextureShader.StopShaders();
	//--------------------------------//

	//-------------------------------//
	TextureShader.RunShaders();

	glActiveTexture	(GL_TEXTURE0);
	glBindTexture	(GL_TEXTURE_2D, TexList[DEFAULT_TEX]);
	glMatrixMode	(GL_TEXTURE);
	glLoadIdentity	();
	glMatrixMode	(GL_MODELVIEW);
	tex0 = glGetUniformLocation(TextureShader.Program, "Texture");
	glUniform1i(tex0, 0);

	HealthBar_TopLeft = Point(BackBar_TopLeft.x + InnerPadding, BackBar_TopLeft.y + InnerPadding, BarCenter.z);
	HealthBar_BotLeft = Point(BackBar_BotLeft.x + InnerPadding, BackBar_BotLeft.y - InnerPadding, BarCenter.z);
	HealthBar_TopRight = Point(HealthBar_TopLeft.x + HealthBarWidth, HealthBar_TopLeft.y, BarCenter.z);
	HealthBar_BotRight = Point(HealthBar_BotLeft.x + HealthBarWidth, HealthBar_BotLeft.y, BarCenter.z);
	
	glColor3f(HealthColor.R, HealthColor.G, HealthColor.B);
	glBegin		(GL_QUADS);

	glNormal3f	(0.0f, 0.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f	(HealthBar_BotLeft.x, HealthBar_BotLeft.y, HealthBar_BotLeft.z);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f	(HealthBar_BotRight.x, HealthBar_BotRight.y, HealthBar_BotRight.z);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f	(HealthBar_TopRight.x, HealthBar_TopRight.y, HealthBar_TopRight.z);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f	(HealthBar_TopLeft.x, HealthBar_TopLeft.y, HealthBar_TopLeft.z);

	glEnd		();

	TextureShader.StopShaders();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);

	Vector PowerBarScale(BackBarWidth, BackBarHeight / 3.0f, 0.0f);
	Point PowerBarPos(BarCenter.x, BarCenter.y + (BackBarHeight/2.0f) + (PowerBarScale.y/2.0f) + 5.0f, BarCenter.z);
	Color PowerBarColor(EARTH_R, EARTH_G, EARTH_B);
	DrawBar(PowerBarPos, PowerBarScale, PowerBarColor, Orbs[0]->Power);
	Point OrbPos = ScreenToWorld(Point(12.0f, PowerBarPos.y, PowerBarPos.z));
	float DistFromNear = 5.2f;
	Vector EyeVec = OrbPos - sCamera.Pos;
	Point OrbPos2 = OrbPos + (EyeVec * DistFromNear);
	glPushMatrix();
	glTranslatef(OrbPos2.x, OrbPos2.y, OrbPos2.z);
	glColor3f(PowerBarColor.R, PowerBarColor.G, PowerBarColor.B);
	glutSolidSphere(0.045f, 12, 12);
	glPopMatrix();

	PowerBarPos.y += (PowerBarScale.y + 1.0f);
	PowerBarColor = Color(FIRE_R, FIRE_G, FIRE_B);
	DrawBar(PowerBarPos, PowerBarScale, PowerBarColor, Orbs[1]->Power);
	OrbPos = ScreenToWorld(Point(12.0f, PowerBarPos.y, PowerBarPos.z));
	EyeVec = OrbPos - sCamera.Pos;
	OrbPos2 = OrbPos + (EyeVec * DistFromNear);
	glPushMatrix();
	glTranslatef(OrbPos2.x, OrbPos2.y, OrbPos2.z);
	glColor3f(PowerBarColor.R, PowerBarColor.G, PowerBarColor.B);
	glutSolidSphere(0.045f, 12, 12);
	glPopMatrix();

	PowerBarPos.y += (PowerBarScale.y + 1.0f);
	PowerBarColor = Color(WIND_R, WIND_G, WIND_B);
	DrawBar(PowerBarPos, PowerBarScale, PowerBarColor, Orbs[2]->Power);
	OrbPos = ScreenToWorld(Point(12.0f, PowerBarPos.y, PowerBarPos.z));
	EyeVec = OrbPos - sCamera.Pos;
	OrbPos2 = OrbPos + (EyeVec * DistFromNear);
	glPushMatrix();
	glTranslatef(OrbPos2.x, OrbPos2.y, OrbPos2.z);
	glColor3f(PowerBarColor.R, PowerBarColor.G, PowerBarColor.B);
	glutSolidSphere(0.045f, 12, 12);
	glPopMatrix();

	PowerBarPos.y += (PowerBarScale.y + 1.0f);
	PowerBarColor = Color(WATER_R, WATER_G, WATER_B);
	DrawBar(PowerBarPos, PowerBarScale, PowerBarColor, Orbs[3]->Power);
	OrbPos = ScreenToWorld(Point(12.0f, PowerBarPos.y, PowerBarPos.z));
	EyeVec = OrbPos - sCamera.Pos;
	OrbPos2 = OrbPos + (EyeVec * DistFromNear);
	glPushMatrix();
	glTranslatef(OrbPos2.x, OrbPos2.y, OrbPos2.z);
	glColor3f(PowerBarColor.R, PowerBarColor.G, PowerBarColor.B);
	glutSolidSphere(0.045f, 12, 12);
	glPopMatrix();
}

void DrawBar(const Point &pos, const Vector &scale, const Color &col, const float percentage)
{
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, wind_width, wind_height, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	Point BackBar_BotLeft, BackBar_BotRight, BackBar_TopRight, BackBar_TopLeft;
	Point HealthBar_BotLeft, HealthBar_BotRight, HealthBar_TopRight, HealthBar_TopLeft;
	float BackBarWidth = scale.x, BackBarHeight = scale.y;
	float InnerPadding = scale.y / 5.0f;
	float HealthBarWidth = BackBarWidth - (InnerPadding * 2.0f),
		  HealthBarHeight = BackBarHeight - (InnerPadding * 2.0f);
	// draw in immediate mode
	//-------------------------------//
	TextureShader.RunShaders();

	glActiveTexture	(GL_TEXTURE0);
	glBindTexture	(GL_TEXTURE_2D, TexList[METAL_ROOF]);
	glMatrixMode	(GL_TEXTURE);
	glLoadIdentity	();
	glMatrixMode	(GL_MODELVIEW);
	GLint tex0 = glGetUniformLocation(TextureShader.Program, "Texture");
	glUniform1i(tex0, 0);

	Point BarCenter = pos;
	HealthBarWidth *= percentage / 100.0f;

	BackBar_TopLeft = Point(BarCenter.x - (BackBarWidth / 2.0f), BarCenter.y - (BackBarHeight / 2.0f), BarCenter.z);
	BackBar_BotLeft = Point(BarCenter.x - (BackBarWidth / 2.0f), BarCenter.y + (BackBarHeight / 2.0f), BarCenter.z);
	BackBar_TopRight = Point(BarCenter.x + (BackBarWidth / 2.0f), BarCenter.y - (BackBarHeight / 2.0f), BarCenter.z);
	BackBar_BotRight = Point(BarCenter.x + (BackBarWidth / 2.0f), BarCenter.y + (BackBarHeight / 2.0f), BarCenter.z);

	glColor3f(1.0f, 1.0f, 1.0f);

	
	glBegin		(GL_QUADS);

	glNormal3f	(0.0f, 0.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f	(BackBar_BotLeft.x, BackBar_BotLeft.y, BackBar_BotLeft.z);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f	(BackBar_BotRight.x, BackBar_BotRight.y, BackBar_BotRight.z);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f	(BackBar_TopRight.x, BackBar_TopRight.y, BackBar_TopRight.z);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f	(BackBar_TopLeft.x, BackBar_TopLeft.y, BackBar_TopLeft.z);

	glEnd		();

	TextureShader.StopShaders();
	//--------------------------------//

	//-------------------------------//
	TextureShader.RunShaders();

	glActiveTexture	(GL_TEXTURE0);
	glBindTexture	(GL_TEXTURE_2D, TexList[DEFAULT_TEX]);
	glMatrixMode	(GL_TEXTURE);
	glLoadIdentity	();
	glMatrixMode	(GL_MODELVIEW);
	tex0 = glGetUniformLocation(TextureShader.Program, "Texture");
	glUniform1i(tex0, 0);

	HealthBar_TopLeft = Point(BackBar_TopLeft.x + InnerPadding, BackBar_TopLeft.y + InnerPadding, BarCenter.z);
	HealthBar_BotLeft = Point(BackBar_BotLeft.x + InnerPadding, BackBar_BotLeft.y - InnerPadding, BarCenter.z);
	HealthBar_TopRight = Point(HealthBar_TopLeft.x + HealthBarWidth, HealthBar_TopLeft.y, BarCenter.z);
	HealthBar_BotRight = Point(HealthBar_BotLeft.x + HealthBarWidth, HealthBar_BotLeft.y, BarCenter.z);
	
	glColor3f(col.R, col.G, col.B);
	glBegin		(GL_QUADS);

	glNormal3f	(0.0f, 0.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f	(HealthBar_BotLeft.x, HealthBar_BotLeft.y, HealthBar_BotLeft.z);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f	(HealthBar_BotRight.x, HealthBar_BotRight.y, HealthBar_BotRight.z);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f	(HealthBar_TopRight.x, HealthBar_TopRight.y, HealthBar_TopRight.z);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f	(HealthBar_TopLeft.x, HealthBar_TopLeft.y, HealthBar_TopLeft.z);

	glEnd		();

	TextureShader.StopShaders();
	//--------------------------------//

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void EnemyObj::DrawLifeBar(void) const
{
	float fovy = float(FOV_Y);
	float Wh = 2.0f * tan(DegToRad(fovy / 2.0f)) * float(NEAR_VAL); //find the dimensions of the projection window
	float AR = wind_width / wind_height;
	Matrix4x4 wtoc, ntov;
	wtoc.WtoC(sCamera.Pos, sCamera.Dir, sCamera.Up);
	ntov.NDCtoV(wind_width, wind_height);

	Matrix4x4 ProjMtx;
	glGetFloatv(GL_PROJECTION_MATRIX, ProjMtx.M);
	ProjMtx.Transpose();

	Point BackBar_BotLeft, BackBar_BotRight, BackBar_TopRight, BackBar_TopLeft;
	Point HealthBar_BotLeft, HealthBar_BotRight, HealthBar_TopRight, HealthBar_TopLeft;
	float BackBarWidth = 100.0f, BackBarHeight = 25.0f;
	float InnerPadding = 5.0f, BottomPadding = 7.0f;
	float HealthBarWidth = BackBarWidth - (InnerPadding * 2.0f),
		  HealthBarHeight = BackBarHeight - (InnerPadding * 2.0f);
	
	//--ortho mode stuff
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, wind_width, wind_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	//-----
	
	//choose a fixed point in world space above the enemy
	Point BarCenter = Pos + (Vector(0.0f,1.0f,0.0f) * (GetScale().y / 2.0f));

	//convert the point to screen space coordinates
	Point BC = wtoc * BarCenter; 
	BarCenter = ProjMtx * BC;

	//convert to NDC space
	BarCenter.x /= BarCenter.w,
	BarCenter.y /= BarCenter.w,
	BarCenter.z /= BarCenter.w,
	BarCenter.w /= BarCenter.w;

	//convert NDC to viewport
	BarCenter = ntov * BarCenter;

	//add some padding between the bottom of the bar and the top of the enemy
	BarCenter.y -= (BottomPadding + (BackBarHeight / 2.0f)); 

	//find the edges of the bar, make sure it doesn't go off screen
	float BackBar_Bottom = BarCenter.y + (BackBarHeight / 2.0f),
		  BackBar_Top = BarCenter.y - (BackBarHeight / 2.0f),
		  BackBar_Right = BarCenter.x + (BackBarWidth / 2.0f),
		  BackBar_Left = BarCenter.x - (BackBarWidth / 2.0f);

	//adjust it to be on screen
	if(BackBar_Bottom > wind_height)
		BarCenter.y -= (BackBar_Bottom - wind_height); 
	else if(BackBar_Top < 0.0f)
		BarCenter.y -= BackBar_Top;

	if(BackBar_Right > wind_width)
		BarCenter.x -= (BackBar_Right - wind_width);
	else if(BackBar_Left < 0.0f)
		BarCenter.x -= BackBar_Left;

	BarCenter.z = 0.0f;

	float HealthPercentage = Health / 100.0f;
	HealthBarWidth *= HealthPercentage;
	Color HealthColor;

	if(EnemyType == OBSTACLE_ENEMY)
		HealthColor = Color(1.0f, 1.0f, 0.0f);
	else
		HealthColor = Color(1.0f, 0.0f, 0.0f);

	//calculate background bar corners
	BackBar_TopLeft = Point(BarCenter.x - (BackBarWidth / 2.0f), BarCenter.y - (BackBarHeight / 2.0f), BarCenter.z);
	BackBar_BotLeft = Point(BarCenter.x - (BackBarWidth / 2.0f), BarCenter.y + (BackBarHeight / 2.0f), BarCenter.z);
	BackBar_TopRight = Point(BarCenter.x + (BackBarWidth / 2.0f), BarCenter.y - (BackBarHeight / 2.0f), BarCenter.z);
	BackBar_BotRight = Point(BarCenter.x + (BackBarWidth / 2.0f), BarCenter.y + (BackBarHeight / 2.0f), BarCenter.z);

	//calculate health bar corners
	HealthBar_TopLeft = Point(BackBar_TopLeft.x + InnerPadding, BackBar_TopLeft.y + InnerPadding, BarCenter.z);
	HealthBar_BotLeft = Point(BackBar_BotLeft.x + InnerPadding, BackBar_BotLeft.y - InnerPadding, BarCenter.z);
	HealthBar_TopRight = Point(HealthBar_TopLeft.x + HealthBarWidth, HealthBar_TopLeft.y, BarCenter.z);
	HealthBar_BotRight = Point(HealthBar_BotLeft.x + HealthBarWidth, HealthBar_BotLeft.y, BarCenter.z);

	// draw in immediate mode
	//-------------------------------//
	TextureShader.RunShaders();

	glActiveTexture	(GL_TEXTURE0);
	glBindTexture	(GL_TEXTURE_2D, TexList[METAL_ROOF]);
	glMatrixMode	(GL_TEXTURE);
	glLoadIdentity	();
	glMatrixMode	(GL_MODELVIEW);
	GLint tex0 = glGetUniformLocation(TextureShader.Program, "Texture");
	glUniform1i(tex0, 0);

	//-----------Draw Background Bar--------------------//
	glColor3f(1.0f, 1.0f, 1.0f);
	
	glBegin		(GL_QUADS);

	glNormal3f	(0.0f, 0.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f	(BackBar_BotLeft.x, BackBar_BotLeft.y, BackBar_BotLeft.z);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f	(BackBar_BotRight.x, BackBar_BotRight.y, BackBar_BotRight.z);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f	(BackBar_TopRight.x, BackBar_TopRight.y, BackBar_TopRight.z);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f	(BackBar_TopLeft.x, BackBar_TopLeft.y, BackBar_TopLeft.z);

	glEnd		();

	TextureShader.StopShaders();
	//--------------------------------//

	//------------Draw Health Bar-------------------//
	TextureShader.RunShaders();

	glActiveTexture	(GL_TEXTURE0);
	glBindTexture	(GL_TEXTURE_2D, TexList[DEFAULT_TEX]);
	glMatrixMode	(GL_TEXTURE);
	glLoadIdentity	();
	glMatrixMode	(GL_MODELVIEW);
	tex0 = glGetUniformLocation(TextureShader.Program, "Texture");
	glUniform1i(tex0, 0);
	
	glColor3f(HealthColor.R, HealthColor.G, HealthColor.B);

	glBegin		(GL_QUADS);

	glNormal3f	(0.0f, 0.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f	(HealthBar_BotLeft.x, HealthBar_BotLeft.y, HealthBar_BotLeft.z);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f	(HealthBar_BotRight.x, HealthBar_BotRight.y, HealthBar_BotRight.z);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f	(HealthBar_TopRight.x, HealthBar_TopRight.y, HealthBar_TopRight.z);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f	(HealthBar_TopLeft.x, HealthBar_TopLeft.y, HealthBar_TopLeft.z);

	glEnd		();

	TextureShader.StopShaders();
	//--------------------------------//
	
	//--end ortho mode
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	//-----
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
	LoadTexture("data\\textures\\Tiles.tga", METAL_ROOF);
	LoadTexture("data\\textures\\fase.tga", FASE);
	LoadTexture("data\\textures\\default.tga", DEFAULT_TEX);
	LoadTexture("data\\textures\\blue_lines.tga", BLUE_LINES);
	LoadTexture("data\\textures\\white_lines.tga", WHITE_LINES);
	LoadTexture("data\\textures\\Earth.tga", EARTH_TEX);
	LoadTexture("data\\textures\\Flame.tga", FIRE_TEX);
	LoadTexture("data\\textures\\WaterDrop.tga", WATER_TEX);
	LoadTexture("data\\textures\\Wind.tga", WIND_TEX);
	LoadTexture("data\\textures\\LoadScreen.tga", LOAD_SCREEN);
	LoadTexture("data\\textures\\PlayButton.tga", PLAY_BUTTON);
	LoadTexture("data\\textures\\CreditsButton.tga", CREDITS_BUTTON);
	LoadTexture("data\\textures\\QuitButton.tga", QUIT_BUTTON);
	LoadTexture("data\\textures\\ResumeButton.tga", RESUME_BUTTON);
	LoadTexture("data\\textures\\YesButton.tga", YES_BUTTON);
	LoadTexture("data\\textures\\NoButton.tga", NO_BUTTON);
	LoadTexture("data\\textures\\OnButton.tga", ON_BUTTON);
	LoadTexture("data\\textures\\OffButton.tga", OFF_BUTTON);
	LoadTexture("data\\textures\\InstructionsButton.tga", INSTRUCTIONS_BUTTON);
	LoadTexture("data\\textures\\OptionsButton.tga", OPTIONS_BUTTON);
	LoadTexture("data\\textures\\TitleScreen.tga", TITLE_SCREEN);
	LoadTexture("data\\textures\\DP_Screen.tga", DP_SCREEN);
	LoadTexture("data\\textures\\Credits.tga", CREDITS_SCREEN);
	LoadTexture("data\\textures\\ConfirmAction.tga", CONFIRM_SCREEN);
	LoadTexture("data\\textures\\HowToPlay.tga", INSTRUCTIONS_SCREEN);
	LoadTexture("data\\textures\\OptionsScreen.tga", OPTIONS_SCREEN);
	LoadTexture("data\\textures\\FullscreenButton.tga", FULLSCREEN_BUTTON);
	LoadTexture("data\\textures\\MusicButton.tga", MUSIC_BUTTON);
	LoadTexture("data\\textures\\SoundFxButton.tga", SOUNDFX_BUTTON);
	LoadTexture("data\\textures\\WinScreen.tga", WIN_SCREEN);
	LoadTexture("data\\textures\\LoseScreen.tga", LOSE_SCREEN);
}

Point WorldSpaceCursor(void)
{
	POINT CursorPos;
	WINDOWINFO wi;
	GetCursorPos(&CursorPos); //screen space coordinates, now convert back to NDC
	ScreenToClient(GetForegroundWindow(), &CursorPos);
	/*GetWindowInfo(GetActiveWindow(), &wi);
	CursorPos.x -= wi.rcClient.left;
	CursorPos.y -= wi.rcClient.top;*/

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


void ObjectSelection(void)
{
	Point wsc = WorldSpaceCursor();        //find the world space coordinates of the mouse
	Vector cursor_vec = wsc - sCamera.Pos;
	cursor_vec.NormalizeMe();
	Ray cursor_ray(wsc, cursor_vec);  //cast a ray from the camera through the mouse to select mouseover targets

	bool selection = false;
	float current_t = 100000.0f;
	for(unsigned i = 0; i < NpcObj.size(); i++) //find which object the mouse is hovered over
	{
		Intersection inter;
		int type = NpcObj[i]->GetModelType();
		if(type == PLANE)
		{
			AABB box(NpcObj[i]->GetPos(), NpcObj[i]->GetScale() / 2.0f); //want a better way for planes, but this works for now
			if(cursor_ray.Intersect(box, inter))
			{
				if(inter.T < current_t)
				{
					selected_obj = NpcObj[i];
					selection = true;
					current_t = inter.T;
				}
			}
		}
		else if(type == CUBE)
		{
			AABB box(NpcObj[i]->GetPos(), NpcObj[i]->GetScale() / 2.0f);
			if(cursor_ray.Intersect(box, inter))
			{
				if(inter.T < current_t)
				{
					selected_obj = NpcObj[i];
					selection = true;
					current_t = inter.T;
				}
			}
		}
		else if(type == SPHERE)
		{
			Vector scalevec = NpcObj[i]->GetScale();
			float diam = Largest(scalevec.x, scalevec.y, scalevec.z);
			Sphere s(NpcObj[i]->GetPos(), diam / 2.0f);
			if(cursor_ray.Intersect(s, inter))
			{
				if(inter.T < current_t)
				{
					selected_obj = NpcObj[i];
					selection = true;
					current_t = inter.T;
				}
			}
		}
	}

	if(!selection)
		selected_obj = NULL;
}


Point ScreenToWorld(const Point &p)
{
	POINT CursorPos;
	CursorPos.x = p.x, CursorPos.y = p.y;
	//ScreenToClient(GetForegroundWindow(), &CursorPos);
	//dont want to do ^ this for HUD objects, makes it with respect to the entire screen

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

void ToggleFullScreen(void)
{
	//RECT Desktop;
	//HWND DesktopWindow = GetDesktopWindow();
	//GetWindowRect(DesktopWindow, &Desktop);

	if(Full_Screen)//(wind_width == Desktop.right && wind_height == Desktop.bottom)
	{
		glutReshapeWindow(800, 600);
		glutPositionWindow(10, 30);
		reshape(800, 600);
		Full_Screen = false;
	}
	else
	{
		glutFullScreen();
		Full_Screen = true;
	}
}
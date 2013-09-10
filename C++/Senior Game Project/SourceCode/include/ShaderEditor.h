/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: shadereditor.h
Purpose: definition of the shader editor functions
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 5/22/2012
End Header --------------------------------------------------------*/

#ifndef __SHADER_EDITOR__
#define __SHADER_EDITOR__

#include "Utils.h"
#include <vector>

#include "ObjReader.h"
#include "tgaimage.h"
#include "Geometry.h"
#include "GameObj.h"

#include "GameStateManager.h"

// Global declarations
#define SE_WINDOW_WIDTH  800
#define SE_WINDOW_HEIGHT 600
#define SE_WINDOW_POS_X	 0
#define SE_WINDOW_POS_Y	 0
//clipping plane values
#define NEAR_VAL 1
#define FAR_VAL 10000
#define FOV_Y 45

// function declarations 
void display( void );
void idle( void );
void keyboard( unsigned char, int, int );
void reshape( int w, int h );

void ObjectSelection(void);

void GetTimeHelper(double *t);

extern Color4f  clearColor;

extern int wireframe_bool, bvh_bool;
extern int tex_num;
extern int b_bool;
extern int rg_num;
extern int wind_width, wind_height;
extern bool Full_Screen;

void DrawSquare(void);
void LoadTexture(std::string file, int index);
void LoadTextures(void);

void DrawCursor(void);

void DrawBar(const Point &pos, const Vector &scale, const Color &col, const float percentage);

Point ScreenToWorld(const Point &p);

void Main_Menu_Load(void);
void Main_Menu_Initialize(void);
void Main_Menu_Update(void);
void Main_Menu_Draw(unsigned index);
void Main_Menu_Unload(void);

void Level_1_Load(void);
void Level_1_Initialize(void);
void Level_1_Update(void);
void Level_1_Draw(unsigned index);
void Level_1_Unload(void);

void Pause_Load(void);
void Pause_Initialize(void);
void Pause_Update(void);
void Pause_Draw(unsigned index);
void Pause_Unload(void);

void Splash_Load(void);
void Splash_Initialize(void);
void Splash_Update(void);
void Splash_Draw(unsigned index);
void Splash_Unload(void);

void Credits_Load(void);
void Credits_Initialize(void);
void Credits_Update(void);
void Credits_Draw(unsigned index);
void Credits_Unload(void);

void Confirm_Load(void);
void Confirm_Initialize(void);
void Confirm_Update(void);
void Confirm_Draw(unsigned index);
void Confirm_Unload(void);

void Instructions_Load(void);
void Instructions_Initialize(void);
void Instructions_Update(void);
void Instructions_Draw(unsigned index);
void Instructions_Unload(void);

void Options_Load(void);
void Options_Initialize(void);
void Options_Update(void);
void Options_Draw(unsigned index);
void Options_Unload(void);

void Win_Load(void);
void Win_Initialize(void);
void Win_Update(void);
void Win_Draw(unsigned index);
void Win_Unload(void);

void Lose_Load(void);
void Lose_Initialize(void);
void Lose_Update(void);
void Lose_Draw(unsigned index);
void Lose_Unload(void);

void ToggleFullScreen(void);

extern std::map<int, unsigned int> TexList, NormMapList, HeightMapList;
extern GameStateManager GSM;

#endif
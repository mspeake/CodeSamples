/*--------------------------------------------------
Defines the core functions and important variables
--------------------------------------------------*/
#ifndef CORE_H
#define CORE_H

#include "Utils.h"
#include <vector>

#include "ObjReader.h"
#include "tgaimage.h"
#include "Geometry.h"
#include "GameObj.h"

// Global declarations
#define SP_WINDOW_WIDTH  800
#define SP_WINDOW_HEIGHT 600
#define SP_WINDOW_POS_X	 0
#define SP_WINDOW_POS_Y	 0
//clipping plane values
#define NEAR_VAL 1
#define FAR_VAL 10000
#define FOV_Y 45

// function declarations 
void display(void);
void idle(void);
void keyboard(unsigned char, int, int);
void reshape(int w, int h);

void ObjectSelection(void);

void GetTimeHelper(double *t);

extern Color clearColor;

extern int wireframe_bool;
extern int tex_num;
extern int rg_num, shape_num, use_nm;
extern int wind_width, wind_height;

void BindTexture(unsigned TexNum, unsigned Texture);
void BindShaderTexture(unsigned TexNum, unsigned ShaderProgram, char *Name, unsigned Texture);

void FrameBufferGenTexture(unsigned &Pic);
void GenDepthBuffer(unsigned &Pic);
void GenFrameBuffer(unsigned &Pic, unsigned &FBuffer, unsigned &DBuffer);
void CreateFrameBuffer(void);
void RenderToFrameBuffer(Point Pos, Vector Dir, Vector Up, unsigned Pic, unsigned &FBuffer);
void GenerateEnvMap(void);
void DrawSquare(unsigned Pic, Vector Pos, float Width, float Height);
void DrawEnvMap(void);

void DrawEnv(void);

void LoadTexture(std::string file, int index);
void LoadTextures(void);

extern std::map<int, unsigned int> TexList, NormMapList, HeightMapList;

extern unsigned LeftPic, RightPic, TopPic, BottomPic, FrontPic, BackPic;
extern unsigned NormalMapID;

#endif
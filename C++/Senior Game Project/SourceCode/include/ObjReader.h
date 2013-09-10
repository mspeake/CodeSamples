/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: ObjReader.h
Purpose: definition of my Shape and Object classes
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 5/22/2012
End Header --------------------------------------------------------*/
#ifndef OBJREADER_H
#define OBJREADER_H

#include "Utils.h"
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <string>
#include <math.h>
#include "Geometry.h"

#include "GameObj.h"

class Face
{
public:
	int p1, p2, p3;
	Vector nrm, T, B;
	int t1, t2, t3;
};

enum ShapeType
{
	PLANE,
	CUBE,
	SPHERE
};

enum ObjectType
{
	ENVIRONMENT_OBJ,
	PLAYER_OBJ,
	ENEMY_OBJ,
	HUD_OBJ
};

void NormalizeMe(Vector3f &vec);
void CrossProd(const Vector3f L, const Vector3f R, Vector3f &dest);
void FBCB(int num);
void KDCB(int num);
void CalculateTB(Face *t, const float *TCs, const float *Verts);

void DrawShape(Shape *obj);

bool LoadScene(std::string filename);

extern Shape *test_shape;
extern std::vector<Object *> object_list;
extern int Tree_Depth;
extern LevelHelper LL;
extern GameObj* selected_obj;

#endif
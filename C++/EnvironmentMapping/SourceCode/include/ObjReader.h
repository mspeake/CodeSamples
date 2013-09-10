/*--------------------------------------------------
Defines the functions needed for loading scenes
--------------------------------------------------*/
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

void CalculateTB(Face *t, const float *TCs, const float *Verts);

void DrawShape(Shape *obj);

bool LoadScene(std::string filename);

extern std::vector<Object *> object_list;
extern LevelHelper* LL;
extern std::string FileName;

#endif
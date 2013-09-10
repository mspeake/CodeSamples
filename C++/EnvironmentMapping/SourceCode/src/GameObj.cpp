/*--------------------------------------------------
Implements the game objects class and related functions
--------------------------------------------------*/
#include "GameObj.h"
#include "ObjReader.h"

std::vector<GameObj *> EnvObj;

//-----GameObj class functions-----
GameObj::GameObj(const int _type, const int _sub, const float sx, const float sy, const float sz, const int tex)
{
	Dir = Vector(0.0f, 0.0f, -1.0f);
	Graph = new GraphicsComp(_type, _sub, Vector(sx, sy, sz), tex);
}

GameObj::~GameObj(void)
{
	delete Graph;
}

Point GameObj::GetPos(void) const
{
	return Pos;
}

Vector GameObj::GetDir(void) const
{
	return Dir;
}

void GameObj::SetPos(const Point p)
{
	 Pos = p;
}

void GameObj::SetDir(const Vector d)
{
	Dir = d;
}

float GameObj::GetAngle(void)
{
	return Graph->angle;
}

void GameObj::SetAngle(const float a)
{
	if(a > 360.0f)
		Graph->angle = a - 360.0f;

	else if(a < 0.0f)
		Graph->angle = a + 360.0f;

	else
		Graph->angle = a;
}

Vector GameObj::GetScale(void) const
{
	return Graph->scale;
}

int GameObj::GetModelType(void)
{
	return Graph->GetType();
}

Color GameObj::GetColor(void) const
{
	return Graph->color;
}

void GameObj::SetColor(Color c)
{
	Graph->color = c;
}

int GameObj::GetElementalType(void) const
{
	return ElementalType;
}

void GameObj::SetElementalType(const int type)
{
	ElementalType = type;
}
//-----GameObj class functions-----

//-----GraphicsComp class functions-----
GraphicsComp::GraphicsComp(void)
{
	model = NULL; 
	scale = Vector(1.0f, 1.0f, 1.0f);
	axis = Vector(0.0f, 1.0f, 0.0f);
	angle = 0.0f;
	texture = 0;
}
GraphicsComp::GraphicsComp(const int _type, const int _sub, const Vector &sc, const int tex)
{
	model = new Shape(_type, _sub);

	scale = sc;
	axis = Vector(0.0f, 1.0f, 0.0f);
	angle = 0.0f;
	texture = tex;

	Matrix4x4 trans, rot, scalemat, tr, transform;
	trans.Identity();//Translate(0.0f, 0.0f, 0.0f);
	rot.Identity();//Rotate(axis, angle);
	scalemat.Scale(sc.x, sc.y, sc.z);
	tr = trans * rot;
	transform = scalemat;//tr * scalemat;
	for(unsigned i = 0; i < model->vert_count; i++)
	{
		Point p0(model->VA[i*3 + 0], model->VA[i*3 + 1], model->VA[i*3 + 2]);
		Vector n0(model->VN[i*3 + 0], model->VN[i*3 + 1], model->VN[i*3 + 2]);
		Point p1 = transform * p0;
		Vector n1 = rot * n0;
		model->VA[i*3 + 0] = p1.x;
		model->VA[i*3 + 1] = p1.y;
		model->VA[i*3 + 2] = p1.z;
		model->VN[i*3 + 0] = n1.x;
		model->VN[i*3 + 1] = n1.y;
		model->VN[i*3 + 2] = n1.z;
	}
}

GraphicsComp::~GraphicsComp(void)
{
	delete model;
}

int GraphicsComp::GetType(void)
{
	return model->type;
}
//-----GraphicsComp class functions-----

//-----EnvironmentObj class functions-----
EnvironmentObj::EnvironmentObj(const int _type, const int _sub, const float sx, const float sy, const float sz, const int tex)
{
	Dir = Vector(0.0f, 0.0f, -1.0f);
	Graph = new GraphicsComp(_type, _sub, Vector(sx, sy, sz), tex);
}
void EnvironmentObj::Update(void)
{
	//
}
//-----EnvironmentObj class functions-----

//-----Misc functions-----
int LookupTexture(std::string &str)
{
	if(!str.compare("TOP_TEX"))
		return TOP_TEX;

	if(!str.compare("BOTTOM_TEX"))
		return BOTTOM_TEX;

	if(!str.compare("LEFT_TEX"))
		return LEFT_TEX;

	if(!str.compare("WHITE_LINES"))
		return WHITE_LINES;

	if(!str.compare("RIGHT_TEX"))
		return RIGHT_TEX;

	if(!str.compare("FRONT_TEX"))
		return FRONT_TEX;

	if(!str.compare("BACK_TEX"))
		return BACK_TEX;

	return WHITE_LINES;
}

std::string LookupTexture(int ind)
{
	switch(ind)
	{
	case TOP_TEX:
		return "TOP_TEX";

	case BOTTOM_TEX:
		return "BOTTOM_TEX";

	case LEFT_TEX:
		return "LEFT_TEX";

	case WHITE_LINES:
		return "WHITE_LINES";

	case RIGHT_TEX:
		return "RIGHT_TEX";

	case FRONT_TEX:
		return "FRONT_TEX";

	case BACK_TEX:
		return "BACK_TEX";

	default:
		return "WHITE_LINES";
	}
}
//-----Misc functions-----

/*--------------------------------------------------
Defines the game objects class and related variables
--------------------------------------------------*/

#ifndef GAMEOBJ_H
#define GAMEOBJ_H

#include "Geometry.h"
#include <map>

class GameObj;

class GraphicsComp
{
private:
	Shape* model;
	Vector scale;
	Vector axis;
	float angle;
	Color color;
	int texture;
	int GetType();

public:
	GraphicsComp();
	GraphicsComp(const int _type, const int _sub, const Vector &sc, const int tex);
	~GraphicsComp();

	friend class GameObj;
	friend class LevelHelper;
};

class GameObj
{
protected:
	Point Pos;
	Vector Dir;

	GraphicsComp* Graph;
	int ElementalType;

public:
	GameObj(const int _type = 1, const int _sub = 5, 
		    const float sx = 1.0f, const float sy = 1.0f, const float sz = 1.0f, 
			const int tex = 0);

	~GameObj();

	Point GetPos() const;
	void SetPos(const Point p);
	Vector GetDir() const;
	void SetDir(const Vector d);
	float GetAngle();
	void SetAngle(const float a);
	Vector GetScale() const;
	int GetModelType();

	Color GetColor(void) const;
	void SetColor(Color c);
	int GetElementalType(void) const;
	void SetElementalType(const int type);

	void Draw();

	virtual void Update() {};

	friend class LevelHelper;
};

class EnvironmentObj: public GameObj
{
private:
public:
	EnvironmentObj(const int _type = 1, const int _sub = 5, 
		           const float sx = 1.0f, const float sy = 1.0f, const float sz = 1.0f, 
				   const int tex = 0);

	~EnvironmentObj() {};

	void Update();
};


int LookupTexture(std::string &str);
std::string LookupTexture(int ind);


enum OBJ_TYPE
{
	NO_TYPE,
	ENVIRONMENT_TYPE
};

class LevelHelper
{
public:
	LevelHelper();
	~LevelHelper() {delete obj;};

	bool LoadStuff(std::string filename);

	GameObj *obj;
};

enum TextureList
{
	WHITE_LINES = 0,
	TOP_TEX,
	BOTTOM_TEX,
	LEFT_TEX,
	RIGHT_TEX,
	FRONT_TEX,
	BACK_TEX
};

extern std::vector<GameObj *> EnvObj;

#endif
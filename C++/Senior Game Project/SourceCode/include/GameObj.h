/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: GameObj.h
Purpose: declaration of the GameObj base class and it's derived classes
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 9/22/2012
End Header --------------------------------------------------------*/
#ifndef GAMEOBJ_H
#define GAMEOBJ_H

#include "Geometry.h"
#include "Utils.h"
#include <map>

#define PATROL_SPEED 15

#define EARTH_R 127.0 / 127.0
#define EARTH_G 85.0 / 127.0
#define EARTH_B 0.0

#define FIRE_R 0.9
#define FIRE_G 0.1
#define FIRE_B 0.0

#define WIND_R 200.0 / 255.0
#define WIND_G 255.0 / 255.0
#define WIND_B 230.0 / 255.0

#define WATER_R 0.0
#define WATER_G 0.1
#define WATER_B 0.9

class LevelHelper
{
public:
	LevelHelper() {};
	~LevelHelper() {};

	bool LoadStuff(std::string filename);
	bool LoadPaths(std::string filename);
	bool LoadEnemyData(std::string filename);
};

class GameObj;
class PlayerObj;

class PhysicsComp
{
private:
	Point* Pos;
	Vector* Dir;
	
public:
	PhysicsComp(Point* _p = NULL, Vector* _d = NULL);
	~PhysicsComp() {};

	void TestingStuff();
	friend class GameObj;
};

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
	friend class PlayerObj;
	friend class EnemyObj;
	friend class LevelHelper;
	friend class MPR;
	friend class Orb;
	friend class Camera;
};

class GameObj
{
protected:
	Point Pos;
	Vector Dir;

	PhysicsComp* Phy;
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
	float GetAngle() const;
	void SetAngle(const float a);
	Vector GetAxis() const;
	Vector GetScale() const;
	int GetModelType() const;
	unsigned GetVertCount(void) const;
	Point GetVertex(const unsigned num) const;
	Color GetColor(void) const;
	void SetColor(Color c);
	int GetElementalType(void) const;
	void SetElementalType(const int type);

	bool LineOfSightToPlayer(void);

	void Draw();

	void DrawHud(void);

	virtual void Update() {};

	int cull;

	friend class LevelHelper;
	friend class Camera;
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

class EnemyObj: public GameObj
{
private:
	int CurrentState;
	int EnemyType;
	float T, Duration;
	Point Origin, Destination;
	std::vector<Point> PatrolPath;
	float Health;

public:
	EnemyObj(const int _type = 1, const int _sub = 5, 
		           const float sx = 1.0f, const float sy = 1.0f, const float sz = 1.0f, 
				   const int tex = 0);

	~EnemyObj() {};

	void Update();
	void InitializePatrol(void);
	void AddPatrolPoint(const Point &p);
	void PatrolToWait(void);
	void WaitToPatrol(void);
	void ChasePlayer(void);
	void ReturnToPatrol(void);
	void AttackPlayer(void);
	float GetHealth(void) const;
	void SetHealth(float h);
	void ReceiveDamage(int type, float amount = 25.0f);
	int GetEnemyType(void) const;
	void SetEnemyType(const int t);
	int GetCurrentState(void) const;
	void CollideWithEnvironment(void);

	void DrawLifeBar(void) const;

	friend class LevelHelper;
};

class Orb: public GameObj
{
private:
	float OrbAngle, SecondaryAngle;

	int PrevState, CurrentState, NextState;
	GameObj *TargetObj;

	float T;

public:
	Orb(const int tex = 0);
	~Orb() {};

	Point Destination, CenterOfOrbit;
	std::vector<Point> Path;

	float GetOrbAngle(void) const;
	void SetOrbAngle(float a);

	float Get2ndAngle(void) const;
	void Set2ndAngle(float a);

	int GetOrbType(void) const;
	void SetOrbType(const int t);

	void DrawThisOrb(void) const;

	void ShootOrb(void);
	void ReturnToPlayer(void);
	void WaitFor(float duration = 0.0f);
	void Update();

	void IncreasePower(float val);

	float Power;

	friend class PlayerObj;
};

class PlayerObj: public GameObj
{
private:
	std::vector<Orb *> Orbs;
	int CurrentState;
	float T, StateDuration;
	float Health;

public:
	PlayerObj(const int _type = 1, const int _sub = 5, 
		      const float sx = 1.0f, const float sy = 1.0f, const float sz = 1.0f, 
			  const int tex = 0);

	~PlayerObj() {};

	void AddOrb(int type = 0);
	void DrawOrbs(bool with_shaders = false);
	void Update();
	void ShootIdleOrb(void);
	void ShootOrb(int num);
	void ShootChargingOrbs(void);
	void OrbStateChange(int num);
	void ChargeOrb(int num);
	unsigned GetChargeCount(void) const;
	Point GetOrbPos(int num) const;
	Color GetOrbColor(int num) const;

	void CollideWithEnvironment(void);
	void CollideWithEnemies(void);

	int GetCurrentState(void);
	void ReceiveDamage(float amount);
	void IncreaseHealth(float amount);

	void NormalToInvulnerable(void);
	void InvulnerableToNormal(void);

	void DrawLifeBar(void) const;

	float GetHealth(void);
};

enum BUTTON_TYPE
{
	BT_PLAY,
	BT_QUIT,
	BT_RESUME,
	BT_BACK_TO_MENU,
	BT_NON_INTERACTIVE,
	BT_CREDITS,
	BT_CONFIRM,
	BT_CANCEL,
	BT_HOW_TO_PLAY,
	BT_OPTIONS,
	BT_BOOL
};

class Button
{
public:
	Button(Point p = Point(), Vector s = Vector(1.0f, 1.0f, 0.1f), Color c = Color(), unsigned tex = 0, unsigned type = BT_PLAY);
	Point Pos;
	Vector Scale;
	Color color;
	unsigned Tex;
	unsigned ButtonType;

	void Draw(void);
	void InteractWith(void);
};

enum ELEMENTAL_TYPE
{
	ET_EARTH,		//beats wind
	ET_FIRE,		//beats earth
	ET_WIND,		//beats water
	ET_WATER,		//beats fire
	ET_NEUTRAL
};

enum BEHAVIOR_STATE
{
	IDLE,
	CHARGING,
	TRAVELLING,
	SHOOTING,
	RETURNING,
	WAITING,
	PATROLLING,
	DYING,
	INVULNERABLE,
	NORMAL_STATE,
	CHASING,
	ATTACKING
};

enum ENEMY_TYPE
{
	PATROL_ENEMY,
	OBSTACLE_ENEMY
};

int LookupTexture(std::string &str);

enum TextureList
{
	DEFAULT_TEX = 0,
	METAL_ROOF,
	FASE,
	BLUE_LINES,
	WHITE_LINES,
	LOAD_SCREEN,
	EARTH_TEX,
	FIRE_TEX,
	WATER_TEX,
	WIND_TEX,
	PLAY_BUTTON,
	CREDITS_BUTTON,
	QUIT_BUTTON,
	YES_BUTTON,
	NO_BUTTON,
	RESUME_BUTTON,
	INSTRUCTIONS_BUTTON,
	OPTIONS_BUTTON,
	ON_BUTTON,
	OFF_BUTTON,
	TITLE_SCREEN,
	DP_SCREEN,
	CREDITS_SCREEN,
	CONFIRM_SCREEN,
	INSTRUCTIONS_SCREEN,
	OPTIONS_SCREEN,
	FULLSCREEN_BUTTON,
	MUSIC_BUTTON,
	SOUNDFX_BUTTON,
	WIN_SCREEN,
	LOSE_SCREEN
};

extern std::vector<GameObj *> EnvObj, NpcObj;
extern std::vector<Button*> HudObj;

extern PlayerObj *Player;
extern double frame_time;

#endif
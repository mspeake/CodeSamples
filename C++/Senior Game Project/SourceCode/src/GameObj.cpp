/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: GameObj.cpp
Purpose: implementation of the GameObj base class and it's derived classes
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 9/22/2012
End Header --------------------------------------------------------*/

#include "GameObj.h"
#include "ObjReader.h"
#include "GameStateManager.h"
#include "Sound.h"

extern GameStateManager GSM;

PlayerObj *Player = NULL;

std::vector<GameObj *> EnvObj, NpcObj;
std::vector<Button*> HudObj;

//-----GameObj class functions-----
GameObj::GameObj(const int _type, const int _sub, const float sx, const float sy, const float sz, const int tex)
{
	Dir = Vector(0.0f, 0.0f, -1.0f);

	Phy = new PhysicsComp(&Pos, &Dir);
	Graph = new GraphicsComp(_type, _sub, Vector(sx, sy, sz), tex);
	cull = 0;
	ElementalType = ET_NEUTRAL;
}

GameObj::~GameObj(void)
{
	delete Phy;
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

float GameObj::GetAngle(void) const
{
	return Graph->angle; //DEGREES
}

Vector GameObj::GetAxis(void) const
{
	return Graph->axis;
}

void GameObj::SetAngle(const float a)
{
	if(a > 360.0f) //DEGREES(obviously)
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

int GameObj::GetModelType(void) const
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


unsigned GameObj::GetVertCount(void) const
{
	return unsigned(Graph->model->vert_count);
}

Point GameObj::GetVertex(const unsigned num) const
{
	return Point(Graph->model->VA[num*3 + 0], Graph->model->VA[num*3 + 1], Graph->model->VA[num*3 + 2]);
}

int GameObj::GetElementalType(void) const
{
	return ElementalType;
}

void GameObj::SetElementalType(const int type)
{
	ElementalType = type;
}

bool GameObj::LineOfSightToPlayer(void)
{
	Point NpcPos = Pos;
	Point PlayerPos = Player->GetPos();
	Vector RayVec = NpcPos - PlayerPos;
	//float Dist = (NpcPos - PlayerPos).Magnitude();

	for(unsigned i = 0; i < EnvObj.size(); i++)
	{
		//if(EnvObj[i]->cull)
		//	continue;

		if(EnvObj[i]->GetModelType() == PLANE)
			continue;

		//RayVec.NormalizeMe();
		Ray R(PlayerPos, RayVec);

		Intersection NpcInter, EnvInter;
		Vector NpcScale = EnvObj[i]->GetScale();
		NpcScale /= 2.0f;

		AABB EnvBox(EnvObj[i]->GetPos(), NpcScale);
		Sphere EnemySphere(NpcPos, 1.0f); //dont need to do accurate intersection on enemy

		if(R.Intersect(EnvBox, EnvInter))
		{
			if(R.Intersect(EnemySphere, NpcInter)) 
			{
				if(NpcInter.T > EnvInter.T) //not in line of sight, revert to previous target
				{
					return false;
				}
			}
		}
	}
	return true;
}
//-----GameObj class functions-----

//-----PhysicsComp class functions-----
PhysicsComp::PhysicsComp(Point* _p, Vector* _d)
{
	Pos = _p;
	Dir = _d;
}

void PhysicsComp::TestingStuff(void)
{
	//
}
//-----PhysicsComp class functions-----

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

	//Matrix4x4 trans, rot, scalemat, tr, transform;
	//trans.Identity();//Translate(0.0f, 0.0f, 0.0f);
	//rot.Identity();//Rotate(axis, angle);
	//scalemat.Identity();//scalemat.Scale(sc.x, sc.y, sc.z);
	//tr = trans * rot;
	//transform = scalemat;//tr * scalemat;
	//for(unsigned i = 0; i < model->vert_count; i++)
	//{
	//	Point p0(model->VA[i*3 + 0], model->VA[i*3 + 1], model->VA[i*3 + 2]);
	//	Vector n0(model->VN[i*3 + 0], model->VN[i*3 + 1], model->VN[i*3 + 2]);
	//	Point p1 = transform * p0;
	//	Vector n1 = rot * n0;
	//	model->VA[i*3 + 0] = p1.x;
	//	model->VA[i*3 + 1] = p1.y;
	//	model->VA[i*3 + 2] = p1.z;
	//	model->VN[i*3 + 0] = n1.x;
	//	model->VN[i*3 + 1] = n1.y;
	//	model->VN[i*3 + 2] = n1.z;
	//}
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

	Phy = new PhysicsComp(&Pos, &Dir);
	Graph = new GraphicsComp(_type, _sub, Vector(sx, sy, sz), tex);
	ElementalType = ET_NEUTRAL;
}
void EnvironmentObj::Update(void)
{
	//
}
//-----EnvironmentObj class functions-----

//-----EnemyObj class functions-----
EnemyObj::EnemyObj(const int _type, const int _sub, const float sx, const float sy, const float sz, const int tex)
{
	Dir = Vector(0.0f, 0.0f, -1.0f);

	Phy = new PhysicsComp(&Pos, &Dir);
	Graph = new GraphicsComp(_type, _sub, Vector(sx, sy, sz), tex);

	CurrentState = WAITING;
	T = 0.0f;
	Duration = 1.0f;
	ElementalType = ET_NEUTRAL;
	EnemyType = PATROL_ENEMY;
	Health = 100.0f;
}

void EnemyObj::InitializePatrol(void)
{
	if(PatrolPath.empty())
		Destination = Pos;
	else
		Destination = PatrolPath[0];
}

void EnemyObj::AddPatrolPoint(const Point &p)
{
	PatrolPath.push_back(p);
}

void EnemyObj::Update(void)
{
	if(GetModelType() == SPHERE)
	{
		float alpha = 0.0f; //find the angle about the Y-axis to update the rotation
		float X = Dir.x, Z = -(Dir.z);
		if(X != 0.0f)
		{
			if(X > 0.0f)
				alpha = atan(Z / X);
			else
			{
				if(Z >= 0.0f)
					alpha = atan(Z / X) + PI;
				else
					alpha = atan(Z / X) - PI;
			}
		}
		else
		{
			if(Z > 0.0f)
				alpha = PI / 2.0f;
			else if(Z < 0.0f)
				alpha = PI / -2.0f;
			else
				alpha = 0.0f;
		}

		SetAngle(RadToDeg(alpha) - 180.0f); //need to offset this angle by 180 deg to make the "front" of the sphere match up
	}

	if(GetEnemyType() == PATROL_ENEMY && CurrentState != CHASING)
	{
		float DistToPlayer = (Player->GetPos() - Pos).Magnitude();
		if(DistToPlayer < 125.0f)
		{
			if(LineOfSightToPlayer())
				ChasePlayer();
		}
	}

	if(Health == 0.0f)
		CurrentState = DYING;

	switch(CurrentState)
	{
	case WAITING:
		{
			T += (frame_time / Duration);

			if(T >= 1.0f)
				WaitToPatrol();

			break;
		}
	case PATROLLING:
		{
			T += (frame_time / Duration);

			if(Pos == Destination)
				T = 1.0f;

			if(T >= 1.0f)
			{
				Pos = Destination;
				PatrolToWait();
			}

			else
			{
				Vector path_dir = (Destination - Origin);
				float dist = path_dir.Magnitude();
				path_dir /= dist;
				Pos = Origin + (path_dir * (dist * T));
				Dir = path_dir;
			}

			break;
		}
	case CHASING:
		{
			Destination = Player->GetPos();
			Vector path_dir = (Destination - Pos);
			float dist = path_dir.Magnitude();
			if(dist > 125.0f)
			{
				ReturnToPatrol();
				break;
			}
			path_dir /= dist;
			Pos = Pos + (path_dir * PATROL_SPEED * 3.0f * frame_time);
			Dir = path_dir;
			break;
		}
	case ATTACKING:
		{
			Destination = Player->GetPos();
			Vector path_dir = (Destination - Pos);
			float dist = path_dir.Magnitude();
			path_dir /= dist;
			Pos = Pos + (path_dir * PATROL_SPEED * 3.0f * frame_time);
			Dir = path_dir;
			break;
		}
	case DYING:
		{
			break;
		}
	}

	CollideWithEnvironment();
}

void EnemyObj::PatrolToWait(void)
{
	CurrentState = WAITING;
	Duration = 1.0f; //wait 1 second before resuming patrol
	T = 0.0f;
}

void EnemyObj::WaitToPatrol(void)
{
	CurrentState = PATROLLING;
	unsigned index = 0;
	for(; index < PatrolPath.size(); index++)
	{
		if(PatrolPath[index] == Destination)
			break;
	}

	Origin = PatrolPath[index];

	if(index == PatrolPath.size() - 1)
		Destination = PatrolPath[0];
	else
		Destination = PatrolPath[index + 1];

	float dist = (Destination - Origin).Magnitude();
	Duration = dist / PATROL_SPEED;
	T = 0.0f;
}

void EnemyObj::ChasePlayer(void)
{
	CurrentState = CHASING;
	Destination = Player->GetPos();
	Play_Sound(ENEMY_ATTACK);
}

void EnemyObj::ReturnToPatrol(void)
{
	CurrentState = PATROLLING;

	Origin = Pos;

	float NearestDist = (PatrolPath[0] - Pos).Magnitude();
	Destination = PatrolPath[0];

	for(unsigned i = 1; i < PatrolPath.size(); i++)
	{
		float dist = (PatrolPath[i] - Pos).Magnitude();
		if(dist < NearestDist)
		{
			NearestDist = dist;
			Destination = PatrolPath[i];
		}
	}

	float dist = (Destination - Origin).Magnitude();
	Duration = dist / PATROL_SPEED;
	T = 0.0f;
}

void EnemyObj::AttackPlayer(void)
{
	CurrentState = ATTACKING;
	Destination = Player->GetPos();
	Play_Sound(ENEMY_ATTACK);
}

float EnemyObj::GetHealth(void) const
{
	return Health;
}

void EnemyObj::SetHealth(float h)
{
	Health = h;
}

void EnemyObj::ReceiveDamage(int type, float amount)
{
	if(EnemyType == OBSTACLE_ENEMY) //walls can only be destroyed with the element that beats theirs
	{
		switch(ElementalType)
		{
		case ET_EARTH:
			{
				if(type == ET_EARTH)//if(type == ET_FIRE)
					Health = 0.0f;

				break;
			}

		case ET_FIRE:
			{
				if(type == ET_FIRE)//if(type == ET_WATER)
					Health = 0.0f;

				break;
			}

		case ET_WIND:
			{
				if(type == ET_WIND)//if(type == ET_EARTH)
					Health = 0.0f;

				break;
			}

		case ET_WATER:
			{
				if(type == ET_WATER)//if(type == ET_WIND)
					Health = 0.0f;

				break;
			}

		default:
			{
				break;
			}
		}
	}

	else if(EnemyType == PATROL_ENEMY) //double damage now comes from hitting enemies of the same element
	{           
		Play_Sound(HIT_ENEMY);

		switch(ElementalType)
		{
		case ET_EARTH:
			{
				/*if(type == ET_FIRE)
					Health -= (amount * 2.0f);
				else if(type == ET_EARTH || type == ET_WIND)
					Health -= (amount / 2.0f);*/
				if(type == ET_EARTH)
					Health -= (amount * 2.0f);
				else
					Health -= amount;

				break;
			}

		case ET_FIRE:
			{
				/*if(type == ET_WATER)
					Health -= (amount * 2.0f);
				else if(type == ET_FIRE || type == ET_EARTH)
					Health -= (amount / 2.0f);*/
				if(type == ET_FIRE)
					Health -= (amount * 2.0f);
				else
					Health -= amount;

				break;
			}

		case ET_WIND:
			{
				/*if(type == ET_EARTH)
					Health -= (amount * 2.0f);
				else if(type == ET_WIND || type == ET_WATER)
					Health -= (amount / 2.0f);*/
				if(type == ET_WIND)
					Health -= (amount * 2.0f);
				else
					Health -= amount;

				break;
			}

		case ET_WATER:
			{
				/*if(type == ET_WIND)
					Health -= (amount * 2.0f);
				else if(type == ET_WATER || type == ET_FIRE)
					Health -= (amount / 2.0f);*/
				if(type == ET_WATER)
					Health -= (amount * 2.0f);
				else
					Health -= amount;

				break;
			}

		default:
			{
				Health -= amount;
				break;
			}
		}

		if(CurrentState != ATTACKING)
			AttackPlayer();
	}

	if(Health < 0.0f)
		Health = 0.0f;
}

int EnemyObj::GetEnemyType(void) const
{
	return EnemyType;
}

void EnemyObj::SetEnemyType(const int t)
{
	EnemyType = t;
}

int EnemyObj::GetCurrentState(void) const
{
	return CurrentState;
}

void EnemyObj::CollideWithEnvironment(void)
{
	for(unsigned i = 0; i < EnvObj.size(); i++)
	{
		if(CurrentState != ATTACKING && CurrentState != CHASING) //this should fix a problem where enemies were passing through
		{														 //culled walls in pursuit of the player
			if(EnvObj[i]->cull)
				continue;
		}

		float EnemyRad = GetScale().x / 2.0f;
		Sphere EnemySphere(Pos, EnemyRad);
		AABB EnemyBox(GetPos(), GetScale() / 2.0f);

		Vector ObjSc = EnvObj[i]->GetScale();
		Vector ObjExt = Vector(abs(ObjSc.x), abs(ObjSc.y), abs(ObjSc.z)) / 2.0f;
		float ObjRad = ObjExt.Magnitude();
		Point ObjPos = EnvObj[i]->GetPos();
		int ObjModelType = EnvObj[i]->GetModelType();
		int EnemyModelType = Graph->GetType();

		if(ObjModelType == SPHERE)
			ObjRad = Largest(ObjSc.x, ObjSc.y, ObjSc.z) / 2.0f;

		Sphere ObjSphere(ObjPos, ObjRad);

		if(EnemySphere.Intersect(ObjSphere))
		{
			if(EnemyModelType == SPHERE && ObjModelType == SPHERE)
			{
				Point OP = ObjPos;
				OP.Project(Plane(Pos, Vector(0.0f, 1.0f, 0.0f)));
				Vector repel = Pos - OP;
				repel.NormalizeMe();
				Pos = OP + (repel * (EnemyRad + ObjRad));
			}

			else if(EnemyModelType == SPHERE && ObjModelType == CUBE)
			{
				GameObj *pl = reinterpret_cast<GameObj*>(this);
				MPR Contact(pl, EnvObj[i]);

				AABB ObjBox(ObjPos, ObjSc / 2.0f);
				if(EnemyBox.Intersect(ObjBox))
				{
					//find which face is closest
					Point FrontPoint = ObjPos + Vector(0.0f, 0.0f, ObjSc.z / 2.0f),
						  BackPoint  = ObjPos + Vector(0.0f, 0.0f, ObjSc.z / -2.0f),
						  LeftPoint  = ObjPos + Vector(ObjSc.x / -2.0f, 0.0f, 0.0f),
						  RightPoint = ObjPos + Vector(ObjSc.x / 2.0f, 0.0f, 0.0f);
					Plane FrontPlane(FrontPoint, Vector(0.0f, 0.0f, 1.0f)),
						  BackPlane(BackPoint, Vector(0.0f, 0.0f, -1.0f)),
						  LeftPlane(LeftPoint, Vector(-1.0f, 0.0f, 0.0f)),
						  RightPlane(RightPoint, Vector(1.0f, 0.0f, 0.0f));

					Plane Equal(Point(0.0f,0.0f,0.0f), Vector(0.0f,0.0f,0.0f)); //edge case plane
					Plane *ClosestPlane = &FrontPlane;
					Point EnemyPos = Pos;
					EnemyPos.Project(FrontPlane);
					float ClosestDist = Pos.Distance(EnemyPos);

					EnemyPos = Pos;
					EnemyPos.Project(BackPlane);
					float Dist2 = Pos.Distance(EnemyPos);
					if(Dist2 < ClosestDist)
					{
						ClosestPlane = &BackPlane;
						ClosestDist = Dist2;
					}
					else if(Dist2 == ClosestDist) //edge case: on one of the vertical edges of the box
					{
						Equal.N = (Pos - ObjPos);
						Equal.N.NormalizeMe();
						Equal.P = ObjPos + Vector(Equal.N.x * ObjSc.x, 0.0f, Equal.N.z * ObjSc.z);
					}

					EnemyPos = Pos;
					EnemyPos.Project(LeftPlane);
					Dist2 = Pos.Distance(EnemyPos);
					if(Dist2 < ClosestDist)
					{
						ClosestPlane = &LeftPlane;
						ClosestDist = Dist2;
					}
					else if(Dist2 == ClosestDist)
					{
						Equal.N = Pos - ObjPos;
						Equal.N.NormalizeMe();
						Equal.P = ObjPos + Vector(Equal.N.x * ObjSc.x, 0.0f, Equal.N.z * ObjSc.z);
					}

					EnemyPos = Pos;
					EnemyPos.Project(RightPlane);
					Dist2 = Pos.Distance(EnemyPos);
					if(Dist2 < ClosestDist)
					{
						ClosestPlane = &RightPlane;
						ClosestDist = Dist2;
					}
					else if(Dist2 == ClosestDist)
					{
						Equal.N = Pos - ObjPos;
						Equal.N.NormalizeMe();
						Equal.P = ObjPos + Vector(Equal.N.x * ObjSc.x, 0.0f, Equal.N.z * ObjSc.z);
					}

					EnemyPos = Pos;
					EnemyPos.Project(*ClosestPlane);
					Pos = EnemyPos + (ClosestPlane->N * EnemyRad);
				}
			}
		}
	}
}
//-----EnemyObj class functions-----

//-----Orb class functions-----
Orb::Orb(const int tex)
{
	Dir = Vector(0.0f, 0.0f, -1.0f);

	Phy = new PhysicsComp(&Pos, &Dir);
	Graph = new GraphicsComp(2, 10, Vector(5.0f, 5.0f, 5.0f), tex);
	OrbAngle = 0.0f;
	SecondaryAngle = 0.0f;
	T = 0.0f;
	PrevState = CurrentState = NextState = IDLE;
	Destination = Point(0.0f, 0.0f, 0.0f);
	CenterOfOrbit = Point(0.0f, 0.0f, 0.0f);
	TargetObj = NULL;
	Power = 0.0f;
}

float Orb::GetOrbAngle(void) const
{
	return OrbAngle;
}

void Orb::SetOrbAngle(float a)
{
	OrbAngle = a;
}

float Orb::Get2ndAngle(void) const
{
	return SecondaryAngle;
}

void Orb::Set2ndAngle(float a)
{
	SecondaryAngle = a;
}

int Orb::GetOrbType(void) const
{
	return GetElementalType();
}
void Orb::SetOrbType(const int t)
{
	SetElementalType(t);
}

void Orb::Update(void)
{
	float AngleIncrement = 180.0f;
	OrbAngle += AngleIncrement * frame_time;
	SecondaryAngle += AngleIncrement * frame_time;
	float TravelDuration = 0.9f;
	float MinDist = 10.0f, MaxDist = 180.0f, MinDur = 0.1f, MaxDur = 0.9f;

	Point O(0.0f, 0.0f, 0.0f);

	int OrbType = GetElementalType();
	switch(CurrentState)
	{
	case IDLE:
		{
			float radius = Player->GetScale().x + (GetScale().x / 2.0f);
			Vector orbital_axis = Vector(0.0f, 1.0f, 0.0f),
				   up_axis = Player->GetDir(); //in this case, any y=0 vector will do, but may as well use dir
			CenterOfOrbit = Player->GetPos();

			Point o_pos = O + (up_axis * radius);
			Matrix4x4 rot;
			rot.Rotate(orbital_axis, OrbAngle);
			o_pos = rot * o_pos;
			o_pos += CenterOfOrbit;
			SetPos(o_pos);

			break;
		}

	case CHARGING:
		{
			float radius = (GetScale().x / 2.0f) + 1.0f;
			float rot_speed = 1.0f;						  //rotation will "skip" if this number isn't whole, also does weird
			rot_speed += float(Player->GetChargeCount()); //things if it is equal to number of charging orbs, this fixes that

			Vector orbital_axis = Player->GetDir(),
				   up_axis = Vector(0.0f, 1.0f, 0.0f);
			CenterOfOrbit = Player->GetPos() + (orbital_axis * (GetScale().x / 2.0f + Player->GetScale().x / 2.0f));

			Point o_pos = O + (up_axis * radius);
			Matrix4x4 rot;
			rot.Rotate(orbital_axis, SecondaryAngle*rot_speed); 
			o_pos = rot * o_pos;
			o_pos += CenterOfOrbit;
			SetPos(o_pos);

			break;
		}

	case SHOOTING:
		{
			//update the end point in case the target is moving
			Path[Path.size() - 1] = TargetObj->GetPos();

			float TravelDistance = (Pos - Path[Path.size() - 1]).Magnitude();

			if(TravelDistance < MinDist)
				TravelDistance = MinDist;
			if(TravelDistance > MaxDist)
				TravelDistance = MaxDist;

			float duration = MinDur + ((TravelDistance - MinDist) / (MaxDist - MinDist)) * (MaxDur - MinDur);
			T += (frame_time / duration);

			if(T > 1.0f)
				T = 1.0f;

			CenterOfOrbit = CalcBBPoint(T, Path, Path.size());
			Point o_pos = O;
			o_pos += CenterOfOrbit;
			SetPos(o_pos);

			if(T == 1.0f)
			{
				EnemyObj* target_enemy = reinterpret_cast<EnemyObj*>(TargetObj);
				int EnemyElement = target_enemy->GetElementalType();
				float BaseDamage = 25.0f;
				float Modifier = 1.0f + (Power / 100.0f);

				if(ElementalType == ET_WIND || ElementalType == ET_WATER)
				{
					if(EnemyElement == ElementalType)
					{
						if(PrevState == CHARGING)
						{
							Player->IncreaseHealth(BaseDamage * 2.0f * Modifier);
							Power = 0.0f;
						}
						else
						{
							target_enemy->ReceiveDamage(ElementalType, BaseDamage);
							IncreasePower(30.0f);
						}
					}
					else
					{
						if(PrevState == CHARGING)
						{
							Player->IncreaseHealth(BaseDamage * 2.0f * Modifier);
							Power = 0.0f;
						}
						else
						{
							target_enemy->ReceiveDamage(ElementalType, BaseDamage);
							//IncreasePower(30.0f);
						}
					}
				}
				else
				{
					if(EnemyElement == ElementalType)
					{
						if(PrevState == CHARGING)
						{
							target_enemy->ReceiveDamage(ElementalType, BaseDamage * Modifier);
							Power = 0.0f;
						}
						else
						{
							target_enemy->ReceiveDamage(ElementalType, BaseDamage);
							IncreasePower(30.0f);
						}
					}
					else
					{
						if(PrevState == CHARGING)
						{
							target_enemy->ReceiveDamage(EnemyElement, BaseDamage);
							Power = 0.0f;
						}
						else
						{
							target_enemy->ReceiveDamage(ElementalType, BaseDamage);
							//IncreasePower(30.0f);
						}
					}
				}

				WaitFor(0.1f);
			}

			break;
		}

	case RETURNING:
		{
			float radius = Player->GetScale().x + (GetScale().x / 2.0f);
			Vector orbital_axis = Vector(0.0f, 1.0f, 0.0f),
				   up_axis = Player->GetDir(); //in this case, any y=0 vector will do, but may as well use dir
			CenterOfOrbit = Player->GetPos();

			Point o_pos = O + (up_axis * radius);
			Matrix4x4 rot;
			rot.Rotate(orbital_axis, OrbAngle);
			o_pos = rot * o_pos;
			o_pos += CenterOfOrbit;

			Path[Path.size() - 1] = o_pos;

			float TravelDistance = (Pos - CenterOfOrbit).Magnitude();

			if(TravelDistance < MinDist)
				TravelDistance = MinDist;
			if(TravelDistance > MaxDist)
				TravelDistance = MaxDist;

			float duration = MinDur + ((TravelDistance - MinDist) / (MaxDist - MinDist)) * (MaxDur - MinDur);
			T += (frame_time / duration);

			if(T > 1.0f)
				T = 1.0f;

			o_pos = CalcBBPoint(T, Path, Path.size());

			SetPos(o_pos);

			if(T == 1.0f)
				CurrentState = IDLE;

			break;
		}

	case WAITING:
		{
			T -= frame_time;

			if(T < 0.0f)
				T = 0.0f;

			if(T == 0.0f)
				ReturnToPlayer();
			break;
		}
	default:
		{
			CurrentState = IDLE;
			break;
		}
	}
	OrbAngle = ClampTo360(OrbAngle);
	SecondaryAngle = ClampTo360(SecondaryAngle);
}

void Orb::ReturnToPlayer(void)
{
	Path.clear();
	T = 0.0f;

	float radius = Player->GetScale().x + (GetScale().x / 2.0f);
	Vector orbital_axis = Vector(0.0f, 1.0f, 0.0f),
		   up_axis = Player->GetDir(); //in this case, any y=0 vector will do, but may as well use dir
	CenterOfOrbit = Player->GetPos();

	Point o_pos = Point(0.0f, 0.0f, 0.0f) + (up_axis * radius);
	Matrix4x4 rot;
	rot.Rotate(orbital_axis, OrbAngle);
	o_pos = rot * o_pos;
	o_pos += CenterOfOrbit;

	Vector PlayerDir = Player->GetDir();
	PrevState = CurrentState;
	CurrentState = RETURNING;
	Destination = o_pos;
	Vector up_vec = Vector(0.0f, 1.0f, 0.0f);
	Vector right_vec = PlayerDir.Cross(up_vec);
	right_vec.NormalizeMe();

	if(OrbAngle >= 270.0f || OrbAngle <= 90.0f)
		right_vec *= -1.0f; //make it move left instead of right if orb is in front

	float modifier = 10.0f; //further in the direction vectors
	Point p0 = GetPos();
	Path.push_back(p0); //point 0: start
	
	p0 += (right_vec * GetScale().x * modifier);
	Path.push_back(p0); //point 1: to the right

	p0 += (PlayerDir * 1.0f * GetScale().x * modifier);
	Path.push_back(p0); //point 2: to the right & forward

	Vector diag = Path[2] - Path[1];
	diag.NormalizeMe();
	p0 += (diag * GetScale().x * modifier);
	Path.push_back(p0); //point 3: p2 + (p2 - p1)

	p0 = Destination;
	Path.push_back(p0); //end point: target
	Play_Sound(ORB_RETURN);
}

void Orb::WaitFor(float duration)
{
	T = duration;
	PrevState = CurrentState;
	CurrentState = WAITING;
}

void Orb::ShootOrb(void)
{
	Point TargetPos;
	if(CurrentState == CHARGING)
	{
		if(ElementalType == ET_EARTH || ElementalType == ET_FIRE)
		{
			if(selected_obj == NULL)
				return;

			TargetPos = selected_obj->GetPos();
			TargetObj = selected_obj;
		}
		else if(ElementalType == ET_WIND || ElementalType == ET_WATER)
		{
			TargetPos = Player->GetPos();
			TargetObj = Player;
		}
	}
	else//if(CurrentState != CHARGING)
	{
		if(selected_obj == NULL)
			return;

		TargetPos = selected_obj->GetPos();
		TargetObj = selected_obj;
	}

	Vector p_dir = (TargetPos - Pos);//Player->GetDir();
	p_dir.NormalizeMe();
	Path.clear();
	T = 0.0f;

	Destination = Pos + (p_dir * 500.0f);
	Vector up_vec = Vector(0.0f, 1.0f, 0.0f);
	Vector right_vec = p_dir.Cross(up_vec);
	right_vec.NormalizeMe();

	
	PrevState = CurrentState;
	CurrentState = SHOOTING;

	if(OrbAngle >= 270.0f || OrbAngle <= 90.0f)
		right_vec *= -1.0f; //make it move left instead of right if orb is in front

	float modifier = 5.0f; //further in the direction vectors
	Point p0 = Pos;
	Path.push_back(p0); //point 0: start
	
	p0 += (right_vec * Player->GetScale().x * modifier);
	Path.push_back(p0); //point 1: to the right

	p0 += (p_dir * Player->GetScale().x * modifier);
	Path.push_back(p0); //point 2: to the right & forward

	Vector diag = Path[2] - Path[1];
	diag.NormalizeMe();
	p0 += (diag * Player->GetScale().x * modifier);
	Path.push_back(p0); //point 3: p2 + (p2 - p1)

	//p0 = selected_obj->GetPos();
	Path.push_back(TargetPos); //end point: target
	Play_Sound(PLAYER_SHOOT);
}

void Orb::IncreasePower(float val)
{
	Power += val;

	if(Power > 100.0f)
		Power = 100.0f;

	if(Power < 0.0f)
		Power = 0.0f;
}
//-----Orb class functions-----

//-----PlayerObj class functions-----
PlayerObj::PlayerObj(const int _type, const int _sub, const float sx, const float sy, const float sz, const int tex)
{
	Dir = Vector(0.0f, 0.0f, -1.0f);

	Phy = new PhysicsComp(&Pos, &Dir);
	Graph = new GraphicsComp(_type, _sub, Vector(sx, sy, sz), tex);

	CurrentState = NORMAL_STATE;
	T = 0.0f, StateDuration = -1.0f, Health = 100.0f;
}

void PlayerObj::AddOrb(int type)
{
	Orb *n_orb = new Orb();
	switch(type)
	{
	case ET_WATER:
		{
			n_orb->SetColor(Color(WATER_R, WATER_G, WATER_B));
			break;
		}

	case ET_FIRE:
		{
			n_orb->SetColor(Color(FIRE_R, FIRE_G, FIRE_B));
			break;
		}
	case ET_EARTH:
		{
			n_orb->SetColor(Color(EARTH_R, EARTH_G, EARTH_B));
			break;
		}
	case ET_WIND:
		{
			n_orb->SetColor(Color(WIND_R, WIND_G, WIND_B));
			break;
		}
	}
	n_orb->SetOrbType(type);
	Orbs.push_back(n_orb);

	for(unsigned i = 0; i < Orbs.size(); i++)
		Orbs[i]->SetOrbAngle(i * 360.0f / float(Orbs.size()));
}

void PlayerObj::DrawOrbs(bool with_shaders)
{
	if(with_shaders)
	{
		for(unsigned i = 0; i < Orbs.size(); i++)
			Orbs[i]->Draw();
	}
	else
	{
		for(unsigned i = 0; i < Orbs.size(); i++)
			Orbs[i]->DrawThisOrb();
	}
}

void PlayerObj::Update(void)
{
	if(StateDuration != -1.0f)
		T += float(frame_time);

	for(unsigned i = 0; i < Orbs.size(); i++)
	{
		Orbs[i]->Update();
	}

	switch(CurrentState)
	{
	case INVULNERABLE:
		{
			if(T >= StateDuration)
				InvulnerableToNormal();
		}
	default:
		break;
	}

	CollideWithEnvironment();
	CollideWithEnemies();
}

void PlayerObj::OrbStateChange(int num)
{
	if(num >= Orbs.size() || num < 0)
		return;

	Orbs[num]->CurrentState = (Orbs[num]->CurrentState + 1) % 3;

	if(Orbs[num]->CurrentState == TRAVELLING)
		Orbs[num]->Destination = Orbs[num]->Pos + (Dir * 1000.0f);
}

void PlayerObj::ChargeOrb(int num)
{
	if(num >= Orbs.size() || num < 0)
		return;

	Orbs[num]->PrevState = Orbs[num]->CurrentState;

	if(Orbs[num]->CurrentState == CHARGING)
		Orbs[num]->CurrentState = IDLE;
	else if(Orbs[num]->CurrentState == IDLE)
		Orbs[num]->CurrentState = CHARGING;

	unsigned charge_count = GetChargeCount();

	for(unsigned i = 0, charge_num = 0; i < Orbs.size(); i++)
	{
		if(Orbs[i]->CurrentState == CHARGING)
		{
			Orbs[i]->Set2ndAngle(charge_num * 360.0f / float(charge_count));
			charge_num++;
		}
	}
}

void PlayerObj::ShootIdleOrb(void)
{
	for(unsigned i = 0; i < Orbs.size(); i++)
	{
		if(Orbs[i]->CurrentState == IDLE)
		{
			Orbs[i]->ShootOrb();
			break;
		}
	}
}

void PlayerObj::ShootOrb(int num)
{
	if(num >= Orbs.size() || num < 0)
		return;

	if(Orbs[num]->CurrentState == IDLE || Orbs[num]->CurrentState == CHARGING)
		Orbs[num]->ShootOrb();
}

void PlayerObj::ShootChargingOrbs(void)
{
	for(unsigned i = 0; i < Orbs.size(); i++)
	{
		if(Orbs[i]->CurrentState == CHARGING)
			ShootOrb(i);
	}
}

unsigned PlayerObj::GetChargeCount(void) const
{
	unsigned charge_count = 0;
	for(unsigned i = 0; i < Orbs.size(); i++)
	{
		if(Orbs[i]->CurrentState == CHARGING)
			charge_count++;
	}
	return charge_count;
}

Point PlayerObj::GetOrbPos(int num) const
{
	if(num < 0 || num > Orbs.size())
		return Point();

	return Orbs[num]->GetPos();
}

Color PlayerObj::GetOrbColor(int num) const
{
	if(num < 0 || num > Orbs.size())
		return Color();

	return Orbs[num]->GetColor();
}

void PlayerObj::CollideWithEnvironment(void)
{
	for(unsigned i = 0; i < EnvObj.size(); i++)
	{
		if(EnvObj[i]->cull)
			continue;

		float PlayerRad = GetScale().x / 2.0f;
		Sphere PlayerSphere(Pos, PlayerRad);
		AABB PlayerBox(GetPos(), GetScale() / 2.0f);

		Vector ObjSc = EnvObj[i]->GetScale();
		Vector ObjExt = Vector(abs(ObjSc.x), abs(ObjSc.y), abs(ObjSc.z)) / 2.0f;
		float ObjRad = ObjExt.Magnitude();
		Point ObjPos = EnvObj[i]->GetPos();
		int ObjModelType = EnvObj[i]->GetModelType();
		int PlayerModelType = Graph->GetType();

		if(ObjModelType == SPHERE)
			ObjRad = Largest(ObjSc.x, ObjSc.y, ObjSc.z) / 2.0f;

		Sphere ObjSphere(ObjPos, ObjRad);

		if(PlayerSphere.Intersect(ObjSphere))
		{
			if(PlayerModelType == SPHERE && ObjModelType == SPHERE)
			{
				Point OP = ObjPos;
				OP.Project(Plane(Pos, Vector(0.0f, 1.0f, 0.0f)));
				Vector repel = Pos - OP;
				repel.NormalizeMe();
				Pos = OP + (repel * (PlayerRad + ObjRad));
			}

			else if(PlayerModelType == SPHERE && ObjModelType == CUBE)
			{
				GameObj *pl = reinterpret_cast<GameObj*>(this);
				MPR Contact(pl, EnvObj[i]);

				AABB ObjBox(ObjPos, ObjSc / 2.0f);
				if(PlayerBox.Intersect(ObjBox))
				{
					//find which face is closest
					Point FrontPoint = ObjPos + Vector(0.0f, 0.0f, ObjSc.z / 2.0f),
						  BackPoint  = ObjPos + Vector(0.0f, 0.0f, ObjSc.z / -2.0f),
						  LeftPoint  = ObjPos + Vector(ObjSc.x / -2.0f, 0.0f, 0.0f),
						  RightPoint = ObjPos + Vector(ObjSc.x / 2.0f, 0.0f, 0.0f);
					Plane FrontPlane(FrontPoint, Vector(0.0f, 0.0f, 1.0f)),
						  BackPlane(BackPoint, Vector(0.0f, 0.0f, -1.0f)),
						  LeftPlane(LeftPoint, Vector(-1.0f, 0.0f, 0.0f)),
						  RightPlane(RightPoint, Vector(1.0f, 0.0f, 0.0f));

					Plane Equal(Point(0.0f,0.0f,0.0f), Vector(0.0f,0.0f,0.0f)); //edge case plane
					Plane *ClosestPlane = &FrontPlane;
					Point PlayerPos = Pos;
					PlayerPos.Project(FrontPlane);
					float ClosestDist = Pos.Distance(PlayerPos);

					PlayerPos = Pos;
					PlayerPos.Project(BackPlane);
					float Dist2 = Pos.Distance(PlayerPos);
					if(Dist2 < ClosestDist)
					{
						ClosestPlane = &BackPlane;
						ClosestDist = Dist2;
					}
					else if(Dist2 == ClosestDist) //edge case: on one of the vertical edges of the box
					{
						Equal.N = (Pos - ObjPos);
						Equal.N.NormalizeMe();
						Equal.P = ObjPos + Vector(Equal.N.x * ObjSc.x, 0.0f, Equal.N.z * ObjSc.z);
					}

					PlayerPos = Pos;
					PlayerPos.Project(LeftPlane);
					Dist2 = Pos.Distance(PlayerPos);
					if(Dist2 < ClosestDist)
					{
						ClosestPlane = &LeftPlane;
						ClosestDist = Dist2;
					}
					else if(Dist2 == ClosestDist)
					{
						Equal.N = Pos - ObjPos;
						Equal.N.NormalizeMe();
						Equal.P = ObjPos + Vector(Equal.N.x * ObjSc.x, 0.0f, Equal.N.z * ObjSc.z);
					}

					PlayerPos = Pos;
					PlayerPos.Project(RightPlane);
					Dist2 = Pos.Distance(PlayerPos);
					if(Dist2 < ClosestDist)
					{
						ClosestPlane = &RightPlane;
						ClosestDist = Dist2;
					}
					else if(Dist2 == ClosestDist)
					{
						Equal.N = Pos - ObjPos;
						Equal.N.NormalizeMe();
						Equal.P = ObjPos + Vector(Equal.N.x * ObjSc.x, 0.0f, Equal.N.z * ObjSc.z);
					}

					PlayerPos = Pos;
					PlayerPos.Project(*ClosestPlane);
					Pos = PlayerPos + (ClosestPlane->N * PlayerRad);
				}
			}
		}
	}
}

void PlayerObj::CollideWithEnemies(void)
{
	for(unsigned i = 0; i < NpcObj.size(); i++)
	{
		if(NpcObj[i]->cull)
			continue;

		float PlayerRad = GetScale().x / 2.0f;
		Sphere PlayerSphere(Pos, PlayerRad);
		AABB PlayerBox(GetPos(), GetScale() / 2.0f);

		Vector ObjSc = NpcObj[i]->GetScale();
		Vector ObjExt = Vector(abs(ObjSc.x), abs(ObjSc.y), abs(ObjSc.z)) / 2.0f;
		float ObjRad = ObjExt.Magnitude();
		Point ObjPos = NpcObj[i]->GetPos();
		int ObjModelType = NpcObj[i]->GetModelType();
		int PlayerModelType = Graph->GetType();

		if(ObjModelType == SPHERE)
			ObjRad = Largest(ObjSc.x, ObjSc.y, ObjSc.z) / 2.0f;

		Sphere ObjSphere(ObjPos, ObjRad);
		EnemyObj *EO = reinterpret_cast<EnemyObj*>(NpcObj[i]);

		if(PlayerSphere.Intersect(ObjSphere))
		{
			if(PlayerModelType == SPHERE && ObjModelType == SPHERE)
			{
				Point OP = ObjPos;
				OP.Project(Plane(Pos, Vector(0.0f, 1.0f, 0.0f)));
				Vector repel = Pos - OP;
				repel.NormalizeMe();
				Pos = OP + (repel * (PlayerRad + ObjRad));

				if(EO->GetEnemyType() != OBSTACLE_ENEMY)
				{
					int PlayerState = Player->GetCurrentState();
					if(PlayerState != INVULNERABLE && PlayerState != DYING)
					{
						ReceiveDamage(25.0f);
						NormalToInvulnerable();
					}
				}
			}
			else if(PlayerModelType == SPHERE && ObjModelType == CUBE)
			{
				GameObj *pl = reinterpret_cast<GameObj*>(this);
				MPR Contact(pl, NpcObj[i]);

				AABB ObjBox(ObjPos, ObjSc / 2.0f);
				if(PlayerBox.Intersect(ObjBox))
				{
					//find which face is closest
					Point FrontPoint = ObjPos + Vector(0.0f, 0.0f, ObjSc.z / 2.0f),
						  BackPoint  = ObjPos + Vector(0.0f, 0.0f, ObjSc.z / -2.0f),
						  LeftPoint  = ObjPos + Vector(ObjSc.x / -2.0f, 0.0f, 0.0f),
						  RightPoint = ObjPos + Vector(ObjSc.x / 2.0f, 0.0f, 0.0f);
					Plane FrontPlane(FrontPoint, Vector(0.0f, 0.0f, 1.0f)),
						  BackPlane(BackPoint, Vector(0.0f, 0.0f, -1.0f)),
						  LeftPlane(LeftPoint, Vector(-1.0f, 0.0f, 0.0f)),
						  RightPlane(RightPoint, Vector(1.0f, 0.0f, 0.0f));

					Plane Equal(Point(0.0f,0.0f,0.0f), Vector(0.0f,0.0f,0.0f)); //edge case plane
					Plane *ClosestPlane = &FrontPlane;
					Point PlayerPos = Pos;
					PlayerPos.Project(FrontPlane);
					float ClosestDist = Pos.Distance(PlayerPos);

					PlayerPos = Pos;
					PlayerPos.Project(BackPlane);
					float Dist2 = Pos.Distance(PlayerPos);
					if(Dist2 < ClosestDist)
					{
						ClosestPlane = &BackPlane;
						ClosestDist = Dist2;
					}
					else if(Dist2 == ClosestDist) //edge case: on one of the vertical edges of the box
					{
						Equal.N = (Pos - ObjPos);
						Equal.N.NormalizeMe();
						Equal.P = ObjPos + Vector(Equal.N.x * ObjSc.x, 0.0f, Equal.N.z * ObjSc.z);
					}

					PlayerPos = Pos;
					PlayerPos.Project(LeftPlane);
					Dist2 = Pos.Distance(PlayerPos);
					if(Dist2 < ClosestDist)
					{
						ClosestPlane = &LeftPlane;
						ClosestDist = Dist2;
					}
					else if(Dist2 == ClosestDist)
					{
						Equal.N = Pos - ObjPos;
						Equal.N.NormalizeMe();
						Equal.P = ObjPos + Vector(Equal.N.x * ObjSc.x, 0.0f, Equal.N.z * ObjSc.z);
					}

					PlayerPos = Pos;
					PlayerPos.Project(RightPlane);
					Dist2 = Pos.Distance(PlayerPos);
					if(Dist2 < ClosestDist)
					{
						ClosestPlane = &RightPlane;
						ClosestDist = Dist2;
					}
					else if(Dist2 == ClosestDist)
					{
						Equal.N = Pos - ObjPos;
						Equal.N.NormalizeMe();
						Equal.P = ObjPos + Vector(Equal.N.x * ObjSc.x, 0.0f, Equal.N.z * ObjSc.z);
					}

					PlayerPos = Pos;
					PlayerPos.Project(*ClosestPlane);
					Pos = PlayerPos + (ClosestPlane->N * PlayerRad);

					if(EO->GetEnemyType() != OBSTACLE_ENEMY)
					{
						int PlayerState = Player->GetCurrentState();
						if(PlayerState != INVULNERABLE && PlayerState != DYING)
						{
							ReceiveDamage(25.0f);
							NormalToInvulnerable();
						}
					}
				}
			}
		}
	}
}

void PlayerObj::NormalToInvulnerable(void)
{
	if(CurrentState == DYING)
		return;

	CurrentState = INVULNERABLE;
	T = 0.0f;
	StateDuration = 3.0f;
}

void PlayerObj::InvulnerableToNormal(void)
{
	CurrentState = NORMAL_STATE;
	T = 0.0f;
	StateDuration = -1.0f;
}

void PlayerObj::ReceiveDamage(float amount)
{
	Health -= amount;

	if(Health <= 0.0f)
	{
		Health = 0.0f;
		CurrentState = DYING;
	}

	Play_Sound(HIT_PLAYER);
}

void PlayerObj::IncreaseHealth(float amount)
{
	Health += amount;

	if(Health >= 100.0f)
		Health = 100.0f;
}

int PlayerObj::GetCurrentState(void)
{
	return CurrentState;
}

float PlayerObj::GetHealth(void)
{
	return Health;
}
//-----PlayerObj class functions-----

//-----Button class functions-----//
Button::Button(Point p, Vector s, Color c, unsigned tex, unsigned type)
{
	Pos = p;
	Scale = s;
	color = c;
	Tex = tex;
	ButtonType = type;
}

void Button::InteractWith(void)
{
	bool ButtonClicked = true;
	switch(ButtonType)
	{
	case BT_PLAY:
		{
			GSM.ExitToState(GST_LEVEL_1);
			break;
		}
	case BT_QUIT:
		{
			//GSM.ExitAllStates();
			GSM.AddState(GST_CONFIRM);
			GSM.SetNextState(GST_QUIT);
			break;
		}
	case BT_RESUME:
		{
			GSM.ExitState();
			break;
		}
	case BT_BACK_TO_MENU:
		{
			//GSM.ExitToState(GST_MAIN_MENU);
			GSM.AddState(GST_CONFIRM);
			GSM.SetNextState(GST_MAIN_MENU);
			break;
		}
	case BT_CREDITS:
		{
			GSM.AddState(GST_CREDITS_SCREEN);
			break;
		}
	case BT_HOW_TO_PLAY:
		{
			GSM.AddState(GST_HOW_TO_PLAY);
			break;
		}
	case BT_OPTIONS:
		{
			GSM.AddState(GST_OPTIONS_MENU);
			break;
		}
	case BT_CONFIRM:
		{
			unsigned SwitchTo = GSM.GetNextState();
			GSM.ExitState();
			GSM.ExitToState(SwitchTo);
			break;
		}
	case BT_CANCEL:
		{
			GSM.ExitState();
			break;
		}
	default:
		{
			ButtonClicked = false;
			break;
		}
	}

	if(ButtonClicked)
		Play_Sound(MENU_SELECT);
}
//-----Button class functions-----//

//-----Misc functions-----
int LookupTexture(std::string &str)
{
	if(!str.compare("METAL_ROOF"))
		return METAL_ROOF;

	if(!str.compare("FASE"))
		return FASE;

	if(!str.compare("BLUE_LINES"))
		return BLUE_LINES;

	if(!str.compare("WHITE_LINES"))
		return WHITE_LINES;

	if(!str.compare("LOAD_SCREEN"))
		return LOAD_SCREEN;

	if(!str.compare("FIRE_TEX"))
		return FIRE_TEX;

	if(!str.compare("WATER_TEX"))
		return WATER_TEX;

	if(!str.compare("WIND_TEX"))
		return WIND_TEX;

	if(!str.compare("PLAY_BUTTON"))
		return PLAY_BUTTON;

	if(!str.compare("CREDITS_BUTTON"))
		return CREDITS_BUTTON;

	if(!str.compare("QUIT_BUTTON"))
		return QUIT_BUTTON;

	if(!str.compare("YES_BUTTON"))
		return YES_BUTTON;

	if(!str.compare("NO_BUTTON"))
		return NO_BUTTON;

	if(!str.compare("ON_BUTTON"))
		return ON_BUTTON;

	if(!str.compare("OFF_BUTTON"))
		return OFF_BUTTON;

	if(!str.compare("FULLSCREEN_BUTTON"))
		return FULLSCREEN_BUTTON;

	if(!str.compare("MUSIC_BUTTON"))
		return MUSIC_BUTTON;

	if(!str.compare("SOUNDFX_BUTTON"))
		return SOUNDFX_BUTTON;

	if(!str.compare("RESUME_BUTTON"))
		return RESUME_BUTTON;

	if(!str.compare("INSTRUCTIONS_BUTTON"))
		return INSTRUCTIONS_BUTTON;

	if(!str.compare("OPTIONS_BUTTON"))
		return OPTIONS_BUTTON;

	if(!str.compare("EARTH_TEX"))
		return EARTH_TEX;

	if(!str.compare("TITLE_SCREEN"))
		return TITLE_SCREEN;

	if(!str.compare("DP_SCREEN"))
		return DP_SCREEN;

	if(!str.compare("CREDITS_SCREEN"))
		return CREDITS_SCREEN;

	if(!str.compare("CONFIRM_SCREEN"))
		return CONFIRM_SCREEN;

	if(!str.compare("INSTRUCTIONS_SCREEN"))
		return INSTRUCTIONS_SCREEN;

	if(!str.compare("OPTIONS_SCREEN"))
		return OPTIONS_SCREEN;

	if(!str.compare("WIN_SCREEN"))
		return WIN_SCREEN;

	if(!str.compare("LOSE_SCREEN"))
		return LOSE_SCREEN;

	return DEFAULT_TEX;
}
//-----Misc functions-----

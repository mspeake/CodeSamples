/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Geometry.h
Purpose: definition of my Geometry library classes, KD Tree and BVH tree
         classes and MPR functions
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Project: matthew.speake_CS350_4
Author: Matt Speake, matthew.speake, 50006908
Creation date: 5/22/2012
End Header --------------------------------------------------------*/
#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <math.h>
#include <float.h>
#include <string.h>
#include <vector>
#include <time.h>

#define PI 3.14159
#define MAX_TREE_DEPTH 10

class Vector;
class Line;
class LineSegment;
class Ray;
class Plane;
class Triangle;
class Sphere;
class AABB;
class Intersection;
class KD_Node;

class GameObj;

typedef float Range[2];

class Color
{
public:
	Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
	~Color();
	float R, G, B, A;
};

class Point
{
public:
	Point(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f);
	Point(const Point& p);
	~Point();

	float x, y, z, w;

	Vector operator-(const Point& rhs) const;
	Point operator-(const Vector& rhs) const;
	Point operator+(const Vector& rhs) const;
	bool operator==(const Point& rhs) const;
	void operator+=(const Vector& rhs);
	void operator-=(const Vector& rhs);

	float Dot(const Point& p) const;
	float Dot(const Vector& v) const;

	void Project(const Line& l);
	void Project(const Plane& p);

	void Swap(Point &p);

	float Distance(const Point& p);
	float Distance(const Line& l);
	float Distance(const LineSegment& l);
	float Distance(const Ray& r);
	float Distance(const Plane& p);
	float Distance(const Triangle& t);
	float Distance(const Sphere &s);
	float Distance(const AABB& a);

	bool IsContained(const Line& l);
	bool IsContained(const LineSegment& l);
	bool IsContained(const Ray& r);
	bool IsContained(const Plane& p);
	bool IsContained(const Triangle& t);
	bool IsContained(const Sphere& s);
	bool IsContained(const AABB& a);

	bool IsOn(const AABB& a);
	bool IsIn(const AABB& a);
};

class Vector
{
public:
	Vector(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f);
	Vector(const Vector& v);
	Vector(const Point& p);
	~Vector();

	float x, y, z, w;

	Vector operator/(float rhs) const;
	void operator/=(float rhs);
	Vector operator*(float rhs) const;
	void operator*=(float rhs);
	bool operator==(const Vector& rhs) const;
	void NormalizeMe();
	Vector Cross(const Vector& rhs) const;
	float Dot(const Vector& rhs) const;
	float Magnitude(void);
	Vector Rotate(const float angle, const Vector &axis) const;
	void RotateMe(const float angle, const Vector &axis);
};

class Line
{
public:
	Line(Point p, Vector v);
	~Line();

	Point P;
	Vector V;

	float Distance(const Line& l);
	float Distance(const Sphere& s);

	bool Intersect(const Plane& p, Intersection& inter);

	float Angle(const Line& l);
	float Angle(const Plane& p);

	bool Parallel(const Line& l);
	bool Parallel(const Plane& p);

	bool Perpendicular(const Line& l);
	bool Perpendicular(const Plane& p);

	bool Coplanar(const Plane& p);
};

class LineSegment
{
public:
	LineSegment(Point p, Point q);
	~LineSegment();

	Point P, Q;

	float Distance(const LineSegment& l);
	bool Intersect(const Plane& p, Intersection& inter);
	bool Intersect(const Triangle& t, Intersection& inter);
};

class Ray
{
public:
	Ray(Point p, Vector v);
	~Ray();

	Point P;
	Vector V;

	void HalfSpace(const Plane& p, Range &range);

	bool Intersect(const Plane& p, Intersection& inter);
	bool Intersect(const Triangle& t, Intersection& inter);
	bool Intersect(const Sphere& s, Intersection& inter);
	bool Intersect(const AABB& a, Intersection& inter);
};

class Triangle
{
public:
	Triangle(Point p, Point q, Point r);
	~Triangle();

	Point P, Q, R;
	Color col;

	bool Intersect(const Triangle& t, Intersection& inter);

	void DrawSelf(void);
};

class Plane
{
public:
	Plane(Point p, Vector n);
	~Plane();

	Point P;
	Vector N;

	float Angle(const Plane& p);
	bool Parallel(const Plane& p);
	bool Perpendicular(const Plane& p);
	bool Coplanar(const Plane& p);
};

class Sphere
{
public:
	Sphere(Point c, float r);
	~Sphere();

	Point C;
	float R;

	bool Intersect(const Sphere& s);
};

class AABB
{
public:
	AABB(Point c, Vector e);
	AABB(const AABB& b1, const AABB& b2);
	~AABB();

	Point C;
	Vector E;

	bool Intersect(const AABB& a);

	void DrawSelf(void);
};



class Intersection
{
public:
	Intersection(){};
	~Intersection(){};

	Point P;
	float T;
};


class Matrix4x4
{
public:
	Matrix4x4();
	~Matrix4x4(){};

	float M[16];
	float* GetRow(int i) const;
	float* GetCol(int i) const;
	void Identity(void);
	void Translate(float tx, float ty, float tz);
	void Rotate(const Vector& v, float a);
	void Scale(float sx = 1.0f, float sy = 1.0f, float sz = 1.0f);
	void WtoC(const Point& pos, const Vector& dir, const Vector& up);
	void CtoP(float n, float f, float w, float h);
	void PtoV(float Vw, float Vh, float Ww, float Wh);
	void NDCtoV(float w, float h);
	void Transpose(void);
	Point operator*(const Point& rhs);
	Vector operator*(const Vector& rhs);
	Matrix4x4 operator*(const Matrix4x4& rhs);
	Matrix4x4 operator*(const float& rhs);
	Matrix4x4 operator+(const Matrix4x4& rhs);
};

class Camera
{
public:

	Camera();

	Point Tar;
	Point Pos;
	Vector Up;
	Vector Dir;
	std::vector<Plane> Frustum;

	float Mat[16];
	
	float Alpha;
	float Beta;

	float Distance;

	void Reset(void);
	void Update(void);
	void LoadMatrix(void);

	void AlphaClamp(float alpha);
	void BetaClamp(float beta);

	void CalculateFrustum(void);
	int FrustumCull(const GameObj* obj);
};


void RangeIntersect(Range &r1, Range &r2);
float MyMin(const float a, const float b);
float MyMax(const float a, const float b);

float Smallest(const float& a, const float& b, const float& c);
float Largest(const float& a, const float& b, const float& c);

class Quaternion
{
public:
	Quaternion(float s = 0.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f);
	Quaternion(const Vector &v, float s = 0.0f);
	~Quaternion(void);

	Quaternion operator*(const Quaternion &rhs) const;
	Quaternion operator*(const float &rhs) const;
	Quaternion operator+(const Quaternion &rhs) const;
	Quaternion operator-(const Quaternion &rhs) const;

	float Dot(const Quaternion &rhs) const;
	float Magnitude(void) const;
	Quaternion Inverse(void) const;
	void InvertMe(void);
	Quaternion Conjugate(void) const;
	void ConjMe(void);
	void Identity(void);

	void Rotator(const float angle, const Vector &axis);

	float S;
	Vector V;
};

class Shape
{
private:
	int type;
	int stacks;
	int slices;
public:
	Shape(int _type, int _s);
	~Shape();
	void DrawSelf(bool wire = false);

	Color color;
	float *VA; //vertex/face array, normal, and normal arrays
	float *VN;
	float *TCA; //texture coords
	unsigned short *FA;

	float *TanA;
	float *BitA;

	int vert_count;
	int face_count;

	friend class GraphicsComp;
};

class Object
{
public:
	Object();
	~Object();

	void MoveMe(const Vector& d);
	int type, subdivisions;
	float angle;
	Shape *S;
	Point position;
	Vector scale, axis;
	Color color;
};

class KD_Node
{
public:
	KD_Node();
	KD_Node(const AABB& b);
	~KD_Node();

	AABB* box;
	KD_Node* Left;
	KD_Node* Right;
	std::vector<Triangle> objects;
	int depth;
};

class BVH_Node
{
public:
	BVH_Node();
	BVH_Node(const BVH_Node& n);
	~BVH_Node();

	BVH_Node *Left;
	BVH_Node *Right;
	BVH_Node *Parent;
	int type;
	AABB *Box;
	Object *obj;
};

enum NodeType
{
	NODE,
	LEAF
};

struct Event
{
	float Value;
	int Type[3];
};

enum EventType
{
	STARTING,
	ENDING,
	COPLANAR
};

void GenerateKDTree(void);
void SplitNode(KD_Node* node);
void DrawNode(KD_Node* node);
float SurfaceArea(float w, float h, float d);
void NodeRender(KD_Node* node, int depth);
void GenerateBVHTree(void);
void CalculateFrustum(void);
float BVH_Cost(const AABB &b1, const AABB &b2);
void DrawBVHLeaves(const BVH_Node* n);
void InsertNode(BVH_Node* n);
void RemoveNode(BVH_Node* n);
int TreeDepth(const BVH_Node* n);
void BalanceNode(BVH_Node* n);
void BalanceHelper(BVH_Node* n);
void BalanceTree(void);
void BVHNodeRender(BVH_Node* node, int depth, int current_depth);
enum BVH_Generation_Method
{
	BOTTOM_UP,
	INSERTION
};

class MPR
{
public:
	MPR(){Depth=0.0f; Obj0=NULL; Obj1=NULL;};
	//MPR(Object* o0, Object* o1);
	MPR(GameObj* o0, GameObj* o1);
	~MPR(){};

	void InitialPortal(void);
	bool Discovery(void);
	bool Refinement(void);

	bool Collision(void);
	void DrawPortals(void);
	void ContactInfo(void);
	//Object *Obj0;
	//Object *Obj1;
	GameObj *Obj0;
	GameObj *Obj1;
	Point C, P1, P2, P3,
		  SP11, SP12, SP21, SP22;
	Vector Norm;
	float Depth;
};

enum Portal_Draw
{
	NO_PORTAL,
	INITIAL_PORTAL,
	DISCOVERY_PORTAL,
	REFINEMENT_PORTAL
};

Point SupportFunction(const Vector& d, const Object* obj);
Point Bias(void);

Point WorldSpaceCursor(void); //get the world space coordinates of the cursor(on the projection window)

extern int vert_norms_bool, face_norms_bool, collide_bool;
extern Camera sCamera;

extern KD_Node* root_node;
extern BVH_Node* bvh_root;
extern std::vector<BVH_Node*> bvh_nodes;
extern Point BIAS;
#endif
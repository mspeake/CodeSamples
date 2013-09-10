/*--------------------------------------------------
Math/Geometry library and related functions/variables
--------------------------------------------------*/
#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <math.h>
#include <float.h>
#include <string.h>
#include <vector>
#include <time.h>

#define PI 3.14159

class Vector;
class Line;
class LineSegment;
class Ray;
class Plane;
class Triangle;
class Sphere;
class AABB;
class Intersection;

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

	float Distance(const Point& p) const;
	float Distance(const Line& l) const;
	float Distance(const LineSegment& l) const;
	float Distance(const Ray& r) const;
	float Distance(const Plane& p) const;
	float Distance(const Triangle& t) const;
	float Distance(const Sphere &s) const;
	float Distance(const AABB& a) const;

	bool IsContained(const Line& l) const;
	bool IsContained(const LineSegment& l) const;
	bool IsContained(const Ray& r) const;
	bool IsContained(const Plane& p) const;
	bool IsContained(const Triangle& t) const;
	bool IsContained(const Sphere& s) const;
	bool IsContained(const AABB& a) const;

	bool IsOn(const AABB& a) const;
	bool IsIn(const AABB& a) const;
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
	float Magnitude(void) const;
};

class Line
{
public:
	Line(Point p, Vector v);
	~Line();

	Point P;
	Vector V;

	float Distance(const Line& l) const;
	float Distance(const Sphere& s) const;

	bool Intersect(const Plane& p, Intersection& inter) const;

	float Angle(const Line& l) const;
	float Angle(const Plane& p) const;

	bool Parallel(const Line& l) const;
	bool Parallel(const Plane& p) const;

	bool Perpendicular(const Line& l) const;
	bool Perpendicular(const Plane& p) const;

	bool Coplanar(const Plane& p) const;
};

class LineSegment
{
public:
	LineSegment(Point p, Point q);
	~LineSegment();

	Point P, Q;

	float Distance(const LineSegment& l) const;
	bool Intersect(const Plane& p, Intersection& inter) const;
	bool Intersect(const Triangle& t, Intersection& inter) const;
};

class Ray
{
public:
	Ray(Point p, Vector v);
	~Ray();

	Point P;
	Vector V;

	void HalfSpace(const Plane& p, Range &range) const;

	bool Intersect(const Plane& p, Intersection& inter) const;
	bool Intersect(const Triangle& t, Intersection& inter) const;
	bool Intersect(const Sphere& s, Intersection& inter) const;
	bool Intersect(const AABB& a, Intersection& inter) const;
};

class Triangle
{
public:
	Triangle(Point p, Point q, Point r);
	~Triangle();

	Point P, Q, R;
	Color col;

	bool Intersect(const Triangle& t, Intersection& inter) const;

	void DrawSelf(void) const;
};

class Plane
{
public:
	Plane(Point p, Vector n);
	~Plane();

	Point P;
	Vector N;

	float Angle(const Plane& p) const;
	bool Parallel(const Plane& p) const;
	bool Perpendicular(const Plane& p) const;
	bool Coplanar(const Plane& p) const;
};

class Sphere
{
public:
	Sphere(Point c, float r);
	~Sphere();

	Point C;
	float R;

	bool Intersect(const Sphere& s) const;
};

class AABB
{
public:
	AABB(Point c, Vector e);
	AABB(const AABB& b1, const AABB& b2);
	~AABB();

	Point C;
	Vector E;

	bool Intersect(const AABB& a) const;

	void DrawSelf(void) const;
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
	Point operator*(const Point& rhs) const;
	Vector operator*(const Vector& rhs) const;
	Matrix4x4 operator*(const Matrix4x4& rhs) const;
	Matrix4x4 operator*(const float& rhs) const;
	Matrix4x4 operator+(const Matrix4x4& rhs) const;
};

class Camera
{
public:

	Camera();

	Vector Tar;
	Point Pos;
	Vector Up;
	Vector Dir;

	float Mat[16];
	
	float Alpha;
	float Beta;

	void Reset(void);
	void Update(void);
	void LoadMatrix(void);

	void AlphaClamp(float alpha);
	void BetaClamp(float beta);
};


void RangeIntersect(Range &r1, Range &r2);
float MyMin(const float a, const float b);
float MyMax(const float a, const float b);

float Smallest(const float& a, const float& b, const float& c);
float Largest(const float& a, const float& b, const float& c);

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

	unsigned vert_count;
	unsigned face_count;

	friend class GraphicsComp;
	friend class LevelHelper;
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


float SurfaceArea(float w, float h, float d);
void CalculateFrustum(void);

Point WorldSpaceCursor(void); //get the world space coordinates of the cursor(on the projection window)

extern Camera sCamera;
#endif
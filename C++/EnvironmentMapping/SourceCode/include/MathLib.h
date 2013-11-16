/********************************************************************
**MathLib.h
**
**-Defines the various math related classes and utility functions
** that the math library implements
********************************************************************/

#ifndef MATHLIB_H
#define MATHLIB_H

#include <math.h>
#include <float.h>
#include <iostream>

#define PI 3.14159

//used for clamping, if the difference between A and B is no
//bigger than this, A can be considered close enough to clamp
#define TOLERANCE 0.001f

class Vector;
class Line;
class LineSegment;
class Ray;
class Plane;
class Triangle;
class Sphere;
class AABB;
class Intersection;

class Range
{
public:
	float Min, Max;

	Range(const float min = 0.0f, const float max = 0.0f) : Min(min), Max(max) {};

	void IntersectWith(const Range& r);
};

class Point
{
public:
	float X, Y, Z, W;

	Point(const float x = 0.0f, const float y = 0.0f, const float z = 0.0f) : X(x), Y(y), Z(z) {W = 1.0f;};
	Point(const Vector& v);
	static Point Subtract(const Point& lhs, const Point& rhs);

	Point operator+(const Point& rhs) const;
	Point operator+(const Vector& rhs) const;
	Vector operator-(const Point& rhs) const;
	Point operator-(const Vector& rhs) const;
	bool operator==(const Point& rhs) const;
	void operator+=(const Vector& rhs);
	void operator-=(const Vector& rhs);

	float Dot(const Point& p) const;
	float Dot(const Vector& v) const;
	void Swap(Point& p);
	bool SameSide(const Point& p, const LineSegment& ls) const;

	void Project(const Line& l);
	void Project(const Plane& pl);

	float Distance(const Point& p) const;
	float Distance(const Line& l) const;
	float Distance(const LineSegment& ls) const;
	float Distance(const Ray& r) const;
	float Distance(const Plane& pl) const;
	float Distance(const Triangle& t) const;
	float Distance(const Sphere& s) const;
	float Distance(const AABB& a) const;

	bool IsContained(const Line& l) const;
	bool IsContained(const LineSegment& ls) const;
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
	float X, Y, Z, W;

	Vector(const float x = 0.0f, const float y = 0.0f, const float z = 0.0f) : X(x), Y(y), Z(z) {W = 0.0f;};
	Vector(const Point& p);

	Vector operator/(const float rhs) const;
	void operator/=(const float rhs);
	Vector operator*(const float rhs) const;
	void operator*=(const float rhs);
	bool operator==(const Vector& rhs) const;
	Vector operator+(const Vector& rhs) const;
	Vector operator-(const Vector& rhs) const;
	void operator+=(const Vector& rhs);
	void operator-=(const Vector& rhs);

	void NormalizeMe(void);
	Vector Cross(const Vector& rhs) const;
	float Dot(const Vector& rhs) const;
	float Dot(const Point& p) const;
	float Magnitude(void) const;

	bool IsZeroVector(void) const;
	bool IsParallel(const Vector& v) const;
	bool IsPerpendicular(const Vector& v) const;
};

class Line
{
public:
	Point P;
	Vector V;

	Line(const Point& p, const Vector& v) : P(p), V(v) {};

	float Distance(const Line& l) const;
	float Distance(const Sphere& s) const;

	bool Intersects(const Plane& p, Intersection& inter) const;

	float Angle(const Line& l) const;
	float Angle(const Plane& p) const;

	bool IsParallel(const Line& l) const;
	bool IsParallel(const Plane& p) const;

	bool IsPerpendicular(const Line& l) const;
	bool IsPerpendicular(const Plane& p) const;

	bool IsCoplanar(const Plane& p) const;
};

class LineSegment
{
public:
	Point P, Q;

	LineSegment(const Point& p, const Point& q) : P(p), Q(q) {};

	float Distance(const LineSegment& ls) const;

	bool Intersects(const Plane& p, Intersection& inter) const;
	bool Intersects(const Triangle& t, Intersection& inter) const;
};

class Ray
{
public:
	Point P;
	Vector V;

	Ray(const Point& p, const Vector& v) : P(p), V(v) {};

	Range GetHalfSpace(const Plane& p) const;

	bool Intersects(const Plane& p, Intersection& inter) const;
	bool Intersects(const Triangle& t, Intersection& inter) const;
	bool Intersects(const Sphere& s, Intersection& inter) const;
	bool Intersects(const AABB& a, Intersection& inter) const;
};

class Triangle
{
public:
	Point P, Q, R;

	Triangle(const Point& p, const Point& q, const Point& r) : P(p), Q(q), R(r) {};
	Plane GetPlane(void) const;
	bool Intersects(const Triangle& t, Intersection& inter) const;
};

class Plane
{
public:
	Point P;
	Vector N;

	Plane(const Point& p, const Vector& n) : P(p), N(n) {};

	float Angle(const Plane& p) const;
	bool IsParallel(const Plane& p) const;
	bool IsPerpendicular(const Plane& p) const;
	bool IsCoplanar(const Plane& p) const;
};

class Sphere
{
public:
	Point C;
	float R;

	Sphere(const Point& c, const float r) : C(c), R(r) {};

	bool Intersects(const Sphere& s) const;
};

class AABB
{
public:
	Point C;
	Vector E;

	AABB(const Point& c, const Vector& e) : C(c), E(e) {};

	bool Intersects(const AABB& a) const;
};

class Intersection
{
public:
	Point P;
	float T;
};


class Row
{
public:
	float R[4];

	Row(const float r0 = 0.0f, const float r1 = 0.0f,
		const float r2 = 0.0f, const float r3 = 0.0f);
	float& operator[](const unsigned i);
	const float& operator[](const unsigned i) const;
	float operator*(const Row& r) const;
	float operator*(const Vector& v) const;
	float operator*(const Point& p) const;
};

class Matrix4x4
{
public:
	Row M[4];

	Matrix4x4(void);

	Row& GetRow(const unsigned i);
	const Row& GetRow(const unsigned i) const;
	Row GetCol(const unsigned i) const;

	Row& operator[](const unsigned i);
	const Row& operator[](const unsigned i) const;
	Matrix4x4 operator*(const Matrix4x4& rhs) const;
	Matrix4x4 operator*(const float rhs) const;
	void operator*=(const float rhs);
	Point operator*(const Point& rhs) const;
	Vector operator*(const Vector& rhs) const;
	Matrix4x4 operator+(const Matrix4x4& rhs) const;

	void Identity(void);
	void Translate(const float tx = 0.0f, const float ty = 0.0f, const float tz = 0.0f);
	void Rotate(const Vector& v, const float a, bool deg = false);
	void Scale(const float sx = 0.0f, const float sy = 0.0f, const float sz = 0.0f);
	void WtoC(const Point& pos, const Vector& dir, const Vector& up);
	void CtoP(const float n, const float f, const float w, const float h);
};

class Quaternion
{
public:
	float S;
	Vector V;

	Quaternion(const float s = 0.0f, const float x = 0.0f, const float y = 0.0f, const float z = 0.0f);
	Quaternion(const Vector& v, const float s = 0.0f) : V(v), S(s) {};

	Quaternion operator*(const Quaternion& rhs) const;
	Quaternion operator*(const float& rhs) const;
	Quaternion operator/(const float& rhs) const;
	Quaternion operator+(const Quaternion& rhs) const;
	Quaternion operator-(const Quaternion& rhs) const;
	void operator*=(const float& rhs);
	void operator/=(const float& rhs);
	void operator+=(const Quaternion& rhs);
	void operator-=(const Quaternion& rhs);

	float Dot(const Quaternion& rhs) const;
	float SquareMag(void) const;
	float Magnitude(void) const;
	Quaternion Inverse(void) const;
	void InvertMe(void);
	Quaternion Conjugate(void) const;
	void ConjMe(void);
	void Identity(void);
	void Rotator(const float angle, const Vector& axis);
};

namespace MathUtils
{
	float DegToRad(const float a);
	bool InRange(const float val, const float lhs, const float rhs);
	float Clamp(const float val, const float low, const float high);
	float ClampTo(const float val, const float clamp, const float diff);
	bool NearZero(const float val);
	float MyMin(const float a, const float b);
	float MyMax(const float a, const float b);
}

#endif
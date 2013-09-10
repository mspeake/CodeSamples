/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Geometry.cpp
Purpose: implementation of my geometry library functions, KD Tree and BVH Tree
         functions and MPR functions
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 5/22/2012
End Header --------------------------------------------------------*/
#include "Geometry.h"
#include "ObjReader.h"
#include "ShaderEditor.h"

#include "GameObj.h"

int vert_norms_bool = 0;
int face_norms_bool = 0;
int collide_bool = 0;
std::vector<BVH_Node*> bvh_nodes;

/////Color class functions/////
Color::Color(float r, float g, float b, float a)
{
	R = r;
	G = g;
	B = b;
	A = a;
}

Color::~Color(void)
{
}
//////////

/////Point class functions/////
Point::Point(float _x, float _y, float _z)
{
	x = _x;
	y = _y;
	z = _z;
	w = 1.0f;
}

Point::Point(const Point& p)
{
	*this = p;
}
Point::~Point(void)
{
}

Vector Point::operator-(const Point &rhs) const
{
	return Vector(x - rhs.x, y - rhs.y, z - rhs.z);
}

Point Point::operator-(const Vector &rhs) const
{
	return Point(x - rhs.x, y - rhs.y, z - rhs.z);
}

Point Point::operator+(const Vector &rhs) const
{
	return Point(x + rhs.x, y + rhs.y, z + rhs.z);
}

bool Point::operator==(const Point& rhs) const
{
	if((x == rhs.x) && (y == rhs.y) && (z == rhs.z))
		return true;
	else
		return false;
}

void Point::operator+=(const Vector& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
}

void Point::operator-=(const Vector& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
}

float Point::Dot(const Point &p) const
{
	return (x * p.x) + (y * p.y) + (z * p.z);
}

float Point::Dot(const Vector &v) const
{
	return (x * v.x) + (y * v.y) + (z * v.z);
}

void Point::Project(const Line &l)
{
	Vector v = l.V;
	v.NormalizeMe();
	float dist = Vector(*this - l.P).Dot(v);
	Point Q = l.P;
	*this = Q + (v * dist);
}

void Point::Project(const Plane &p)
{
	Vector N = p.N;
	Point copy = *this;
	N.NormalizeMe();
	Point Q = p.P;
	Vector pq = (copy - Q);
	//pq.NormalizeMe();
	float dist = pq.Dot(N);
	copy = *this - (N * dist);
	*this = copy;
}

void Point::Swap(Point &p)
{
	Point temp = *this;
	*this = p;
	p = temp;
}

float Point::Distance(const Point &p)
{
	return (*this - p).Magnitude();
}

float Point::Distance(const Line &l)
{
	Point p = *this;
	p.Project(l);
	return (*this - p).Magnitude();
}

float Point::Distance(const LineSegment &l)
{
	Point copy = *this;
	Vector V = Point(l.Q) - l.P;
	Line L(l.P, V);
	copy.Project(L);

	Vector lhs = copy - l.P;
	Vector rhs = V / (V.Dot(V));

	float t = lhs.Dot(rhs);

	if(t < 0.0f)      
		copy =  l.P;
	else if(t > 1.0f) 
		copy = l.Q; 

	return (*this - copy).Magnitude();
}

float Point::Distance(const Ray &r)
{
	Point copy = *this;
	Vector V = r.V;
	Line L(r.P, V);
	copy.Project(L);

	Vector lhs = copy - r.P;
	Vector rhs = V / (V.Dot(V));

	float t = lhs.Dot(rhs);

	if(t < 0.0f)      
		copy =  r.P;

	return (*this - copy).Magnitude();
}

float Point::Distance(const Plane &p)
{
	Point copy = *this;
	copy.Project(p);
	return (*this - copy).Magnitude();
}

float Point::Distance(const Triangle& t)
{
	Vector a(Point(t.Q) - t.P);//2 - 1
	Vector b(Point(t.R) - t.P);//3 - 1
	Vector c(Point(t.R) - t.Q);//3 - 2

	Vector N = a.Cross(b); //get the normal of the triangle's plane
	Plane p(t.P, N);
	Point copy(*this);
	copy.Project(p); //project a copy of this point on to the plane

	//first check if it's outside a
	Vector d(copy - t.P);
	Vector cp1 = N; //a X b
	Vector cp2 = a.Cross(d);
	if(cp1.Dot(cp2) < 0.0f) //point is outside a
	{
		return this->Distance(LineSegment(t.Q, t.P));
	}

	//next check if it's outside b
	cp1 = b.Cross(a);
	cp2 = b.Cross(d);
	if(cp1.Dot(cp2) < 0.0f) //point is outside b
	{
		return this->Distance(LineSegment(t.R, t.P));
	}

	//now check if it's outside c
	d = copy - t.Q;
	cp1 = c.Cross(Point(t.P) - t.Q);
	cp2 = c.Cross(d);
	if(cp1.Dot(cp2) < 0.0f) //point is outside c
	{
		return this->Distance(LineSegment(t.R, t.Q));
	}

	//function hasn't returned yet, thus the projected point is inside the triangle, dist = |copy - p|
	return this->Distance(copy);
}

float Point::Distance(const Sphere &s)
{
	return (this->Distance(s.C)) - s.R;
}

float Point::Distance(const AABB& a)
{
	Point pmin = Point(a.C) - a.E;
	Point pmax = Point(a.C) + a.E;
	Point copy = *this;

	bool x_inside = true, y_inside = true, z_inside = true;

	if(copy.x < pmin.x)
	{
		copy.x = pmin.x;
		x_inside = false;
	}
	else if(copy.x > pmax.x)
	{
		copy.x = pmax.x;
		x_inside = false;
	}

	if(copy.y < pmin.y)
	{
		copy.y = pmin.y;
		y_inside = false;
	}
	else if(copy.y > pmax.y)
	{
		copy.y = pmax.y;
		y_inside = false;
	}

	if(copy.z < pmin.z)
	{
		copy.z = pmin.z;
		z_inside = false;
	}
	else if(copy.z > pmax.z)
	{
		copy.z = pmax.z;
		z_inside = false;
	}

	if(x_inside && y_inside && z_inside)
		return 0.0f;

	return this->Distance(copy);
}

bool Point::IsContained(const Line& l)
{
	if(this->Distance(l) == 0.0f)
		return true;
	else
		return false;
}

bool Point::IsContained(const LineSegment& l)
{
	if(this->Distance(l) == 0.0f)
		return true;
	else
		return false;
}

bool Point::IsContained(const Ray& r)
{
	if(this->Distance(r) == 0.0f)
		return true;
	else
		return false;
}

bool Point::IsContained(const Plane& p)
{
	Vector v1 = *this - p.P;
	Vector pv = (p.P - Point(0.0f, 0.0f, 0.0f));
	//v1.NormalizeMe();
	float dotprod = v1.Dot(p.N);

	ZeroClamp(dotprod);
	if(dotprod == 0.0f)
		return true;
	//else if(dotprod > 0.0f && dotprod < 0.000009f) //floating point errors are evil
	//	return true;
	//else if(dotprod < 0.0f && dotprod > -0.000009f)
	//	return true;
	else
		return false;
}

bool Point::IsContained(const Triangle& t)
{
	Vector v1 = t.Q - t.P, v2 = t.R - t.P;
	Vector n = v1.Cross(v2);
	n.NormalizeMe();
	Plane pl(t.P, n);
	if(!this->IsContained(pl))
		return false;

	float dist = this->Distance(t);
	if(dist == 0.0f)
		return true;
	else if(dist > 0.0f && dist < 0.000009f) //floating point errors are evil
		return true;
	else if(dist < 0.0f && dist > -0.000009f)
		return true;
	else
		return false;

	//barycentric method, not sure if failed
	/*Vector vAB = v1, vAC = v2, vAP = *this - t.P;
	float dotAA = vAB.Dot(vAB), dotAB = vAB.Dot(vAC), dotBB = vAC.Dot(vAC);
	float dotPA = vAP.Dot(vAB), dotPB = vAP.Dot(vAC);
	float denom = (dotAA * dotBB) - (dotAB * dotAB);

	Point BC;
	BC.y = ((dotBB * dotPA) - (dotAB * dotPB)) / denom;
	BC.z = ((dotAA * dotPB) - (dotAB * dotPA)) / denom;
	BC.x = 1.0f - BC.y - BC.z;

	float s_ = BC.y;

	float t_ = BC.z;

	if(s_ < 0.0f || s_ > 1.0f)
		return false;

	if(t_ < 0.0f || t_ > 1.0f)
		return false;

	float ost = BC.x;

	if(ost < 0.0f || ost > 1.0f)
		return false;*/


	//a different method involving normals, not sure if failed
	//Vector nAC = v1.Cross(v2).Cross(v2); //normal to AC, pointing outside
	//nAC.NormalizeMe();
	//float dist = nAC.Dot(*this - t.P);
	//float near_zero = -0.000000009f;
	//if(dist >= near_zero) //positive dist means the point is outside the line
	//	return false;

	//Vector nAB = (t.R - t.Q).Cross(v1).Cross(v1);
	//nAB.NormalizeMe();
	//dist = nAB.Dot(*this - t.P);
	//if(dist >= near_zero) //positive dist means the point is outside the line
	//	return false;

	//Vector nBC = (t.P - t.R).Cross(t.Q - t.R).Cross(t.Q - t.R);
	//nBC.NormalizeMe();
	//dist = nBC.Dot(*this - t.Q);
	//if(dist >= near_zero) //positive dist means the point is outside the line
	//	return false;

	//return true;
}

bool Point::IsContained(const Sphere& s)
{
	if(this->Distance(s) <= 0.0f)
		return true;
	else
		return false;
}
bool Point::IsContained(const AABB &a)
{
	if(this->Distance(a) == 0.0f)
		return true;
	else
		return false;
}

bool Point::IsOn(const AABB& a)
{
	if((x == a.C.x + a.E.x) || (x == a.C.x - a.E.x))
	{
		if((y <= a.C.y + a.E.y) && (y >= a.C.y - a.E.y))
			return true;
		else if((z <= a.C.z + a.E.z) && (z >= a.C.z - a.E.z))
			return true;
	}
	if((y == a.C.y + a.E.y) || (y == a.C.y - a.E.y))
	{
		if((z <= a.C.z + a.E.z) && (z >= a.C.z - a.E.z))
			return true;
		else if((x <= a.C.x + a.E.x) && (x >= a.C.x - a.E.x))
			return true;
	}
	if((z == a.C.z + a.E.z) || (z == a.C.z - a.E.z))
	{
		if((x <= a.C.x + a.E.x) && (x >= a.C.x - a.E.x))
			return true;
		else if((y <= a.C.y + a.E.y) && (y >= a.C.y - a.E.y))
			return true;
	}

	return false;
}

bool Point::IsIn(const AABB& a)
{
	Vector dist = *this - a.C;
	if((dist.x < a.E.x) && (dist.y < a.E.y) &&(dist.z < a.E.z))
		return true;

	return false;
}
//////////

/////Vector class functions/////
Vector::Vector(float _x, float _y, float _z)
{
	x = _x;
	y = _y;
	z = _z;
	w = 0.0f;
}

Vector::Vector(const Vector& v)
{
	*this = v;
}

Vector::Vector(const Point& p)
{
	x = p.x;
	y = p.y;
	z = p.z;
	w = 0.0f;
}

Vector::~Vector(void)
{
}

Vector Vector::operator/(float rhs) const
{
	return Vector(x / rhs, y / rhs, z / rhs);
}

void Vector::operator/=(float f)
{
	x /= f;
	y /= f;
	z /= f;
}

Vector Vector::operator*(float rhs) const
{
	return Vector(x * rhs, y * rhs, z * rhs);
}

void Vector::operator*=(float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
}

bool Vector::operator==(const Vector& rhs) const
{
	if((x == rhs.x) && (y == rhs.y) && (z == rhs.z))
		return true;
	else
		return false;
}

void Vector::NormalizeMe(void)
{
	if(x == 0.0f && y == 0.0f && z == 0.0f) //dont try to normalize a zero vector
		return;
	*this /= this->Magnitude();
}

Vector Vector::Cross(const Vector &rhs) const
{
	Vector ret;
	ret.x = (y * rhs.z) - (z * rhs.y);
	ret.y = (z * rhs.x) - (x * rhs.z);
	ret.z = (x * rhs.y) - (y * rhs.x);
	return ret;
}

float Vector::Dot(const Vector &rhs) const
{
	double ret = (double(x) * double(rhs.x)) + (double(y) * double(rhs.y)) + (double(z) * double(rhs.z));
	return float(ret);
}

float Vector::Magnitude(void)
{
	return sqrt((x*x) + (y*y) + (z*z));
}

Vector Vector::Rotate(const float angle, const Vector &axis) const
{
	Quaternion rot, old_vec(*this);
	rot.Rotator(angle, axis);
	Quaternion new_vec = rot * old_vec * rot.Inverse();
	return new_vec.V;
}

void Vector::RotateMe(const float angle, const Vector &axis)
{
	Quaternion rot, old_vec(*this);
	rot.Rotator(angle, axis);
	Quaternion new_vec = rot * old_vec * rot.Inverse();
	*this = new_vec.V;
}
//////////

/////Line class functions/////
Line::Line(Point p, Vector v)
{
	P = p;
	V = v;
}

Line::~Line(void)
{
}

float Line::Distance(const Line &l)
{
	Point Q1 = this->P, Q2 = l.P;
	Vector V1 = this->V, V2 = l.V, u0 = Q1 - Q2;
	float a = V1.Dot(V1), b = V1.Dot(V2), c = V2.Dot(V2);
	float d = V1.Dot(u0), e = V2.Dot(u0);
	float denom = (a*c) - (b*b), s = 0.0f, t = 0.0f;

	if(denom == 0.0f)
		t = d / b;
	else
	{
		s = (b*e - c*d) / denom;
		t = (a*e - b*d) / denom;
	}

	Point P1 = Q1 + (V1 * s);
	Point P2 = Q2 + (V2 * t);

	return P1.Distance(P2);
}

float Line::Distance(const Sphere &s)
{
	Line copy = *this;
	Point center = s.C;
	return (center.Distance(copy) - s.R);
}

bool Line::Intersect(const Plane& p, Intersection& inter)
{
	Point Q(this->P), C(p.P);
	Vector v(this->V), N(p.N);

	float denom = (N.Dot(v));
	if(denom != 0.0f)
	{
		float t = (-N.Dot(C) - N.Dot(Q)) / denom;
		inter.T = t;
		inter.P = P + (v * t);
		return true;
	}
	else
		return false;
}

float Line::Angle(const Line &l)
{
	Vector V1(this->V), V2(l.V);

	return acos(V1.Dot(V2) / (V1.Magnitude() * V2.Magnitude()));
}

float Line::Angle(const Plane &p)
{
	Vector v(this->V), n(p.N);

	return (PI / 2.0f) - acos(v.Dot(n) / v.Magnitude() * n.Magnitude());
}

bool Line::Parallel(const Line &l)
{
	Vector V1(this->V), V2(l.V);

	float thing = V1.Dot(V2) / (V1.Magnitude() * V2.Magnitude());

	if(abs(thing) == 1.0f)
		return true;
	else
		return false;
}

bool Line::Parallel(const Plane& p)
{
	Vector v(this->V), n(p.N);

	if(v.Dot(n) == 0.0f)
		return true;
	else
		return false;
}

bool Line::Perpendicular(const Line &l)
{
	Vector V1(this->V), V2(l.V);

	if(V1.Dot(V2) == 0.0f)
		return true;
	else
		return false;
}

bool Line::Perpendicular(const Plane& p)
{
	Vector v(this->V), n(p.N);
	float thing = abs(v.Dot(n) / (v.Magnitude() * n.Magnitude()));

	if(thing == 1.0f)
		return true;
	else
		return false;
}

bool Line::Coplanar(const Plane &p)
{
	Line L(this->P, this->V);
	Point Q(this->P);

	if(L.Parallel(p) && Q.IsContained(p))
		return true;
	else
		return false;
}
//////////

/////LineSegment class functions/////
LineSegment::LineSegment(Point p, Point q)
{
	P = p;
	Q = q;
}

LineSegment::~LineSegment(void)
{
}

float LineSegment::Distance(const LineSegment& l)
{
	Point Q1 = this->P, Q2 = this->Q, Q3 = l.P, Q4 = l.Q;
	Vector V1 = (Q2 - Q1), V2 = (Q4 - Q3);
	float a = V1.Dot(V1), b = V1.Dot(V2), c = V2.Dot(V2);
	float d = V1.Dot(Q1 - Q3), e = V2.Dot(Q3 - Q1);

	float s = 0.0f, t = 0.0f;
	float denom = (a*c) - (b*b);

	if(denom != 0.0f)
		s = (b*e - c*d) / denom;

	if(s < 0.0f)
		s = 0.0f;
	else if(s > 1.0f)
		s = 1.0f;

	t = (e + s*b) / c;

	if(t < 0.0f)
	{
		t = 0.0f;
		s = -c / a;
	}
	else if(t > 1.0f)
	{
		t = 1.0f;
		s = (b - c) / a;
	}

	Point P1 = Q1 + (V1 * s), P2 = Q3 + (V2 * t);

	return P1.Distance(P2);
}

bool LineSegment::Intersect(const Plane& p, Intersection& inter)
{
	Point Q(this->P), C(p.P);
	Vector v(this->Q - this->P), N(p.N);

	float denom = (N.Dot(v));
	if(denom == 0.0f)
		return false;

	float t = (-N.Dot(C) - N.Dot(Q)) / denom;

	if(t < 0.0f)
		return false;
	else if( t > 1.0f)
		return false;

	inter.T = t;
	inter.P = P + (v * t);
	return true;
}

bool LineSegment::Intersect(const Triangle& t, Intersection& inter)
{
	Vector a(Point(t.Q) - t.P), b(Point(t.R) - t.P);
	Vector n = a.Cross(b);
	Plane pl(t.P, n);
	bool ret = Intersect(pl, inter);

	if(!ret)
		return false;

	if(!inter.P.IsContained(t))
		return false;

	return true;
}
//////////

/////Ray class functions/////
Ray::Ray(Point p, Vector v)
{
	P = p;
	V = v;
}

Ray::~Ray(void)
{
}

void Ray::HalfSpace(const Plane& p, Range &range)
{
	Point Q(this->P), C(p.P);
	Vector V(this->V), N(p.N);

	float d1 = (Q - C).Dot(N), d2 = V.Dot(N);

	if(d2 < 0.0f)
	{
		float t = -d1 / d2;
		range[0] = t, range[1] = FLT_MAX;
	}
	else if(d2 > 0.0f)
	{
		float t = -d1 / d2;
		range[0] = FLT_MIN, range[1] = t;
	}
	else if(d1 < 0.0f)
	{
		range[0] = FLT_MIN, range[1] = FLT_MAX;
	}
}

bool Ray::Intersect(const Plane& p, Intersection& inter)
{
	Point Q(this->P), C(p.P);
	Vector v(this->V), N(p.N);

	float denom = (N.Dot(v));
	if(denom == 0.0f)
		return false;

	float t = ((C - Q).Dot(N)) / denom;

	if(t < 0.0f)
		return false;

	inter.T = t;
	inter.P = Q + (v * t);
	return true;
}

bool Ray::Intersect(const Triangle& t, Intersection& inter)
{
	Vector a(t.Q - t.P), b(t.R - t.P);
	Vector n = a.Cross(b);
	n.NormalizeMe();
	Plane pl(t.P, n);
	bool ret = this->Intersect(pl, inter);

	if(!ret)
		return false;

	if(!inter.P.IsContained(t))
		return false;

	return true;
}

bool Ray::Intersect(const Sphere& s, Intersection& inter)
{
	Point Q(this->P), C(s.C);
	Vector v(this->V), QC = Q - C;
	float R = s.R, a = v.Dot(v), b = 2.0f * (v.Dot(QC)), c = QC.Dot(QC) - (R*R);

	float disc = (b*b) - (4.0f*a*c);

	if(disc < 0.0f) // no intersection
		return false;
	else if(disc == 0.0f) //1 intersection
	{
		float t = -b / (2.0f * a);
		if(t <= 0.0f) //intersection behind the ray = no
			return false;
		else
		{
			inter.T = t;
			inter.P = Q + (v * t);
			return true;
		}
	}
	else //2 intersections
	{
		float t1 = (-b + sqrt(disc)) / (2.0f * a), t2 = (-b - sqrt(disc)) / (2.0f * a);

		if(t1 >= 0.0f && t2 >= 0.0f)
		{
			inter.T = t1 < t2 ? t1 : t2;
			inter.P = Q + (v * inter.T);
			return true;
		}
		else if(t1 >= 0.0f && t2 < 0.0f)
		{
			inter.T = t1;
			inter.P = Q + (v * t1);
			return true;
		}
		else if(t1 < 0.0f && t2 >= 0.0f)
		{
			inter.T = t2;
			inter.P = Q + (v * t2);
			return true;
		}
		else //t1 < 0.0f && t2 < 0.0f
			return false;
	}
}

bool Ray::Intersect(const AABB& a, Intersection& inter)
{
	Point c(a.C), Q(this->P);
	Vector E(a.E), v(this->V);
	//Line L(Q, V);
	Plane H1(c + Vector(0.0f, 0.0f, E.z), Vector(0.0f, 0.0f, 1.0f));
	Plane H2(c - Vector(0.0f, 0.0f, E.z), Vector(0.0f, 0.0f, -1.0f));
	Plane H3(c - Vector(E.x, 0.0f, 0.0f), Vector(-1.0f, 0.0f, 0.0f));
	Plane H4(c + Vector(E.x, 0.0f, 0.0f), Vector(1.0f, 0.0f, 0.0f));
	Plane H5(c - Vector(0.0f, E.y, 0.0f), Vector(0.0f, -1.0f, 0.0f));
	Plane H6(c + Vector(0.0f, E.y, 0.0f), Vector(0.0f, 1.0f, 0.0f));

	//find the intersections
	Range r1, r2, r3, r4, r5, r6;
	HalfSpace(H1, r1);
	HalfSpace(H2, r2);
	HalfSpace(H3, r3);
	HalfSpace(H4, r4);
	HalfSpace(H5, r5);
	HalfSpace(H6, r6);

	RangeIntersect(r1, r2);
	RangeIntersect(r1, r3);
	RangeIntersect(r1, r4);
	RangeIntersect(r1, r5);
	RangeIntersect(r1, r6);

	float tmin = r1[0], tmax = r1[1];

	if( (tmin <= tmax) && (tmin >= 0.0f || tmax >= 0.0f) )
	{
		inter.T = tmin;
		inter.P = Q + (v * inter.T);
		return true;
	}

	return false;
}
//////////

/////Triangle class functions/////
Triangle::Triangle(Point p, Point q, Point r)
{
	P = p;
	Q = q;
	R = r;
	col.R = 0.0f, col.G = 0.0f, col.B = 0.0f;
}

Triangle::~Triangle(void)
{
}

bool Triangle::Intersect(const Triangle &t, Intersection &inter)
{
	Point T1(this->P), T2(this->Q), T3(this->R),
		  T4(t.P), T5(t.Q), T6(t.R);
	LineSegment LS1(T2, T1), LS2(T3, T1), LS3(T3, T2),
				LS4(T5, T4), LS5(T6, T4), LS6(T6, T5);

	if(LS1.Intersect(t, inter))
		return true;

	else if(LS2.Intersect(t, inter))
		return true;

	else if(LS3.Intersect(t, inter))
		return true;

	if(LS4.Intersect(*this, inter))
		return true;
	else if(LS5.Intersect(*this, inter))
		return true;
	else if(LS6.Intersect(*this, inter))
		return true;

	return false;
}
//////////

/////Plane class functions/////
Plane::Plane(Point p, Vector n)
{
	P = p;
	N = n;
}

Plane::~Plane(void)
{
}

float Plane::Angle(const Plane &p)
{
	Vector N1(this->N), N2(p.N);

	return acos(N1.Dot(N2) / N1.Magnitude() * N2.Magnitude());
}

bool Plane::Parallel(const Plane &p)
{
	Vector N1(this->N), N2(p.N);

	float thing = abs(N1.Dot(N2) / (N1.Magnitude() * N2.Magnitude()));

	if(thing == 1.0f)
		return true;
	else
		return false;
}

bool Plane::Perpendicular(const Plane& p)
{
	Vector N1(this->N), N2(p.N);

	if(N1.Dot(N2) == 0.0f)
		return true;
	else
		return false;
}

bool Plane::Coplanar(const Plane& p)
{
	Point C1(this->P), C2(p.P);

	if(this->Parallel(p) && (C1.IsContained(p) || C2.IsContained(*this)))
		return true;
	else
		return false;
}
//////////

/////Sphere class functions/////
Sphere::Sphere(Point c, float r)
{
	C = c;
	R = r;
}

Sphere::~Sphere(void)
{
}

bool Sphere::Intersect(const Sphere& s)
{
	Point C1(this->C), C2(s.C);
	float R_Sum = this->R + s.R, C_Dist = C1.Distance(C2);

	if(C_Dist <= R_Sum)
		return true;
	else
		return false;
}
//////////

/////AABB class functions/////
AABB::AABB(Point c, Vector e)
{
	C = c;
	E = e;
}

AABB::AABB(const AABB& b1, const AABB& b2)
{
	Point Max1 = b1.C + b1.E, Min1 = b1.C + (b1.E * -1.0f);
	Point Max2 = b2.C + b2.E, Min2 = b2.C + (b2.E * -1.0f);

	Point Max3(MyMax(Max1.x, Max2.x), MyMax(Max1.y, Max2.y), MyMax(Max1.z, Max2.z));
	Point Min3(MyMin(Min1.x, Min2.x), MyMin(Min1.y, Min2.y), MyMin(Min1.z, Min2.z));
	
	Vector E3 = (Max3 - Min3) / 2.0f;
	Point C3 = Max3 - E3;

	C = C3;
	E = E3;
}

AABB::~AABB(void)
{
}

bool AABB::Intersect(const AABB& a)
{
	float dx = abs(a.C.x - C.x), dy = abs(a.C.y - C.y), dz = abs(a.C.z - C.z);
	float sx = a.E.x + E.x, sy = a.E.y + E.y, sz = a.E.z + E.z;

	if(dx > sx)
		return false;

	if(dy > sy)
		return false;

	if(dz > sz)
		return false;

	return true;
}
//////////

/////Matrix4x4 class functions/////
Matrix4x4::Matrix4x4(void)
{
	Identity();
}

void Matrix4x4::Identity(void)
{
	for(unsigned i = 0; i < 16; i++)
	{
		if(i == 0 || i == 5 || i == 10 || i == 15)
			M[i] = 1.0f;
		else
			M[i] = 0.0f;
	}
}
float* Matrix4x4::GetRow(int i) const
{
	float RowI[4] = {M[i*4 + 0], M[i*4 + 1], M[i*4 + 2], M[i*4 + 3]};
	return RowI;
}

float* Matrix4x4::GetCol(int i) const
{
	float ColI[4] = {M[0*4 + i], M[1*4 + i], M[2*4 + i], M[3*4 + i]};
	return ColI;
}

Point Matrix4x4::operator*(const Point& rhs)
{
	float res[4] = {0.0f};
	for(unsigned i = 0; i < 4; i++)
	{
		float *RowI = GetRow(i);
		float sum = (*(RowI + 0) * rhs.x) + (*(RowI + 1) * rhs.y) + (*(RowI + 2) * rhs.z) + (*(RowI + 3) * rhs.w);
		res[i] = sum;
	}
	Point ret(res[0], res[1], res[2]);
	ret.w = res[3];
	return ret;
}

Vector Matrix4x4::operator*(const Vector& rhs)
{
	float res[4] = {0.0f};
	for(unsigned i = 0; i < 4; i++)
	{
		float *RowI = GetRow(i);
		float sum = (*(RowI + 0) * rhs.x) + (*(RowI + 1) * rhs.y) + (*(RowI + 2) * rhs.z) + (*(RowI + 3) * rhs.w);
		res[i] = sum;
	}
	return Vector(res[0], res[1], res[2]);
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &rhs)
{
	Matrix4x4 ret;
	for(int row = 0; row < 4; row++)
	{
		float* ri = GetRow(row);
		float RowI[4];
		memcpy(RowI, ri, 4 * sizeof(float));

		for(int col = 0; col < 4; col++)
		{
			float* ci = rhs.GetCol(col);
			float ColI[4];
			memcpy(ColI, ci, 4 * sizeof(float));
			float sum = (RowI[0] * ColI[0]) + (RowI[1] * ColI[1]) + (RowI[2] * ColI[2]) + (RowI[3] * ColI[3]);
			ret.M[row*4 + col] = sum;
		}
	}
	return ret;
}

Matrix4x4 Matrix4x4::operator*(const float &rhs)
{
	Matrix4x4 ret;
	for(unsigned i = 0; i < 16; i++)
	{
		ret.M[i] = rhs * M[i];
	}
	return ret;
}

Matrix4x4 Matrix4x4::operator+(const Matrix4x4 &rhs)
{
	Matrix4x4 ret;
	for(unsigned i = 0; i < 16; i++)
	{
		ret.M[i] = M[i] + rhs.M[i];
	}
	return ret;
}

void Matrix4x4::Translate(float tx, float ty, float tz)
{
	M[0*4 + 3] = tx;
	M[1*4 + 3] = ty;
	M[2*4 + 3] = tz;
}

void Matrix4x4::Scale(float sx, float sy, float sz)
{
	M[0*4 + 0] = sx;
	M[1*4 + 1] = sy;
	M[2*4 + 2] = sz;
}

void Matrix4x4::Rotate(const Vector &v, float a)
{
	float alpha = (a / 180.0f) * PI;
	float c = cos(alpha), s = sin(alpha), t = 1.0f - c;

	M[0] = t*(v.x*v.x) + c, M[1] = t*(v.x*v.y) - s*v.z, M[2] = t*(v.x*v.z) + s*v.y, M[3] = 0.0f;
	M[4] = t*(v.x*v.y) + s*v.z, M[5] = t*(v.y*v.y) + c, M[6] = t*(v.y*v.z) - s*v.x, M[7] = 0.0f;
	M[8] = t*(v.x*v.z) - s*v.y, M[9] = t*(v.y*v.z) + s*v.x, M[10] = t*(v.z*v.z) + c, M[11] = 0.0f;
	M[12] = 0.0f, M[13] = 0.0f, M[14] = 0.0f, M[15] = 1.0f;
}

void Matrix4x4::WtoC(const Point& pos, const Vector& dir, const Vector& up)
{
	Vector r = dir.Cross(up);
	Matrix4x4 tr, rot;
	tr.Translate(-pos.x, -pos.y, -pos.z);
	rot.M[0] = r.x, rot.M[1] = r.y, rot.M[2] = r.z, rot.M[3] = 0.0f;
	rot.M[4] = up.x, rot.M[5] = up.y, rot.M[6] = up.z, rot.M[7] = 0.0f;
	rot.M[8] = -dir.x, rot.M[9] = -dir.y, rot.M[10] = -dir.z, rot.M[11] = 0.0f;
	rot.M[12] = 0.0f, rot.M[13] = 0.0f, rot.M[14] = 0.0f, rot.M[15] = 1.0f;
	*this = rot * tr;
}

void Matrix4x4::CtoP(float n, float f, float w, float h)
{
	M[0] = (2.0f*n) / w, M[1] = 0.0f, M[2] = 0.0f, M[3] = 0.0f;
	M[4] = 0.0f, M[5] = (2.0f*n) / h, M[6] = 0.0f, M[7] = 0.0f;
	M[8] = 0.0f, M[9] = 0.0f, M[10] = (-n - f) / (f - n), M[11] = (-2.0f*n*f) / (f - n);
	M[12] = 0.0f, M[13] = 0.0f, M[14] = -1.0f, M[15] = 0.0f;

	/*float fovy = float(FOV_Y),
		  e = 1.0f / tan(DegToRad(fovy / 2.0F)),
		  a = h / w;
	M[0] = e, M[1] = 0.0f, M[2] = 0.0f, M[3] = 0.0f;
	M[4] = 0.0f, M[5] = e / a, M[6] = 0.0f, M[7] = 0.0f;
	M[8] = 0.0f, M[9] = 0.0f, M[10] = (-n - f) / (f - n), M[11] = (-2.0f*n*f) / (f - n);
	M[12] = 0.0f, M[13] = 0.0f, M[14] = -1.0f, M[15] = 0.0f;*/
}

void Matrix4x4::NDCtoV(float w, float h)
{
	Identity();
	M[0] = w/2;
	M[1] = 0;
	M[2] = 0;
	M[3] = w/2;

	M[4] = 0;
	M[5] = -h/2;
	M[6] = 0;
	M[7] = h/2;
}

void Matrix4x4::PtoV(float Vw, float Vh, float Ww, float Wh)
{
	
	Identity();
	M[0] = (Vw / Ww);
	M[1] = 0;
	M[2] = 0;
	M[3] = (Vw / 2);

	M[4] = 0;
	M[5] = -(Vh / Wh);
	M[6] = 0;
	M[7] = (Vh / 2);
}

void Matrix4x4::Transpose(void)
{
	Matrix4x4 copy = *this;
	copy.M[0*4 + 0] = M[0*4 + 0];
	copy.M[0*4 + 1] = M[1*4 + 0];
	copy.M[0*4 + 2] = M[2*4 + 0];
	copy.M[0*4 + 3] = M[3*4 + 0];

	copy.M[1*4 + 0] = M[0*4 + 1];
	copy.M[1*4 + 1] = M[1*4 + 1];
	copy.M[1*4 + 2] = M[2*4 + 1];
	copy.M[1*4 + 3] = M[3*4 + 1];

	copy.M[2*4 + 0] = M[0*4 + 2];
	copy.M[2*4 + 1] = M[1*4 + 2];
	copy.M[2*4 + 2] = M[2*4 + 2];
	copy.M[2*4 + 3] = M[3*4 + 2];

	copy.M[3*4 + 0] = M[0*4 + 3];
	copy.M[3*4 + 1] = M[1*4 + 3];
	copy.M[3*4 + 2] = M[2*4 + 3];
	copy.M[3*4 + 3] = M[3*4 + 3];

	*this = copy;
}
//////////

void Object::MoveMe(const Vector& d)
{
	for(unsigned i = 0; i < S->vert_count; i++)
	{
		S->VA[i*3 + 0] += d.x;
		S->VA[i*3 + 1] += d.y;
		S->VA[i*3 + 2] += d.z;
	}

	position.x += d.x;
	position.y += d.y;
	position.z += d.z;
}

/////KD_Node class functions/////
KD_Node::KD_Node(void)
{
	Left = NULL;
	Right = NULL;
	box = NULL;
	depth = 1;
}

KD_Node::KD_Node(const AABB& b)
{
	box = new AABB(b.C, b.E);
	Left = NULL;
	Right = NULL;
	depth = 1;
}

KD_Node::~KD_Node(void)
{
	if(box != NULL)
		delete box;
	if(Left != NULL)
		delete Left;
	if(Right != NULL)
		delete Right;

	objects.clear();
}
//////////

float MyMin(const float a, const float b)
{
	return a < b ? a : b;
}

float MyMax(const float a, const float b)
{
	return a > b ? a : b;
}

void RangeIntersect(Range &r1, Range &r2)
{
	float a = r1[0], b = r1[1], c = r2[0], d = r2[1];
	r1[0] = MyMax(a, c);
	r1[1] = MyMin(b, d);
}

/////Camer class functions/////
Camera::Camera(void) 
{	
	Tar = Point(0.0f, 0.0f, 0.0f);
	Pos = Point(0.0f, 0.0f, 500.0f); 
	Up = Vector(0.0f, 1.0f, 0.0f); 
	Alpha = 0.0f; 
	Beta = 0.0f;
	Distance = 50.0f;//100.0f;
	for (unsigned int i = 0; i < 16; i++)
		Mat[i] = 0.0f;
}

void Camera::Reset()
{
	Alpha = 0.0f;
	Beta  = 0.0f;

	Update();
}

//////////


float Smallest(const float& a, const float& b, const float& c)
{
	float ret = a;
	if(b < ret)
		ret = b;
	if(c < ret)
		ret = c;
	return ret;
}

float Largest(const float& a, const float& b, const float& c)
{
	float ret = a;
	if(b > ret)
		ret = b;
	if(c > ret)
		ret = c;
	return ret;
}

void GenerateKDTree(void)
{
	float xmin, xmax, ymin, ymax, zmin, zmax;
	root_node = new KD_Node();
	for(unsigned i = 0; i < object_list.size(); i++)
	{
		for(unsigned j = 0; j < object_list[i]->S->face_count; j++)
		{
			unsigned short ind1 = object_list[i]->S->FA[j*3 + 0];
			unsigned short ind2 = object_list[i]->S->FA[j*3 + 1];
			unsigned short ind3 = object_list[i]->S->FA[j*3 + 2];
			Point p1(object_list[i]->S->VA[ind1*3 + 0], object_list[i]->S->VA[ind1*3 + 1], object_list[i]->S->VA[ind1*3 + 2]);
			Point p2(object_list[i]->S->VA[ind2*3 + 0], object_list[i]->S->VA[ind2*3 + 1], object_list[i]->S->VA[ind2*3 + 2]);
			Point p3(object_list[i]->S->VA[ind3*3 + 0], object_list[i]->S->VA[ind3*3 + 1], object_list[i]->S->VA[ind3*3 + 2]);

			Triangle T(p1, p2, p3);
			T.col = Color(object_list[i]->color.R, object_list[i]->color.G, object_list[i]->color.B);
			root_node->objects.push_back(T);
			float minx = Smallest(p1.x, p2.x, p3.x),
				  maxx = Largest(p1.x, p2.x, p3.x),
				  miny = Smallest(p1.y, p2.y, p3.y),
				  maxy = Largest(p1.y, p2.y, p3.y),
				  minz = Smallest(p1.z, p2.z, p3.z),
				  maxz = Largest(p1.z, p2.z, p3.z);
			if(i == 0 && j == 0)
			{
				xmin = minx, xmax = maxx;
				ymin = miny, ymax = maxy;
				zmin = minz, zmax = maxz;
			}
			else
			{
				xmin = MyMin(xmin, minx), xmax = MyMax(xmax, maxx);
				ymin = MyMin(ymin, miny), ymax = MyMax(ymax, maxy);
				zmin = MyMin(zmin, minz), zmax = MyMax(zmax, maxz);
			}

		}
	}
	Point tbc( xmax - ((xmax - xmin) / 2.0f), ymax - ((ymax - ymin) / 2.0f), zmax - ((zmax - zmin) / 2.0f));
	Point max_ex(xmax, ymax, zmax);
	Vector tbe = max_ex - tbc;
	//test_box = new AABB(tbc, tbe);
	root_node->box = new AABB(tbc, tbe);;
	SplitNode(root_node);
}

void SplitNode(KD_Node* node)
{
	if(node->objects.size() <= 1)
		return;

	float minX = node->box->C.x - node->box->E.x, maxX = node->box->C.x + node->box->E.x;
	float minY = node->box->C.y - node->box->E.y, maxY = node->box->C.y + node->box->E.y;
	float minZ = node->box->C.z - node->box->E.z, maxZ = node->box->C.z + node->box->E.z;
	float root_w = maxX - minX, root_h = maxY - minY, root_d = maxZ - minZ;
	float SA_Root = SurfaceArea(root_w, root_h, root_d);
	std::vector<Event> x_list, y_list, z_list;
	//generate the event list for each axis
	for(unsigned axis = 0; axis < 3; axis++) //for each axis, examine every triangle
	{
		for(unsigned i = 0; i < node->objects.size(); i++) //find the min/max value of each triangle for the current axis
		{
			float minV, maxV;
			Event E2;
			bool event_2 = false;
			Event E;
			E.Type[STARTING] = 0, E.Type[ENDING] = 0, E.Type[COPLANAR] = 0;
			switch(axis)
			{
			case 0: //x-axis
				{
					minV = Smallest(node->objects[i].P.x, node->objects[i].Q.x, node->objects[i].R.x);
					maxV = Largest(node->objects[i].P.x, node->objects[i].Q.x, node->objects[i].R.x);
					if(minV == maxV)
					{
						E.Type[COPLANAR] = 1;
						E.Value = minV;
					}
					else
					{
						E.Type[STARTING] = 1;
						E.Value = minV;
						E2.Type[STARTING] = 0;
						E2.Type[ENDING] = 1;
						E2.Type[COPLANAR] = 0;
						E2.Value = maxV;
						event_2 = true;
					}

					if(x_list.empty()) //if the event list is empty, add this first value in
					{
						x_list.push_back(E);
						if(event_2 == true)
							x_list.push_back(E2);
					}
					else //if not, find where it belongs(smallest to largest order)
					{
						std::vector<Event>::iterator it = x_list.begin(), it2;
						while(it != x_list.end())
						{
							if(it->Value >= E.Value) //find if the value is already present, or the first larger value
								break;
							it++;
						}
						if(it == x_list.end()) //if value is largest, add to end of list
						{
							x_list.push_back(E);
							if(event_2 == true)
								x_list.push_back(E2);
						}
						else if(it->Value == E.Value) //if value already present, increase the counters
						{
							it->Type[STARTING] += E.Type[STARTING];
							it->Type[ENDING] += E.Type[ENDING];
							it->Type[COPLANAR] += E.Type[COPLANAR];
						}
						else //if value not present, insert it before the next largest present value
							x_list.insert(it, E);

						if(event_2 == true)
						{
							//std::vector<Event>::iterator it = x_list.begin();
							it2 = x_list.begin();
							while(it2 != x_list.end())
							{
								if(it2->Value >= E2.Value) //find if the value is already present, or the first larger value
									break;
								it2++;
							}
							if(it2 == x_list.end()) //if value is largest, add to end of list
								x_list.push_back(E2);
							else if(it2->Value == E2.Value) //if value already present, increase the counters
							{
								it2->Type[STARTING] += E2.Type[STARTING];
								it2->Type[ENDING] += E2.Type[ENDING];
								it2->Type[COPLANAR] += E2.Type[COPLANAR];
							}
							else //if value not present, insert it before the next largest present value
								x_list.insert(it2, E2);
						}
					}
					break;
				}
			case 1: //y-axis
				{
					minV = Smallest(node->objects[i].P.y, node->objects[i].Q.y, node->objects[i].R.y);
					maxV = Largest(node->objects[i].P.y, node->objects[i].Q.y, node->objects[i].R.y);
					if(minV == maxV)
					{
						E.Type[COPLANAR] = 1;
						E.Value = minV;
					}
					else
					{
						E.Type[STARTING] = 1;
						E.Value = minV;
						E2.Type[STARTING] = 0;
						E2.Type[ENDING] = 1;
						E2.Type[COPLANAR] = 0;
						E2.Value = maxV;
						event_2 = true;
					}

					if(y_list.empty()) //if the event list is empty, add this first value in
					{
						y_list.push_back(E);
						if(event_2 == true)
							y_list.push_back(E2);
					}
					else //if not, find where it belongs(smallest to largest order)
					{
						std::vector<Event>::iterator it = y_list.begin(), it2;
						while(it != y_list.end())
						{
							if(it->Value >= E.Value) //find if the value is already present, or the first larger value
								break;
							it++;
						}
						if(it == y_list.end()) //if value is largest, add to end of list
						{
							y_list.push_back(E);
							if(event_2 == true)
								y_list.push_back(E2);
						}
						else if(it->Value == E.Value) //if value already present, increase the counters
						{
							it->Type[STARTING] += E.Type[STARTING];
							it->Type[ENDING] += E.Type[ENDING];
							it->Type[COPLANAR] += E.Type[COPLANAR];
						}
						else //if value not present, insert it before the next largest present value
							y_list.insert(it, E);

						if(event_2 == true)
						{
							//std::vector<Event>::iterator it = y_list.begin();
							it2 = y_list.begin();
							while(it2 != y_list.end())
							{
								if(it2->Value >= E2.Value) //find if the value is already present, or the first larger value
									break;
								it2++;
							}
							if(it2 == y_list.end()) //if value is largest, add to end of list
								y_list.push_back(E2);
							else if(it2->Value == E2.Value) //if value already present, increase the counters
							{
								it2->Type[STARTING] += E2.Type[STARTING];
								it2->Type[ENDING] += E2.Type[ENDING];
								it2->Type[COPLANAR] += E2.Type[COPLANAR];
							}
							else //if value not present, insert it before the next largest present value
								y_list.insert(it2, E2);
						}
					}
					break;
				}
			case 2: //z-axis
				{
					minV = Smallest(node->objects[i].P.z, node->objects[i].Q.z, node->objects[i].R.z);
					maxV = Largest(node->objects[i].P.z, node->objects[i].Q.z, node->objects[i].R.z);
					if(minV == maxV)
					{
						E.Type[COPLANAR] = 1;
						E.Value = minV;
					}
					else
					{
						E.Type[STARTING] = 1;
						E.Value = minV;
						E2.Type[STARTING] = 0;
						E2.Type[ENDING] = 1;
						E2.Type[COPLANAR] = 0;
						E2.Value = maxV;
						event_2 = true;
					}

					if(z_list.empty()) //if the event list is empty, add this first value in
					{
						z_list.push_back(E);
						if(event_2 == true)
							z_list.push_back(E2);
					}
					else //if not, find where it belongs(smallest to largest order)
					{
						std::vector<Event>::iterator it = z_list.begin(), it2;
						while(it != z_list.end())
						{
							if(it->Value >= E.Value) //find if the value is already present, or the first larger value
								break;
							it++;
						}
						if(it == z_list.end()) //if value is largest, add to end of list
						{
							z_list.push_back(E);
							if(event_2 == true)
								z_list.push_back(E2);
						}
						else if(it->Value == E.Value) //if value already present, increase the counters
						{
							it->Type[STARTING] += E.Type[STARTING];
							it->Type[ENDING] += E.Type[ENDING];
							it->Type[COPLANAR] += E.Type[COPLANAR];
						}
						else //if value not present, insert it before the next largest present value
							z_list.insert(it, E);

						if(event_2 == true)
						{
							//std::vector<Event>::iterator it = z_list.begin();
							it2 = z_list.begin();
							while(it2 != z_list.end())
							{
								if(it2->Value >= E2.Value) //find if the value is already present, or the first larger value
									break;
								it2++;
							}
							if(it2 == z_list.end()) //if value is largest, add to end of list
								z_list.push_back(E2);
							else if(it2->Value == E2.Value) //if value already present, increase the counters
							{
								it2->Type[STARTING] += E2.Type[STARTING];
								it2->Type[ENDING] += E2.Type[ENDING];
								it2->Type[COPLANAR] += E2.Type[COPLANAR];
							}
							else //if value not present, insert it before the next largest present value
								z_list.insert(it2, E2);
						}
					}
					break;
				}
			}
		}
	}

	unsigned split_axis = 0;
	float split_val = x_list[0].Value, lowest_cost = node->objects.size();
	//event lists generated, use them to determin the lowest cost, the plane to split with and the value to split at
	for(unsigned axis = 0; axis < 3; axis++)
	{
		int Ns = 0, Ne = 0, Nc = 0, Tl = 0, Tr = node->objects.size(), Tc = 0;
		switch(axis)
		{
		case 0: //x-axis
			{
				for(unsigned i = 0; i < x_list.size(); i++)
				{
					int Ns1 = Ns, Nc1 = Nc;
					Ns = x_list[i].Type[STARTING];
					Ne = x_list[i].Type[ENDING];
					Nc = x_list[i].Type[COPLANAR];
					Tl += (Ns1 + Nc1);
					Tr -= (Ne + Nc);
					Tc = Nc;
					float width_L = x_list[i].Value - minX, width_R = root_w - width_L;
					float SAL = SurfaceArea(width_L, root_h, root_d), SAR = SurfaceArea(width_R, root_h, root_d);
					float cost = ((Tl*SAL + Tr*SAR) / SA_Root) + Tc;
					if(cost < lowest_cost)
					{
						lowest_cost = cost;
						split_axis = axis;
						split_val = x_list[i].Value;
					}
				}
				break;
			}
		case 1: //y-axis
			{
				for(unsigned i = 0; i < y_list.size(); i++)
				{
					int Ns1 = Ns, Nc1 = Nc;
					Ns = y_list[i].Type[STARTING];
					Ne = y_list[i].Type[ENDING];
					Nc = y_list[i].Type[COPLANAR];
					Tl += (Ns1 + Nc1);
					Tr -= (Ne + Nc);
					Tc = Nc;
					float height_L = y_list[i].Value - minY, height_R = root_h - height_L;
					float SAL = SurfaceArea(root_w, height_L, root_d), SAR = SurfaceArea(root_w, height_R, root_d);
					float cost = ((Tl*SAL + Tr*SAR) / SA_Root) + Tc;
					if(cost < lowest_cost)
					{
						lowest_cost = cost;
						split_axis = axis;
						split_val = y_list[i].Value;
					}
				}
				break;
			}
		case 2: //z-axis
			{
				for(unsigned i = 0; i < z_list.size(); i++)
				{
					int Ns1 = Ns, Nc1 = Nc;
					Ns = z_list[i].Type[STARTING];
					Ne = z_list[i].Type[ENDING];
					Nc = z_list[i].Type[COPLANAR];
					Tl += (Ns1 + Nc1);
					Tr -= (Ne + Nc);
					Tc = Nc;
					float depth_L = z_list[i].Value - minZ, depth_R = root_d - depth_L;
					float SAL = SurfaceArea(root_w, root_h, depth_L), SAR = SurfaceArea(root_w, root_h, depth_R);
					float cost = ((Tl*SAL + Tr*SAR) / SA_Root) + Tc;
					if(cost < lowest_cost)
					{
						lowest_cost = cost;
						split_axis = axis;
						split_val = z_list[i].Value;
					}
				}
				break;
			}
		}
	}

	//if the lowest cost is the same as not splitting, don't split
	if(lowest_cost >= node->objects.size())
	{
		node->Left = NULL;
		node->Right = NULL;
		return;
	}

	//else, split the AABB at the calculated plane
	float LW, LH, LD, RW, RH, RD;
	//Point root_center = node->box->C;
	
	switch(split_axis)
	{
	case 0: //x-axis
		{
			LH = root_h, RH = root_h;
			LD = root_d, RD = root_d;
			LW = split_val - minX, RW = root_w - LW;
			AABB* left_box = new AABB(Point(minX + (LW/2.0f), node->box->C.y, node->box->C.z),
				                      Vector(LW/2.0f, LH/2.0f, LD/2.0f));
			AABB* right_box = new AABB(Point(maxX - (RW/2.0f), node->box->C.y, node->box->C.z),
				                       Vector(RW/2.0f, RH/2.0f, RD/2.0f));
			node->Left = new KD_Node(*left_box);
			node->Left->depth = node->depth + 1;
			node->Right = new KD_Node(*right_box);
			node->Right->depth = node->depth + 1;
			break;
		}
	case 1: //y-axis
		{
			LW = root_w, RW = root_w;
			LD = root_d, RD = root_d;
			LH = split_val - minY, RH = root_h - LH;
			AABB* left_box = new AABB(Point(node->box->C.x, minY + (LH/2.0f), node->box->C.z),
				                      Vector(LW/2.0f, LH/2.0f, LD/2.0f));
			AABB* right_box = new AABB(Point(node->box->C.x, maxY - (RH/2.0f), node->box->C.z),
				                       Vector(RW/2.0f, RH/2.0f, RD/2.0f));
			node->Left = new KD_Node(*left_box);
			node->Left->depth = node->depth + 1;
			node->Right = new KD_Node(*right_box);
			node->Right->depth = node->depth + 1;
			break;
		}
	case 2: //z-axis
		{
			LW = root_w, RW = root_w;
			LH = root_h, RH = root_h;
			LD = split_val - minZ, RD = root_d - LD;
			AABB* left_box = new AABB(Point(node->box->C.x, node->box->C.y, minZ + (LD/2.0f)),
				                      Vector(LW/2.0f, LH/2.0f, LD/2.0f));
			AABB* right_box = new AABB(Point(node->box->C.x, node->box->C.y, maxZ - (RD/2.0f)),
				                       Vector(RW/2.0f, RH/2.0f, RD/2.0f));
			node->Left = new KD_Node(*left_box);
			node->Left->depth = node->depth + 1;
			node->Right = new KD_Node(*right_box);
			node->Right->depth = node->depth + 1;
			break;
		}
	}

	//place the triangles in their corresponding boxes
	for(unsigned i = 0; i < node->objects.size(); i++)
	{
		Triangle t = node->objects[i];
		int points_left = 0, points_right = 0, points_on = 0;
		if(t.P.IsIn(*node->Left->box))
		{
			if((t.P.IsOn(*node->Left->box) && (t.P.IsOn(*node->Right->box))))
				++points_on;
			else
				++points_left;
		}
		else if(t.P.IsIn(*node->Right->box))
		{
			if((t.P.IsOn(*node->Left->box) && (t.P.IsOn(*node->Right->box))))
				++points_on;
			else
				++points_right;
		}
		else
		{
			if(t.P.Distance(*node->Left->box) < t.P.Distance(*node->Right->box))
				++points_left;
			else
				++points_right;
		}

		if(t.Q.IsIn(*node->Left->box))
		{
			if((t.Q.IsOn(*node->Left->box) && (t.Q.IsOn(*node->Right->box))))
				++points_on;
			else
				++points_left;
		}
		else if(t.Q.IsIn(*node->Right->box))
		{
			if((t.Q.IsOn(*node->Left->box) && (t.Q.IsOn(*node->Right->box))))
				++points_on;
			else
				++points_right;
		}
		else
		{
			if(t.Q.Distance(*node->Left->box) < t.Q.Distance(*node->Right->box))
				++points_left;
			else
				++points_right;
		}

		if(t.R.IsIn(*node->Left->box))
		{
			if((t.R.IsOn(*node->Left->box) && (t.R.IsOn(*node->Right->box))))
				++points_on;
			else
				++points_left;
		}
		else if(t.R.IsIn(*node->Right->box))
		{
			if((t.R.IsOn(*node->Left->box) && (t.R.IsOn(*node->Right->box))))
				++points_on;
			else
				++points_right;
		}
		else
		{
			if(t.R.Distance(*node->Left->box) < t.R.Distance(*node->Right->box))
				++points_left;
			else
				++points_right;
		}

		if(points_left == 0)
			node->Right->objects.push_back(t); //add to right side if all points on the right or the edge
		else if(points_left == 1)
		{
			if(points_on == 0 || points_on == 1) 
			{
				Triangle t2 = t;
				node->Right->objects.push_back(t);
				node->Left->objects.push_back(t2);
			}
			else //if(points_on == 2)
				node->Left->objects.push_back(t);
		}
		else if(points_left == 2)
		{
			if(points_on == 1)
				node->Left->objects.push_back(t);
			else //if(points_on == 0)
			{
				Triangle t2 = t;
				node->Right->objects.push_back(t);
				node->Left->objects.push_back(t2);
			}

		}
		else //if(points_left == 3)
			node->Left->objects.push_back(t);
			
	}
	int left_size = node->Left->objects.size(), right_size = node->Right->objects.size();
	if( (node->Left->objects.empty()) || (node->Right->objects.empty()) )
	{
		delete node->Left;
		node->Left = NULL;
		delete node->Right;
		node->Right = NULL;
		return;
	}

	if(node->depth == MAX_TREE_DEPTH) //dont split anymore when the max tree depth is reached
		return;

	//now split the left and right boxes until an exit condition is met
	SplitNode(node->Left);
	SplitNode(node->Right);
	node->objects.clear();
}


float SurfaceArea(float w, float h, float d)
{
	return 2.0f * (w*h + w*d + h*d);
}

void NodeRender(KD_Node* node, int depth)
{
	if(depth == 0)
		return;

	if(node->depth == depth)
	{
		node->box->DrawSelf();
	}
	else
	{
		node->box->DrawSelf();
		if(node->Right != NULL)
			NodeRender(node->Right, depth);
		if(node->Left != NULL)
			NodeRender(node->Left, depth);
	}
}

void DrawNode(KD_Node* node)
{
	if(node->Right == NULL && node->Left == NULL)
	{
		for(unsigned i = 0; i < node->objects.size(); i++)
			node->objects[i].DrawSelf();
	}
	else
	{
		if(node->Right != NULL)
			DrawNode(node->Right);

		if(node->Left != NULL)
			DrawNode(node->Left);
	}
}

void Camera::CalculateFrustum(void)
{
	Point p = Pos;
	Vector v = Dir, up = Up;
	v.NormalizeMe();
	Vector w = v.Cross(up); //camera's right vector

	Point near_center = p + (v * NEAR_VAL),
		  far_center = p + (v * FAR_VAL);

	float aspect_ratio = float(wind_width) / float(wind_height);
	float H_near = 2.0f * tan(FOV_Y / 2.0f) * NEAR_VAL,
		  W_near = H_near * aspect_ratio;
	float H_far = 2.0f * tan(FOV_Y / 2.0f) * FAR_VAL,
		  W_far = H_far * aspect_ratio;

	Point NTL = near_center + (up * (H_near/2.0f)) - (w * (W_near/2.0f));
	Point NTR = near_center + (up * (H_near/2.0f)) + (w * (W_near/2.0f));
	Point NBL = near_center - (up * (H_near/2.0f)) - (w * (W_near/2.0f));
	Point NBR = near_center - (up * (H_near/2.0f)) + (w * (W_near/2.0f));

	Point FTL = far_center + (up * (H_far/2.0f)) - (w * (W_far/2.0f));
	Point FTR = far_center + (up * (H_far/2.0f)) + (w * (W_far/2.0f));
	Point FBL = far_center - (up * (H_far/2.0f)) - (w * (W_far/2.0f));
	Point FBR = far_center - (up * (H_far/2.0f)) + (w * (W_far/2.0f));

	Frustum.clear();
	Vector v1 = NBL - NTL, v2 = NTR - NTL, norm = v1.Cross(v2);
	norm.NormalizeMe();
	Frustum.push_back(Plane(NTL, norm)); //front plane

	v1 = FBR - FTR, v2 = FTL - FTR, norm = v1.Cross(v2);
	norm.NormalizeMe();
	Frustum.push_back(Plane(FTR, norm)); //back plane

	v1 = NTL - FTL, v2 = FTR - FTL, norm = v1.Cross(v2);
	norm.NormalizeMe();
	Frustum.push_back(Plane(FTL, norm)); //top plane

	v1 = NBR - FBR, v2 = FBL - FBR, norm = v1.Cross(v2);
	norm.NormalizeMe();
	Frustum.push_back(Plane(FBR, norm)); //bottom plane

	v1 = FTL - NTL, v2 = NBL - NTL, norm = v1.Cross(v2);
	norm.NormalizeMe();
	Frustum.push_back(Plane(NTL, norm)); //left plane

	v1 = NBR - NTR, v2 = FTR - NTR, norm = v1.Cross(v2);
	norm.NormalizeMe();
	Frustum.push_back(Plane(NTR, norm)); //right plane
}


/////BVH_Node class functions/////
BVH_Node::BVH_Node(void)
{
	Left = NULL;
	Right = NULL;
	Parent = NULL;
	Box = NULL;
	obj = NULL;
	type = LEAF;
}

BVH_Node::BVH_Node(const BVH_Node& n)
{
	if(n.Left != NULL)
		Left = new BVH_Node(*(n.Left));
	else
		Left = NULL;

	if(n.Right != NULL)
		Right = new BVH_Node(*(n.Right));
	else
		Right = NULL;

	Parent = n.Parent;
	if(n.Box != NULL)
		Box = new AABB(n.Box->C, n.Box->E);
	else
		Box = NULL;
	if(n.obj != NULL)
		obj = n.obj;
	else
		obj = NULL;
	type = n.type;
}

BVH_Node::~BVH_Node(void)
{
	if(Left != NULL)
		delete Left;
	Left = NULL;

	if(Right != NULL)
		delete Right;
	Right = NULL;

	Parent = NULL;
	if(Box != NULL)
		delete Box;
	Box = NULL;
	//obj = NULL;
}
//////////

void GenerateBVHTree(void)
{
	bvh_nodes.clear();
	bvh_root = NULL;
	for(unsigned i = 0; i < object_list.size(); i++) //create boxes for every object
	{
		float xmin, xmax, ymin, ymax, zmin, zmax;
		for(unsigned j = 0; j < object_list[i]->S->vert_count; j++)
		{
			Point p(object_list[i]->S->VA[j*3 + 0], object_list[i]->S->VA[j*3 + 1], object_list[i]->S->VA[j*3 + 2]);
			if(j == 0)
			{
				xmin = p.x, xmax = p.x,
				ymin = p.y, ymax = p.y,
				zmin = p.z, zmax = p.z;
			}
			
			if(p.x < xmin)
				xmin = p.x;
			if(p.x > xmax)
				xmax = p.x;
			if(p.y < ymin)
				ymin = p.y;
			if(p.y > ymax)
				ymax = p.y;
			if(p.z < zmin)
				zmin = p.z;
			if(p.z > zmax)
				zmax = p.z;
		}
		Point pmax(xmax, ymax, zmax);
		Point c(xmax - ((xmax-xmin) / 2.0f), ymax - ((ymax-ymin) / 2.0f), zmax - ((zmax-zmin) / 2.0f));
		Vector e = pmax - c;
		BVH_Node *node = new BVH_Node();
		node->Box = new AABB(c, e);
		bvh_nodes.push_back(node);
	}

	if(rg_num == BOTTOM_UP)
	{
		int node_count = bvh_nodes.size();
		while(node_count > 1)
		{
			std::vector<BVH_Node*>::iterator it1, it2, n1, n2;
			float min_cost;
			for(it1 = bvh_nodes.begin(); it1 != bvh_nodes.end(); it1++)
			{
				it2 = it1;
				++it2;
				if(it1 == bvh_nodes.begin())
				{
					n1 = it1;
					n2 = it2;
					min_cost = BVH_Cost(*(*n1)->Box, *(*n2)->Box);
				}
				for(; it2 != bvh_nodes.end(); it2++)
				{
					if((*it2)->Parent != NULL)
						continue;
					float current_cost = BVH_Cost(*(*it1)->Box, *(*it2)->Box);
					if(current_cost < min_cost)
					{
						n1 = it1;
						n2 = it2;
						min_cost = current_cost;
					}
				}
			}
			BVH_Node *node2 = new BVH_Node();
			node2->Left = new BVH_Node(*(*n1));
			//node2->Left->obj = (*n1)->obj;
			node2->Left->Parent = node2;

			node2->Right = new BVH_Node(*(*n2));
			//node2->Right->obj = (*n2)->obj;
			node2->Right->Parent = node2;

			node2->Box = new AABB(*(*n1)->Box, *(*n2)->Box);
			node2->type = NODE;
			//node2->obj = -1;

			bvh_nodes.erase(n2);
			bvh_nodes.erase(n1);
			bvh_nodes.push_back(node2);
			node_count = bvh_nodes.size();
		}

		bvh_root = bvh_nodes.back();
	}
	else if(rg_num == INSERTION)
	{
		for(unsigned i = 0; i < bvh_nodes.size(); i++)
			InsertNode(bvh_nodes[i]);
	}

	//balance the tree after generation
	BalanceTree();
}

void DrawBVHLeaves(const BVH_Node* n)
{
	if(n->type == LEAF)
		n->Box->DrawSelf();

	else
	{
		if(n->Left != NULL)
			DrawBVHLeaves(n->Left);

		if(n->Right != NULL)
			DrawBVHLeaves(n->Right);
	}
}

float BVH_Cost(const AABB &b1, const AABB &b2)
{
	AABB b3(b1, b2);
	float w = b3.E.x * 2.0f,
		  h = b3.E.y * 2.0f,
		  d = b3.E.z * 2.0f;
	return SurfaceArea(w, h, d);
}

void InsertNode(BVH_Node* n)
{
	if(bvh_root == NULL) //no root, this node is now the root
	{
		bvh_root = n;
		return;
	}

	BVH_Node* node = bvh_root;
	while(node->type != LEAF) //go down the tree until a leaf node is found
	{
		float cost_left = BVH_Cost(*(n->Box), *(node->Left->Box)); //find which side is cheaper to combine the node with
		float cost_right = BVH_Cost(*(n->Box), *(node->Right->Box));
		if(cost_left <= cost_right) //if left is cheaper, go down to the left, else go down to the right
			node = node->Left;
		else
			node = node->Right;
	}

	BVH_Node* node2 = new BVH_Node(); //make a new node, it's children will be the leaf node and the insertion node
	node2->type = NODE;
	node2->Parent = node->Parent;

	if(node == bvh_root) //if the leaf node is the root, have to make the new node be the root
		bvh_root = node2;
	else if(node == node->Parent->Left) //find out which child the leaf node is so
		node->Parent->Left = node2;     //that the correct pointer can be updated
	else
		node->Parent->Right = node2;

	node2->Left = n;         //combine the given node and the chosen node into a new node,
	node2->Right = node;     //set the given and chosen nodes to its children
	n->Parent = node2;
	node->Parent = node2;

	node2->Box = new AABB(*(n->Box), *(node->Box));

	BVH_Node *boxes = node2->Parent;
	while(boxes != NULL) //go back up the tree and adjust the boxes
	{
		delete boxes->Box;
		boxes->Box = new AABB(*(boxes->Left->Box), *(boxes->Right->Box));
		boxes = boxes->Parent;
	}
	//balance the tree after insertion
	BalanceTree();
}

void RemoveNode(BVH_Node* n)
{
	if(n->type != LEAF) //only remove leaf nodes
		return;

	if(bvh_root == n)
	{
		delete n;
		bvh_root = NULL;
		return;
	}

	BVH_Node *np = n->Parent; //keep track of the node's parent and grandparent, if it has one
	BVH_Node *ngp = np->Parent;
	BVH_Node *sib = NULL;
	
	if(n == np->Left) //find the node's sibling
		sib = np->Right;
	else
		sib = np->Left;

	if(ngp != NULL) //if the node has a grandparent, adjust it's pointers
	{
		if(np == ngp->Left)
			ngp->Left = sib;
		else
			ngp->Right = sib;
		
		sib->Parent = ngp;

		delete n;
		delete np;

		while(ngp != NULL) //go back up the tree and update the boxes
		{
			delete ngp->Box;
			ngp->Box = new AABB(*(ngp->Left->Box), *(ngp->Right->Box));
			ngp = ngp->Parent;
		}
	}
	else //no grandparent to deal with
	{
		if(n == np->Left) //find which side to adjust the boxes of
		{
			np->Left = NULL;
			delete np->Box;
			np->Box = new AABB(*(np->Right->Box), *(np->Right->Box));
		}
		else
		{
			np->Right = NULL;
			delete np->Box;
			np->Box = new AABB(*(np->Left->Box), *(np->Left->Box));
		}

		delete n;
	}

	//balance the tree after removal
	BalanceTree();
}

int TreeDepth(const BVH_Node* n)
{
	if(n->type == LEAF) //no children, depth = 0
		return 0;
	else if((n->Left->type != LEAF) && (n->Right->type != LEAF)) //both children have children, take larger depth
		return int(MyMax( float(TreeDepth(n->Left)), float(TreeDepth(n->Right)) ));
	else if(n->Left->type != LEAF) //only left child has children
		return 1 + TreeDepth(n->Left);
	else if(n->Right->type != LEAF) //only right child has children
		return 1 + TreeDepth(n->Right);
	else //neither child has children(both are leaf nodes)
		return 1;
}

void BalanceNode(BVH_Node* n)
{
	if(n->type == LEAF) //leaf nodes and nodes with 2 leaf children are already balanced
		return;
	else if( (n->Left->type == LEAF) && (n->Right->type == LEAF) )
		return;

	int depthL = TreeDepth(n->Left),
		depthR = TreeDepth(n->Right);

	if(abs(depthL - depthR) < 2) //tree is already balanced
		return;

	BVH_Node* old_n = n; //keep track of the node we want to balance, as well as it's parent
	BVH_Node* old_p = n->Parent;

	if(depthL > depthR) //rotate left to right
	{
		n = n->Left;
		int dL = TreeDepth(n->Left), dR = TreeDepth(n->Right);

		if(dL <= dR) //move the child with less depth over
		{
			old_n->Left = n->Left;
			n->Left->Parent = old_n;
			n->Left = old_n;
			old_n->Parent = n;
		}
		else
		{
			old_n->Left = n->Right;
			n->Right->Parent = old_n;
			n->Right = old_n;
			old_n->Parent = n;
		}
	}
	else //rotate right-to-left
	{
		n = n->Right;
		int dL = TreeDepth(n->Left), dR = TreeDepth(n->Right);

		if(dL <= dR)
		{
			old_n->Right = n->Left;
			n->Left->Parent = old_n;
			old_n->Parent = n;
			n->Left = old_n;
		}
		else
		{
			old_n->Right = n->Right;
			n->Right->Parent = old_n;
			old_n->Parent = n;
			n->Right = old_n;
		}
	}

	//update the box of old_n, then n
	// (n's new box is dependent on the box of old_n, so old_n's box must be updated first)
	delete old_n->Box;
	old_n->Box = new AABB(*(old_n->Left->Box), *(old_n->Right->Box));
	delete n->Box;
	n->Box = new AABB(*(n->Left->Box), *(n->Right->Box));

	//if we were balancing the root, readjust the root pointer and parent pointer
	if(old_p == NULL)
	{
		n->Parent = NULL;
		bvh_root = n;
	}
	else
	{
		if(old_p->Left == old_n) //find which child pointer needs to be updated
		{
			old_p->Left = n;
			n->Parent = old_p;   //and update the node's parent pointer
		}
		else
		{
			old_p->Right = n;
			n->Parent = old_p;
		}
		//update the old parent's box
		delete old_p->Box;
		old_p->Box = new AABB(*(old_p->Left->Box), *(old_p->Right->Box));
	}
}

void BalanceHelper(BVH_Node* n)
{
	if(n->type == LEAF)
		return;

	int dL = TreeDepth(n->Left), dR = TreeDepth(n->Right);
	int diff = abs(dL - dR);
	while(diff >= 2)
	{
		BalanceNode(n);
		dL = TreeDepth(n->Left), dR = TreeDepth(n->Right);
		diff = abs(dL - dR);
	}
	BalanceHelper(n->Left);
	BalanceHelper(n->Right);
}

void BalanceTree(void)
{
	BalanceHelper(bvh_root); //balance the tree at the node
}

void BVHNodeRender(BVH_Node* node, int depth, int current_depth)
{
	if(depth == 0) //root depth = 1, render no nodes if depth is 0
		return;

	if(node->type == LEAF)
	{
		node->Box->DrawSelf();
		return;
	}

	if(current_depth == depth)
	{
		node->Box->DrawSelf();
		return;
	}
	else
	{

		BVHNodeRender(node->Left, depth, current_depth+1);
		BVHNodeRender(node->Right, depth, current_depth+1);
		return;
	}
}

Point SupportFunction(const Vector& d, const GameObj* obj)
{
	Point sp;
	float max_t = -900001.0f;
	Vector d2 = d;
	d2.NormalizeMe();

	Matrix4x4 trans, rot, sc, tr, transform, irot;
	Point pos = obj->GetPos();
	Vector scale = obj->GetScale();
	//d2.x -= pos.x, d2.y -= pos.y, d2.z -= pos.z;
	//irot.Rotate(obj->GetAxis(), obj->GetAngle());
	//irot.Transpose();
	//d2 = irot * d2;

	trans.Translate(pos.x, pos.y, pos.z);
	rot.Rotate(obj->GetAxis(), obj->GetAngle());
	sc.Scale(scale.x, scale.y, scale.z);
	tr = trans * rot;
	transform = tr * sc;

	for(unsigned i = 0; i < obj->GetVertCount(); i++) //find the point on the object that is furthest in the direction
	{
		Point p = obj->GetVertex(i);
		p = transform * p;
		float t = p.Dot(d2);
		if(t > max_t)
		{
			max_t = t;
			sp = p;
		}
	}
	return sp;
}

/////MPR class functions/////
MPR::MPR(GameObj* o0, GameObj* o1)
{
	Depth = 0.0f;
	Obj0 = o0;
	Obj1 = o1;
}

void MPR::InitialPortal(void)
{
	Vector v = Obj1->GetPos() - Obj0->GetPos(); //object_list[1]->position - object_list[0]->position;
	C = Point(v.x, v.y, v.z) + BIAS; //the center in minkowski space is the difference of the 2 centers
	//add some bias to C to prevent null cross products
	Point O(0.0f, 0.0f, 0.0f);
	Vector SV = O - C; //first support vector goes from the center to the origin

	Point SP1 = SupportFunction((SV * -1.0f), Obj0); //calculate the first 2 support points
	SP11 = SP1; //keep track of these for contact info

	Point SP2 = SupportFunction(SV, Obj1);
	SP12 = SP2;

	P1 = Point(SP2.x - SP1.x, SP2.y - SP1.y, SP2.z - SP1.z); //use the first 2 support points to calculate the first point
	SV = ((P1 - C).Cross(O - C)); //new SV is the normal of the new plane

	SP1 = SupportFunction((SV * -1.0f), Obj0); //use the new SV to find the next 2 support points
	SP21 = SP1;

	SP2 = SupportFunction(SV, Obj1);
	SP12 = SP2;

	P2 = Point(SP2.x - SP1.x, SP2.y - SP1.y, SP2.z - SP1.z); //use the new support points to calculate the second point
	SV = (P2-C).Cross(P1-C); //new SV is the normal of the plane containing P1, P2 and C

	SP1 = SupportFunction((SV * -1.0f), Obj0); //use the new SV to find the last 2 support points
	SP2 = SupportFunction(SV, Obj1);

	P3 = Point(SP2.x - SP1.x, SP2.y - SP1.y, SP2.z - SP1.z); //use the new support points to calculate the third point, we now have the initial portal
	//now that we have the 3 points, we want to make sure theyre in CCW order
	Vector TriNorm = (P2-P1).Cross(P3-P1);
	float dotprod = TriNorm.Dot(P1-C);
	if(dotprod < 0.0f) //if the normal is facing the center, swap P1 and P2
	{
		P1.Swap(P2);
		SP11.Swap(SP12);
		SP21.Swap(SP22);
	}
}

bool MPR::Discovery(void)
{
	Point O(0.0f, 0.0f, 0.0f);

	Vector OC = (O-C);
	OC.NormalizeMe();
	Triangle T(P1, P2, P3);
	Intersection inter;
	if(Ray(C, OC).Intersect(T, inter)) //origin already within the infinite volume
		return true;

	float near_zero = -0.00000009f; //minor rounding errors
	for(unsigned i = 0; i < 25; i++) //only attempt to find a better portal a finite number of times
	{
		//O is outside one of the frustum planes, find which one
		//planes = C_P1_P3, C_P2_P1, C_P3_P2
		float dist = 0.0f;
		Point *replace = NULL;
		Vector N;

		Vector N13 = (P1-C).Cross(P3-C); //normal of the plane C_P1_P3
		N13.NormalizeMe();
		dist = N13.Dot(O - P1);
		if(dist >= near_zero) //positive dist means the point is outside the plane
		{
			replace = &P2; //outside C_P1_P3, replace P2 to shift containment
			N = N13;
		}

		Vector N21 = (P2-C).Cross(P1-C); //normal of the plane C_P2_P1
		N21.NormalizeMe();
		dist = N21.Dot(O - P2);
		if(dist >= near_zero) 
		{	
			replace = &P3; //outside C_P2_P1, replace P3 to shift containment
			N = N21;
		}

		Vector N32 = (P3-C).Cross(P2-C); //normal of the plane C_P3_P2
		N32.NormalizeMe();
		dist = N32.Dot(O - P3);
		if(dist >= near_zero)
		{
			replace = &P1; //outside C_P3_P2, replace P1 to shift containment
			N = N32;
		}

		if(replace == NULL) //should never happen, somehow does
			continue;

		Point SP1 = SupportFunction((N * -1.0f), Obj0); //calculate support points
		Point SP2 = SupportFunction(N, Obj1);
		*replace = Point(SP2.x - SP1.x, SP2.y - SP1.y, SP2.z - SP1.z); //use the support points to calculate the new point

		Triangle T2(P1, P2, P3);
		if(Ray(C, OC).Intersect(T2, inter)) //origin within the infinite volume yet?
		{
			//now that we have the 3 points, we want to make sure theyre in CCW order
			Vector TriNorm = (P2-P1).Cross(P3-P1);
			float dotprod = TriNorm.Dot(P1-C);
			if(dotprod < 0.0f) //if the normal is facing the origin, swap P1 and P2
			{
				P1.Swap(P2);
				SP11.Swap(SP12); //swap these too, needed for calculating contact points
				SP21.Swap(SP22);
			}
			
			return true;
		}
	}

	return false;
}

bool MPR::Refinement(void)
{
	Point O(0.0f, 0.0f, 0.0f);
	Vector OC = O - C;
	OC.NormalizeMe();

	Intersection inter; //intersection class for the ray-triangle intersection checks

	Vector N = (P2-P1).Cross(P3-P1);
	N.NormalizeMe();

	float dist = N.Dot(O - P1);
	if(dist < 0.01f) //origin already within the frustum
	{
		Norm = N;
		Depth = abs(dist);
		return true;
	}
	else
	{
		for(unsigned i = 0; i < 25; i++) //same idea as discovery, limit the number of iterations
		{
			Vector SV = N; //new support vector is the plane's normal
			Point SP1 = SupportFunction((SV * -1.0f), Obj0);
			Point SP2 = SupportFunction(SV, Obj1);
			Point sp = Point(SP2.x - SP1.x, SP2.y - SP1.y, SP2.z - SP1.z); //new support point, but where to put it...

			Point *replace = NULL;
			//try replacing the first point with it, see what happens
			Triangle nt(sp, P2, P3);
			Vector TriNorm = (nt.Q - nt.P).Cross(nt.R - nt.P);
			float dotprod = TriNorm.Dot(nt.P - C);
			if(dotprod < 0.0f) //if the normal is facing the origin, swap P and Q
			{
				nt.P.Swap(nt.Q);
				TriNorm = (nt.Q - nt.P).Cross(nt.R - nt.P); //then update the normal
			}

			TriNorm.NormalizeMe();
			float dist1 = TriNorm.Dot(O - nt.P);
			if(dist1 < dist) //this point brings it closer
			{
				if(Ray(C, OC).Intersect(nt, inter)) //only swap this point if it keeps the origin inside the frustum
				{
					dist = dist1;
					replace = &P1;
				}
			}

			nt = Triangle(P1, sp, P3); //now see how close it gets when swapping it with P2
			TriNorm = (nt.Q - nt.P).Cross(nt.R - nt.P);
			dotprod = TriNorm.Dot(nt.Q - C);
			if(dotprod < 0.0f) //if the normal is facing the origin, swap P and Q
			{
				nt.Q.Swap(nt.P);
				TriNorm = (nt.Q - nt.P).Cross(nt.R - nt.P);
			}

			TriNorm.NormalizeMe();
			float dist2 = TriNorm.Dot(O - nt.Q);
			if(dist2 < dist) //if this point is closer, mark it for replacement
			{
				if(Ray(C, OC).Intersect(nt, inter)) //only swap this point if it keeps the origin inside the frustum
				{
					dist = dist2;
					replace = &P2;
				}
			}

			nt = Triangle(P1, P2, sp); //now try with replacing P3
			TriNorm = (nt.Q - nt.P).Cross(nt.R - nt.P);
			dotprod = TriNorm.Dot(nt.R - C);
			if(dotprod < 0.0f) //if the normal is facing the origin, swap Q and R
			{
				nt.R.Swap(nt.Q);
				TriNorm = (nt.Q - nt.P).Cross(nt.R - nt.P);
			}

			TriNorm.NormalizeMe();
			float dist3 = TriNorm.Dot(O - nt.R);
			if(dist3 < dist) //if this distance is smaller, mark it for replacement
			{
				if(Ray(C, OC).Intersect(nt, inter)) //only swap this point if it keeps the origin inside the frustum
				{
					dist = dist3;
					replace = &P3;
				}
			}

			if(replace!= NULL) //if a replacement has been found
			{
				*replace = sp; //swap the point out, and check where the new normal is facing
				TriNorm = (P2-P1).Cross(P3-P1);
				dotprod = TriNorm.Dot(P3 - C);
				if(dotprod < 0.0f) //if the normal is facing the origin, swap P1 and P2
				{
					P1.Swap(P2);
					SP11.Swap(SP12);
					SP21.Swap(SP22);
				}

				TriNorm = (P2-P1).Cross(P3-P1);
				TriNorm.NormalizeMe();
				N = TriNorm; //SV just copies N rather than calculate every iteration

				//now that the portal is closer to enveloping the origin, check if it's inside yet
				if(dist < 0.01f) //is the origin inside now?
				{
					Norm = N;
					Depth = abs(dist);
					return true;
				}
			}
			else //no new point means the triangle wont change, which means the support vector wont change
				break;
		}
	}

	Triangle t_final(P1, P2, P3);
	//if(Ray(C, OC).Intersect(t_final, inter)) //this should still be the case, but just to make sure
	//{
	if(dist < 0.01f)
	{
		Vector norm_final = (P2-P1).Cross(P3-P1);
		float dotprod = norm_final.Dot(P3 - C);
		if(dotprod < 0.0f) //if the normal is facing the origin, swap P1 and P2
		{
			P1.Swap(P2);
			SP11.Swap(SP12);
			SP21.Swap(SP22);
		}
		norm_final = (P2-P1).Cross(P3-P1);
		norm_final.NormalizeMe();
		Norm = norm_final;
		Depth = abs(dist);
		return true;
	}
	//}
	return false;
}

bool MPR::Collision(void)
{
	InitialPortal();

	if(Discovery())
	{
		if(Refinement())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void MPR::ContactInfo(void)
{
	Point O(0.0f, 0.0f, 0.0f);
	float denom = Vector(P2 - P1).Dot(P2 - P1);
	float t = (Vector(O - P1).Dot(P2 - P1)) / denom;
	Point CP1 = SP11 + Vector(SP12 - SP11) * t;
	Point CP2 = SP21 + Vector(SP22 - SP21) * t;
	std::cout << "Normal = (" << Norm.x << ", " << Norm.y << ", " << Norm.z << ")" << std::endl;
	std::cout << "Depth of Collision = " << Depth << std::endl;
	std::cout << "Contact Point 1 = (" << CP1.x << ", " << CP1.y << ", " << CP1.z << ")" << std::endl;
	std::cout << "Contact Point 2 = (" << CP2.x << ", " << CP2.y << ", " << CP2.z << ")" << std::endl;
	std::cout << std::endl;
}
//////////

Point Bias(void)
{
	float max_val = 0.2f;
	float r1 = (float)rand() / ((float)RAND_MAX / max_val); //generate a random number between 0.0 and max_val
	float r2 = (float)rand() / ((float)RAND_MAX / max_val);
	float r3 = (float)rand() / ((float)RAND_MAX / max_val);
	return Point(r1, r2, r3);
}

/////Quaternion class functions/////
Quaternion::Quaternion(float s, float x, float y, float z)
{
	S = s;
	V = Vector(x, y, z);
}

Quaternion::Quaternion(const Vector &v, float s)
{
	S = s;
	V = v;
}

Quaternion::~Quaternion(void)
{
}

Quaternion Quaternion::operator*(const Quaternion &rhs) const
{
	Quaternion ret;
	ret.S = (S * rhs.S) - V.Dot(rhs.V);

	ret.V = (rhs.V * S);
	ret.V.x += (V.x * rhs.S), ret.V.y += (V.y * rhs.S), ret.V.z += (V.z * rhs.S);
	Vector vcv = V.Cross(rhs.V);
	ret.V.x += vcv.x, ret.V.y += vcv.y, ret.V.z += vcv.z;
	ZeroClamp(ret.S);
	ZeroClamp(ret.V.x);
	ZeroClamp(ret.V.y);
	ZeroClamp(ret.V.z);
	return ret;
}

Quaternion Quaternion::operator*(const float &rhs) const
{
	Quaternion ret;
	ret.S = S * rhs;
	ret.V = V * S;
	return ret;
}

Quaternion Quaternion::operator+(const Quaternion &rhs) const
{
	Quaternion ret;
	ret.S = S + rhs.S;
	ret.V = Vector(V.x + rhs.V.x, V.y + rhs.V.y, V.z + rhs.V.z);
	return ret;
}

Quaternion Quaternion::operator-(const Quaternion &rhs) const
{
	Quaternion ret;
	ret.S = S - rhs.S;
	ret.V = Vector(V.x - rhs.V.x, V.y - rhs.V.y, V.z - rhs.V.z);
	return ret;
}

float Quaternion::Dot(const Quaternion &rhs) const
{
	float s1s2 = S * rhs.S;
	float v1_v2 = V.Dot(rhs.V);
	ZeroClamp(v1_v2);
	return s1s2 * v1_v2;
}

float Quaternion::Magnitude(void) const
{
	return sqrt(S*S + V.x*V.x + V.y*V.y + V.z*V.z);
}

Quaternion Quaternion::Inverse(void) const
{
	Quaternion ret;
	float denom = (S*S + V.x*V.x + V.y*V.y + V.z*V.z);
	ret.S = S / denom;
	ret.V = V / -denom;
	return ret;
}

void Quaternion::InvertMe(void)
{
	float denom = (S*S + V.x*V.x + V.y*V.y + V.z*V.z);
	S /= denom;
	V /= -denom;
}

void Quaternion::Identity(void)
{
	S = 1.0f;
	V = Vector(0.0f, 0.0f, 0.0f);
}

Quaternion Quaternion::Conjugate(void) const
{
	return Quaternion(V * -1.0f, S);
}

void Quaternion::ConjMe(void)
{
	V *= -1.0f;
}

void Quaternion::Rotator(const float angle, const Vector &axis)
{
	S = cos(angle / 2.0f);
	V = axis * sin(angle / 2.0f);
}
/////
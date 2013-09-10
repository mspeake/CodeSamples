/*--------------------------------------------------
Implements the Math/Geometry library and related functions
--------------------------------------------------*/
#include "Geometry.h"
#include "ObjReader.h"
#include "Core.h"

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
	N.NormalizeMe();
	Point Q = p.P;
	Vector pq = (*this - Q);
	pq.NormalizeMe();
	float dist = pq.Dot(N);
	*this = *this - (N * dist);
}

void Point::Swap(Point &p)
{
	Point temp = *this;
	*this = p;
	p = temp;
}

float Point::Distance(const Point &p) const
{
	return (*this - p).Magnitude();
}

float Point::Distance(const Line &l) const
{
	Point p = *this;
	p.Project(l);
	return (*this - p).Magnitude();
}

float Point::Distance(const LineSegment &l) const
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

float Point::Distance(const Ray &r) const
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

float Point::Distance(const Plane &p) const
{
	Point copy = *this;
	copy.Project(p);
	return (*this - copy).Magnitude();
}

float Point::Distance(const Triangle& t) const
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

float Point::Distance(const Sphere &s) const
{
	return (this->Distance(s.C)) - s.R;
}

float Point::Distance(const AABB& a) const
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

bool Point::IsContained(const Line& l) const
{
	if(this->Distance(l) == 0.0f)
		return true;
	else
		return false;
}

bool Point::IsContained(const LineSegment& l) const
{
	if(this->Distance(l) == 0.0f)
		return true;
	else
		return false;
}

bool Point::IsContained(const Ray& r) const
{
	if(this->Distance(r) == 0.0f)
		return true;
	else
		return false;
}

bool Point::IsContained(const Plane& p) const
{
	Vector v1 = *this - p.P;
	v1.NormalizeMe();
	Point p2 = *this;
	p2.Project(p);
	float dist = this->Distance(p2);
	if(abs(dist) < 0.5f)
		return true;
	float dotprod = v1.Dot(p.N);

	if(dotprod == 0.0f)
		return true;
	else if(dotprod > 0.0f && dotprod < 0.000009f) //floating point errors are evil
		return true;
	else if(dotprod < 0.0f && dotprod > -0.000009f)
		return true;
	else
		return false;
}

bool Point::IsContained(const Triangle& t) const
{
	Vector v1 = t.Q - t.P, v2 = t.R - t.P;
	Vector n = v1.Cross(v2);
	n.NormalizeMe();
	
	Plane pl(t.P, n);
	if(!this->IsContained(pl))
		return false;

	Vector nAC = v1.Cross(v2).Cross(v2); //normal to AC, pointing outside
	nAC.NormalizeMe();
	float dist = nAC.Dot(*this - t.P);
	float near_zero = -0.000000009f;
	if(dist >= near_zero) //positive dist means the point is outside the line
		return false;

	Vector nAB = (t.R - t.Q).Cross(v1).Cross(v1);
	nAB.NormalizeMe();
	dist = nAB.Dot(*this - t.P);
	if(dist >= near_zero) //positive dist means the point is outside the line
		return false;

	Vector nBC = (t.P - t.R).Cross(t.Q - t.R).Cross(t.Q - t.R);
	nBC.NormalizeMe();
	dist = nBC.Dot(*this - t.Q);
	if(dist >= near_zero) //positive dist means the point is outside the line
		return false;

	return true;
}

bool Point::IsContained(const Sphere& s) const
{
	if(this->Distance(s) <= 0.0f)
		return true;
	else
		return false;
}
bool Point::IsContained(const AABB &a) const
{
	if(this->Distance(a) == 0.0f)
		return true;
	else
		return false;
}

bool Point::IsOn(const AABB& a) const
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

bool Point::IsIn(const AABB& a) const
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
	return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
}
float Vector::Magnitude(void) const
{
	return sqrt((x*x) + (y*y) + (z*z));
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

float Line::Distance(const Line &l) const
{
	Point Q1 = P, Q2 = l.P;
	Vector V1 = V, V2 = l.V, u0 = Q1 - Q2;
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

float Line::Distance(const Sphere &s) const
{
	Line copy = *this;
	Point center = s.C;
	return (center.Distance(copy) - s.R);
}

bool Line::Intersect(const Plane& p, Intersection& inter) const
{
	float denom = (p.N.Dot(p.N));
	if(denom != 0.0f)
	{
		float t = (-p.N.Dot(p.P) - p.N.Dot(P)) / denom;
		inter.T = t;
		inter.P = P + (V * t);
		return true;
	}
	else
		return false;
}

float Line::Angle(const Line &l) const
{
	return acos(V.Dot(l.V) / (V.Magnitude() * l.V.Magnitude()));
}

float Line::Angle(const Plane &p) const
{
	return (PI / 2.0f) - acos(V.Dot(p.N) / V.Magnitude() * p.N.Magnitude());
}

bool Line::Parallel(const Line &l) const
{
	float res = V.Dot(l.V) / (V.Magnitude() * l.V.Magnitude());

	if(abs(res) == 1.0f)
		return true;
	else
		return false;
}

bool Line::Parallel(const Plane& p) const
{
	if(V.Dot(p.N) == 0.0f)
		return true;
	else
		return false;
}

bool Line::Perpendicular(const Line &l) const
{
	if(V.Dot(l.V) == 0.0f)
		return true;
	else
		return false;
}

bool Line::Perpendicular(const Plane& p) const
{
	float res = abs(V.Dot(p.N) / (V.Magnitude() * p.N.Magnitude()));

	if(res == 1.0f)
		return true;
	else
		return false;
}

bool Line::Coplanar(const Plane &p) const
{
	if(this->Parallel(p) && P.IsContained(p))
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

float LineSegment::Distance(const LineSegment& l) const
{
	Vector V1 = (Q - P), V2 = (l.Q - l.P);
	float a = V1.Dot(V1), b = V1.Dot(V2), c = V2.Dot(V2);
	float d = V1.Dot(P - l.P), e = V2.Dot(l.P - P);

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

	Point P1 = P + (V1 * s), P2 = l.P + (V2 * t);

	return P1.Distance(P2);
}

bool LineSegment::Intersect(const Plane& p, Intersection& inter) const
{
	Vector V(Q - P);

	float denom = (p.N.Dot(V));
	if(denom == 0.0f)
		return false;

	float t = (-p.N.Dot(p.P) - p.N.Dot(P)) / denom;

	if(t < 0.0f)
		return false;
	else if( t > 1.0f)
		return false;

	inter.T = t;
	inter.P = P + (V * t);
	return true;
}

bool LineSegment::Intersect(const Triangle& t, Intersection& inter) const
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

void Ray::HalfSpace(const Plane& p, Range &range) const
{
	float d1 = (P - p.P).Dot(p.N), d2 = V.Dot(p.N);

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

bool Ray::Intersect(const Plane& p, Intersection& inter) const
{
	float denom = (p.N.Dot(V));
	if(denom == 0.0f)
		return false;

	float t = (-p.N.Dot(p.P) - p.N.Dot(P)) / denom;

	if(t < 0.0f)
		return false;

	inter.T = t;
	inter.P = P + (V * t);
	return true;
}

bool Ray::Intersect(const Triangle& t, Intersection& inter) const
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

bool Ray::Intersect(const Sphere& s, Intersection& inter) const
{
	Vector PC = P - s.C;
	float R = s.R, a = V.Dot(V), b = 2.0f * (V.Dot(PC)), c = PC.Dot(PC) - (R*R);

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
			inter.P = P + (V * t);
			return true;
		}
	}
	else //2 intersections
	{
		float t1 = (-b + sqrt(disc)) / (2.0f * a), t2 = (-b - sqrt(disc)) / (2.0f * a);

		if(t1 >= 0.0f && t2 >= 0.0f)
		{
			inter.T = t1 < t2 ? t1 : t2;
			inter.P = P + (V * inter.T);
			return true;
		}
		else if(t1 >= 0.0f && t2 < 0.0f)
		{
			inter.T = t1;
			inter.P = P + (V * t1);
			return true;
		}
		else if(t1 < 0.0f && t2 >= 0.0f)
		{
			inter.T = t2;
			inter.P = P + (V * t2);
			return true;
		}
		else //t1 < 0.0f && t2 < 0.0f
			return false;
	}
}

bool Ray::Intersect(const AABB& a, Intersection& inter) const
{	
	Plane H1(a.C + Vector(0.0f, 0.0f, a.E.z), Vector(0.0f, 0.0f, 1.0f));
	Plane H2(a.C - Vector(0.0f, 0.0f, a.E.z), Vector(0.0f, 0.0f, -1.0f));
	Plane H3(a.C - Vector(a.E.x, 0.0f, 0.0f), Vector(-1.0f, 0.0f, 0.0f));
	Plane H4(a.C + Vector(a.E.x, 0.0f, 0.0f), Vector(1.0f, 0.0f, 0.0f));
	Plane H5(a.C - Vector(0.0f, a.E.y, 0.0f), Vector(0.0f, -1.0f, 0.0f));
	Plane H6(a.C + Vector(0.0f, a.E.y, 0.0f), Vector(0.0f, 1.0f, 0.0f));

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
		inter.P = P + (V * inter.T);
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

bool Triangle::Intersect(const Triangle &t, Intersection &inter) const
{
	LineSegment LS1(Q, P), LS2(R, P), LS3(R, Q),
		LS4(t.Q, t.P), LS5(t.R, t.P), LS6(t.R, t.Q);

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

float Plane::Angle(const Plane &p) const
{
	return acos(N.Dot(p.N) / N.Magnitude() * p.N.Magnitude());
}

bool Plane::Parallel(const Plane &p) const
{
	float thing = abs(N.Dot(p.N) / (N.Magnitude() * p.N.Magnitude()));

	if(thing == 1.0f)
		return true;
	else
		return false;
}

bool Plane::Perpendicular(const Plane& p) const
{
	if(N.Dot(p.N) == 0.0f)
		return true;
	else
		return false;
}

bool Plane::Coplanar(const Plane& p) const
{
	if(this->Parallel(p) && (P.IsContained(p) || p.P.IsContained(*this)))
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

bool Sphere::Intersect(const Sphere& s) const
{
	float R_Sum = R + s.R, C_Dist = C.Distance(s.C);

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

bool AABB::Intersect(const AABB& a) const
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

Point Matrix4x4::operator*(const Point& rhs) const
{
	float res[4] = {0.0f};
	for(unsigned i = 0; i < 4; i++)
	{
		float *RowI = GetRow(i);
		float sum = (*(RowI + 0) * rhs.x) + (*(RowI + 1) * rhs.y) + (*(RowI + 2) * rhs.z) + (*(RowI + 3) * rhs.w);
		res[i] = sum;
	}
	return Point(res[0], res[1], res[2]);
}

Vector Matrix4x4::operator*(const Vector& rhs) const
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

Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &rhs) const
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

Matrix4x4 Matrix4x4::operator*(const float &rhs) const
{
	Matrix4x4 ret;
	for(unsigned i = 0; i < 16; i++)
	{
		ret.M[i] = rhs * M[i];
	}
	return ret;
}

Matrix4x4 Matrix4x4::operator+(const Matrix4x4 &rhs) const
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
	Tar = Vector(0.0f, 0.0f, 0.0f);
	Pos = Point(0.0f, 0.0f, 500.0f); 
	Up = Vector(0.0f, 1.0f, 0.0f); 
	Alpha = 0.0f; 
	Beta = 0.0f;
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


float SurfaceArea(float w, float h, float d)
{
	return 2.0f * (w*h + w*d + h*d);
}

void CalculateFrustum(void)
{
	Point p = sCamera.Pos;
	Vector v = sCamera.Dir, up = sCamera.Up;
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
}


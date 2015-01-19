#include "MathLib.h"

//-----Point Class Functions-----//
Point::Point(const Vector& v) : X(v.X), Y(v.Y), Z(v.Z)
{
	W = 1.0f;
}

Point Point::Subtract(const Point& lhs, const Point& rhs)
{
	return Point(lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z);
}

Point Point::operator+(const Point& rhs) const
{
	return Point(X + rhs.X, Y + rhs.Y, Z + rhs.Z);
}

Point Point::operator+(const Vector& rhs) const
{
	return Point(X + rhs.X, Y + rhs.Y, Z + rhs.Z);
}

Vector Point::operator-(const Point& rhs) const
{
	return Vector(X - rhs.X, Y - rhs.Y, Z - rhs.Z);
}

Point Point::operator-(const Vector& rhs) const
{
	return Point(X - rhs.X, Y - rhs.Y, Z - rhs.Z);
}

bool Point::operator==(const Point& rhs) const
{
	if((X == rhs.X) && (Y == rhs.Y) && (Z == rhs.Z))
		return true;
	else
		return false;
}

void Point::operator+=(const Vector& rhs)
{
	X += rhs.X;
	Y += rhs.Y;
	Z += rhs.Z;
}

void Point::operator-=(const Vector& rhs)
{
	X -= rhs.X;
	Y -= rhs.Y;
	Z -= rhs.Z;
}

float Point::Dot(const Point& p) const
{
	return (X * p.X) + (Y * p.Y) + (Z * p.Z);
}

float Point::Dot(const Vector& v) const
{
	return (X * v.X) + (Y * v.Y) + (Z * v.Z);
}

void Point::Swap(Point& p)
{
	Point Copy(*this);
	*this = p;
	p = Copy;
}

bool Point::SameSide(const Point& p, const LineSegment& ls) const
{
	Vector L(ls.P - ls.Q);
	Vector V1((*this - ls.P).Cross(L)), V2((p - ls.P).Cross(L));
	float DotProd = ClampTo(V1.Dot(V2), 0.0f, TOLERANCE); //in case of very near zero
	if(DotProd >= 0.0f)
		return true;
	else
		return false;
}

void Point::Project(const Line& l)
{
	float Mag = l.V.Magnitude();
	float Dist = (*this - l.P).Dot(l.V / Mag);
	*this = l.P + (l.V * (Dist / Mag));
}

void Point::Project(const Plane& pl)
{
	float Mag = pl.N.Magnitude();
	float Dist = (*this - pl.P).Dot(pl.N / Mag);
	*this = *this - (pl.N * (Dist / Mag));
}

float Point::Distance(const Point& p) const
{
	Vector V(p - *this);
	return V.Magnitude();
}

float Point::Distance(const Line& l) const
{
	Point P = *this;
	P.Project(l);
	return P.Distance(*this);
}

float Point::Distance(const LineSegment& ls) const
{
	Vector V(ls.Q - ls.P);
	Line L(ls.P, V);
	Point P = *this;
	P.Project(L);

	Vector LHS = P - ls.P,
		   RHS = V / (V.Dot(V));

	float T = LHS.Dot(RHS);

	if(T < 0.0f)
		P = ls.P;
	else if(T > 1.0f)
		P = ls.Q;

	return P.Distance(*this);
}

float Point::Distance(const Ray& r) const
{
	Vector V = r.V;
	Line L(r.P, V);
	Point P = *this;
	P.Project(L);

	Vector LHS = P - r.P,
		   RHS = V / (V.Dot(V));

	float T = LHS.Dot(RHS);

	if(T < 0.0f)
		P = r.P;

	return P.Distance(*this);
}

float Point::Distance(const Plane& pl) const
{
	Point P = *this;
	P.Project(pl);
	return P.Distance(*this);
}

float Point::Distance(const Triangle& t) const
{
	Plane PL(t.Q, (t.R - t.Q).Cross(t.P - t.Q));
	Point P1 = *this;
	P1.Project(PL);

	LineSegment E1(t.P, t.Q), E2(t.Q, t.R), E3(t.R, t.P);

	if(!P1.SameSide(t.R, E1))
		return this->Distance(E1);
	else if(!P1.SameSide(t.P, E2))
		return this->Distance(E2);
	else if(!P1.SameSide(t.Q, E3))
		return this->Distance(E3);
	else
		return this->Distance(P1);
}

float Point::Distance(const Sphere& s) const
{
	return this->Distance(s.C) - s.R;
}

float Point::Distance(const AABB& a) const
{
	Point Min(a.C - a.E), Max(a.C + a.E);
	Point P1(Clamp(X, Min.X, Max.X), Clamp(Y, Min.Y, Max.Y), Clamp(Z, Min.Z, Max.Z));
	return this->Distance(P1);
}

bool Point::IsContained(const Line& l) const
{
	return l.V.IsParallel((*this - l.P));
}

bool Point::IsContained(const LineSegment& ls) const
{
	Line L(*this, (ls.Q - ls.P));
	if(this->IsContained(L))
	{
		float Numer = (*this - ls.P).Dot(L.V), 
			  Denom = L.V.Magnitude();

		if(Denom == 0.0f) //check for division by 0
			return false;

		float Res = Numer / (Denom * Denom);
		return InRange(Res, 0.0f, 1.0f); 
	}
	else
		return false;
}

bool Point::IsContained(const Ray& r) const
{
	Line L(r.P, r.V);
	if(this->IsContained(L))
	{
		float Res = (*this - r.P).Dot(r.V);

		if(Res >= 0.0f)
			return true;
		else
			return false;
	}
	else
		return false;
}

bool Point::IsContained(const Plane& p) const
{
	return p.N.IsPerpendicular((*this - p.P));
}

bool Point::IsContained(const Triangle& t) const
{	
	Plane PL = t.GetPlane();
	if(!this->IsContained(PL))
		return false; //not contained on the plane, can't be in the triangle

	LineSegment PQ(t.P, t.Q), QR(t.Q, t.R), RP(t.R, t.P);
	//represent the 3 sides as linesegments, see if this point is outside any of them

	if(!this->SameSide(t.R, PQ)) //outside of side PQ, not in triangle
		return false;

	if(!this->SameSide(t.P, QR)) //outside of side QR, not in triangle
		return false;

	if(!this->SameSide(t.Q, RP)) //outside of side RP, not in triangle
		return false;

	return true;
}

bool Point::IsContained(const Sphere& s) const
{
	float Dist = (*this - s.C).Magnitude();
	if(ClampTo(Dist, s.R, TOLERANCE) <= s.R) //consider very close to R to be close enough
		return true;
	else
		return false;
}

bool Point::IsContained(const AABB& a) const
{
	Point Min(a.C - a.E), Max(a.C + a.E);

	if(!InRange(X, Min.X, Max.X)) //X does not lie within the extents, not contained
		return false;

	if(!InRange(Y, Min.Y, Max.Y)) //Y does not lie within the extents, not contained
		return false;

	if(!InRange(Z, Min.Z, Max.Z)) //Z does not lie within the extents, not contained
		return false;

	return true;
}

bool Point::IsOn(const AABB& a) const
{
	if((X == a.C.X + a.E.X) || (X == a.C.X - a.E.X)) //sits on one of the X faces
	{
		if((Y <= a.C.Y + a.E.Y) && (Y >= a.C.Y - a.E.Y))
			return true;
		else if((Z <= a.C.Z + a.E.Z) && (Z >= a.C.Z - a.E.Z))
			return true;
	}
	if((Y == a.C.Y + a.E.Y) || (Y == a.C.Y - a.E.Y)) //sits on one of the Y faces
	{
		if((Z <= a.C.Z + a.E.Z) && (Z >= a.C.Z - a.E.Z))
			return true;
		else if((X <= a.C.X + a.E.X) && (X >= a.C.X - a.E.X))
			return true;
	}
	if((Z == a.C.Z + a.E.Z) || (Z == a.C.Z - a.E.Z)) //sits on one of the Z faces
	{
		if((X <= a.C.X + a.E.X) && (X >= a.C.X - a.E.X))
			return true;
		else if((Y <= a.C.Y + a.E.Y) && (Y >= a.C.Y - a.E.Y))
			return true;
	}

	return false;
}

bool Point::IsIn(const AABB& a) const
{
	Vector Dist = *this - a.C;
	if((Dist.X < a.E.X) && (Dist.Y < a.E.Y) &&(Dist.Z < a.E.Z))
		return true;

	return false;
}
//-----Point Class Functions-----//

//-----Vector Class Functions-----//
Vector::Vector(const Point& p) : X(p.X), Y(p.Y), Z(p.Z) 
{
	W = 0.0f;
}

Vector Vector::operator/(const float rhs) const
{
	if(rhs == 0.0f)
	{
		std::cout << "Cannot divide a vector by 0" << std::endl;
		return *this;
	}
	return Vector(X / rhs, Y / rhs, Z / rhs);
}

void Vector::operator/=(const float rhs)
{
	*this = *this / rhs;
}

Vector Vector::operator*(const float rhs) const
{
	return Vector(X * rhs, Y * rhs, Z * rhs);
}

void Vector::operator*=(const float rhs)
{
	X *= rhs;
	Y *= rhs;
	Z *= rhs;
}

bool Vector::operator==(const Vector& rhs) const
{
	if((X == rhs.X) && (Y == rhs.Y) && (Z == rhs.Z))
		return true;
	else
		return false;
}

Vector Vector::operator+(const Vector& rhs) const
{
	return Vector(X + rhs.X, Y + rhs.Y, Z + rhs.Z);
}

Vector Vector::operator-(const Vector& rhs) const
{
	return Vector(X - rhs.X, Y - rhs.Y, Z - rhs.Z);
}

void Vector::operator+=(const Vector& rhs)
{
	X += rhs.X;
	Y += rhs.Y;
	Z += rhs.Z;
}

void Vector::operator-=(const Vector& rhs)
{
	X -= rhs.X;
	Y -= rhs.Y;
	Z -= rhs.Z;
}

void Vector::NormalizeMe(void)
{
	*this /= Magnitude();
}

Vector Vector::Cross(const Vector& rhs) const
{
	return Vector((Y * rhs.Z) - (Z * rhs.Y),
		          (Z * rhs.X) - (X * rhs.Z),
				  (X * rhs.Y) - (Y * rhs.X));
}

float Vector::Dot(const Vector& rhs) const
{
	return (X * rhs.X) + (Y * rhs.Y) + (Z * rhs.Z);
}

float Vector::Dot(const Point& p) const
{
	return (X * p.X) + (Y * p.Y) + (Z * p.Z);
}

float Vector::Magnitude(void) const
{
	return sqrt((X * X) + (Y * Y) + (Z * Z));
}

bool Vector::IsZeroVector(void) const
{
	if(!NearZero(X))
		return false; //X is not 0, not the zero vector
	else if(!NearZero(Y))
		return false; //Y is not 0, not the zero vector
	else if(!NearZero(Z))
		return false; //Z is not 0, not the zero vector
	else
		return true; //all are 0, this is the zero vector
}

bool Vector::IsParallel(const Vector& v) const
{
	float Numer = this->Dot(v), Denom = this->Magnitude() * v.Magnitude();

	if(Denom == 0.0f) //check for division by 0
		return false; //(one of the vectors is zero, not parallel)

	float Res = abs(Numer / Denom); //Need to compare the absolute value to 1.0f
	if(ClampTo(Res, 1.0f, TOLERANCE) == 1.0f)
		return true; //Result was 1(or very close), these are parallel
	else
		return false;
}

bool Vector::IsPerpendicular(const Vector& v) const
{
	float Res = this->Dot(v);
	return NearZero(Res);
}
//-----Vector Class Functions-----//

//-----Line Class Functions-----//
float Line::Distance(const Line& l) const
{
	Vector u0 = P - l.P;
	float A = V.Dot(V), B = V.Dot(l.V), C = l.V.Dot(l.V), D = V.Dot(u0), E = l.V.Dot(u0);
	float S1 = 0.0f, T1 = 0.0f, Denom = (A*C) - (B*B);

	if(Denom == 0.0f)
		T1 = D / B;
	else
	{
		S1 = (B*E - C*D) / Denom;
		T1 = (A*E - B*D) / Denom;
	}

	Point P1 = P + (V*S1), P2 = l.P + (l.V*T1);
	return P1.Distance(P2);
}

float Line::Distance(const Sphere& s) const
{
	return s.C.Distance(*this) - s.R;
}

bool Line::Intersects(const Plane& p, Intersection& inter) const
{
	if(this->IsParallel(p))
		return false;
	
	float Numer = -(p.N.Dot(p.P)) - p.N.Dot(P), Denom = p.N.Dot(V);
	inter.T = Numer / Denom;     //no need to check for 0 division, the above call
	inter.P = P + (V * inter.T); //to IsParallel compares the same value to 0

	return true;
}

float Line::Angle(const Line& l) const
{
	float M1 = V.Magnitude(), M2 = l.V.Magnitude();
	float Denom = M1 * M2;

	if(Denom == 0.0f) //a zero vector would cause this to be 0
		return 0.0f;
	else
		return acos(V.Dot(l.V) / Denom);
}

float Line::Angle(const Plane& p) const
{
	float M1 = V.Magnitude(), M2 = p.N.Magnitude();
	float Denom = M1 * M2;

	if(Denom == 0.0f) //a zero vector would cause this to be 0
		return float(PI / 2.0f);
	else
		return float((PI / 2.0f) - acos(V.Dot(p.N) / Denom));
}

bool Line::IsParallel(const Line& l) const
{
	float M1 = V.Magnitude(), M2 = l.V.Magnitude();
	float Denom = M1 * M2;

	if(Denom == 0.0f) //a zero vector would cause this to be 0
		return false;
	else
	{
		float Res = abs(V.Dot(l.V) / Denom);
		if(ClampTo(Res, 1.0f, TOLERANCE) == 1.0f) //close enough to 1
			return true;
		else
			return false;
	}
}

bool Line::IsParallel(const Plane& p) const
{
	float Res = V.Dot(p.N);
	return NearZero(Res); //parallel if the dot product is 0, or close enough
}

bool Line::IsPerpendicular(const Line& l) const
{
	return l.V.IsPerpendicular(V);
}

bool Line::IsPerpendicular(const Plane& p) const
{
	return p.N.IsParallel(V);
}

bool Line::IsCoplanar(const Plane& p) const
{
	if(this->IsParallel(p) && P.IsContained(p))
		return true;
	else
		return false;
}
//-----Line Class Functions-----//

//-----LineSegment Class Functions-----//
float LineSegment::Distance(const LineSegment& ls) const
{
	Vector V1 = (Q - P), V2 = (ls.Q - ls.P), V3 = (P - ls.P);

	float A = V1.Dot(V1), B = V1.Dot(V2), C = V2.Dot(V2), D = V1.Dot(V3), E = V2.Dot(V3);
	float S1 = 0.0f, T1 = 0.0f, Denom = (A*C) - (B*B);

	if(Denom != 0.0f)
		S1 = Clamp((B*E - C*D) / Denom, 0.0f, 1.0f);

	T1 = (E + S1*B) / C;

	if(T1 < 0.0f)
	{
		T1 = 0.0f;
		S1 = Clamp(-C / A, 0.0f, 1.0f);
	}
	else if(T1 > 1.0f)
	{
		T1 = 1.0f;
		S1 = Clamp((B - C) / A, 0.0f, 1.0f);
	}

	Point P1 = P + (V1*S1), P2 = ls.P + (V2*T1);
	return P1.Distance(P2);
}

bool LineSegment::Intersects(const Plane& p, Intersection& inter) const
{
	Line L(P, Q - P);
	if(L.IsParallel(p))
		return false;

	float Numer = -(p.N.Dot(p.P)) - p.N.Dot(P), Denom = p.N.Dot(L.V);
	float T = Numer / Denom; //the above call to IsParallel would pass if Denom was 0

	if(!InRange(T, 0.0f, 1.0f))
		return false;

	inter.T = T;
	inter.P = P + (L.V * T);

	return true;
}

bool LineSegment::Intersects(const Triangle& t, Intersection& inter) const
{
	Plane PL = t.GetPlane();
	if(this->Intersects(PL, inter))
	{
		if(inter.P.IsContained(t))
			return true;
		else
			return false;
	}
	else
		return false;
}
//-----LineSegment Class Functions-----//

//-----Ray Class Functions-----//
Range Ray::GetHalfSpace(const Plane& p) const
{
	float D1 = (P - p.P).Dot(p.N), D2 = V.Dot(p.N);

	if((D2 == 0.0f) || (D1 > 0.0f))
		return Range(0.0f, 0.0f);

	float T = -D1 / D2;

	if(D2 < 0.0f)
		return Range(T, FLT_MAX);
	else if(D2 > 0.0f)
		return Range(FLT_MIN, T);
	else
		return Range(FLT_MIN, FLT_MAX);
}

bool Ray::Intersects(const Plane& p, Intersection& inter) const
{
	Line L(P, V);
	if(L.IsParallel(p))
		return false;

	float Numer = -(p.N.Dot(p.P)) - p.N.Dot(P), Denom = p.N.Dot(V);
	float T = Numer / Denom; //the above call to IsParallel would pass if Denom was 0

	if(ClampTo(T, 0.0f, TOLERANCE) < 0.0f)
		return false;

	inter.T = T;
	inter.P = P + (V * T);

	return true;
}

bool Ray::Intersects(const Triangle& t, Intersection& inter) const
{
	Plane PL = t.GetPlane();
	if(this->Intersects(PL, inter))
	{
		if(inter.P.IsContained(t))
			return true;
		else
			return false;
	}
	else
		return false;
}

bool Ray::Intersects(const Sphere& s, Intersection& inter) const
{
	Vector PtoC = P - s.C;
	float PM = PtoC.Magnitude(), VM = V.Magnitude();
	float A = VM * VM, B = 2.0f * V.Dot(PtoC), C = (PM * PM) - (s.R * s.R);
	float Disc = (B * B) - (4.0f * A * C);

	if(Disc < 0.0f)
		return false;
	else if(Disc == 0.0f)
	{
		inter.T = -B / (2.0f * A);
		if(inter.T > 0.0f)
		{
			inter.P = P + (V * inter.T);
			return true;
		}
		else
			return false;
	}
	else //Disc > 0.0f
	{
		float T1 = (-B + sqrt(Disc)) / (2.0f * A), T2 = (-B - sqrt(Disc)) / (2.0f * A);

		if((T1 >= 0.0f) && (T2 >= 0.0f))
		{
			inter.T = MyMin(T1, T2);
			inter.P = P + (V * inter.T);
			return true;
		}
		else if((T1 >= 0.0f) && (T2 < 0.0f))
		{
			inter.T = T1;
			inter.P = P + (V * inter.T);
			return true;
		}
		else if((T1 < 0.0f) && (T2 >= 0.0f))
		{
			inter.T = T2;
			inter.P = P + (V * inter.T);
			return true;
		}
		else //both < 0.0f
		{
			return false;
		}
	}
}

bool Ray::Intersects(const AABB& a, Intersection& inter) const
{
	Vector X(1.0f, 0.0f, 0.0f), Y(0.0f, 1.0f, 0.0f), Z(0.0f, 0.0f, 1.0f);
	Vector EX = X * a.E.X, EY = Y * a.E.Y, EZ = Z * a.E.Z;

	Plane H1(a.C + EZ, Z), H2(a.C - EZ, Z * -1.0f), H3(a.C - EX, X * -1.0f),
	      H4(a.C + EX, X), H5(a.C - EY, Y * -1.0f), H6(a.C + EY, Y);

	//find the intersections
	Range R1 = this->GetHalfSpace(H1), R2 = this->GetHalfSpace(H2), 
		  R3 = this->GetHalfSpace(H3), R4 = this->GetHalfSpace(H4), 
		  R5 = this->GetHalfSpace(H5), R6 = this->GetHalfSpace(H6);

	R1.IntersectWith(R2);
	R1.IntersectWith(R3);
	R1.IntersectWith(R4);
	R1.IntersectWith(R5);
	R1.IntersectWith(R6);

	float Tmin = R1.Min, Tmax = R1.Max;

	if((Tmin <= Tmax) && (Tmin >= 0.0f || Tmax >= 0.0f))
	{
		inter.T = Tmin;
		inter.P = P + (V * inter.T);
		return true;
	}

	return false;
}
//-----Ray Class Functions-----//

//-----Triangle Class Functions-----//
Plane Triangle::GetPlane(void) const
{
	Vector V1 = Q - P, V2 = R - P;
	Vector N = V1.Cross(V2);
	N.NormalizeMe();
	
	return Plane(P, N);
}

bool Triangle::Intersects(const Triangle& t, Intersection& inter) const
{
	LineSegment LS1(Q, P), LS2(R, P), LS3(R, Q),
		        LS4(t.Q, t.P), LS5(t.R, t.P), LS6(t.R, t.Q);

	if(LS1.Intersects(t, inter)) //check if any sides of this tri intersect t
		return true;
	else if(LS2.Intersects(t, inter))
		return true;
	else if(LS3.Intersects(t, inter))
		return true;

	if(LS4.Intersects(*this, inter)) //check if any sides of t intersect this tri
		return true;
	else if(LS5.Intersects(*this, inter))
		return true;
	else if(LS6.Intersects(*this, inter))
		return true;

	return false;
}
//-----Triangle Class Functions-----//

//-----Plane Class Functions-----//
float Plane::Angle(const Plane& p) const
{
	float M1 = N.Magnitude(), M2 = p.N.Magnitude();
	float Denom = M1 * M2;

	if(Denom == 0.0f)
		return 0.0f;
	else
		return float(acos(N.Dot(p.N) / Denom));
}

bool Plane::IsParallel(const Plane& p) const
{
	return p.N.IsParallel(N); //if the normals are parallel, so are the planes
}

bool Plane::IsPerpendicular(const Plane& p) const
{
	return p.N.IsPerpendicular(N); //if the normals are perpendicular, so are the planes
}

bool Plane::IsCoplanar(const Plane& p) const
{
	if(this->IsParallel(p))
	{
		if(P.IsContained(p) || p.P.IsContained(*this))
			return true;
		else
			return false;
	}
	else
		return false;
}
//-----Plane Class Functions-----//

//-----Sphere Class Functions-----//
bool Sphere::Intersects(const Sphere& s) const
{
	float Dist = C.Distance(s.C), Sum = R + s.R;

	if(Dist <= Sum)
		return true;
	else
		return false;
}
//-----Sphere Class Functions-----//

//-----AABB Class Functions-----//
AABB::AABB(const AABB& b1, const AABB& b2)
{
	Point Max1 = b1.C + b1.E, Min1 = b1.C + (b1.E * -1.0f);
	Point Max2 = b2.C + b2.E, Min2 = b2.C + (b2.E * -1.0f);

	Point Max3(MyMax(Max1.X, Max2.X), MyMax(Max1.Y, Max2.Y), MyMax(Max1.Z, Max2.Z));
	Point Min3(MyMin(Min1.X, Min2.X), MyMin(Min1.Y, Min2.Y), MyMin(Min1.Z, Min2.Z));

	Vector E3 = (Max3 - Min3) / 2.0f;
	Point C3 = Max3 - E3;

	C = C3;
	E = E3;
}

bool AABB::Intersects(const AABB& a) const
{
	Point Dist = Point::Subtract(C, a.C);
	Vector Sum = E + a.E;

	if(Dist.X > Sum.X)
		return false;
	else if(Dist.Y > Sum.Y)
		return false;
	else if(Dist.Z > Sum.Z)
		return false;

	return true;
}
//-----AABB Class Functions-----//

//-----Row Class Functions-----//
Row::Row(const float r0, const float r1, const float r2, const float r3)
{
	R[0] = r0;
	R[1] = r1;
	R[2] = r2;
	R[3] = r3; 
}

float& Row::operator[](const unsigned i)
{
	if(i >= 4)
	{
		std::cout << "invalid row index: " << i << std::endl;
		return R[3];
	}
	else
		return R[i];
}

const float& Row::operator[](const unsigned i) const
{
	if(i >= 4)
	{
		std::cout << "invalid row index: " << i << std::endl;
		return R[3];
	}
	else
		return R[i];
}

float Row::operator*(const Row& r) const
{
	return (R[0] * r[0]) + (R[1] * r[1]) + (R[2] * r[2]) + (R[3] * r[3]);
}

float Row::operator*(const Vector& v) const
{
	return (R[0] * v.X) + (R[1] * v.Y) + (R[2] * v.Z) + (R[3] * v.W);
}

float Row::operator*(const Point& p) const
{
	return (R[0] * p.X) + (R[1] * p.Y) + (R[2] * p.Z) + (R[3] * p.W);
}
//-----Row Class Functions-----//

//-----Matrix4x4 Class Functions-----//
Matrix4x4::Matrix4x4(void)
{
	M[0] = Row(0.0f, 0.0f, 0.0f, 0.0f);
	M[1] = Row(0.0f, 0.0f, 0.0f, 0.0f);
	M[2] = Row(0.0f, 0.0f, 0.0f, 0.0f);
	M[3] = Row(0.0f, 0.0f, 0.0f, 0.0f);
}

Row& Matrix4x4::GetRow(const unsigned i)
{
	if(i >= 4)
	{
		std::cout << "invalid row index: " << i << std::endl;
		return M[3];
	}
	else
		return M[i];
}

const Row& Matrix4x4::GetRow(const unsigned i) const
{
	if(i >= 4)
	{
		std::cout << "invalid row index: " << i << std::endl;
		return M[3];
	}
	else
		return M[i];
}

Row Matrix4x4::GetCol(const unsigned i) const
{
	if(i >= 4)
	{
		std::cout << "invalid row index: " << i << std::endl;
		return Row(M[0][3], M[1][3], M[2][3], M[3][3]);
	}
	else
		return Row(M[0][i], M[1][i], M[2][i], M[3][i]);
}

Row& Matrix4x4::operator[](const unsigned i)
{
	if(i >= 4)
	{
		std::cout << "invalid matrix index: " << i << std::endl;
		return M[3];
	}
	else
		return M[i];
}

const Row& Matrix4x4::operator[](const unsigned i) const
{
	if(i >= 4)
	{
		std::cout << "invalid matrix index: " << i << std::endl;
		return M[3];
	}
	else
		return M[i];
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& rhs) const
{
	Matrix4x4 Ret;
	for(unsigned i = 0; i < 4; i++)
	{
		Row Ci = rhs.GetCol(i);
		for(unsigned j = 0; j < 4; j++)
		{
			Ret[j][i] = M[j] * Ci;
		}
	}
	return Ret;
}

Matrix4x4 Matrix4x4::operator*(const float rhs) const
{
	Matrix4x4 Ret;
	for(unsigned i = 0; i < 4; i++)
	{
		for(unsigned j = 0; j < 4; j++)
		{
			Ret[i][j] = M[i][j] * rhs;
		}
	}
	return Ret;
}

void Matrix4x4::operator*=(const float rhs)
{
	for(unsigned i = 0; i < 4; i++)
	{
		for(unsigned j = 0; j < 4; j++)
		{
			M[i][j] *= rhs;
		}
	}
}

Point Matrix4x4::operator*(const Point& rhs) const
{
	Point Ret;
	Ret.X = M[0] * rhs;
	Ret.Y = M[1] * rhs;
	Ret.Z = M[2] * rhs;
	Ret.W = M[3] * rhs;
	return Ret;
}

Vector Matrix4x4::operator*(const Vector& rhs) const
{
	Vector Ret;
	Ret.X = M[0] * rhs;
	Ret.Y = M[1] * rhs;
	Ret.Z = M[2] * rhs;
	Ret.W = M[3] * rhs;
	return Ret;
}

Matrix4x4 Matrix4x4::operator+(const Matrix4x4& rhs) const
{
	Matrix4x4 Ret;
	for(unsigned i = 0; i < 4; i++)
	{
		for(unsigned j = 0; j < 4; j++)
		{
			Ret[i][j] = M[i][j] + rhs[i][j];
		}
	}
	return Ret;
}

void Matrix4x4::Identity(void)
{
	M[0] = Row(1.0f, 0.0f, 0.0f, 0.0f);
	M[1] = Row(0.0f, 1.0f, 0.0f, 0.0f);
	M[2] = Row(0.0f, 0.0f, 1.0f, 0.0f);
	M[3] = Row(0.0f, 0.0f, 0.0f, 1.0f);
}

void Matrix4x4::Translate(const float tx, const float ty, const float tz)
{
	M[0] = Row(1.0f, 0.0f, 0.0f, tx);
	M[1] = Row(0.0f, 1.0f, 0.0f, ty);
	M[2] = Row(0.0f, 0.0f, 1.0f, tz);
	M[3] = Row(0.0f, 0.0f, 0.0f, 1.0f);
}

void Matrix4x4::Rotate(const Vector& v, const float a, bool deg)
{
	float A = a;
	if(deg)
		A = DegToRad(a);
	float C = cos(A), S = sin(A), T = 1.0f - C;

	M[0] = Row(T * (v.X*v.X) + C, T * (v.X*v.Y) - (S*v.Z), 
		       T * (v.X*v.Z) + (S*v.Y), 0.0f);
	M[1] = Row(T * (v.X*v.Y) + (S*v.Z), T * (v.Y*v.Y) + C, 
		       T * (v.Y*v.Z) - (S*v.X), 0.0f);
	M[2] = Row(T * (v.X*v.Z) - (S*v.Y), T * (v.Y*v.Z) + (S*v.X), 
		       T * (v.Z*v.Z) + C, 0.0f);
	M[3] = Row(0.0f, 0.0f, 0.0f, 1.0f);
}

void Matrix4x4::Scale(const float sx, const float sy, const float sz)
{
	M[0] = Row(sx, 0.0f, 0.0f, 0.0f);
	M[1] = Row(0.0f, sy, 0.0f, 0.0f);
	M[2] = Row(0.0f, 0.0f, sz, 0.0f);
	M[3] = Row(0.0f, 0.0f, 0.0f, 1.0f);
}

void Matrix4x4::WtoC(const Point& pos, const Vector& dir, const Vector& up)
{
	Vector R = dir.Cross(up);
	Matrix4x4 Tr, Rot;
	Tr.Translate(-pos.X, -pos.Y, -pos.Z);

	Rot.M[0] = Row(R.X, R.Y, R.Z, 0.0f);
	Rot.M[1] = Row(up.X, up.Y, up.Z, 0.0f);
	Rot.M[2] = Row(-dir.X, -dir.Y, -dir.Z, 0.0f);
	Rot.M[3] = Row(0.0f, 0.0f, 0.0f, 1.0f);

	*this = Rot * Tr;
}

void Matrix4x4::CtoP(const float n, const float f, const float w, const float h)
{
	M[0] = Row((2.0f * n) / w, 0.0f, 0.0f, 0.0f);
	M[1] = Row(0.0f, (2.0f * n) / h, 0.0f, 0.0f);
	M[2] = Row(0.0f, 0.0f, (-n - f) / (f - n), (-2.0f * n * f) / (f - n));
	M[3] = Row(0.0f, 0.0f, -1.0f, 0.0f);
}
//-----Matrix4x4 Class Functions-----//

//-----Quaternion Class Functions-----//
Quaternion::Quaternion(const float s, const float x, const float y, const float z)
{
	S = s;
	V = Vector(x, y, z);
}

Quaternion Quaternion::operator*(const Quaternion& rhs) const
{
	Quaternion Ret;
	Vector VcV = V.Cross(rhs.V);

	Ret.S = (S * rhs.S) - V.Dot(rhs.V);
	Ret.V = (rhs.V * S) + (V * rhs.S) + VcV;

	Ret.S = ClampTo(Ret.S, 0.0f, TOLERANCE);
	Ret.V.X = ClampTo(Ret.V.X, 0.0f, TOLERANCE);
	Ret.V.Y = ClampTo(Ret.V.Y, 0.0f, TOLERANCE);
	Ret.V.Z = ClampTo(Ret.V.Z, 0.0f, TOLERANCE);

	return Ret;
}

Quaternion Quaternion::operator*(const float& rhs) const
{
	return Quaternion(V * rhs, S * rhs);
}

Quaternion Quaternion::operator/(const float& rhs) const
{
	if(rhs == 0.0f)
	{
		std::cout << "Cannot divide a quaternion by 0" << std::endl;
		return *this;
	}
	else
		return Quaternion(V / rhs, S / rhs);
}

Quaternion Quaternion::operator+(const Quaternion& rhs) const
{
	return Quaternion(V + rhs.V, S + rhs.S);
}

Quaternion Quaternion::operator-(const Quaternion& rhs) const
{
	return Quaternion(V - rhs.V, S - rhs.S);
}

void Quaternion::operator*=(const float& rhs)
{
	*this = *this * rhs;
}

void Quaternion::operator/=(const float& rhs)
{
	*this = *this / rhs;
}

void Quaternion::operator+=(const Quaternion& rhs)
{
	*this = *this + rhs;
}

void Quaternion::operator-=(const Quaternion& rhs)
{
	*this = *this - rhs;
}

float Quaternion::Dot(const Quaternion& rhs) const
{
	return (S * rhs.S) + V.Dot(rhs.V);
}

float Quaternion::SquareMag(void) const
{
	//Because calling magnitude and squaring it wastes an sqrt call
	return (S*S + V.X*V.X + V.Y*V.Y + V.Z*V.Z);
}

float Quaternion::Magnitude(void) const
{
	return sqrt(this->SquareMag());
}

Quaternion Quaternion::Inverse(void) const
{
	Quaternion Ret = this->Conjugate();
	float Denom = this->SquareMag();

	if(Denom != 0.0f) //will only happen with all elements 0, but just in case
		Ret /= Denom;

	return Ret;
}

void Quaternion::InvertMe(void)
{
	float Denom = this->SquareMag();

	if(Denom != 0.0f) //will only happen with all elements 0, but just in case
	{
		S /= Denom;
		V /= -Denom;
	}
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

void Quaternion::Rotator(const float angle, const Vector& axis)
{
	S = cos(angle / 2.0f);
	V = axis * sin(angle / 2.0f);
}
//-----Quaternion Class Functions-----//

//-----Range Class Functions-----//
void Range::IntersectWith(const Range& r)
{
	float A = Min, B = Max, C = r.Min, D = r.Max;
	Min = MyMax(A, C);
	Max = MyMin(B, D);
}
//-----Range Class Functions-----//

//-----Misc Functions-----//
float DegToRad(const float& a)
{
	return float((a / 180.0f) * PI);
}

bool InRange(const float& val, const float& lhs, const float& rhs)
{
	if((val >= lhs) && (val <= rhs))
		return true;
	else
		return false;
}

//Clamp val to the given range(low - high)
float Clamp(const float& val, const float& low, const float& high)
{
	if(val < low)
		return low;
	else if(val > high)
		return high;
	else
		return val;
}

//Clamp "val" to "clamp" if they differ by no more than "diff", used for rounding errors
float ClampTo(const float& val, const float& clamp, const float& diff)
{
	if(InRange(val, clamp - diff, clamp + diff))
		return clamp;
	else
		return val;
}

bool NearZero(const float& val)
{
	if(ClampTo(val, 0.0f, TOLERANCE) == 0.0f)
		return true;
	else
		return false;
}

float MyMin(const float& a, const float& b)
{
	if(a < b)
		return a;
	else
		return b;
}

float MyMax(const float& a, const float& b)
{
	if(a > b)
		return a;
	else
		return b;
}

float SurfaceArea(const float& w, const float& h, const float& d)
{
	return 2.0f * (w*h + w*d + h*d);
}
//-----Misc Functions-----/
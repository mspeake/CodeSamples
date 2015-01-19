#include "MathLib.h"
#include <iostream>
#include <vector>

std::vector<Object *> ObjectList;

void PrintPoint(const Point& p)
{
	std::cout << p.X << ", " << p.Y << ", " << p.Z << ", " << p.W << std::endl;
}
void PrintVector(const Vector& v)
{
	std::cout << v.X << ", " << v.Y << ", " << v.Z << ", " << v.W << std::endl;
}

void PrintMatrix(Matrix4x4& m)
{
	//*
	std::cout << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << std::endl;
	std::cout << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << std::endl;
	std::cout << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << std::endl;
	std::cout << m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << std::endl;
	/**/
}

int main(void)
{
	/*
	Row r;// = {1.1f, 2.2f, 3.3f, 4.4f};
	r[0] = 5.6f;
	std::cout << r[0] << std::endl;
	std::cout << r[1] << std::endl;
	std::cout << r[2] << std::endl;
	std::cout << r[3] << std::endl;
	/**/

	Point p(1.0f, 2.0f, 3.0f);
	Plane pl(Point(0.0f, 0.0f, 0.0f), Vector(1.0f, 1.0f, 0.0f));
	p.Project(pl);
	PrintPoint(p);
	return 0;
}
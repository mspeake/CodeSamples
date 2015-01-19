#ifndef SHAPELIB_H
#define SHAPELIB_H

#include "MathLib.h"
#include <vector>
#include <map>

enum ShapeType
{
	PLANE,
	CUBE,
	SPHERE,
	CONE
};

class Face
{
public:
	unsigned P1, P2, P3;
	unsigned T1, T2, T3;
	Vector Nrm, T, B;

	void CalculateTB(const float* tcs, const float* verts);
};

//some typedefs for shorthand
typedef std::vector<Point> PointVec;
typedef std::vector<Vector> VectorVec;
typedef std::vector<Face> FaceVec;

void CalculateTBData(const FaceVec& faces, VectorVec& vtan, VectorVec& vbit, const unsigned vnum, const unsigned fnum);
void CalculatePlaneData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces);
void CalculateCubeData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces);
void CalculateSphereData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces);
void CalculateConeData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces);

#endif
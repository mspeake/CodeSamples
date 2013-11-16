/********************************************************************
**ShapeLib.h
**
**-Defines the Shape class, as well as the types of shape it can
** represent, and the utility functions involved in generating these
** shapes' vertices and triangles
********************************************************************/

#ifndef SHAPELIB_H
#define SHAPELIB_H

#include <vector>
#include <map>

#include "MathLib.h"
#include "Graphics.h"

enum ShapeType
{
	PLANE,
	CUBE,
	SPHERE,
	CONE,
	CYLINDER,
	TORUS
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

//some default value defines
#define TORUS_ID_DEFAULT 0.7f
#define TORUS_OD_DEFAULT 1.3f

class Color
{
public:
	float R, G, B, A;

	Color(const float r = 1.0f, const float g = 1.0f, const float b = 1.0f, const float a = 1.0f);
};

class Shape
{
private:
	unsigned Type;
	unsigned Subdivisions;
	unsigned VertCount;
	unsigned FaceCount;

	float* VertArray;
	float* VertNorms;
	float* TexCoordArray;
	unsigned short* FaceArray;
	float* TanArray;
	float* BitanArray;

	void CalculateData(const unsigned s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces);

	//special constructor for torus that utilizes user-defined inner and outer diameter, used only by CreateTorus function
	Shape(const unsigned sub, const float id, const float od);

public:
	Color MyColor;

	Shape(const unsigned type, const unsigned s);
	~Shape();

	static Shape* CreateTorus(const unsigned sub, const float inner_d, const float outer_d);

	void SetVertexArrays(const PointVec& verts, const PointVec& tcs, const VectorVec& norms);
	void SetFaceArray(const FaceVec& faces);
	void SetTBArrays(const VectorVec& tan, const VectorVec& bitan);
	void ReScaleVertices(const Vector& s);
	unsigned GetType(void) const;

	void DrawSelf(void) const;
	void DrawNormals(void) const;
};

namespace ShapeUtils
{
	//-----Plane Shape Functions-----//
	void CalculatePlaneData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces);
	void CalculatePlaneVerts(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms);
	void CalculatePlaneTris(const int s, FaceVec& faces);
	//-----Plane Shape Functions-----//

	//-----Cube Shape Functions-----//
	void CalculateCubeData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces);
	//-----Cube Shape Functions-----//

	//-----Sphere Shape Functions-----//
	void CalculateSphereData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces);
	void CalculateSphereVerts(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms);
	void CalculateSphereTris(const int s, FaceVec& faces);
	//-----Sphere Shape Functions-----//

	//-----Cone Shape Functions-----//
	void CalculateConeData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces);
	void CalculateConeVertsTop(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms);
	void CalculateConeVertsBot(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms);
	void CalculateConeTrisTop(const int s, FaceVec& faces);
	void CalculateConeTrisBot(const int s, FaceVec& faces);
	//-----Cone Shape Functions-----//

	//-----Cylinder Shape Functions-----//
	void CalculateCylinderData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces);
	void CalculateCylinderVertsMid(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms);
	void CalculateCylinderVertsTop(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms);
	void CalculateCylinderVertsBot(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms);
	void CalculateCylinderTrisMid(const int s, FaceVec& faces);
	void CalculateCylinderTrisTop(const int s, FaceVec& faces);
	void CalculateCylinderTrisBot(const int s, FaceVec& faces);
	//-----Cylinder Shape Functions-----//

	//-----Torus Shape Functions-----//
	void CalculateTorusData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces);
	void CalculateTorusVerts(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, 
							 const float id = TORUS_ID_DEFAULT, const float od = TORUS_OD_DEFAULT);
	void CalculateTorusTris(const int s, FaceVec& faces);
	//-----Torus Shape Functions-----//

	//-----Misc Functions-----//
	void CalculateTBData(const FaceVec& faces, VectorVec& vtan, VectorVec& vbit, const unsigned vnum, const unsigned fnum);
	//-----Misc Functions-----//
}

#endif
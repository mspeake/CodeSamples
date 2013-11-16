/********************************************************************
**ShapeLib.cpp
**
**-Implements the Shape class, it allows the user to create 3D shapes
** to be drawn
********************************************************************/

#include "ShapeLib.h"
#include "Shaders.h"

//-----Color Class Functions-----//
Color::Color(const float r, const float g, const float b, const float a)
{
	R = r;
	G = g;
	B = b;
	A = a;
}
//-----Color Class Functions-----//

//-----Shape Class Functions-----//
Shape::Shape(const unsigned type, const unsigned s)
{
	Type = type;
	MyColor = Color();
	Subdivisions = s;

	PointVec TexCoords, Vertices;
	VectorVec Normals, VertT, VertB;
	FaceVec TriangleList;

	this->CalculateData(s, Vertices, TexCoords, Normals, TriangleList);

	VertCount = Vertices.size(), FaceCount = TriangleList.size();

	VertArray = new float[VertCount * 3];
	VertNorms = new float[VertCount * 3];
	FaceArray = new unsigned short[FaceCount * 3];
	TexCoordArray = new float[VertCount * 3];
	TanArray = new float[VertCount * 3];
	BitanArray = new float[VertCount * 3];

	this->SetVertexArrays(Vertices, TexCoords, Normals); //fill the vertex array
	this->SetFaceArray(TriangleList); //fill the face array

	for(unsigned i = 0; i < FaceCount; i++) //calculate T and B for each face
		TriangleList[i].CalculateTB(TexCoordArray, VertArray);

	ShapeUtils::CalculateTBData(TriangleList, VertT, VertB, VertCount, FaceCount);
	this->SetTBArrays(VertT, VertB); //fill the tan and bit arrays
}

//special constructor for torus
Shape::Shape(const unsigned sub, const float id, const float od)
{
	Type = TORUS;
	MyColor = Color();
	Subdivisions = sub;

	PointVec TexCoords, Vertices;
	VectorVec Normals, VertT, VertB;
	FaceVec TriangleList;

	ShapeUtils::CalculateTorusVerts(sub, Vertices, TexCoords, Normals, id, od);
	ShapeUtils::CalculateTorusTris(sub, TriangleList);

	VertCount = Vertices.size(), FaceCount = TriangleList.size();

	VertArray = new float[VertCount * 3];
	VertNorms = new float[VertCount * 3];
	FaceArray = new unsigned short[FaceCount * 3];
	TexCoordArray = new float[VertCount * 3];
	TanArray = new float[VertCount * 3];
	BitanArray = new float[VertCount * 3];

	this->SetVertexArrays(Vertices, TexCoords, Normals); //fill the vertex array
	this->SetFaceArray(TriangleList); //fill the face array

	for(unsigned i = 0; i < FaceCount; i++) //calculate T and B for each face
		TriangleList[i].CalculateTB(TexCoordArray, VertArray);

	ShapeUtils::CalculateTBData(TriangleList, VertT, VertB, VertCount, FaceCount);
	this->SetTBArrays(VertT, VertB); //fill the tan and bit arrays
}

Shape::~Shape(void)
{
	delete [] VertArray;
	delete [] VertNorms;
	delete [] FaceArray;
	delete [] TexCoordArray;
	delete [] TanArray;
	delete [] BitanArray;
}

Shape* Shape::CreateTorus(const unsigned sub, const float inner_d, const float outer_d)
{
	return new Shape(sub, inner_d, outer_d);
}

void Shape::SetVertexArrays(const PointVec& verts, const PointVec& tcs, const VectorVec& norms)
{
	for(unsigned int i = 0; i < VertCount; i++)
	{
		VertArray[i*3 + 0] = verts[i].X;
		VertArray[i*3 + 1] = verts[i].Y;
		VertArray[i*3 + 2] = verts[i].Z;

		VertNorms[i*3 + 0] = norms[i].X;
		VertNorms[i*3 + 1] = norms[i].Y;
		VertNorms[i*3 + 2] = norms[i].Z;

		TexCoordArray[i*3 + 0] = tcs[i].X;
		TexCoordArray[i*3 + 1] = tcs[i].Y;
		TexCoordArray[i*3 + 2] = tcs[i].Z;
	}
}

void Shape::SetFaceArray(const FaceVec& faces)
{
	for(unsigned int i = 0; i < FaceCount; i++)
	{
		FaceArray[i*3 + 0] = faces[i].P1;
		FaceArray[i*3 + 1] = faces[i].P2;
		FaceArray[i*3 + 2] = faces[i].P3;
	}
}

void Shape::SetTBArrays(const VectorVec& tan, const VectorVec& bitan)
{
	for(unsigned i = 0; i < VertCount; i++)//transfer the T&B values to the arrays
	{
		TanArray[i*3 + 0] = tan[i].X;
		TanArray[i*3 + 1] = tan[i].Y;
		TanArray[i*3 + 2] = tan[i].Z;

		BitanArray[i*3 + 0] = bitan[i].X;
		BitanArray[i*3 + 1] = bitan[i].Y;
		BitanArray[i*3 + 2] = bitan[i].Z;
	}
}

void Shape::ReScaleVertices(const Vector& s)
{
	//its better to scale vertices before passing them to OpenGL
	//than to use glScalef, as it causes issues with normals
	for(unsigned i = 0; i < VertCount; i++)
	{
		VertArray[i*3 + 0] *= s.X;
		VertArray[i*3 + 1] *= s.Y;
		VertArray[i*3 + 2] *= s.Z;
	}
}

unsigned Shape::GetType(void) const
{
	return Type;
}

void Shape::DrawSelf(void) const
{
	ShaderManager* SM = Shaders::ShaderController;
	std::string CurrentShader = SM->GetCurrentShader();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, VertArray);
	glNormalPointer(GL_FLOAT, 0, VertNorms);
	glTexCoordPointer(3, GL_FLOAT, 0, TexCoordArray);

	SM->PassAttribute(CurrentShader.c_str(), "ModelTangent", TanArray);
	SM->PassAttribute(CurrentShader.c_str(), "ModelBitangent", BitanArray);
	glDrawElements(GL_TRIANGLES, FaceCount * 3, GL_UNSIGNED_SHORT, FaceArray);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Shape::DrawNormals(void) const
{
	for(unsigned i = 0; i < VertCount; i++)
	{
		Point V(VertArray[i*3 + 0], VertArray[i*3 + 1], VertArray[i*3 + 2]);
		Vector N(VertNorms[i*3 + 0], VertNorms[i*3 + 1], VertNorms[i*3 + 2]);
		N.NormalizeMe();
		GraphicsUtils::DrawLine(V, V + (N * 5.0f));
	}
}

void Shape::CalculateData(const unsigned s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces)
{
	switch(Type) //generate the vertices, tex coords, normals and triangle lists based on shape type
	{
	case PLANE:
		{
			ShapeUtils::CalculatePlaneData(s, verts, tcs, norms, faces);
			break;
		}

	case CUBE:
		{
			ShapeUtils::CalculateCubeData(s, verts, tcs, norms, faces);
			break;
		}

	case SPHERE:
		{
			ShapeUtils::CalculateSphereData(s, verts, tcs, norms, faces);
			break;
		}

	case CONE:
		{
			ShapeUtils::CalculateConeData(s, verts, tcs, norms, faces);
			break;
		}

	case CYLINDER:
		{
			ShapeUtils::CalculateCylinderData(s, verts, tcs, norms, faces);
			break;
		}

	case TORUS:
		{
			ShapeUtils::CalculateTorusData(s, verts, tcs, norms, faces);
			break;
		}
	}
}
//-----Shape Class Functions-----//

//-----Face Class Functions-----//
void Face::CalculateTB(const float* tcs, const float* verts)
{
	//by manipulating the equations [P = s1*T + t1*B]  and  [Q = s2*T + t2*B]
	//we get [T = (t1*Q - t2*P) / (s2*t1 - s1*t2)]
	//and    [B = (s2*P - s1*Q) / (s2*t1 - s1*t2)]

	//s1 = v2x-v1x, s2 = v3x-v1x(texture coords)
	float Sx1 = tcs[T2*3 + 0] - tcs[T1*3 + 0];
	float Sx2 = tcs[T3*3 + 0] - tcs[T1*3 + 0];

	//t1 = v2y-v1y, t2 = v3y-v1y(texture coords)
	float Tx1 = tcs[T2*3 + 1] - tcs[T1*3 + 1];
	float Tx2 = tcs[T3*3 + 1] - tcs[T1*3 + 1];

	float Denom = (Sx2 * Tx1) - (Sx1 * Tx2);
	if(Denom == 0.0f)
	{	//if dividing by 0, set T and B to 0
		T = Vector(0.0f, 0.0f, 0.0f);
		B = Vector(0.0f, 0.0f, 0.0f);
		return;
	}

	//P = v2 - v1, Q = v3 - v1
	Point V1(verts[P1*3 + 0], verts[P1*3 + 1], verts[P1*3 + 2]),
		  V2(verts[P2*3 + 0], verts[P2*3 + 1], verts[P2*3 + 2]),
		  V3(verts[P3*3 + 0], verts[P3*3 + 1], verts[P3*3 + 2]);

	Vector P(V2 - V1), Q(V3 - V1);
	P.NormalizeMe();
	Q.NormalizeMe();

	T = (Q * Tx1) - (P * Tx2) / Denom;
	B = (P * Sx2) - (Q * Sx1) / Denom;
}
//-----Face Class Functions-----//

namespace ShapeUtils
{
	//This contains the functions used in calculating the vertices, triangles, and normals for the shapes

	//-----Plane Shape Functions-----//
	void CalculatePlaneData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces)
	{
		CalculatePlaneVerts(s, verts, tcs, norms);
		CalculatePlaneTris(s, faces);
	}

	void CalculatePlaneVerts(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms)
	{
		for(int i = 0; i <= s; i++)
		{
			float Y = float(i) / float(s);

			for(int j = 0; j <= s; j++)
			{
				float X = float(j) / float(s);

				Point V0(X - 0.5f, Y - 0.5f, 0.0f);
				Point Tc0(X, 1.0f - Y, 0.0f);
				Vector Nrm0(0.0f, 0.0f, 1.0f);

				verts.push_back(V0);
				norms.push_back(Nrm0);
				tcs.push_back(Tc0);
			}
		}
	}

	void CalculatePlaneTris(const int s, FaceVec& faces)
	{
		for(int i = 0; i < s; i++)
		{
			for(int j = 0; j < s; j++)
			{
				Face F1, F2;

				F1.P1 = (i + 1) * (s + 1) + j;
				F1.P2 = i * (s + 1) + j;
				F1.P3 = (i + 1) * (s + 1) + j + 1;

				F2.P1 = F1.P3;
				F2.P2 = F1.P2;
				F2.P3 = F1.P2 + 1;

				F1.T1 = F1.P1, F1.T2 = F1.P2, F1.T3 = F1.P3;
				F2.T1 = F2.P1, F2.T2 = F2.P2, F2.T3 = F2.P3;

				faces.push_back(F1);
				faces.push_back(F2);
			}
		}
	}
	//-----Plane Shape Functions-----//


	//-----Cube Shape Functions-----//
	void CalculateCubeData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces)
	{
		CalculatePlaneData(s, verts, tcs, norms, faces); //start with a plane's verts and transform them

		unsigned VertNum = verts.size(), FaceNum = faces.size();
		float A = float(PI) / 2.0f;
		Matrix4x4 Rot;

		for(int i = 1; i <= 6; i++)
		{
			Vector Trans, Nrm;
			switch(i)
			{
			case 1: //right side of the cube
				{
					Trans.X = 0.5f;
					Rot.Rotate(Vector(0.0f, 1.0f, 0.0f), A);
					Nrm = Vector(1.0f, 0.0f, 0.0f);
					break;
				}
			case 2: //left side of the cube
				{
					Trans.X = -0.5f; A *= -1.0f;
					Rot.Rotate(Vector(0.0f, 1.0f, 0.0f), A);
					Nrm = Vector(-1.0f, 0.0f, 0.0f);
					break;
				}
			case 3: //top side of the cube
				{
					Trans.Y = 0.5f;
					Rot.Rotate(Vector(1.0f, 0.0f, 0.0f), A);
					Nrm = Vector(0.0f, 1.0f, 0.0f);
					break;
				}
			case 4: //bottom side of the cube
				{
					Trans.Y = -0.5f; A *= -1.0f;
					Rot.Rotate(Vector(1.0f, 0.0f, 0.0f), A);
					Nrm = Vector(0.0f, -1.0f, 0.0f);
					break;
				}
			case 5: //back side of the cube
				{
					Trans.Z = -0.5f; A *= 2.0f;
					Rot.Rotate(Vector(0.0f, 1.0f, 0.0f), A);
					Nrm = Vector(0.0f, 0.0f, -1.0f);
					break;
				}
			case 6: //the front face, only need to translate the first plane along +z
				{
					Trans.Z = 0.5f;
					break;
				}
			}

			if(i < 6)
			{
				for(unsigned j = 0; j < VertNum; j++)
				{
					Point P2(verts[j]), TC(tcs[j]);
					P2 = (Rot * P2) + Trans;

					verts.push_back(P2);
					norms.push_back(Nrm);
					tcs.push_back(TC);
				}

				for(unsigned j = 0; j < FaceNum; j++)
				{
					Face F2(faces[j]);
					F2.P1 += (i * VertNum); F2.P2 += (i * VertNum); F2.P3 += (i * VertNum);
					F2.T1 = F2.P1, F2.T2 = F2.P2, F2.T3 = F2.P3;
					faces.push_back(F2);
				}
			}
			else
			{
				for(unsigned j = 0; j < VertNum; j++)
					verts[j] += Trans;
			}
		}
	}
	//-----Cube Shape Functions-----//

	//-----Sphere Shape Functions-----//
	void CalculateSphereData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces)
	{
		CalculateSphereVerts(s, verts, tcs, norms);
		CalculateSphereTris(s, faces);
	}

	void CalculateSphereVerts(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms)
	{
		float HalfPi = float(PI) / 2.0f;

		for(int i = 0; i <= s; i++)
		{
			float Y = float(i) / float(s);

			for(int j = 0; j <= (s * 2); j++)
			{
				float X = float(j) / float(s * 2);

				Point V0(0.5f * sin(X * 2.0f * float(PI)) * cos(Y * float(PI) - HalfPi), 
						 0.5f * sin(Y * float(PI) - HalfPi),
						 0.5f * cos(X * 2.0f * float(PI)) * cos(Y * float(PI) - HalfPi));
				Point Tc0(X, 1.0f - Y, 0.0f);
				Vector Nrm0(V0);
				Nrm0.NormalizeMe();

				verts.push_back(V0);
				norms.push_back(Nrm0);
				tcs.push_back(Tc0);
			}
		}
	}

	void CalculateSphereTris(const int s, FaceVec& faces)
	{
		for(int i = 0; i < s; i++)
		{
			for(int j = 0; j < (s * 2); j++)
			{
				Face F1, F2;

				F1.P1 = (i + 1) * ((s * 2) + 1) + j;
				F1.P2 = i * ((s * 2) + 1) + j;
				F1.P3 = (i + 1) * ((s * 2) + 1) + j + 1;

				F2.P1 = F1.P3;
				F2.P2 = F1.P2;
				F2.P3 = F1.P2 + 1;

				F1.T1 = F1.P1, F1.T2 = F1.P2, F1.T3 = F1.P3;
				F2.T1 = F2.P1, F2.T2 = F2.P2, F2.T3 = F2.P3;

				faces.push_back(F1);
				faces.push_back(F2);
			}
		}
	}
	//-----Sphere Shape Functions-----//

	//-----Cone Shape Functions-----//
	void CalculateConeData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces)
	{
		CalculateConeVertsTop(s, verts, tcs, norms);
		CalculateConeVertsBot(s, verts, tcs, norms);
		CalculateConeTrisTop(s, faces);
		CalculateConeTrisBot(s, faces);
	}

	void CalculateConeVertsTop(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms)
	{
		//construct the verts for the top
		for(int i = 0; i <= s; i++)
		{
			float Y = float(i) / float(s);

			for(int j = 0; j <= (s * 2); j++)
			{
				float X = float(j) / float(s * 2);
				float A = X * 2.0f * float(PI);
				Point V0((sin(A) / 2.0f), Y - 0.5f, (cos(A) / 2.0f));
				Point Tc0(X, 1.0f - Y, 0.0f);
				Vector Nrm0(V0.X, sqrt((V0.X * V0.X) + (V0.Z * V0.Z)) / 2.0f, V0.Z);
				Nrm0.NormalizeMe();

				V0.X *= (1.0f - Y); //need to calculate the normal before this,
				V0.Z *= (1.0f - Y); //otherwise the top will have a 0,0,0 normal

				verts.push_back(V0);
				norms.push_back(Nrm0);
				tcs.push_back(Tc0);
			}
		}
	}

	void CalculateConeVertsBot(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms)
	{
		//central vert for the bottom
		verts.push_back(Point(0.0f, -0.5f, 0.0f));
		norms.push_back(Vector(0.0f, -1.0f, 0.0f));
		tcs.push_back(Point(0.5f, 0.5f, 0.0f));

		//construct the verts for the bottom
		for(int i = 0; i <= (s * 2); i++)
		{
			float X = float(i) / float(s * 2);
			float A = X * 2.0f * float(PI);
			Point V0(-sin(A) / 2.0f, -0.5f, cos(A) / 2.0f);
			Vector Nrm0(0.0f, -1.0f, 0.0f);
			Point Tc0(-V0.X + 0.5f, V0.Z + 0.5f, 0.0f);

			verts.push_back(V0);
			norms.push_back(Nrm0);
			tcs.push_back(Tc0);
		}
	}

	void CalculateConeTrisTop(const int s, FaceVec& faces)
	{
		//construct the tri list for the top
		for(int i = 0; i < s; i++)
		{
			for(int j = 0; j < (s * 2); j++)
			{
				Face F1, F2;

				F1.P1 = (i + 1) * ((s * 2) + 1) + j;
				F1.P2 = i * ((s * 2) + 1) + j;
				F1.P3 = (i + 1) * ((s * 2) + 1) + j + 1;

				F2.P1 = F1.P3;
				F2.P2 = F1.P2;
				F2.P3 = F1.P2 + 1;

				F1.T1 = F1.P1, F1.T2 = F1.P2, F1.T3 = F1.P3;
				F2.T1 = F2.P1, F2.T2 = F2.P2, F2.T3 = F2.P3;

				faces.push_back(F1);
				faces.push_back(F2);
			}
		}
	}

	void CalculateConeTrisBot(const int s, FaceVec& faces)
	{
		unsigned Bottom = (s + 1) * ((s * 2) + 1);
		//construct the tri list for the bottom
		for(int i = 0; i < (s * 2); i++)
		{
			Face F1;
			F1.P1 = Bottom;
			F1.P2 = Bottom + i + 1;
			F1.P3 = Bottom + i + 2;

			F1.T1 = F1.P1, F1.T2 = F1.P2, F1.T3 = F1.P3;
			faces.push_back(F1);
		}
	}
	//-----Cone Shape Functions-----//

	//-----Cylinder Shape Functions-----//
	void CalculateCylinderData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces)
	{
		CalculateCylinderVertsMid(s, verts, tcs, norms);
		CalculateCylinderVertsTop(s, verts, tcs, norms);
		CalculateCylinderVertsBot(s, verts, tcs, norms);
		CalculateCylinderTrisMid(s, faces);
		CalculateCylinderTrisTop(s, faces);
		CalculateCylinderTrisBot(s, faces);
	}

	void CalculateCylinderVertsMid(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms)
	{
		//construct the midsection vertices
		for(int i = 0; i <= s; i++)
		{
			float Y = float(i) / float(s);

			for(int j = 0; j <= (s * 2); j++)
			{
				float X = float(j) / float(s * 2);
				float A = X * 2.0f * float(PI);
				Point V0(sin(A) / 2.0f, Y - 0.5f, cos(A) / 2.0f);
				Point Tc0(X, 1.0f - Y, 0.0f);
				Vector Nrm0(V0.X * 2.0f, 0.0f, V0.Z * 2.0f);
				Nrm0.NormalizeMe();

				verts.push_back(V0);
				norms.push_back(Nrm0);
				tcs.push_back(Tc0);
			}
		}
	}

	void CalculateCylinderVertsTop(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms)
	{
		verts.push_back(Point(0.0f, 0.5f, 0.0f));
		norms.push_back(Vector(0.0f, 1.0f, 0.0f));
		tcs.push_back(Point(0.5f, 0.5f, 0.0f));

		//construct the top vertices
		for(int i = 0; i <= (s * 2); i++)
		{
			float X = float(i) / float(s * 2);
			float A = X * 2.0f * float(PI);
			Point V0(sin(A) / 2.0f, 0.5f, cos(A) / 2.0f);
			Point Tc0(V0.X + 0.5f, V0.Z + 0.5f, 0.0f);
			Vector Nrm0(0.0f, 1.0f, 0.0f);

			verts.push_back(V0);
			norms.push_back(Nrm0);
			tcs.push_back(Tc0);
		}
	}

	void CalculateCylinderVertsBot(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms)
	{
		//center vert for the bottom
		verts.push_back(Point(0.0f, -0.5f, 0.0f));
		norms.push_back(Vector(0.0f, -1.0f, 0.0f));
		tcs.push_back(Point(0.5f, 0.5f, 0.0f));

		//construct the bottom verts
		for(int i = 0; i <= (s * 2); i++)
		{
			float X = float(i) / float(s * 2);
			float A = X * 2.0f * float(PI);
			Point V0(-sin(A) / 2.0f, -0.5f, cos(A) / 2.0f);
			Point Tc0(-V0.X + 0.5f, V0.Z + 0.5f, 0.0f);
			Vector Nrm0(0.0f, -1.0f, 0.0f);

			verts.push_back(V0);
			norms.push_back(Nrm0);
			tcs.push_back(Tc0);
		}
	}

	void CalculateCylinderTrisMid(const int s, FaceVec& faces)
	{
		//construct tri list for mid section
		for(int i = 0; i < s; i++)
		{
			for(int j = 0; j < (s * 2); j++)
			{
				Face F1, F2;

				F1.P1 = (i + 1) * ((s * 2) + 1) + j;
				F1.P2 = i * ((s * 2) + 1) + j;
				F1.P3 = (i + 1) * ((s * 2) + 1) + j + 1;

				F2.P1 = F1.P3;
				F2.P2 = F1.P2;
				F2.P3 = F1.P2 + 1;

				F1.T1 = F1.P1, F1.T2 = F1.P2, F1.T3 = F1.P3;
				F2.T1 = F2.P1, F2.T2 = F2.P2, F2.T3 = F2.P3;

				faces.push_back(F1);
				faces.push_back(F2);
			}
		}
	}

	void CalculateCylinderTrisTop(const int s, FaceVec& faces)
	{
		unsigned Top = (s + 1) * ((s * 2) + 1);

		//construct the tri list for the top
		for(int i = 0; i < (s * 2); i++)
		{
			Face F1;
			F1.P1 = Top;
			F1.P2 = Top + i + 1;
			F1.P3 = Top + i + 2;

			F1.T1 = F1.P1, F1.T2 = F1.P2, F1.T3 = F1.P3;
			faces.push_back(F1);
		}
	}

	void CalculateCylinderTrisBot(const int s, FaceVec& faces)
	{
		unsigned Top = (s + 1) * ((s * 2) + 1), Bottom = Top + (s * 2) + 2;

		//construct the tri list for the bottom
		for(int i = 0; i < (s * 2); i++)
		{
			Face F1;
			F1.P1 = Bottom;
			F1.P2 = Bottom + i + 1;
			F1.P3 = Bottom + i + 2;

			F1.T1 = F1.P1, F1.T2 = F1.P2, F1.T3 = F1.P3;
			faces.push_back(F1);
		}
	}
	//-----Cylinder Shape Functions-----//

	//-----Torus Shape Functions-----//
	void CalculateTorusData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces)
	{
		CalculateTorusVerts(s, verts, tcs, norms);
		CalculateTorusTris(s, faces);
	}

	void CalculateTorusVerts(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, const float id, const float od)
	{
		float RA = (od - id) / 4.0f; //the radius of the ring
		float RB = (id / 2.0f) + RA; //the radius of the core circle

		for(int i = 0; i <= s; i++)
		{
			float Y = float(i) / float(s);
			Point P(0.0f, sin(-Y * 2.0f * float(PI)) * RA, RB - cos(-Y * 2.0f * float(PI)) * RA);
			Vector N(0.0f, sin(-Y * 2.0f * float(PI)), -cos(-Y * 2.0f * float(PI)));

			for(int j = 0; j <= (s * 2); j++)
			{
				float X = float(j) / float(s * 2);

				Point V0(sin(X * 2.0f * PI) * P.Z, P.Y, cos(X * 2.0f * PI) * P.Z);
				Point Tc0(X, 1.0f - Y, 0.0f);
				Vector Nrm0(sin(X * 2.0f * PI) * N.Z, N.Y, cos(X * 2.0f * PI) * N.Z);
				Nrm0.NormalizeMe();

				verts.push_back(V0);
				norms.push_back(Nrm0);
				tcs.push_back(Tc0);
			}
		}
	}

	void CalculateTorusTris(const int s, FaceVec& faces)
	{
		for(int i = 0; i < s; i++)
		{
			for(int j = 0; j < (s * 2); j++)
			{
				Face F1, F2;

				F1.P1 = (i + 1) * ((s * 2) + 1) + j;
				F1.P2 = i * ((s * 2) + 1) + j;
				F1.P3 = (i + 1) * ((s * 2) + 1) + j + 1;

				F2.P1 = F1.P3;
				F2.P2 = F1.P2;
				F2.P3 = F1.P2 + 1;

				F1.T1 = F1.P1, F1.T2 = F1.P2, F1.T3 = F1.P3;
				F2.T1 = F2.P1, F2.T2 = F2.P2, F2.T3 = F2.P3;

				faces.push_back(F1);
				faces.push_back(F2);
			}
		}
	}
	//-----Torus Shape Functions-----//

	//-----Misc Functions-----//
	void CalculateTBData(const FaceVec& faces, VectorVec& vtan, VectorVec& vbit, const unsigned vnum, const unsigned fnum)
	{
		std::map< unsigned, std::vector<int> > cf; //find all connecting faces
		for(unsigned int i = 0; i < fnum; i++)//make a vector of all the faces each vertex is connected to
		{
			cf[faces[i].P1].push_back(i);
			cf[faces[i].P2].push_back(i);
			cf[faces[i].P3].push_back(i);
		}//now we know which faces each vertex is connected to

		//calculate per-vertex T&B by averaging the values of each connecting face
		for(unsigned int i = 0; i < vnum; i++)
		{
			Vector VertTan, VertBit;
			unsigned ConFaceCount = cf[i].size();

			for(unsigned int j = 0; j < ConFaceCount; j++)
			{
				VertTan += faces[cf[i][j]].T;
				VertBit += faces[cf[i][j]].B;
			}

			VertTan /= float(ConFaceCount);
			VertBit /= float(ConFaceCount);

			VertTan.NormalizeMe();
			VertBit.NormalizeMe();

			vtan.push_back(VertTan);
			vbit.push_back(VertBit);
		}
	}
	//-----Misc Functions-----//
}

#include "ShapeLib.h"
#include "Graphics.h"

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
Shape::Shape(int type, int s)
{
	Type = type;
	MyColor = Color(0.0f, 1.0f, 0.0f, 0.0f); //default color of green, because I like green
	Stacks = s;
	Slices = s; //stacks and slices currently always the same, may change sometime

	PointVec TexCoords, Vertices;
	VectorVec Normals, VertT, VertB;
	FaceVec TriangleList;

	switch(type) //generate the vertices, tex coords, normals and triangle lists based on shape type
	{
	case PLANE:
		{
			CalculatePlaneData(s, Vertices, TexCoords, Normals, TriangleList);
			break;
		}

	case CUBE:
		{
			CalculatePlaneData(s, Vertices, TexCoords, Normals, TriangleList); //this makes the last face,
			CalculateCubeData(s, Vertices, TexCoords, Normals, TriangleList);  //this makes the other 5 and positions the last
			break;
		}

	case SPHERE:
		{
			CalculateSphereData(s, Vertices, TexCoords, Normals, TriangleList);
			break;
		}
	case CONE:
		{
			CalculateConeData(s, Vertices, TexCoords, Normals, TriangleList);
			break;
		}
	}

	VertCount = Vertices.size(), FaceCount = TriangleList.size();

	VA = new float[VertCount * 3];
	VN = new float[VertCount * 3];
	FA = new unsigned short[FaceCount * 3];
	TCA = new float[VertCount * 3];

	TanA = new float[VertCount * 3];
	BitA = new float[VertCount * 3];

	this->SetIndexArrays(Vertices, TexCoords, Normals); //fill the index array
	this->SetFaceArray(TriangleList); //fill the face array

	for(unsigned i = 0; i < FaceCount; i++) //calculate T and B for each face
		TriangleList[i].CalculateTB(TCA, VA);

	CalculateTBData(TriangleList, VertT, VertB, VertCount, FaceCount);
	this->SetTBArrays(VertT, VertB); //fill the tan and bit arrays
}

Shape::~Shape(void)
{
	delete [] VA;
	delete [] VN;
	delete [] FA;
	delete [] TCA;

	delete [] TanA;
	delete [] BitA;
}

void Shape::SetIndexArrays(const PointVec& verts, const PointVec& tcs, const VectorVec& norms)
{
	for(unsigned int i = 0; i < VertCount; i++)
	{
		VA[i*3 + 0] = verts[i].X;
		VA[i*3 + 1] = verts[i].Y;
		VA[i*3 + 2] = verts[i].Z;

		VN[i*3 + 0] = norms[i].X;
		VN[i*3 + 1] = norms[i].Y;
		VN[i*3 + 2] = norms[i].Z;

		TCA[i*3 + 0] = tcs[i].X;
		TCA[i*3 + 1] = tcs[i].Y;
		TCA[i*3 + 2] = tcs[i].Z;
	}
}

void Shape::SetFaceArray(const FaceVec& faces)
{
	for(unsigned int i = 0; i < FaceCount; i++)
	{
		FA[i*3 + 0] = faces[i].P1;
		FA[i*3 + 1] = faces[i].P2;
		FA[i*3 + 2] = faces[i].P3;
	}
}

void Shape::SetTBArrays(const VectorVec& tana, const VectorVec& bita)
{
	for(unsigned i = 0; i < VertCount; i++)//transfer the T&B values to the arrays
	{
		TanA[i*3 + 0] = tana[i].X;
		TanA[i*3 + 1] = tana[i].Y;
		TanA[i*3 + 2] = tana[i].Z;

		BitA[i*3 + 0] = bita[i].X;
		BitA[i*3 + 1] = bita[i].Y;
		BitA[i*3 + 2] = bita[i].Z;
	}
}

void Shape::DrawSelf(bool wire)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, VA);
	glNormalPointer(GL_FLOAT, 0, VN);
	glTexCoordPointer(3, GL_FLOAT, 0, TCA);

	if(wire)
	{
		//draw this in wire mode
	}
	else
		glDrawElements(GL_TRIANGLES, FaceCount * 3, GL_UNSIGNED_SHORT, FA);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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

	T = (Q * Tx1) - (P * Tx2) / Denom;
	B = (P * Sx2) - (Q * Sx1) / Denom;
}
//-----Face Class Functions-----//

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

		vtan.push_back(VertTan);
		vbit.push_back(VertBit);
	}
}
void CalculatePlaneData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces)
{
	for(int j = 0; j < s + 1; j++)
	{
		float Y1 = -(float(j) / float(s + 1)) + 0.5f;
		float Y2 = -(float(j + 1) / float(s + 1)) + 0.5f;

		for(int i = 0; i <= s + 1; i++)
		{
			float X = float(i) / float(s + 1);
			//calculates points 2 at a time, calculate the tex coords and normals
			Point Tc0(X, 1.0f - (Y1 + 0.5f)), Tc1(X, 1.0f - (Y2 + 0.5f));
			Point V0(X - 0.5f, Y1, 0.0f), V1(X - 0.5f, Y2, 0.0f);
			Vector Nrm0(0.0f, 0.0f, 1.0f), Nrm1 = Nrm0;

			if(i != 0)
			{
				Face F1, F2;
				unsigned VertNum = verts.size();
				//the last 4 vertices added will form 2 triangles, calculate the faces
				unsigned Ind0 = VertNum - 4, Ind1 = VertNum - 3, Ind2 = VertNum - 2, Ind3 = VertNum - 1;

				F1.P1 = Ind1, F1.P2 = Ind3, F1.P3 = Ind2;
				F2.P1 = Ind2, F2.P2 = Ind0, F2.P3 = Ind1;

				F1.T1 = F1.P1, F1.T2 = F1.P2, F1.T3 = F1.P3;
				F2.T1 = F2.P1, F2.T2 = F2.P2, F2.T3 = F2.P3;

				faces.push_back(F1);
				faces.push_back(F2);
			}

			verts.push_back(V0);
			verts.push_back(V1);

			norms.push_back(Nrm0);
			norms.push_back(Nrm1);

			tcs.push_back(Tc0);
			tcs.push_back(Tc1);
		}
	}
}

void CalculateCubeData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces)
{
	unsigned VertNum = verts.size(), FaceNum = faces.size();
	float A = float(PI) / 2.0f;
	Vector Trans, Nrm;
	Matrix4x4 Rot;

	for(int i = 1; i <= 6; i++)
	{
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
				Trans.Y = 0.5f; A *= -1.0f;
				Rot.Rotate(Vector(1.0f, 0.0f, 0.0f), A);
				Nrm = Vector(0.0f, 1.0f, 0.0f);
				break;
			}
		case 4: //bottom side of the cube
			{
				Trans.Y = -0.5f;
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

void CalculateSphereData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces)
{
	for(int j = 0; j < s + 1; j++)
	{
		float Y1 = -(float(j) / float(s + 1)) + 0.5f;
		float Y2 = -(float(j + 1) / float(s + 1)) + 0.5f;
		float A1 = float(Y1 * PI), A2 = float(Y2 * PI);

		for(int i = 0; i <= s + 1; i++)
		{
			float X = float(i) / float(s + 1);
			float B = float(X * 2.0f * PI);

			Point Tc0(abs(1.0f - X), 1.0f - (Y1 + 0.5f));
			Point Tc1(abs(1.0f - X), 1.0f - (Y2 + 0.5f));
			Point V0(0.5f * cos(A1) * cos(B), 0.5f * sin(A1), 0.5f * cos(A1) * sin(B));
			Point V1(0.5f * cos(A2) * cos(B), 0.5f * sin(A2), 0.5f * cos(A2) * sin(B));
			Vector Nrm0(cos(A1) * cos(B), sin(A1), cos(A1) * sin(B));
			Vector Nrm1(cos(A2) * cos(B), sin(A2), cos(A2) * sin(B));

			verts.push_back(V0);
			verts.push_back(V1);

			norms.push_back(Nrm0);
			norms.push_back(Nrm1);

			if(i != 0)
			{
				Face F1, F2;
				unsigned VertNum = verts.size();
				//the last 4 vertices added will form 2 triangles
				unsigned Ind0 = VertNum - 4, Ind1 = VertNum - 3, Ind2 = VertNum - 2, Ind3 = VertNum - 1;

				F1.P1 = Ind1, F1.P2 = Ind2, F1.P3 = Ind3;
				F2.P1 = Ind1, F2.P2 = Ind0, F2.P3 = Ind2;

				F1.T1 = F1.P1, F1.T2 = F1.P2, F1.T3 = F1.P3;
				F2.T1 = F2.P1, F2.T2 = F2.P2, F2.T3 = F2.P3;

				faces.push_back(F1);
				faces.push_back(F2);
			}

			tcs.push_back(Tc0);
			tcs.push_back(Tc1);
		}
	}
}

void CalculateConeData(const int s, PointVec &verts, PointVec &tcs, VectorVec& norms, FaceVec& faces)
{
	unsigned Bottom = 0;
	//construct the verts for the top
	for(int i = 0; i <= s; i++)
	{
		float Y = float(i) / float(s);

		for(int j = 0; j <= (s * 2); j++, Bottom++)
		{
			float X = float(j) / float(s * 2);
			float A = X * 2.0f * float(PI);
			Point V0(sin(A) / 2.0f, Y - 0.5f, cos(A) / 2.0f);
			Point Tc0(X, 1.0f - Y, 0.0f);
			Vector Nrm0(V0.X, sqrt((V0.X * V0.X) + (V0.Z * V0.Z)) / 2.0f, V0.Z);
			Nrm0.NormalizeMe();

			V0.X *= 1.0f - Y;
			V0.Z *= 1.0f - Y;

			verts.push_back(V0);
			norms.push_back(Nrm0);
			tcs.push_back(Tc0);
		}
	}

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
//-----Misc Functions-----//
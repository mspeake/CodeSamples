/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: ObjReader.cpp
Purpose: implementation of my scene loading functions
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 5/22/2012
End Header --------------------------------------------------------*/
#include "ObjReader.h"
#include <GL/glui.h>
#include "ShaderEditor.h"

std::vector<Object *> object_list;

extern GLUI_FileBrowser *FB;
extern GLUI_EditText *KD_Node_Depth;
extern float CamDist;

std::string FileName = "all_scenes\\environment1.txt";
LevelHelper LL;


void NormalizeMe(Vector3f &vec)
{
	float magnitude = sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
	vec.x /= magnitude;
	vec.y /= magnitude;
	vec.z /= magnitude;
}
void CrossProd(const Vector3f L, const Vector3f R, Vector3f &dest)
{
	dest.x = (L.y * R.z) - (L.z * R.y);
	dest.y = (L.z * R.x) - (L.x * R.z);
	dest.z = (L.x * R.y) - (L.y * R.x);
}

void FBCB(int num)
{
	if(num == 11)//file browser id
	{
		FileName = FB->get_file();
		FILE *fp = fopen(FileName.c_str(), "r");
		if(fp)
		{
			/*object_list.clear();
			LoadScene(FileName.c_str());
			selected_obj = object_list.begin();*/
			EnvObj.clear();
			NpcObj.clear();
			LL.LoadStuff(FileName.c_str());
		}
	}
}

void KDCB(int num)
{
	if(num == 22)
	{
		if(Tree_Depth != MAX_TREE_DEPTH)
		{
			++Tree_Depth;
			KD_Node_Depth->set_float_val(Tree_Depth);
		}
	}
	if(num == 33)
	{
		if(Tree_Depth > 0)
		{
			--Tree_Depth;
			KD_Node_Depth->set_float_val(Tree_Depth);
		}
	}
}


Shape::Shape(int _type, int _s)
{
	type = _type;
	color.R = 0.0f, color.G = 1.0f, color.B = 0.0f;
	stacks = _s;
	slices = _s;

	std::vector<Vector> vertices, normals, tex_coords;
	std::vector<Face> TriangleList;
	std::vector<Vector> vert_T, vert_B;

	for(int j = 0; j < stacks+1; j++)
	{
		float y1 = float(j)   / float(stacks+1);
		float y2 = float(j+1) / float(stacks+1);
		y1 = (-y1+0.5f), y2 = (-y2+0.5f);
		float a1 = y1  * PI, a2 = y2 * PI;

		//if(j != 0)//(int)(stacks/2)) //for looking at individual strips
		//	continue;

		for(int i = 0; i <= slices+1; i++)
		{
			float x = float(i) / float(slices+1);
			float b = (x * 360.0f)/180.0f * PI;
			Vector v0, v1;
			Vector tc0, tc1;

			tc0.x = x, tc0.y = 1.0f - (y1 + 0.5f);
			tc1.x = x, tc1.y = 1.0f - (y2 + 0.5f);

			if(type < 2) //if its a cube, pretend its a plane for now
			{
				v0.x = float(x-0.5), v0.y = y1, v0.z = 0.0f;
				v1.x = float(x-0.5), v1.y = y2, v1.z = 0.0f;
			}
			else
			{
				v0.x = 0.5f * cos(a1) * cos(b), v0.y = 0.5f * sin(a1), v0.z = 0.5f * cos(a1) * sin(b);
				v1.x = 0.5f * cos(a2) * cos(b), v1.y = 0.5f * sin(a2), v1.z = 0.5f * cos(a2) * sin(b);
			}


			Vector nrm0, nrm1;
			if(type < 2)
			{
				nrm0.x = 0.0f, nrm0.y = 0.0f, nrm0.z = 1.0f;
				nrm1 = nrm0;
			}
			else
			{
				nrm0.x = cos(a1) * cos(b), nrm0.y = sin(a1), nrm0.z = cos(a1) * sin(b);
				nrm1.x = cos(a2) * cos(b), nrm1.y = sin(a2), nrm1.z = cos(a2) * sin(b);
				tc0.x = abs(1.0f - x), tc1.x = abs(1.0f - x);
			}

			vertices.push_back(v0);
			vertices.push_back(v1);

			normals.push_back(nrm0);
			normals.push_back(nrm1);

			if(i != 0)
			{
				Face f1, f2;
				int ind_0, ind_1, ind_2, ind_3;
				 //the last 4 vertices added will form 2 triangles
				ind_0 = vertices.size() - 4, ind_1 = vertices.size() - 3;
				ind_2 = vertices.size() - 2, ind_3 = vertices.size() - 1;

				if(type == 2) //this algorithm makes a sphere's triangles clockwise, flip to correct this problem
				{
					f1.p1 = ind_1, f1.p2 = ind_2, f1.p3 = ind_3;
					f2.p1 = ind_1, f2.p2 = ind_0, f2.p3 = ind_2;
				}
				else
				{
					f1.p1 = ind_1, f1.p2 = ind_3, f1.p3 = ind_2;
					f2.p1 = ind_2, f2.p2 = ind_0, f2.p3 = ind_1;
				}

				f1.t1 = f1.p1, f1.t2 = f1.p2, f1.t3 = f1.p3;
				f2.t1 = f2.p1, f2.t2 = f2.p2, f2.t3 = f2.p3;

				TriangleList.push_back(f1);
				TriangleList.push_back(f2);
			}

			tex_coords.push_back(tc0);
			tex_coords.push_back(tc1);
		}
	}

	if(type == CUBE)
	{
		int vert_size = vertices.size(), face_size = TriangleList.size();

		for(int i = 1; i <= 6; i++)
		{
			float a = (90.0f / 180.0f) * PI;
			Vector t(0.0f, 0.0f, 0.0f);
			switch(i)
			{
			case 1: //right side of the cube
				{
					t.x = 0.5f;

					for(unsigned j = 0; j < vert_size; j++)
					{
						Vector p2(vertices[j]), nrm(1.0f, 0.0f, 0.0f), tc(tex_coords[j]);
						RotateY(p2, a);
						Translate(p2, t.x, t.y, t.z);

						vertices.push_back(p2);
						normals.push_back(nrm);
						tex_coords.push_back(tc);
					}

					for(unsigned j = 0; j < face_size; j++)
					{
						Face f2(TriangleList[j]);
						f2.p1 += (i*vert_size); f2.p2 += (i*vert_size); f2.p3 += (i*vert_size);
						f2.t1 = f2.p1, f2.t2 = f2.p2, f2.t3 = f2.p3;
						TriangleList.push_back(f2);
					}
					break;
				}
			case 2: //left side of the cube
				{
					t.x = -0.5f; a *= -1.0f;

					for(unsigned j = 0; j < vert_size; j++)
					{
						Vector p2(vertices[j]), nrm(-1.0f, 0.0f, 0.0f), tc(tex_coords[j]);
						RotateY(p2, a);
						Translate(p2, t.x, t.y, t.z);

						vertices.push_back(p2);
						normals.push_back(nrm);
						tex_coords.push_back(tc);
					}

					for(unsigned j = 0; j < face_size; j++)
					{
						Face f2(TriangleList[j]);
						f2.p1 += (i*vert_size); f2.p2 += (i*vert_size); f2.p3 += (i*vert_size);
						f2.t1 = f2.p1, f2.t2 = f2.p2, f2.t3 = f2.p3;
						TriangleList.push_back(f2);
					}
					break;
				}
			case 3: //top side of the cube
				{
					t.y = 0.5f; a *= -1.0f;

					for(unsigned j = 0; j < vert_size; j++)
					{
						Vector p2(vertices[j]), nrm(0.0f, 1.0f, 0.0f), tc(tex_coords[j]);
						RotateX(p2, a);
						Translate(p2, t.x, t.y, t.z);

						vertices.push_back(p2);
						normals.push_back(nrm);
						tex_coords.push_back(tc);
					}

					for(unsigned j = 0; j < face_size; j++)
					{
						Face f2(TriangleList[j]);
						f2.p1 += (i*vert_size); f2.p2 += (i*vert_size); f2.p3 += (i*vert_size);
						f2.t1 = f2.p1, f2.t2 = f2.p2, f2.t3 = f2.p3;
						TriangleList.push_back(f2);
					}
					break;
				}
			case 4: //bottom side of the cube
				{
					t.y = -0.5f;

					for(unsigned j = 0; j < vert_size; j++)
					{
						Vector p2(vertices[j]), nrm(0.0f, -1.0f, 0.0f), tc(tex_coords[j]);
						RotateX(p2, a);
						Translate(p2, t.x, t.y, t.z);

						vertices.push_back(p2);
						normals.push_back(nrm);
						tex_coords.push_back(tc);
					}

					for(unsigned j = 0; j < face_size; j++)
					{
						Face f2(TriangleList[j]);
						f2.p1 += (i*vert_size); f2.p2 += (i*vert_size); f2.p3 += (i*vert_size);
						f2.t1 = f2.p1, f2.t2 = f2.p2, f2.t3 = f2.p3;
						TriangleList.push_back(f2);
					}
					break;
				}
			case 5: //back side of the cube
				{
					t.z = -0.5f; a *= 2.0f;

					for(unsigned j = 0; j < vert_size; j++)
					{
						Vector p2(vertices[j]), nrm(0.0f, 0.0f, -1.0f), tc(tex_coords[j]);
						RotateY(p2, a);
						Translate(p2, t.x, t.y, t.z);

						vertices.push_back(p2);
						normals.push_back(nrm);
						tex_coords.push_back(tc);
					}

					for(unsigned j = 0; j < face_size; j++)
					{
						Face f2(TriangleList[j]);
						f2.p1 += (i*vert_size); f2.p2 += (i*vert_size); f2.p3 += (i*vert_size);
						f2.t1 = f2.p1, f2.t2 = f2.p2, f2.t3 = f2.p3;
						TriangleList.push_back(f2);
					}
					break;
				}
			case 6: //the front face, only need to translate the first plane along +z
				{
					t.z = 0.5f;
					for(unsigned j = 0; j < vert_size; j++)
					{
						Translate(vertices[j], t.x, t.y, t.z);
					}
					break;
				}
			}
		}
	}

	VA = new float[vertices.size() * 3];
	VN = new float[vertices.size() * 3];
	FA = new unsigned short[TriangleList.size() * 3];
	TCA = new float[vertices.size() * 3];

	TanA = new float[vertices.size() * 3];
	BitA = new float[vertices.size() * 3];

	//fill the index array
	for(unsigned int i  = 0; i < vertices.size(); i++)
	{
		VA[i*3 + 0] = vertices[i].x;
		VA[i*3 + 1] = vertices[i].y;
		VA[i*3 + 2] = vertices[i].z;

		VN[i*3 + 0] = normals[i].x;
		VN[i*3 + 1] = normals[i].y;
		VN[i*3 + 2] = normals[i].z;

		TCA[i*3 + 0] = tex_coords[i].x;
		TCA[i*3 + 1] = tex_coords[i].y;
		TCA[i*3 + 2] = tex_coords[i].z;
	}
	//fill the face array
	for(unsigned int i = 0; i < TriangleList.size(); i++)
	{
		FA[i*3 + 0] = TriangleList[i].p1;
		FA[i*3 + 1] = TriangleList[i].p2;
		FA[i*3 + 2] = TriangleList[i].p3;
	}

	vert_count = vertices.size();
	face_count = TriangleList.size();

	//calculate T and B for each face
	for(unsigned i = 0; i < face_count; i++)
		CalculateTB(&TriangleList[i], TCA, VA);

	std::vector< std::vector<int> > cf; //find all connecting faces
	for(unsigned int i = 0; i < vertices.size(); i++) //fill cf with empty vectors
	{
		std::vector<int> vec;
		cf.push_back(vec);
	}
	for(unsigned int i = 0; i < face_count; i++)//make a vector of all the faces each vertex is connected to
	{
		cf[TriangleList[i].p1].push_back(i);
		cf[TriangleList[i].p2].push_back(i);
		cf[TriangleList[i].p3].push_back(i);
	}//now we know which faces each vertex is connected to

	//calculate per-vertex T&B by averaging the values of each connecting face
	for(unsigned int i = 0; i < vert_count; i++)
	{
		Vector vertT, vertB;
		for(unsigned int j = 0; j < cf[i].size(); j++)
		{
			vertT.x += TriangleList[cf[i][j]].T.x;
			vertT.y += TriangleList[cf[i][j]].T.y;
			vertT.z += TriangleList[cf[i][j]].T.z;

			vertB.x += TriangleList[cf[i][j]].B.x;
			vertB.y += TriangleList[cf[i][j]].B.y;
			vertB.z += TriangleList[cf[i][j]].B.z;
		}
		vertT.x /= cf[i].size();
		vertT.y /= cf[i].size();
		vertT.z /= cf[i].size();

		vertB.x /= cf[i].size();
		vertB.y /= cf[i].size();
		vertB.z /= cf[i].size();

		vert_T.push_back(vertT);
		vert_B.push_back(vertB);
	}

	for(unsigned i = 0; i < vert_count; i++)//transfer the T&B values to the arrays
	{
		TanA[i*3 + 0] = vert_T[i].x;
		TanA[i*3 + 1] = vert_T[i].y;
		TanA[i*3 + 2] = vert_T[i].z;

		BitA[i*3 + 0] = vert_B[i].x;
		BitA[i*3 + 1] = vert_B[i].y;
		BitA[i*3 + 2] = vert_B[i].z;
	}
}

Shape::~Shape(void)
{
	/*this->vertices.clear();
	this->TriangleList.clear();*/

	delete [] VA;
	delete [] VN;
	delete [] FA;
	delete [] TCA;

	delete [] TanA;
	delete [] BitA;
}


Object::Object(void)
{
	S = NULL;
}

Object::~Object(void)
{
}

void CalculateTB(Face *t, const float *TCs, const float *Verts)
{
	//by manipulating the equations [P = s1*T + t1*B]  and  [Q = s2*T + t2*B]
	//we get [T = (t1*Q - t2*P) / (s2*t1 - s1*t2)]
	//and    [B = (s2*P - s1*Q) / (s2*t1 - s1*t2)]

	//s1 = v2x-v1x, s2 = v3x-v1x(texture coords)
	float s1 = TCs[(t->t2)*3 + 0] - TCs[(t->t1)*3 + 0];//t->v2.tex.x - t->v1.tex.x; 
	float s2 = TCs[(t->t3)*3 + 0] - TCs[(t->t1)*3 + 0];//t->v3.tex.x - t->v1.tex.x;

	//t1 = v2y-v1y, s2 = v3y-v1y(texture coords)
	float t1 = TCs[(t->t2)*3 + 1] - TCs[(t->t1)*3 + 1];//t->v2.tex.y - t->v1.tex.y; 
	float t2 = TCs[(t->t3)*3 + 1] - TCs[(t->t1)*3 + 1];//t->v3.tex.y - t->v1.tex.y;

	float denominator = (s2*t1) - (s1*t2);
	if(denominator == 0)
	{	//if dividing by 0, set T and B to 0
		t->T = Vector(0.0f,0.0f,0.0f);
		t->B = Vector(0.0f,0.0f,0.0f);
		return;
	}

	//P = v2 - v1, Q = v3 - v1
	Vector P(Verts[(t->p2)*3 + 0] - Verts[(t->p1)*3 + 0], 
		     Verts[(t->p2)*3 + 1] - Verts[(t->p1)*3 + 1], 
			 Verts[(t->p2)*3 + 2] - Verts[(t->p1)*3 + 2]);

	Vector Q(Verts[(t->p3)*3 + 0] - Verts[(t->p1)*3 + 0], 
		     Verts[(t->p3)*3 + 1] - Verts[(t->p1)*3 + 1], 
			 Verts[(t->p3)*3 + 2] - Verts[(t->p1)*3 + 2]);

	float numB, numT;

	numT = (t1*Q.x) - (t2*P.x);
	t->T.x = numT / denominator;
	numT = (t1*Q.y) - (t2*P.y);
	t->T.y = numT / denominator;
	numT = (t1*Q.z) - (t2*P.z);
	t->T.z = numT / denominator;

	numB = (s2*P.x) - (s1*Q.x);
	t->B.x = numB / denominator;
	numB = (s2*P.y) - (s1*Q.y);
	t->B.y = numB / denominator;
	numB = (s2*P.z) - (s1*Q.z);
	t->B.z = numB / denominator;
}

bool LoadScene(std::string filename)
{
	std::ifstream fs;
	fs.open(filename.c_str());

	if(fs.fail())//file not found
		return false;

	else //file found, load the obj file
	{
		std::string line;

		while(std::getline(fs, line))
		{
			Object *obj = new Object();
			std::stringstream ss(line);
			char comma;

			ss >> obj->type >> comma >> obj->subdivisions >> comma;
			ss >> obj->position.x >> comma >> obj->position.y >> comma >> obj->position.z >> comma;
			ss >> obj->scale.x >> comma >> obj->scale.y >> comma >> obj->scale.z >> comma;
			ss >> obj->axis.x >> comma >> obj->axis.y >> comma >> obj->axis.z >> comma >> obj->angle >> comma;
			ss >> obj->color.R >> comma >> obj->color.G >> comma >> obj->color.B >> comma >> obj->color.A;

			switch(obj->type)
			{
			case PLANE:
				{
					obj->S = new Shape(PLANE, obj->subdivisions);
					break;
				}
			case CUBE:
				{
					obj->S = new Shape(CUBE, obj->subdivisions);
					break;
				}
			case SPHERE:
				{
					obj->S = new Shape(SPHERE, obj->subdivisions);
					break;
				}
			}

			Matrix4x4 trans, rot, sc, tr, transform;
			trans.Translate(obj->position.x, obj->position.y, obj->position.z);
			rot.Rotate(obj->axis, obj->angle);
			sc.Scale(obj->scale.x, obj->scale.y, obj->scale.z);
			tr = trans * rot;
			transform = tr * sc;
			for(unsigned i = 0; i < obj->S->vert_count; i++)
			{
				float tx = obj->position.x, ty = obj->position.y, tz = obj->position.z;
				Point p0(obj->S->VA[i*3 + 0], obj->S->VA[i*3 + 1], obj->S->VA[i*3 + 2]);
				Vector n0(obj->S->VN[i*3 + 0], obj->S->VN[i*3 + 1], obj->S->VN[i*3 + 2]);
				Point p1 = transform * p0;
				Vector n1 = rot * n0;
				obj->S->VA[i*3 + 0] = p1.x;
				obj->S->VA[i*3 + 1] = p1.y;
				obj->S->VA[i*3 + 2] = p1.z;
				obj->S->VN[i*3 + 0] = n1.x;
				obj->S->VN[i*3 + 1] = n1.y;
				obj->S->VN[i*3 + 2] = n1.z;
			}
			object_list.push_back(obj);
		}
		fs.close();
	}

	return true;
}


bool LevelHelper::LoadStuff(std::string filename)
{
	std::ifstream fs;
	fs.open(filename.c_str());

	if(fs.fail())//file not found
		return false;

	else //file found, load the obj file
	{
		std::string line;

		while(std::getline(fs, line))
		{
			GameObj *obj = NULL; //new Object();
			std::stringstream ss(line);
			char comma;
			int obj_type, shape_type, subdivs, tex;
			Vector scale, axis;
			Point pos;
			Color col;
			float angle;
			std::string tex_name;

			ss >> obj_type >> comma;
			ss >> shape_type >> comma >> subdivs >> comma;
			ss >> pos.x >> comma >> pos.y >> comma >> pos.z >> comma;
			ss >> scale.x >> comma >> scale.y >> comma >> scale.z >> comma;
			ss >> axis.x >> comma >> axis.y >> comma >> axis.z >> comma >> angle >> comma;
			ss >> col.R >> comma >> col.G >> comma >> col.B >> comma >> col.A >> comma >> tex_name;

			tex = LookupTexture(tex_name);

			switch(obj_type)
			{
			case ENVIRONMENT_OBJ:
				{
					obj = new EnvironmentObj(shape_type, subdivs, scale.x, scale.y, scale.z, tex);
					break;
				}
			case ENEMY_OBJ:
				{
					obj = new EnemyObj(shape_type, subdivs, scale.x, scale.y, scale.z, tex);
					break;
				}
			case PLAYER_OBJ:
				{
					obj = new PlayerObj(shape_type, subdivs, scale.x, scale.y, scale.z, tex);
					break;
				}
			case HUD_OBJ:
				{
					obj = new GameObj(PLANE, subdivs, scale.x, scale.y, scale.z, tex);
					break;
				}
			}

			obj->Graph->angle = angle;
			obj->Graph->axis = axis;
			obj->Graph->color = col;
			obj->Pos = pos;
			
			switch(obj_type)
			{
			case ENVIRONMENT_OBJ:
				{
					EnvObj.push_back(obj);
					break;
				}
			case ENEMY_OBJ:
				{
					NpcObj.push_back(obj);
					break;
				}
			case PLAYER_OBJ:
				{
					NpcObj.push_back(obj);
					break;
				}
			case HUD_OBJ:
				{
					Button *b = new Button(pos, scale, col, tex, shape_type);
					HudObj.push_back(b);
					delete obj;
					break;
				}
			}

		}
		fs.close();
	}

	return true;
}

bool LevelHelper::LoadPaths(std::string filename)
{
	if(NpcObj.empty())
		return false;

	std::ifstream fs;
	fs.open(filename.c_str());

	if(fs.fail())//file not found
		return false;

	else //file found, load the obj file
	{
		std::string line;

		while(std::getline(fs, line))
		{
			std::stringstream ss(line);
			char comma;
			int EnemyNum;
			Point pos;

			ss >> EnemyNum >> comma;
			ss >> pos.x >> comma >> pos.y >> comma >> pos.z >> comma;

			EnemyObj * EO = reinterpret_cast<EnemyObj*>(NpcObj[EnemyNum]);
			EO->AddPatrolPoint(pos);
		}
		fs.close();
	}

	for(unsigned i = 0; i < NpcObj.size(); i++)
	{
		EnemyObj * EO = reinterpret_cast<EnemyObj*>(NpcObj[i]);
		int type = EO->GetEnemyType();
		if(type == PATROL_ENEMY)
			EO->InitializePatrol();
	}

	return true;
}

bool LevelHelper::LoadEnemyData(std::string filename)
{
	if(NpcObj.empty())
		return false;

	std::ifstream fs;
	fs.open(filename.c_str());

	if(fs.fail())//file not found
		return false;

	else //file found, load the obj file
	{
		std::string line;

		while(std::getline(fs, line))
		{
			std::stringstream ss(line);
			char comma;
			int EnemyNum, Behavior_Type, Elemental_Type;

			ss >> EnemyNum >> comma >> Behavior_Type >> comma >> Elemental_Type;

			if(EnemyNum < NpcObj.size())
			{
				EnemyObj * EO = reinterpret_cast<EnemyObj*>(NpcObj[EnemyNum]);
				EO->SetEnemyType(Behavior_Type);

				switch(Behavior_Type)
				{
				case PATROL_ENEMY:
					{
						EO->CurrentState = PATROLLING;
						break;
					}

				case OBSTACLE_ENEMY:
					{
						EO->CurrentState = IDLE;
						break;
					}
				}

				EO->SetElementalType(Elemental_Type);
				Color EnemyColor;
				switch(Elemental_Type)
				{
				case ET_EARTH:
					{
						EnemyColor.R = EARTH_R;
						EnemyColor.G = EARTH_G;
						EnemyColor.B = EARTH_B;
						break;
					}

				case ET_FIRE:
					{
						EnemyColor.R = FIRE_R;
						EnemyColor.G = FIRE_G;
						EnemyColor.B = FIRE_B;
						break;
					}
				
				case ET_WIND:
					{
						EnemyColor.R = WIND_R;
						EnemyColor.G = WIND_G;
						EnemyColor.B = WIND_B;
						break;
					}

				case ET_WATER:
					{
						EnemyColor.R = WATER_R;
						EnemyColor.G = WATER_G;
						EnemyColor.B = WATER_B;
						break;
					}
				}

				EO->SetColor(EnemyColor);
			}
		}
		fs.close();
	}

	return true;
}
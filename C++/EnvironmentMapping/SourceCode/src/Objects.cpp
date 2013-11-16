/********************************************************************
**Objects.cpp
**
**-Implements the Object class, used to associate Shape class objects
** (the model) with a position, scale, rotation, and texture
********************************************************************/

#include "Objects.h"

//-----Object Class Functions-----//
Object::Object(const std::string& format)
{
	std::stringstream SS(format);
	char Comma; //unused char to eat the comma separators from the CSV string
	unsigned ShapeType, Subdivs;
	float ID = TORUS_ID_DEFAULT, OD = TORUS_OD_DEFAULT;
	Vector Sc, Ax;
	Color Col;

	SS >> ShapeType >> Comma >> Subdivs >> Comma;
	if(ShapeType == TORUS)
	{
		SS >> ID >> Comma >> OD >> Comma;
		Model = Shape::CreateTorus(Subdivs, ID, OD);
	}
	else
		Model = new Shape(ShapeType, Subdivs);

	SS >> Pos.X >> Comma >> Pos.Y >> Comma >> Pos.Z >> Comma;
	SS >> Sc.X >> Comma >> Sc.Y >> Comma >> Sc.Z >> Comma;
	SS >> Ax.X >> Comma >> Ax.Y >> Comma >> Ax.Z >> Comma >> Angle >> Comma;
	SS >> Col.R >> Comma >> Col.G >> Comma >> Col.B >> Comma >> Col.A >> Comma;
	SS >> Texture;

	Scale = Vector(1.0f, 1.0f, 1.0f); //need to initialize scale's value to 1 before rescaling to the final scale
	if(Sc.X <= 0.0f || Sc.Y <= 0.0f || Sc.Z <= 0.0f)
		Sc = Vector(1.0f, 1.0f, 1.0f); //error checking to ensure scale is positive and non-zero
	
	if(Ax.IsZeroVector()) //error checking to ensure rotation axis is non-zero
	{
		Ax = Vector(1.0f, 0.0f, 0.0f);
		Angle = 0.0f;
	}

	this->SetScale(Sc); //need this function call to rescale the model too
	Axis = Ax;
	Model->MyColor = Col;
}

Object::~Object()
{
	delete Model;
}

void Object::DrawSelf(void) const
{
	glPushMatrix();

	glTranslatef(Pos.X, Pos.Y, Pos.Z);
	glRotatef(Angle, Axis.X, Axis.Y, Axis.Z);

	Color Col(Model->MyColor);
	glColor3f(Col.R, Col.G, Col.B);

	Model->DrawSelf();

	glPopMatrix();
}

Vector Object::GetScale(void) const
{
	return Scale;
}

void Object::SetScale(const Vector& sc)
{
	if(sc.IsZeroVector()) //dont allow zero scale
		return;

	Vector Ratio(sc.X / Scale.X, sc.Y / Scale.Y, sc.Z / Scale.Z); //shape rescale multiplies the verts by a vector,
	Model->ReScaleVertices(Ratio); //so we need to pass it a new/old scale ratio vector to get the right size
	Scale = sc;
}

unsigned Object::GetShapeType(void) const
{
	return Model->GetType();
}
//-----Object Class Functions-----//

namespace ObjectReader
{
	std::list<Object*> ObjectsList;

	bool CreatObjectsFromFile(std::list<Object*>& objects, const std::string& filename)
	{
		std::ifstream FS;
		FS.open(filename.c_str());
		
		if(FS.fail()) //file not found
			return false;
		else
		{
			std::string Line;

			while(std::getline(FS, Line))
			{
				Object* NewObj = new Object(Line);
				objects.push_back(NewObj);
			}

			FS.close();
		}

		return true;
	}
}
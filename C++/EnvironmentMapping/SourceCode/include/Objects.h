/********************************************************************
**Objects.h
**
**-Defines the Object class, used to associate Shapes(models) with
** position, scale, rotation, and texture to represent objects that
** can be drawn
********************************************************************/

#ifndef OBJECTS_H
#define OBJECTS_H

#include <list>

#include "ImageLoader.h"
#include "Shaders.h"

class Object
{
private:
	Shape* Model;
	Vector Scale;
	
public:
	Point Pos;
	Vector Axis;
	float Angle;
	std::string Texture;

	Object(const std::string& format);
	~Object();

	void DrawSelf(void) const;
	Vector GetScale(void) const;
	void SetScale(const Vector& sc); //need this to make sure model is rescaled properly
	unsigned GetShapeType(void) const;
};

namespace ObjectReader
{
	extern std::list<Object*> ObjectsList;

	bool CreatObjectsFromFile(std::list<Object*>& objects, const std::string& filename);
}

#endif
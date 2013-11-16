/********************************************************************
**ImageLoader.h
**
**-Defines the ImageLoader class, used for loading textures
********************************************************************/

#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <map>
#include <string>
#include <fstream>
#include <sstream>

#include "MathLib.h"
#include "Graphics.h"

class Image
{
public:
	static GLuint LoadTexture(std::string file);
	static GLuint LoadNormalMap(std::string file);
	static void GenTexture(GLuint& tex);

	const unsigned char* GetData(void) const {return Data;}
	unsigned char* GetData(void) {return Data;}
	unsigned long GetBPP(void) const {return BPP;}
	unsigned long GetSizeX(void) const {return SizeX;}
	unsigned long GetSizeY(void) const {return SizeY;}

	void NormalMap(void);
	void HeightMap(void);

private:
	unsigned char* Data;
	unsigned long BPP;
	unsigned long SizeX;
	unsigned long SizeY;

	Image();
	Image(const Image& rhs);
	~Image();

	static Image* LoadData(const char* pName);
	static void	Free(Image* img);

	bool LoadTGA(unsigned char* data);
	GLuint GenerateImage(void) const;
};

namespace Textures
{
	extern std::map<std::string, GLuint> TexList, NormMapList, HeightMapList;

	void LoadAllTextures(void);
	void LoadAllTextures(const std::string& file);
}

#endif
/********************************************************************
**ImageLoader.cpp
**
**-Implements the ImageLoader class for loading textures, currently
** supports .tga format, will expand to include others in the future
********************************************************************/

#include "ImageLoader.h"

//-----Image Class Functions-----//
Image* Image::LoadData(const char* name)
{
	Image* ImageData = new Image;

	// check the extension
	const char* Ext = name + strlen(name) - 4;

	FILE* Fp = NULL;
	fopen_s(&Fp, name, "rb");

	// get the file size
	fseek(Fp, 0, SEEK_END);
	unsigned long FileSize = ftell(Fp);
	fseek(Fp, 0, SEEK_SET);

	unsigned char* FileData = new unsigned char [FileSize];

	// read in the data
	unsigned long ReadSize = fread(FileData, 1, FileSize, Fp);

	if(ImageData->LoadTGA(FileData))
		return ImageData;

	// deallocate the image
	delete ImageData;

	return 0;
}


void Image::Free(Image* img)
{
    if(!img)
		return;
	
    delete img;
}


Image::Image() :
        Data(0),
        BPP(0),
        SizeX(0),
        SizeY(0)
{
}

Image::~Image()
{
    if(Data)
		delete [] Data;
}

GLuint Image::GenerateImage(void) const
{
	GLuint TexNum;
	
	GenTexture(TexNum);
	glTexImage2D(GL_TEXTURE_2D, 0, (BPP == 32) ? GL_RGBA : GL_RGB, SizeX, SizeY, 
		         0, (BPP == 32) ? GL_BGRA : GL_BGR, GL_UNSIGNED_BYTE, Data);
	
	return TexNum;
}

bool Image::LoadTGA(unsigned char* data)
{
	//get the image type
	unsigned char ImageType = data[2];

	//get the bits per pixel and image size
	BPP = data[16];
	SizeX = (data[13] << 8) | data[12];
	SizeY = (data[15] << 8) | data[14];

	// get the pointer to the image data area
	// * 18 is the header size
	// * the '0' entry is the number of bytes in the image id field (ignored!)
	unsigned char* ImageData = data + 18 + data[0];

	// allocate memory for the data
	Data = new unsigned char [SizeX * SizeY * BPP / 8];

	// get the image descriptor to get the orientation
	// * bit 5 (0 = bottom, 1 = top)
	// * bit 4 (0 = left, 1 = right)
	unsigned char Desc = data[17];
	unsigned long RowSize = SizeX * BPP / 8;

	// check if need to mirror the image vertically
	if((Desc & 0x20) == 0)
	{
		// mirror data upside down
		for(unsigned long y = 0; y < SizeY; ++y)
		{
			unsigned long* Src = (unsigned long*)(ImageData + y * RowSize);
			unsigned long* Dst = (unsigned long*)(Data + (SizeY - 1 - y) * RowSize);

			memcpy(Dst, Src, RowSize);
		}
	}
	else
		memcpy(Data, ImageData, SizeY * RowSize);

	// check if need to mirror the image horizontally
	if(Desc & 0x10)
	{
		for(unsigned long y = 0; y < SizeY; ++y)
		{
			for(unsigned long x = 0; x < SizeX / 2; ++x)
			{
				unsigned char* Src = Data + y * RowSize + x * BPP / 8;
				unsigned char* Dst = Data + y * RowSize + (SizeX - 1 - x) * BPP / 8;

				unsigned MaxCount = 2;
				if(BPP == 32)
					MaxCount = 3;

				for(unsigned i = 0; i <= MaxCount; i++)
				{
					Src[i] ^= Dst[i];
					Dst[i] ^= Src[i];
					Src[i] ^= Dst[i];
				}
			}
		}
	}

	return true;
}

void Image::HeightMap(void)
{
	unsigned char *NewData = new unsigned char[SizeX * SizeY * 3];
	//generate height map
	for(unsigned long y = 0; y < SizeY; y++)
	{
		for(unsigned long x = 0; x < SizeX; x++)
		{
			unsigned char R = Data[0 + 3 * (y * SizeX + x)];
			unsigned char G = Data[1 + 3 * (y * SizeX + x)];
			unsigned char B = Data[2 + 3 * (y * SizeX + x)];
			float Height = float(R + G + B) / 3.0f;

			NewData[0 + 3 * (y * SizeX + x)] = (unsigned char)Height;
			NewData[1 + 3 * (y * SizeX + x)] = (unsigned char)Height;
			NewData[2 + 3 * (y * SizeX + x)] = (unsigned char)Height;
		}
	}

	Data = NewData;
}

void Image::NormalMap(void)
{
	unsigned char *NewData = new unsigned char[SizeX * SizeY * 3];
	HeightMap(); //convert the data to height map first

	//convert to normal map
	for(unsigned long y = 0; y < SizeY; y++)
	{
		for(unsigned long x = 0; x < 3 * SizeX; x += 3)
		{
			Vector S(1.0f, 0.0f, 0.0f), T(0.0f, 1.0f, 0.0f);

			//z component of S:
			int LHS = 0, RHS = SizeX - 1;
			if(x != 3 * SizeX - 3)
				LHS = x + 3;

			if(x != 0)
				RHS = x - 3;

			float A = 2.0f / 1024.0f;
			float Left = Data[0 + 3 * (y * SizeX) + LHS] * A;
			float Right = Data[0 + 3 * (y * SizeX) + RHS] * A;
			S.Z = Left - Right;

			//z component of T:
			LHS = 0, RHS = SizeY - 1;
			if(y != SizeY - 1)
				LHS = y + 1;

			if(y != 0)
				RHS = y - 1;

			Left = Data[0 + 3 * (LHS * SizeX) + x] * A;
			Right = Data[0 + 3 * (RHS * SizeX) + x] * A;
			T.Z = Left - Right; 

			Vector N = S.Cross(T);
			N.NormalizeMe();

			NewData[0 + 3 * (y * SizeX) + x] = unsigned char(255.0f * 0.5f * (N.Z + 1.0f));
			NewData[1 + 3 * (y * SizeX) + x] = unsigned char(255.0f * 0.5f * (N.Y + 1.0f));
			NewData[2 + 3 * (y * SizeX) + x] = unsigned char(255.0f * 0.5f * (N.X + 1.0f));
		}
	}

	Data = NewData;
}

GLuint Image::LoadTexture(std::string file)
{
	Image* Img = Image::LoadData(file.c_str());
	GLuint TexNum = Img->GenerateImage();
	
	Image::Free(Img);
	return TexNum;
}

GLuint Image::LoadNormalMap(std::string file)
{
	Image* Img = Image::LoadData(file.c_str());
	Img->NormalMap();
	GLuint TexNum = Img->GenerateImage();
	
	Image::Free(Img);
	return TexNum;
}

void Image::GenTexture(GLuint& tex)
{
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
//-----Image Class Functions-----//

namespace Textures
{
	std::map<std::string, GLuint> TexList, NormMapList, HeightMapList;

	void LoadAllTextures(void)
	{
		TexList["TEST1"] = Image::LoadTexture("data\\textures\\Test1.tga");
		NormMapList["TEST1"] = Image::LoadNormalMap("data\\textures\\fase.tga");
	}

	void LoadAllTextures(const std::string& file)
	{
		std::ifstream FS;

		FS.open(file.c_str());

		if(FS.fail())
			return;
		else
		{
			std::string Line;

			while(std::getline(FS, Line))
			{
				std::stringstream SS(Line);
				std::string Key, FileName;
				char Sep;

				SS >> Key >> Sep >> FileName;
				TexList[Key.c_str()] = Image::LoadTexture(FileName.c_str());
				NormMapList[Key.c_str()] = Image::LoadNormalMap(FileName.c_str());
			}
			FS.close();
		}
	}
}
/*--------------------------------------------------
Implements the tga image loader, written by Sun Fam
--------------------------------------------------*/
#include "tgaimage.h"

Image* Image::Load(const char* pName)
{
	Image* pImage = new Image;

	// check the extension
	const char* pExt = pName + strlen(pName) - 4;

	FILE* pFile = fopen(pName, "rb");

	// get the file size
	fseek(pFile, 0, SEEK_END);
	unsigned long fileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	unsigned char* pFileData = new unsigned char [fileSize];

	// read in the data
	unsigned long readSize = fread(pFileData, 1, fileSize, pFile);

	if (pImage->loadTGA(pFileData, fileSize))
		return pImage;

	// deallocate the image
	delete pImage;

	return 0;
}

// ---------------------------------------------------------------------------

void Image::Free(Image* pImage)
{
    if (!pImage)
		return;

    // deallocate the image
    delete pImage;
}

// ---------------------------------------------------------------------------

Image::Image() :
        mpData(0),
        mBPP(0),
        mSizeX(0),
        mSizeY(0)
{
}

// ---------------------------------------------------------------------------

Image::~Image()
{
    if (mpData)
		delete [] mpData;
}

// ---------------------------------------------------------------------------

bool Image::loadTGA(unsigned char* pData, unsigned long dataSize)
{
	// get the image type
	unsigned char imageType = pData[2];

	// get the bits per pixel
	mBPP = pData[16];
	// get image size
	mSizeX = (pData[13] << 8) | pData[12];
	mSizeY = (pData[15] << 8) | pData[14];

	// get the pointer to the image data area
	// * 18 is the header size
	// * the '0' entry is the number of bytes in the image id field (ignored!)
	unsigned char* pImageData = pData + 18 + pData[0];

	// allocate memory for the data
	mpData = new unsigned char [mSizeX * mSizeY * mBPP / 8];

	// get the image descriptor to get the orientation
	// * bit 5 (0 = bottom, 1 = top)
	// * bit 4 (0 = left    1 = right)
	unsigned char  desc    = pData[17];
	unsigned long rowSize = mSizeX * mBPP / 8;

	// check if need to mirror the image vertically
	if ((desc & 0x20) == 0)
	{
		// mirror data upside down
		for (signed long y = 0; y < mSizeY; ++y)
		{
			unsigned long* pSrc = (unsigned long*)(pImageData + y * rowSize);
			unsigned long* pDst = (unsigned long*)(mpData     + (mSizeY - 1 - y) * rowSize);

			memcpy(pDst, pSrc, rowSize);
		}
	}
	else
	{
		memcpy(mpData, pImageData, mSizeY * rowSize);
	}

	// check if need to mirror the image horizontally
	if (desc & 0x10)
	{
		for (signed long y = 0; y < mSizeY; ++y)
		{
			for (signed long x = 0; x < mSizeX / 2; ++x)
			{
				unsigned char* pSrc = mpData + y * rowSize + x * mBPP / 8;
				unsigned char* pDst = mpData + y * rowSize + (mSizeX - 1 - x) * mBPP / 8;

				pSrc[0] ^= pDst[0]; pDst[0] ^= pSrc[0]; pSrc[0] ^= pDst[0];
				pSrc[1] ^= pDst[1]; pDst[1] ^= pSrc[1]; pSrc[1] ^= pDst[1];
				pSrc[2] ^= pDst[2]; pDst[2] ^= pSrc[2]; pSrc[2] ^= pDst[2];

				if (mBPP == 32)
						pSrc[3] ^= pDst[3]; pDst[3] ^= pSrc[3]; pSrc[3] ^= pDst[3];
			}
		}
	}

	return true;
}

void Image::HeightMap(void)
{
	unsigned char *data = this->Data();
	unsigned char *data_test = new unsigned char[SizeX()*SizeY()*3];
	//generate height map
	for(int y = 0; y < SizeY(); y++)
	{
		for(int x = 0; x < SizeX(); x++)
		{
			unsigned char r = data[0 + 3 * (y * SizeX() + x)];
			unsigned char g = data[1 + 3 * (y * SizeX() + x)];
			unsigned char b = data[2 + 3 * (y * SizeX() + x)];
			float height = (r + g + b) / 3;

			data_test[0 + 3 * (y * SizeX() + x)] = (unsigned char)height;
			data_test[1 + 3 * (y * SizeX() + x)] = (unsigned char)height;
			data_test[2 + 3 * (y * SizeX() + x)] = (unsigned char)height;

		}
	}

	mpData = data_test;
}

void Image::NormalMap(void)
{
	unsigned char *data = this->Data();
	unsigned char *data_test = new unsigned char[SizeX()*SizeY()*3];
	unsigned char *data_test2 = new unsigned char[SizeX()*SizeY()*3];
	//generate height map
	for(int y = 0; y < SizeY(); y++)
	{
		for(int x = 0; x < SizeX(); x++)
		{
			unsigned char r = data[0 + 3 * (y * SizeX() + x)];
			unsigned char g = data[1 + 3 * (y * SizeX() + x)];
			unsigned char b = data[2 + 3 * (y * SizeX() + x)];
			float height = (r + g + b) / 3;

			data_test[0 + 3 * (y * SizeX() + x)] = (unsigned char)height;
			data_test[1 + 3 * (y * SizeX() + x)] = (unsigned char)height;
			data_test[2 + 3 * (y * SizeX() + x)] = (unsigned char)height;

		}
	}

	//convert to normal map
	for(int y = 0; y < SizeY(); y++)
	{
		for(int x = 0; x < 3*SizeX(); x+=3)
		{
			Vector S; S.x = 1; S.y = 0;
			Vector T; T.x = 0; T.y = 1;

			//z component of S:
			int lhs = 0, rhs = SizeX() - 1;
			if(x != 3*SizeX() - 3)
				lhs = x + 3;

			if(x != 0)
				rhs = x - 3;

			float a = 2.0/1024;
			float left = data_test[0 + 3 * (y * SizeX())+lhs]*(a);
			float right = data_test[0 + 3 * (y * SizeX())+rhs]*(a);
			S.z = left - right;

			//z component of T:
			lhs = 0, rhs = SizeY() - 1;
			if(y != SizeY() - 1)
				lhs = y + 1;

			if(y != 0)
				rhs = y - 1;

			left = data_test[0 + 3 * (lhs * SizeX())+ x]*(a);
			right = data_test[0 + 3 * (rhs * SizeX())+ x]*(a);
			T.z = left - right; 

			Vector N = S.Cross(T);
			N.NormalizeMe();

			data_test2[0 + 3 * (y * SizeX())+x] = 255 * .5 * (N.z + 1);
			data_test2[1 + 3 * (y * SizeX())+x] = 255 * .5 * (N.y + 1);
			data_test2[2 + 3 * (y * SizeX())+x] = 255 * .5 * (N.x + 1);
		}
	}

	mpData = data_test2;
}
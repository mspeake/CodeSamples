/*--------------------------------------------------
Defines the tga image loader, written by Sun Fam
--------------------------------------------------*/
#ifndef TGAIMAGE_H
#define TGAIMAGE_H

#include "Utils.h"
#include "ObjReader.h"

class Image
{
public:
        static Image*	Load	(const char* pName);
        static void		Free	(Image* pImage);

        const unsigned char*		Data	() const	{	return mpData;	}
        unsigned char*				Data	()			{	return mpData;	}
        unsigned long				BPP		() const	{	return mBPP;	}
        unsigned long				SizeX	() const	{	return mSizeX;	}
        unsigned long				SizeY	() const	{	return mSizeY;	}

		void NormalMap(void);
		void HeightMap(void);

private:
        unsigned char*				mpData;
        unsigned long				mBPP;
        unsigned long				mSizeX;
        unsigned long				mSizeY;

        // ---------------------------------------------------------------------------

                                        Image	();
                                        Image	(const Image& rhs);
                                        ~Image	();

        // ---------------------------------------------------------------------------

        bool			loadTGA	(unsigned char* pData, unsigned long dataSize);
};

#endif

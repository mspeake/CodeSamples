/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: tgaimage.h
Purpose: definition of the class to load TGA images
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 5/22/2012
End Header --------------------------------------------------------*/
#ifndef TGAIMAGE_H
#define TGAIMAGE_H

#include "SFW_ASSERT.h" 
#include "Utils.h"
#include "ObjReader.h"

// ---------------------------------------------------------------------------
// typedef for basic variable types

typedef char				s8;
typedef unsigned char		u8;
typedef signed short		s16;
typedef unsigned short		u16;
typedef signed long			s32;
typedef unsigned long		u32;
typedef signed long long	s64;
typedef unsigned long long	u64;
typedef float				f32;
typedef double				f64;
/////////////////////////////////////////////////////////////////////////////

class Image
{
public:
        static Image*	Load	(const char* pName);
        static void		Free	(Image* pImage);

        const u8*		Data	() const	{	return mpData;	}
        u8*				Data	()			{	return mpData;	}
        u32				BPP		() const	{	return mBPP;	}
        u32				SizeX	() const	{	return mSizeX;	}
        u32				SizeY	() const	{	return mSizeY;	}

		void NormalMap(void);
		void HeightMap(void);

private:
        u8*				mpData;
        u32				mBPP;
        u32				mSizeX;
        u32				mSizeY;

        // ---------------------------------------------------------------------------

                                        Image	();
                                        Image	(const Image& rhs);
                                        ~Image	();

        // ---------------------------------------------------------------------------

        bool			loadTGA	(u8* pData, u32 dataSize);
};

#endif // TGAIMAGE_H

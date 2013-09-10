/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: SFW_ASSERT.h
Purpose: asserts for the SFW functionalities(input, images)
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 5/22/2012
End Header --------------------------------------------------------*/
#ifndef SFW_ASSERT_H
#define SFW_ASSERT_H

#include <cstdlib>
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// Assert defines

#ifndef FINAL

// ---------------------------------------------------------------------------

#define PRINT(...)	printf(__VA_ARGS__)

// ---------------------------------------------------------------------------

#define SFW_ASSERT(x)												    \
{																	    \
    if((x) == 0)													    \
    {																    \
    PRINT("SFW_ASSERT: %s\nLine: %d\nFunc: %s\nFile: %s\n",		        \
#x, __LINE__, __FUNCTION__, __FILE__); 					                \
    std::cin.get();												        \
    exit(1);													        \
    }																    \
}

// ---------------------------------------------------------------------------

#define SFW_ASSERT_MESG(x, ...)											\
{																		\
    if((x) == 0)														\
    {																	\
    PRINT("SFW_ASSERT_MESG: %s\nLine: %d\nFunc: %s\nFile: %s\n",	    \
#x, __LINE__, __FUNCTION__, __FILE__);						            \
    PRINT("Mesg: "__VA_ARGS__);										    \
    PRINT("\n");													    \
    std::cin.get();													    \
    exit(1);														    \
    }																	\
}

// ---------------------------------------------------------------------------

#define SFW_ASSERT_PARM(x)												\
{																		\
    if((x) == 0)														\
    {																	\
    PRINT("SFW_ASSERT_PARM: %s\nLine: %d\nFunc: %s\nFile: %s\n",	    \
#x, __LINE__, __FUNCTION__, __FILE__);						            \
    std::cin.get();													    \
    exit(1);														    \
    }																	\
}

// ---------------------------------------------------------------------------

#define SFW_ASSERT_ALLOC(x)												\
{																		\
    if((x) == 0)														\
    {																	\
    PRINT("SFW_ASSERT_ALLOC: %s\nLine: %d\nFunc: %s\nFile: %s\n",	    \
#x, __LINE__, __FUNCTION__, __FILE__);						            \
    std::cin.get();													    \
    exit(1);														    \
    }																	\
}


// ---------------------------------------------------------------------------

#define SFW_WARNING(x)											        \
{																        \
    if((x) == 0)												        \
    {															        \
    PRINT("SFW_WARNING: %s\nLine: %d\nFunc: %s\nFile: %s\n",	        \
#x, __LINE__, __FUNCTION__, __FILE__); 				                    \
    }															        \
}

// ---------------------------------------------------------------------------

#define SFW_WARNING_MESG(x, ...)										\
{																		\
    if((x) == 0)														\
    {																	\
    PRINT("SFW_WARNING_MESG: %s\nLine: %d\nFunc: %s\nFile: %s\n",	    \
#x, __LINE__, __FUNCTION__, __FILE__);						            \
    PRINT("Mesg: "__VA_ARGS__);										    \
    PRINT("\n");													    \
    }																	\
}

// ---------------------------------------------------------------------------

#define SFW_WARNING_PARM(x)												\
{																		\
    if((x) == 0)														\
    {																	\
    PRINT("SFW_WARNING_PARM: %s\nLine: %d\nFunc: %s\nFile: %s\n",	    \
#x, __LINE__, __FUNCTION__, __FILE__);						            \
    }																	\
}

// ---------------------------------------------------------------------------

#else // FINAL

// ---------------------------------------------------------------------------

#define PRINT(...)

#define SFW_ASSERT(x)
#define SFW_ASSERT_MESG(x, ...)
#define SFW_ASSERT_PARM(x)
#define SFW_ASSERT_ALLOC(x)

#define SFW_WARNING(x)
#define SFW_WARNING_MESG(x, ...)
#define SFW_WARNING_PARM(x)
#define SFW_WARNING_ALLOC(x)

// ---------------------------------------------------------------------------

#endif // FINAL

// ---------------------------------------------------------------------------

#define SFW_HALT(...)										    \
{															    \
    PRINT("SFW_HALT:\nLine: %d\nFunc: %s\nFile: %s\n",		    \
    __LINE__, __FUNCTION__, __FILE__);					        \
    PRINT("Mesg: "__VA_ARGS__);								    \
    PRINT("\n");											    \
    std::cin.get();											    \
    exit(1);												    \
}

// ---------------------------------------------------------------------------

#define SFW_GL_ERROR(...)										\
{																\
    GLenum glError = glGetError();								\
    if (glError != GL_NO_ERROR)									\
    {															\
    PRINT("SFW_GL_ERROR:\nLine: %d\nFunc: %s\nFile: %s\n",	    \
    __LINE__, __FUNCTION__, __FILE__);					        \
    PRINT("Mesg: "__VA_ARGS__);								    \
    PRINT("\n");											    \
    std::cin.get();											    \
    exit(1);												    \
    }															\
}

// ---------------------------------------------------------------------------

#endif

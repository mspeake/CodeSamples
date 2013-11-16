/********************************************************************
**SystemModules.h
**
**-Externally defines the primary modules used by the system so that
** they can be accessed wherever in the program they are needed
********************************************************************/

#ifndef SYSTEMMODULES_H
#define SYSTEMMODULES_H

#include "Camera.h"
#include "Input.h"
#include "FrameTimer.h"

#define CHANGE_CENTER_SHAPE 'C'
#define CHANGE_NORMAL_MAPPING 'N'
#define CHANGE_REFLECTION_OPTION 'R'

namespace System
{
	extern Camera* PrimaryCamera;
	extern Input* InputHandler;
	extern FrameTimer* Framerate;
}

namespace ProjectData
{
	extern std::string ProjectName;
	extern std::string TextureList;
	extern std::string SceneFile;
	extern std::string EnvMapVS;
	extern std::string EnvMapFS;
	extern std::string LightingVS;
	extern std::string LightingFS;
	extern bool UseNormalMapping;
	extern bool UseReflection;
	extern unsigned CenterShape;
}

#endif
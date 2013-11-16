/********************************************************************
**SystemModels.cpp
**
**-Defines the primary modules that will be used by the system
** throughout this program
********************************************************************/

#include "SystemModules.h"

namespace System
{
	Camera* PrimaryCamera = NULL;
	Input* InputHandler = NULL;
	FrameTimer* Framerate = NULL;
}

namespace ProjectData
{
	std::string ProjectName = "Environment Mapping";
	std::string TextureList = "data\\textures\\TextureList.txt";
	std::string SceneFile =  "data\\scenes\\Env1.txt";
	std::string EnvMapVS = "data\\shaders\\EnvMapVS.vs";
	std::string EnvMapFS = "data\\shaders\\EnvMapFS.fs";
	std::string LightingVS = "data\\shaders\\Simple Light.vert";
	std::string LightingFS = "data\\shaders\\Simple Light.frag";
	bool UseNormalMapping = false;
	bool UseReflection = true;
	unsigned CenterShape = 2;
}
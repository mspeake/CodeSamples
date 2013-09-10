/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: shaders.h
Purpose: definition of my ShaderManager class
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 5/22/2012
End Header --------------------------------------------------------*/
#ifndef SHADERS_H
#define SHADERS_H

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>

#include <windows.h>

#include "Utils.h"
#include "ShaderEditor.h"
#include "glew.h"

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	void LoadShaders(char *v, char *f);
    void RunShaders();
    void StopShaders();
	void Free();

	GLuint Program;

	GLuint VertexShader;
	char VSName[128];

    GLuint FragmentShader;
	char FSName[128];

	bool IsRunning;
};


void MultiLightShaderData(void);
void SingleLightShaderData(void);

extern ShaderManager MultiLightShader;
extern ShaderManager SingleLightShader;
extern ShaderManager TextureShader;

#endif
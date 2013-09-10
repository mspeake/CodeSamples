/*--------------------------------------------------
Defines the shader manager class
--------------------------------------------------*/
#ifndef SHADERS_H
#define SHADERS_H

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>

#include <windows.h>

#include "Utils.h"
#include "Core.h"
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
	bool Running;

	GLuint VertexShader;
	char VSName[32];

    GLuint FragmentShader;
	char FSName[32];
};


void PassShaderData(void);
void PassEnvMapData(void);

extern ShaderManager MyShader, EnvMapShader;

#endif
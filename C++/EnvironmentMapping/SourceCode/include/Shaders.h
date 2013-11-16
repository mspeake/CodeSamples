/********************************************************************
**Shaders.h
**
**-Defines the ShaderManager and Shader classes, used for utilizing
** glsl shaders
********************************************************************/

#ifndef SHADERS_H
#define SHADERS_H

#include <fstream>
#include <string>
#include <stdio.h>
#include <map>
#include <Windows.h>

#include "ShapeLib.h"

class ShaderObj
{
public:
	GLuint Program, VertexShader, FragmentShader;
	std::string VSName, FSName;
	bool Running;

	ShaderObj();
	~ShaderObj();

	bool LoadShaders(std::string v, std::string f);
    void RunShaders();
    void StopShaders();
	void Free();
};

class ShaderManager
{
private:
	std::map<std::string, ShaderObj*> Shaders;
	std::string CurrentlyRunning;

public:
	ShaderManager();
	~ShaderManager();

	void AddShader(std::string name, std::string v, std::string f);
	void RunShader(std::string name);
	void StopShader(void);
	GLuint GetProgram(void);
	std::string GetCurrentShader(void) const;
	void PassTexture(const std::string shader, const std::string var, const GLuint tex, const int num) const;
	void PassUniform1i(const std::string shader, const std::string var, const int data) const;
	void PassUniform3f(const std::string shader, const std::string var, const Vector& data) const;
	void PassUniform4f(const std::string shader, const std::string var, const Color& data) const;
	void PassAttribute(const std::string shader, const std::string var, const float* data) const;
};

namespace Shaders
{
	extern ShaderManager* ShaderController;

	//-----Shader Utility Functions-----//
	char* ReadShader(std::string name);
	GLuint CompileShader(int type, const char* data);
	void BindTexture(unsigned num, unsigned tex);
	//-----Shader Utility Functions-----//
}

#endif
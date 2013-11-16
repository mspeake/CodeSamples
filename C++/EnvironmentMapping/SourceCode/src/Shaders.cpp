/********************************************************************
**Shaders.cpp
**
**-Implements the ShaderManager and Shader classes. The ShaderManager
** allows the user to manage several different types of shaders, and
** ensures that they do not interfere with each other
********************************************************************/

#include "Shaders.h"

//-----ShaderObj Class Functions-----//
ShaderObj::ShaderObj()
{
	Program = 0;
	VertexShader = 0;
	FragmentShader = 0;
	VSName = "";
	FSName = "";
	Running = false;
}

ShaderObj::~ShaderObj()
{
	this->Free();
}

bool ShaderObj::LoadShaders(std::string v, std::string f)
{
	VSName = v;
	FSName = f;

	const char *VertexData = Shaders::ReadShader(v);
	if(VertexData == NULL)
		return false;

	const char *FragmentData = Shaders::ReadShader(f);
	if(FragmentData == NULL)
	{
		delete [] VertexData;
		return false;
	}

	VertexShader = Shaders::CompileShader(GL_VERTEX_SHADER, VertexData);
	FragmentShader = Shaders::CompileShader(GL_FRAGMENT_SHADER, FragmentData);

	delete [] VertexData;
	delete [] FragmentData;

	Program = glCreateProgram();
	glAttachShader(Program, VertexShader);
	glAttachShader(Program, FragmentShader);
	glLinkProgram(Program);

	GLenum Err = glGetError();
	if(Err)
	{
		MessageBox(NULL, "Error attaching shader", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}

void ShaderObj::RunShaders()
{
	glUseProgram(Program);
	Running = true;
}

void ShaderObj::StopShaders()
{
	glUseProgram(0);
	Running = false;
}

void ShaderObj::Free()
{
	glDetachShader(Program, VertexShader);
	glDetachShader(Program, FragmentShader);
	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);
	glDeleteProgram(Program);
}
//-----ShaderObj Class Functions-----//

//-----ShaderManager Class Functions-----//
ShaderManager::ShaderManager()
{
	CurrentlyRunning = "";
}

ShaderManager::~ShaderManager()
{
	std::map<std::string, ShaderObj*>::iterator It = Shaders.begin();

	while(It != Shaders.end())
		delete It->second;

	Shaders.clear();
}

void ShaderManager::AddShader(std::string name, std::string v, std::string f)
{
	if(Shaders[name] == NULL) //dont create the shader if this slot is taken
	{
		ShaderObj* S = new ShaderObj();

		if(S->LoadShaders(v, f))
			Shaders[name] = S;
		else
			delete S;
	}
}

void ShaderManager::RunShader(std::string name)
{
	if((name.empty()) || (Shaders[name] == NULL)) //given name is null or does not match
		return;                                   //the name of any of the listed shaders

	this->StopShader(); //if there is another shader currently running, stop it
	Shaders[name]->RunShaders();
	CurrentlyRunning = name;
}

void ShaderManager::StopShader(void)
{
	if(CurrentlyRunning.empty()) //no shader currently running
		return;

	Shaders[CurrentlyRunning]->StopShaders();
	CurrentlyRunning = "";
}

GLuint ShaderManager::GetProgram(void)
{
	if(CurrentlyRunning.empty()) //no shader currently running
		return -1;

	return Shaders[CurrentlyRunning]->Program;
}

std::string ShaderManager::GetCurrentShader(void) const
{
	return CurrentlyRunning;
}

void ShaderManager::PassTexture(const std::string shader, const std::string var, const GLuint tex, const int num) const
{
	ShaderManager* SM = Shaders::ShaderController;
	if(SM->GetCurrentShader() == shader)
	{
		GLint Location = glGetUniformLocation(SM->GetProgram(), var.c_str());
		if(Location > 0)
		{
			glUniform1i(Location, num);
			Shaders::BindTexture(num, tex);
		}
	}
}

void ShaderManager::PassUniform1i(const std::string shader, const std::string var, const int data) const
{
	ShaderManager* SM = Shaders::ShaderController;
	if(SM->GetCurrentShader() == shader)
	{
		GLint Location = glGetUniformLocation(SM->GetProgram(), var.c_str());
		if(Location > 0)
		{
			glUniform1i(Location, data);
		}
	}
}

void ShaderManager::PassUniform3f(const std::string shader, const std::string var, const Vector& data) const
{
	ShaderManager* SM = Shaders::ShaderController;
	if(SM->GetCurrentShader() == shader)
	{
		GLfloat Vals[3] = {data.X, data.Y, data.Z};
		GLint Location = glGetUniformLocation(SM->GetProgram(), var.c_str());
		if(Location > 0)
		{
			glUniform3fv(Location, 1, Vals);
		}
	}
}

void ShaderManager::PassUniform4f(const std::string shader, const std::string var, const Color& data) const
{
	ShaderManager* SM = Shaders::ShaderController;
	if(SM->GetCurrentShader() == shader)
	{
		GLfloat Vals[4] = {data.R, data.G, data.B, data.A};
		GLint Location = glGetUniformLocation(SM->GetProgram(), var.c_str());
		if(Location > 0)
		{
			glUniform4fv(Location, 1, Vals);
		}
	}
}

void ShaderManager::PassAttribute(const std::string shader, const std::string var, const float* data) const
{
	ShaderManager* SM = Shaders::ShaderController;
	if(SM->GetCurrentShader() == shader)
	{
		GLuint Location = glGetAttribLocation(SM->GetProgram(), var.c_str());
		if(Location > 0)
		{
			glEnableVertexAttribArray(Location);
			glVertexAttribPointer(Location, 3, GL_FLOAT, false, 0, data);
		}
	}
}
//-----ShaderManager Class Functions-----//

namespace Shaders
{
	ShaderManager* ShaderController = NULL;

	//-----Shader Utility Functions-----//
	char* ReadShader(std::string name)
	{
		char* ShaderData = NULL;
		unsigned int Count = 0;

		FILE* Fp = NULL;
		fopen_s(&Fp, name.c_str(), "rb");
		if(name.c_str() != NULL)
		{
			if(Fp != NULL) 
			{ 				
				fseek(Fp, 0, SEEK_END);
				Count = ftell(Fp);
				rewind(Fp);

				if(Count > 0)
				{
					ShaderData = new char[Count + 1];
					Count = (int)fread(ShaderData, sizeof(char), Count, Fp);
					ShaderData[Count] = '\0';
				}
				fclose(Fp);
			}
		}

		return ShaderData;
	}

	GLuint CompileShader(int type, const char* data)
	{
		GLint Status;
		char LogBuffer[1024];
		GLsizei	LogLength;

		//compile shader
		GLuint Shader = glCreateShader(type);
		glShaderSource(Shader, 1, &data, 0);
		glCompileShader(Shader);
		glGetShaderiv(Shader, GL_COMPILE_STATUS, &Status);

		if(!Status) //shader did not compile
		{
			glGetShaderInfoLog(Shader, 1024, &LogLength, LogBuffer);
			std::cout << LogLength << " | " << LogBuffer << std::endl;

			std::string ErrMsg;

			if(type == GL_VERTEX_SHADER)
				ErrMsg = "Vertex shader did not compile";
			else
				ErrMsg = "Fragment shader did not compile";

			MessageBox(NULL, ErrMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
			return -1;
		}

		return Shader;
	}

	void BindTexture(unsigned num, unsigned tex)
	{
		glActiveTexture(GL_TEXTURE0 + num);
		glBindTexture(GL_TEXTURE_2D, tex);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
	}
	//-----Shader Utility Functions-----//
}

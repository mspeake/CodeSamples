/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Shaders.cpp
Purpose: implementation of the ShaderManager class
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 5/22/2012
End Header --------------------------------------------------------*/
#include "Shaders.h"

ShaderManager::ShaderManager()
{
	Program = 0;
	VertexShader = 0;
	VSName[0] = 0;
	FragmentShader = 0;
	FSName[0] = 0;
	IsRunning = false;
}

ShaderManager::~ShaderManager()
{
	Free();
}

void ShaderManager::LoadShaders(char *v, char *f)
{
	strcpy_s(VSName, v);
	strcpy_s(FSName, f);

	char *VertexData, *FragmentData;
	FILE *fp = NULL;
	unsigned int count = 0;

	//load the vertex shader
	fopen_s(&fp, VSName, "rb");
	if (VSName != NULL)
	{
		if(fp != NULL) 
	    { 				
		    fseek(fp, 0, SEEK_END);
		    count = ftell(fp);
		    rewind(fp);

		    if (count > 0)
		    {
			    VertexData = (char *)malloc(sizeof(char) * (count + 1));
			    count = (int)fread(VertexData, sizeof(char), count, fp);
			    VertexData[count] = '\0';
		    }
		    fclose(fp);
		}
	}

	fp = NULL;
	count = 0;

	//load the fragment shader
	fopen_s(&fp, FSName, "rb");
	if (FSName != NULL)
	{
		if(fp != NULL) 
	    { 				
		    fseek(fp, 0, SEEK_END);
		    count = ftell(fp);
		    rewind(fp);

		    if (count > 0)
		    {
			    FragmentData = (char *)malloc(sizeof(char) * (count + 1));
			    count = (int)fread(FragmentData, sizeof(char), count, fp);
			    FragmentData[count] = '\0';
		    }
		    fclose(fp);
		}
	}

	const char *constVertexData = VertexData;
	const char *constFragmentData = FragmentData;

	GLint	status;
	char	logBuffer[1024];
	GLsizei	logLength;

	//compile vertex shader
	VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &constVertexData, 0);
	glCompileShader(VertexShader);
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &status);

	if (!status) //vertex shader did not compile
	{
		glGetShaderInfoLog(VertexShader, 1024, &logLength, logBuffer);
		printf("%d | %s\n", logLength, logBuffer);

		MessageBox(NULL,"vertex shader did not compile","Error",MB_OK | MB_ICONERROR);
	}

	status = 0;

	//compile fragment shader
	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &constFragmentData, 0 );
	glCompileShader(FragmentShader);
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &status);

	if (!status) //fragment shader did not compile
	{
		glGetShaderInfoLog(FragmentShader, 1024, &logLength, logBuffer);
		printf("%d | %s\n", logLength, logBuffer);

        MessageBox(NULL,"fragment shader did not compile","Error",MB_OK | MB_ICONERROR);
	}

	Program = glCreateProgram();
			
	glAttachShader(Program, VertexShader);
	glAttachShader(Program, FragmentShader);
	glLinkProgram(Program);

	GLenum err = glGetError();
	if(err)
	{
		MessageBox(NULL,"Error attaching shader","Error",MB_OK | MB_ICONERROR);
	}

	free(VertexData);
	free(FragmentData);

}

void ShaderManager::RunShaders()
{
	glUseProgram(Program);	
	IsRunning = true;
}

void ShaderManager::StopShaders()
{
	glUseProgram(0);
	IsRunning = false;
}

void ShaderManager::Free()
{
	glDetachShader(Program, VertexShader);
	glDetachShader(Program, FragmentShader);
	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);
	glDeleteProgram(Program);
}


void MultiLightShaderData(void)
{
	GLuint ShaderProgram = MultiLightShader.Program;
	char temp[256];

	Matrix4x4 ViewMtx;
	glGetFloatv(GL_MODELVIEW_MATRIX, ViewMtx.M);
	ViewMtx.Transpose();


	GLfloat const_att = 0.0f;
	GLfloat lin_att = 0.0f;
	GLfloat quad_att = 0.002f;

	Point C_Pos = ViewMtx * sCamera.Pos;
	GLfloat CPos[3] = {C_Pos.x, C_Pos.y, C_Pos.z};
	glUniform3fv(glGetUniformLocation(ShaderProgram, "CameraPosition"), 1, CPos);

	int orb_count = 4; //# of orbs, change later

	for(unsigned i = 0; i < orb_count; i++)
	{
		Point L_Pos = ViewMtx * Player->GetOrbPos(i);
		GLfloat LPos[3] = {L_Pos.x, L_Pos.y, L_Pos.z};
		sprintf(temp, "LIGHTS[%d].pos", i);
		glUniform3fv(glGetUniformLocation(ShaderProgram, temp), 1, LPos);

		Color Orb_color = Player->GetOrbColor(i);

		GLfloat Amb[4] = {Orb_color.R, Orb_color.G, Orb_color.B, Orb_color.A};//{0.8f, 0.8f, 0.8f, 1.0f};
		sprintf(temp, "LIGHTS[%d].ambient", i);
		glUniform4fv(glGetUniformLocation(ShaderProgram, temp), 1, Amb);

		GLfloat Dif[4] = {Amb[0], Amb[1], Amb[2], Amb[3]};//{0.8f, 0.8f, 0.8f, 1.0f};
		sprintf(temp, "LIGHTS[%d].diffuse", i);
		glUniform4fv(glGetUniformLocation(ShaderProgram, temp), 1, Dif);

		GLfloat Spec[4] = {Amb[0], Amb[1], Amb[2], Amb[3]};//{1.0f, 1.0f, 1.0f, 1.0};
		sprintf(temp, "LIGHTS[%d].specular", i);
		glUniform4fv(glGetUniformLocation(ShaderProgram, temp), 1, Spec);

		GLfloat DistAtt[3] = {const_att, lin_att, quad_att};
		sprintf(temp, "LIGHTS[%d].DistAtt", i);
		glUniform3fv(glGetUniformLocation(ShaderProgram, temp), 1, DistAtt);
	}

	//camera-centered light
	Point L_Pos = C_Pos;
	GLfloat LPos[3] = {L_Pos.x, L_Pos.y, L_Pos.z};
	sprintf(temp, "LIGHTS[%d].pos", orb_count);
	glUniform3fv(glGetUniformLocation(ShaderProgram, temp), 1, LPos);

	Color Orb_color(0.8f, 0.8f, 0.8f, 1.0f);

	GLfloat Amb[4] = {Orb_color.R, Orb_color.G, Orb_color.B, Orb_color.A};//{0.8f, 0.8f, 0.8f, 1.0f};
	sprintf(temp, "LIGHTS[%d].ambient", orb_count);
	glUniform4fv(glGetUniformLocation(ShaderProgram, temp), 1, Amb);

	GLfloat Dif[4] = {Amb[0], Amb[1], Amb[2], Amb[3]};//{0.8f, 0.8f, 0.8f, 1.0f};
	sprintf(temp, "LIGHTS[%d].diffuse", orb_count);
	glUniform4fv(glGetUniformLocation(ShaderProgram, temp), 1, Dif);

	GLfloat Spec[4] = {1.0f, 1.0f, 1.0f, 1.0}; //{0.6f, 0.6f, 0.6f, 1.0};//
	sprintf(temp, "LIGHTS[%d].specular", orb_count);
	glUniform4fv(glGetUniformLocation(ShaderProgram, temp), 1, Spec);

	GLfloat DistAtt[3] = {const_att, lin_att, 0.0001};
	sprintf(temp, "LIGHTS[%d].DistAtt", orb_count);
	glUniform3fv(glGetUniformLocation(ShaderProgram, temp), 1, DistAtt);

	//if(!NpcObj.empty())
	//{
	//	glUniform1i(glGetUniformLocation(ShaderProgram, "LightNum"), orb_count + 2); 
	//	//enemy-centered light
	//	Point E_pos = ViewMtx * NpcObj[0]->GetPos();
	//	GLfloat ELPos[3] = {E_pos.x, E_pos.y, E_pos.z};
	//	sprintf(temp, "LIGHTS[%d].pos", orb_count + 1);
	//	glUniform3fv(glGetUniformLocation(ShaderProgram, temp), 1, ELPos);

	//	Color E_color = NpcObj[0]->GetColor();

	//	GLfloat EAmb[4] = {E_color.R, E_color.G, E_color.B, E_color.A};//{0.8f, 0.8f, 0.8f, 1.0f};
	//	sprintf(temp, "LIGHTS[%d].ambient", orb_count+1);
	//	glUniform4fv(glGetUniformLocation(ShaderProgram, temp), 1, EAmb);

	//	GLfloat EDif[4] = {EAmb[0], EAmb[1], EAmb[2], EAmb[3]};//{0.8f, 0.8f, 0.8f, 1.0f};
	//	sprintf(temp, "LIGHTS[%d].diffuse", orb_count+1);
	//	glUniform4fv(glGetUniformLocation(ShaderProgram, temp), 1, EDif);

	//	GLfloat ESpec[4] = {1.0f, 1.0f, 1.0f, 1.0}; //{0.6f, 0.6f, 0.6f, 1.0};//
	//	sprintf(temp, "LIGHTS[%d].specular", orb_count+1);
	//	glUniform4fv(glGetUniformLocation(ShaderProgram, temp), 1, ESpec);

	//	GLfloat EDistAtt[3] = {const_att, lin_att, quad_att};
	//	sprintf(temp, "LIGHTS[%d].DistAtt", orb_count+1);
	//	glUniform3fv(glGetUniformLocation(ShaderProgram, temp), 1, EDistAtt);
	//}
	//else
	glUniform1i(glGetUniformLocation(ShaderProgram, "LightNum"), orb_count + 1); 
}

void SingleLightShaderData(void)
{
	GLuint ShaderProgram = SingleLightShader.Program;
	char temp[256];

	Matrix4x4 ViewMtx;
	glGetFloatv(GL_MODELVIEW_MATRIX, ViewMtx.M);
	ViewMtx.Transpose();


	GLfloat const_att = 0.0f;
	GLfloat lin_att = 0.0f;
	GLfloat quad_att = 0.0002f;

	Point C_Pos = ViewMtx * sCamera.Pos;
	GLfloat CPos[3] = {C_Pos.x, C_Pos.y, C_Pos.z};
	glUniform3fv(glGetUniformLocation(ShaderProgram, "CameraPosition"), 1, CPos);

	Point L_Pos = C_Pos;
	GLfloat LPos[3] = {L_Pos.x, L_Pos.y, L_Pos.z};
	sprintf(temp, "LIGHT.pos");
	glUniform3fv(glGetUniformLocation(ShaderProgram, temp), 1, LPos);

	Color Orb_color = Color(0.8f, 0.8f, 0.8f, 1.0f);

	GLfloat Amb[4] = {Orb_color.R, Orb_color.G, Orb_color.B, Orb_color.A};//{0.8f, 0.8f, 0.8f, 1.0f};
	sprintf(temp, "LIGHT.ambient");
	glUniform4fv(glGetUniformLocation(ShaderProgram, temp), 1, Amb);

	GLfloat Dif[4] = {Amb[0], Amb[1], Amb[2], Amb[3]};//{0.8f, 0.8f, 0.8f, 1.0f};
	sprintf(temp, "LIGHT.diffuse");
	glUniform4fv(glGetUniformLocation(ShaderProgram, temp), 1, Dif);

	GLfloat Spec[4] = {Amb[0], Amb[1], Amb[2], Amb[3]};//{1.0f, 1.0f, 1.0f, 1.0};
	sprintf(temp, "LIGHT.specular");
	glUniform4fv(glGetUniformLocation(ShaderProgram, temp), 1, Spec);

	GLfloat DistAtt[3] = {const_att, lin_att, quad_att};
	sprintf(temp, "LIGHT.DistAtt");
	glUniform3fv(glGetUniformLocation(ShaderProgram, temp), 1, DistAtt);
}
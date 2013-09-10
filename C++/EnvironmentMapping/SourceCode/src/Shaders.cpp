/*--------------------------------------------------
Implements the shader manager class
--------------------------------------------------*/
#include "Shaders.h"

ShaderManager::ShaderManager()
{
	Program = 0;
	VertexShader = 0;
	VSName[0] = 0;
	FragmentShader = 0;
	FSName[0] = 0;
	Running = false;
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
	Running = true;
}

void ShaderManager::StopShaders()
{
	glUseProgram(0);
	Running = false;
}

void ShaderManager::Free()
{
	glDetachShader(Program, VertexShader);
	glDetachShader(Program, FragmentShader);
	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);
	glDeleteProgram(Program);
}


void PassShaderData(void)
{
	GLuint ShaderProgram = MyShader.Program;

	GLfloat LPos[3] = {sCamera.Pos.x, sCamera.Pos.y, sCamera.Pos.z};
	glUniform3fv(glGetUniformLocation(MyShader.Program, "LightPosition"), 1, LPos);

	GLfloat CPos[3] = {sCamera.Pos.x, sCamera.Pos.y, sCamera.Pos.z};
	glUniform3fv(glGetUniformLocation(MyShader.Program, "CameraPosition"), 1, CPos);

	GLfloat Dif[4] = {0.8f, 0.8f, 0.8f, 1.0f};
	glUniform4fv(glGetUniformLocation(MyShader.Program, "lightDiffuseColor"), 1, Dif);

	GLfloat Spec[4] = {1.0f, 1.0f, 1.0f, 1.0};
	glUniform4fv(glGetUniformLocation(ShaderProgram, "lightSpecColor"), 1, Spec);
}

void PassEnvMapData(void)
{
	GLuint ShaderProgram = EnvMapShader.Program;
	glEnable(GL_TEXTURE_2D);

	BindShaderTexture(1, ShaderProgram, "TexLeft", LeftPic);
	BindShaderTexture(2, ShaderProgram, "TexRight", RightPic);
	BindShaderTexture(3, ShaderProgram, "TexTop", TopPic);
	BindShaderTexture(4, ShaderProgram, "TexBottom", BottomPic);
	BindShaderTexture(5, ShaderProgram, "TexFront", FrontPic);
	BindShaderTexture(6, ShaderProgram, "TexBack", BackPic);
	BindShaderTexture(7, ShaderProgram, "NormalMap", NormalMapID);
	
	GLfloat CPos[3] = {sCamera.Pos.x, sCamera.Pos.y, sCamera.Pos.z};
	glUniform3fv(glGetUniformLocation(ShaderProgram, "CameraPos"), 1, CPos);
	glUniform1i(glGetUniformLocation(ShaderProgram, "Option"), rg_num);
	glUniform1i(glGetUniformLocation(ShaderProgram, "NormFlag"), use_nm);
}
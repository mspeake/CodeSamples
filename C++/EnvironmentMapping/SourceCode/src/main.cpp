#include "WindowManager.h"
#include "SystemModules.h"
#include "Objects.h"

void Initialize(int argc, char ** argv)
{
	glutInit(&argc, argv);
	WindowData::WindowsController = new WindowManager();
	WindowData::WindowsController->AddWindow(ProjectData::ProjectName.c_str());

	GLenum InitGlew = glewInit();
	if(InitGlew != GLEW_OK)
		MessageBox(NULL, "Failed to initialize GLEW", "Error", MB_OK | MB_ICONERROR);

	System::InputHandler = new Input();
	System::PrimaryCamera = new Camera();
	System::Framerate = new FrameTimer();

	Shaders::ShaderController = new ShaderManager();
	Shaders::ShaderController->AddShader("Lighting", ProjectData::LightingVS.c_str(), ProjectData::LightingFS.c_str());
	Shaders::ShaderController->AddShader("EnvMap", ProjectData::EnvMapVS.c_str(), ProjectData::EnvMapFS.c_str());

	Textures::LoadAllTextures(ProjectData::TextureList.c_str());

	ObjectReader::CreatObjectsFromFile(ObjectReader::ObjectsList, ProjectData::SceneFile.c_str());
	FrameBufferObjects::CreateAllFrameBuffers();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	char *argv[] = {"NoCommandWindow"};
	int argc = 1;
	Initialize(argc, (char **) argv);

	System::PrimaryCamera->Reset();

	glutMainLoop();

	return 0;
}
#include "Shaders.h"
#include <Windows.h>
#include <iostream>

// OpenGL includes 
#include <GL/gl.h>
#include <GL/glext.h>

// GLUT includes
#include <GL/glut.h>

// GLUI includes
#include <GL/glui.h>

// Shader Editor includes
#include "Core.h"

#include "ObjReader.h"
#include "Geometry.h"
#include "GameObj.h"

ShaderManager MyShader, EnvMapShader;
Camera sCamera;
Input*	spInput;

// Window variables
GLUI *glui = NULL;
int main_window = 0;
int rg_num = 0, shape_num = 0, use_nm = 0;


// Extern references
extern Color clearColor;
extern std::string FileName;

/*
 *	Function	: initialize
 *	Arguments	: int argc, char * const argv[]
 *	Description	:
 *					The function for initializing the states of the OpenGL and GLUT libraries in
 *					a consistent state. If the libraries cannot be loaded or initialized, the function
 *					returns -1, it returns 0 for success
 */

int initialize( int argc, char ** argv )
{
	// Initialize GLUT 
	glutInit( &argc, argv );
	
	// Initialize the display mode to utilize double buffering, 4-channel framebuffer and depth buffer
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	
	glutInitWindowSize(SP_WINDOW_WIDTH, SP_WINDOW_HEIGHT);
	glutInitWindowPosition(SP_WINDOW_POS_X, SP_WINDOW_POS_Y);
	main_window = glutCreateWindow("Shaders Sample Project");
	
	// Assign callback functions 
	glutDisplayFunc(display);

	glutShowWindow();


	GLenum init_glew = glewInit();
	if(init_glew != GLEW_OK)
		std::cout << "Failed to initialize GLEW" << std::endl;

	MyShader.LoadShaders("data\\shaders\\Simple Light.vert", "data\\shaders\\Simple Light.frag");
	EnvMapShader.LoadShaders("data\\shaders\\envmapVS.vs", "data\\shaders\\envmapFS.fs");

	spInput = Input::Instance();

	LoadTextures();
	CreateFrameBuffer();

	return 0; // successful return
}

/*
 *	Function	: initGLUI
 *	Arguments	: none
 *	Description	:
 *					The function for initializing the states of the GLUI library in
 *					a consistent state. If the libraries cannot be loaded or initialized, the function
 *					returns -1, it returns 0 for success
 */
int initGLUI( void )
{
	if( glui == NULL )
	{
		glui = GLUI_Master.create_glui( "Options GUI" );		
		GLUI_Panel *top_panel = glui->add_panel("Options");
		GLUI_RadioGroup *rg = glui->add_radiogroup_to_panel(top_panel, &rg_num);
		GLUI_RadioButton *dn = glui->add_radiobutton_to_group(rg, "Reflection");
		GLUI_RadioButton *dt = glui->add_radiobutton_to_group(rg, "Refraction");
		GLUI_Checkbox *UseNM = glui->add_checkbox_to_panel(top_panel, "Use Normal Mapping", &use_nm);

		GLUI_Panel *shape_panel = glui->add_panel("Shape");
		GLUI_RadioGroup *srg = glui->add_radiogroup_to_panel(shape_panel, &shape_num);

		GLUI_RadioButton *rbs = glui->add_radiobutton_to_group(srg, "Sphere");
		GLUI_RadioButton *rbc = glui->add_radiobutton_to_group(srg, "Cube");
		GLUI_RadioButton *rbp = glui->add_radiobutton_to_group(srg, "Plane");
		glui->set_main_gfx_window(main_window);
		
		// Redirect "idle" callback to GLUI
		GLUI_Master.set_glutIdleFunc(idle);
		GLUI_Master.set_glutReshapeFunc(reshape);
	}

	
	return 0;
}

/*
 *	Function	: main
 *	Arguments	: int argc, char * const argv[]
 *	Description	:
 *					The entry point for the program ShaderEditor
 */
//int main (int argc, char * const argv[]) 
int WINAPI WinMain(      
	HINSTANCE hInstance,                  // Uncomment these lines to remove
	HINSTANCE hPrevInstance,			  // the console window when running
	LPSTR lpCmdLine,
	int nCmdShow)
{

	char *argv[] = {"NoCommandWindow"};
	char argc = 1;

	LL = new LevelHelper();
	// Initialize the OpenGL, GLUT and GLUI libraries
	initialize( argc, (char **) argv );
	
	initGLUI();

	sCamera.Reset();
	sCamera.Pos = Point(0.0f, 20.0f, 100.0f);
	sCamera.Alpha = DegToRad(15.0f);

	srand((unsigned)time(0));
	LL->LoadStuff(FileName.c_str());

	glutMainLoop();

    return 0;
}

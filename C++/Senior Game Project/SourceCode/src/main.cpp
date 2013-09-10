/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: main.cpp
Purpose: implementation of the main functions
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 5/22/2012
End Header --------------------------------------------------------*/
 
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
#include "ShaderEditor.h"

#include "ObjReader.h"
#include "Geometry.h"
#include "GameObj.h"
#include "GameStateManager.h"

#include "Sound.h"

ShaderManager MultiLightShader;
ShaderManager SingleLightShader;
ShaderManager TextureShader;

Camera sCamera;

Input*	spInput;

Shape *test_shape;

KD_Node* root_node = NULL;
BVH_Node* bvh_root = NULL;

GameStateManager GSM;

Point BIAS;
int portal = 0;

// Window variables
GLUI *glui = NULL;
int main_window = 0;
GLUI_FileBrowser *FB = NULL;
int Tree_Depth = 0;
GLUI_EditText *KD_Node_Depth = NULL;
int rg_num = 0;


// Extern references
extern Color4f  clearColor;

extern int render_vert_norms;
extern int render_face_norms;

extern std::string FileName;

extern float CamDist;

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
	
	glutInitWindowSize(SE_WINDOW_WIDTH, SE_WINDOW_HEIGHT);
	glutInitWindowPosition(SE_WINDOW_POS_X, SE_WINDOW_POS_Y);
	main_window = glutCreateWindow("Orbital");
	
	// Assign callback functions 
	glutDisplayFunc(display);

	// DO NOT register the idle callback with GLUT if we are using GLUI
	 glutIdleFunc(idle); 
	 glutKeyboardFunc(keyboard);
	 glutReshapeFunc(reshape);

	glutShowWindow();
	glutSetCursor(GLUT_CURSOR_NONE);
	//glutFullScreen();
	ToggleFullScreen();

	GLenum init_glew = glewInit();
	if(init_glew != GLEW_OK)
		std::cout << "Failed to initialize GLEW" << std::endl;

	MultiLightShader.LoadShaders("data\\shaders\\Multiple Lights.vert", "data\\shaders\\Multiple Lights.frag");
	SingleLightShader.LoadShaders("data\\shaders\\Single Light.vert", "data\\shaders\\Single Light.frag");
	TextureShader.LoadShaders("data\\shaders\\Texture.vert", "data\\shaders\\Texture.frag");
	

	spInput = Input::Instance();

	LoadTextures();

	CalculatePascalsTriangle();

	srand((unsigned)time(0));
	BIAS = Bias();

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
		glui = GLUI_Master.create_glui( "ShaderEditor" );
		
		// add GLUI components here
		// 1. Top-level panel 
		
		glui->set_main_gfx_window(main_window);
		
		// Redirect "idle" callback to GLUI
		GLUI_Master.set_glutIdleFunc(idle);
		GLUI_Master.set_glutKeyboardFunc(keyboard);
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
	// Initialize the OpenGL, GLUT and GLUI libraries
	initialize( argc, (char **) argv );
	
	SoundInit();

	GSM.AddState(GST_SPLASH_SCREEN); //GST_MAIN_MENU, GST_LEVEL_1

	// Now start the fun !
	glutMainLoop();
	
	// Control should NEVER return here!

    return 0;
}

void SoundInit(void)
{
	FMOD_RESULT result;
	result = FMOD::System_Create(&SoundSys);
	if(result != FMOD_OK)
		return;
	result = SoundSys->init(100, FMOD_INIT_NORMAL, 0);
	if(result != FMOD_OK)
		return;

	result = SoundSys->createSound("sounds\\MenuBack.wav", FMOD_DEFAULT, 0, &MenuBack);
	if(result != FMOD_OK)
		return;

	result = SoundSys->createSound("sounds\\MenuSelect.wav", FMOD_DEFAULT, 0, &MenuSelect);
	if(result != FMOD_OK)
		return;

	result = SoundSys->createSound("sounds\\LevelMusic.wav", FMOD_LOOP_NORMAL, 0, &LevelMusic);
	if(result != FMOD_OK)
		return;
	result = SoundSys->createSound("sounds\\MenuMusic.wav", FMOD_LOOP_NORMAL, 0, &MenuMusic);
	if(result != FMOD_OK)
		return;
	result = SoundSys->createSound("sounds\\WinMusic.mp3", FMOD_LOOP_NORMAL, 0, &WinMusic);
	if(result != FMOD_OK)
		return;

	result = SoundSys->createSound("sounds\\PlayerShoot.wav", FMOD_DEFAULT, 0, &PlayerShoot);
	if(result != FMOD_OK)
		return;

	result = SoundSys->createSound("sounds\\OrbReturn.mp3", FMOD_DEFAULT, 0, &OrbReturn);
	if(result != FMOD_OK)
		return;

	result = SoundSys->createSound("sounds\\HitEnemy.wav", FMOD_DEFAULT, 0, &HitEnemy);
	if(result != FMOD_OK)
		return;
	
	result = SoundSys->createSound("sounds\\HitPlayer.wav", FMOD_DEFAULT, 0, &HitPlayer);
	if(result != FMOD_OK)
		return;

	result = SoundSys->createSound("sounds\\EnemyAttack.wav", FMOD_DEFAULT, 0, &EnemyAttack);
	if(result != FMOD_OK)
		return;

	result = SoundSys->createSound("sounds\\LoseSound.wav", FMOD_DEFAULT, 0, &LoseSound);
	if(result != FMOD_OK)
		return;
}

void Play_Sound(int num)
{
	FMOD_RESULT result;
	bool mute;
	switch(num)
	{
	case MENU_SELECT:
		{
			if(!MuteSFX)
			{
				MenuSoundChan->getMute(&mute);
				if(mute)
					break;
				MenuSoundChan->stop();
				result = SoundSys->playSound(FMOD_CHANNEL_FREE, MenuSelect, false, &MenuSoundChan);
			}
			break;
		}
	case MENU_BACK:
		{
			if(!MuteSFX)
			{
				MenuSoundChan->getMute(&mute);
				if(mute)
					break;
				MenuSoundChan->stop();
				result = SoundSys->playSound(FMOD_CHANNEL_FREE, MenuBack, false, &MenuSoundChan);
			}
			break;
		}
	case LEVEL_MUSIC:
		{
			if(!MuteMusic)
			{
				MusicChan->getMute(&mute);
				if(mute)
					break;
				result = SoundSys->playSound(FMOD_CHANNEL_FREE, LevelMusic, false, &MusicChan);
			}
			break;
		}
	case MENU_MUSIC:
		{
			if(!MuteMusic)
			{
				MusicChan->getMute(&mute);
				if(mute)
					break;
				result = SoundSys->playSound(FMOD_CHANNEL_FREE, MenuMusic, false, &MusicChan);
			}
			break;
		}
	case WIN_MUSIC:
		{
			if(!MuteMusic)
			{
				MusicChan->getMute(&mute);
				if(mute)
					break;
				result = SoundSys->playSound(FMOD_CHANNEL_FREE, WinMusic, false, &MusicChan);
			}
			break;
		}
	case PLAYER_SHOOT:
		{
			if(!MuteSFX)
			{
				SoundChan->getMute(&mute);
				if(mute)
					break;
				SoundChan->stop();
				result = SoundSys->playSound(FMOD_CHANNEL_FREE, PlayerShoot, false, &SoundChan);
			}
			break;
		}
	case ORB_RETURN:
		{
			if(!MuteSFX)
			{
				SoundChan->getMute(&mute);
				if(mute)
					break;
				SoundChan->stop();
				result = SoundSys->playSound(FMOD_CHANNEL_FREE, OrbReturn, false, &SoundChan);
			}
			break;
		}

	case HIT_ENEMY:
		{
			if(!MuteSFX)
			{
				SoundChan->getMute(&mute);
				if(mute)
					break;
				SoundChan->stop();
				result = SoundSys->playSound(FMOD_CHANNEL_FREE, HitEnemy, false, &SoundChan);
			}
			break;
		}
	case HIT_PLAYER:
		{
			if(!MuteSFX)
			{
				SoundChan->getMute(&mute);
				if(mute)
					break;
				SoundChan->stop();
				result = SoundSys->playSound(FMOD_CHANNEL_FREE, HitPlayer, false, &SoundChan);
			}
			break;
		}
	case ENEMY_ATTACK:
		{
			if(!MuteSFX)
			{
				SoundChan->getMute(&mute);
				if(mute)
					break;
				SoundChan->stop();
				result = SoundSys->playSound(FMOD_CHANNEL_FREE, EnemyAttack, false, &SoundChan);
			}
			break;
		}
	case LOSE_SOUND:
		{
			if(!MuteSFX)
			{
				SoundChan->getMute(&mute);
				if(mute)
					break;
				SoundChan->stop();
				result = SoundSys->playSound(FMOD_CHANNEL_FREE, LoseSound, false, &SoundChan);
			}
			break;
		}
	}
}

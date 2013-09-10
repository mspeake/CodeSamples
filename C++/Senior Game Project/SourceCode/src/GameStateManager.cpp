/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: GameStateManager.cpp
Purpose: implementation of the GameStateManager class
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 10/15/2012
End Header --------------------------------------------------------*/
#include "GameStateManager.h"
#include "ShaderEditor.h"

//-----GameState class functions-----//
GameState::GameState(unsigned type)
{
	StateType = type;
	NpcFirst = NpcLast = EnvFirst = EnvLast = -1;

	switch(StateType)
	{
	case GST_MAIN_MENU:
		{
			Load = Main_Menu_Load;
			Initialize = Main_Menu_Initialize;
			Update = Main_Menu_Update;
			Draw = Main_Menu_Draw;
			Unload = Main_Menu_Unload;
			break;
		}

	case GST_LEVEL_1:
		{
			Load = Level_1_Load;
			Initialize = Level_1_Initialize;
			Update = Level_1_Update;
			Draw = Level_1_Draw;
			Unload = Level_1_Unload;
			break;
		}

	case GST_PAUSE:
		{
			Load = Pause_Load;
			Initialize = Pause_Initialize;
			Update = Pause_Update;
			Draw = Pause_Draw;
			Unload = Pause_Unload;
			break;
		}

	case GST_SPLASH_SCREEN:
		{
			Load = Splash_Load;
			Initialize = Splash_Initialize;
			Update = Splash_Update;
			Draw = Splash_Draw;
			Unload = Splash_Unload;
			break;
		}

	case GST_CREDITS_SCREEN:
		{
			Load = Credits_Load;
			Initialize = Credits_Initialize;
			Update = Credits_Update;
			Draw = Credits_Draw;
			Unload = Credits_Unload;
			break;
		}

	case GST_CONFIRM:
		{
			Load = Confirm_Load;
			Initialize = Confirm_Initialize;
			Update = Confirm_Update;
			Draw = Confirm_Draw;
			Unload = Confirm_Unload;
			break;
		}

	case GST_HOW_TO_PLAY:
		{
			Load = Instructions_Load;
			Initialize = Instructions_Initialize;
			Update = Instructions_Update;
			Draw = Instructions_Draw;
			Unload = Instructions_Unload;
			break;
		}

	case GST_OPTIONS_MENU:
		{
			Load = Options_Load;
			Initialize = Options_Initialize;
			Update = Options_Update;
			Draw = Options_Draw;
			Unload = Options_Unload;
			break;
		}

	case GST_WIN:
		{
			Load = Win_Load;
			Initialize = Win_Initialize;
			Update = Win_Update;
			Draw = Win_Draw;
			Unload = Win_Unload;
			break;
		}

	case GST_LOSE:
		{
			Load = Lose_Load;
			Initialize = Lose_Initialize;
			Update = Lose_Update;
			Draw = Lose_Draw;
			Unload = Lose_Unload;
			break;
		}

	default:
		break;
	}
}

unsigned GameState::GetStateType(void)
{
	return StateType;
}
//-----GameState class functions-----//

//-----GameStateManager class functions-----//
void GameStateManager::AddState(unsigned type)
{
	CurrentState = type;
	NextState = type;

	if(type == GST_QUIT)
		ExitAllStates();

	GameState *gs = new GameState(type);

	unsigned index = States.size();
	States.push_back(gs);

	unsigned EnvPrev = EnvObj.size(),
			 NpcPrev = NpcObj.size();

	States[index]->Load();

	unsigned EnvCurr = EnvObj.size(),
			 NpcCurr = NpcObj.size();

	if(EnvCurr > EnvPrev)
	{
		States[index]->EnvFirst = EnvPrev;
		States[index]->EnvLast = EnvCurr - 1;
	}
	else
		States[index]->EnvFirst = States[index]->EnvLast = -1;

	if(NpcCurr > NpcPrev)
	{
		States[index]->NpcFirst = NpcPrev;
		States[index]->NpcLast = NpcCurr - 1;
	}
	else
		States[index]->NpcFirst = States[index]->NpcLast = -1;

	States[index]->Initialize();
}

void GameStateManager::ExitState(void)
{
	unsigned index = States.size() - 1;

	if(index == 0)
		ExitAllStates();

	else
	{
		States[index]->Unload();
		States.pop_back();
		CurrentState = States[index - 1]->StateType;
		NextState = CurrentState;
	}
}

void GameStateManager::ExitAllStates(void)
{
	for(unsigned i = 0; i < States.size(); i++)
		States[i]->Unload();

	States.clear();
	exit(0);
}

void GameStateManager::ExitToState(unsigned state)
{
	for(unsigned i = 0; i < States.size(); i++)
		States[i]->Unload();

	Player = NULL;
	States.clear();

	AddState(state);
}

unsigned GameStateManager::GetCurrentState(void)
{
	return CurrentState;
}

unsigned GameStateManager::GetNextState(void)
{
	return NextState;
}

void GameStateManager::SwitchToState(unsigned state)
{
	NextState = state;
}

void GameStateManager::SetNextState(unsigned s)
{
	NextState = s;
}
//-----GameStateManager class functions-----//
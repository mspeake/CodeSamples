/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: GameStateManager.h
Purpose: implementation of the GameStateManager class
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 10/15/2012
End Header --------------------------------------------------------*/
#ifndef GAMESTATEMANAGER_H
#define GAMESTATEMANAGER_H

#include <vector>

enum GAME_STATE_TYPE
{
	GST_MAIN_MENU,
	GST_LEVEL_1,
	GST_PAUSE,
	GST_SPLASH_SCREEN,
	GST_CREDITS_SCREEN,
	GST_CONFIRM,
	GST_QUIT,
	GST_HOW_TO_PLAY,
	GST_OPTIONS_MENU,
	GST_WIN,
	GST_LOSE
};

class GameStateManager;

class GameState
{
private:
	unsigned StateType;

public:
	GameState(unsigned type);

	void (*Load)(void);
	void (*Initialize)(void);
	void (*Update)(void);
	void (*Draw)(unsigned);
	void (*Unload)(void);

	int NpcFirst, NpcLast;
	int EnvFirst, EnvLast;

	unsigned GetStateType(void);

	friend class GameStateManager;
};

class GameStateManager
{
private:
	unsigned CurrentState;
	unsigned NextState;

public:
	std::vector<GameState*> States;
	void AddState(unsigned type);
	void ExitState(void);
	void ExitAllStates(void);
	void ExitToState(unsigned state);
	unsigned GetCurrentState(void);
	unsigned GetNextState(void);
	void SetNextState(unsigned s);
	void SwitchToState(unsigned state);
};

#endif
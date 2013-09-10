/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Sound.h
Purpose: declaration of the sound-related functions and enumerations
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 12/6/2012
End Header --------------------------------------------------------*/
#ifndef SOUND_H
#define SOUND_H

#include "fmod.hpp"
#include "fmod_errors.h"



enum SOUND_NUM
{
	LEVEL_MUSIC,
	MENU_MUSIC,
	MENU_SELECT,
	MENU_BACK,
	PLAYER_SHOOT,
	ORB_RETURN,
	HIT_ENEMY,
	HIT_PLAYER,
	ENEMY_ATTACK,
	WIN_MUSIC,
	LOSE_SOUND
};

void Play_Sound(int num);
void SoundInit(void);

extern bool MuteMusic;
extern bool MuteSFX;

extern FMOD::Channel *SoundChan;
extern FMOD::Channel *MusicChan;
extern FMOD::Channel *MenuSoundChan;
extern FMOD::System *SoundSys;

extern FMOD::Sound *LevelMusic;
extern FMOD::Sound *MenuMusic;
extern FMOD::Sound *WinMusic;

//sounds
extern FMOD::Sound *MenuSelect;
extern FMOD::Sound *MenuBack;
extern FMOD::Sound *PlayerShoot;
extern FMOD::Sound *OrbReturn;
extern FMOD::Sound *HitEnemy;
extern FMOD::Sound *HitPlayer;
extern FMOD::Sound *EnemyAttack;
extern FMOD::Sound *LoseSound;

#endif
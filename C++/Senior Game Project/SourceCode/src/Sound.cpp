/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Sound.cpp
Purpose: declaration of the sounds used
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Author: Matt Speake, matthew.speake, 50006908
Creation date: 12/6/2012
End Header --------------------------------------------------------*/
#include "Sound.h"

FMOD::System *SoundSys;
FMOD::Channel *SoundChan;
FMOD::Channel *MenuSoundChan;
FMOD::Channel *MusicChan;

FMOD::Sound *LevelMusic;
FMOD::Sound *MenuMusic;
FMOD::Sound *WinMusic;

//sounds
FMOD::Sound *MenuSelect;
FMOD::Sound *MenuBack;
FMOD::Sound *PlayerShoot;
FMOD::Sound *OrbReturn;
FMOD::Sound *HitEnemy;
FMOD::Sound *HitPlayer;
FMOD::Sound *EnemyAttack;
FMOD::Sound *LoseSound;


bool MuteMusic = false;
bool MuteSFX = false;
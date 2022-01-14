// midiplay

// Copyright 2022 by Steve Clark

#ifndef __MUSIC_H__
#define __MUSIC_H__

#define MUSIC_PAUSE         0
#define MUSIC_PLAY          1

#define MUSIC_PLAYONCE      0
#define MUSIC_LOOPING       1

void MUSIC_Init(int);
int MUSIC_Load(void *, int, int);
void MUSIC_Play(int);
void MUSIC_SetVolume(int);
void MUSIC_Output(short *, int);
int MUSIC_IsPlaying(void);
int MUSIC_Time(void);
void MUSIC_Loop(int);
void MUSIC_Restart(void);

#endif

// midiplay

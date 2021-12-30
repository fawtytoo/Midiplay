// midi play

#ifndef __MUSIC_H__
#define __MUSIC_H__

enum
{
    MUSIC_PAUSE,
    MUSIC_RESUME
};

// SAMPLERATE
void MUSIC_Init(int);

// data, size, looping
void MUSIC_Play(void *, int, int);

// pause or resume
void MUSIC_Pause(int);

// volume between 0 & 127
void MUSIC_SetVolume(int);

// music is rendered as stereo 16bit signed samples
void MUSIC_Output(short *, int);

// not really necessary for DOOM but a handy call for testing purposes
int MUSIC_IsPlaying(void);

#endif

// midi play

// midiplay

// Copyright 2022 by Steve Clark

#ifndef __COMMON_H__
#define __COMMON_H__

#ifndef NULL
#define NULL        0
#endif

#define VOICES      24
#define VOLUME      32760 / VOICES

typedef unsigned short  WORD;
typedef unsigned char   BYTE;
typedef unsigned int    UINT;

typedef void(*function)(void);

extern int      musicSamplerate;
extern int      musicPlaying, musicLooping;
extern int      musicVolume;

extern int      beatTicks, beatTempo;
extern int      musicClock;
extern float    tickSamples, playSamples;

extern int      timeTicks;

extern int      numTracks, numTracksEnded;

void loadMusTrack(BYTE *);
int loadMidTracks(int, BYTE *, int);
void initTracks(void);
void trackMusEvents(void);
void trackMidEvents(void);
void updateTime(void);
void updateVolume(int);
void generateSample(short *);

#endif

// midiplay

// midiplay

// Copyright 2022 by Steve Clark

#ifndef __COMMON_H__
#define __COMMON_H__

#ifndef NULL
#define NULL    0
#endif

#define VOICES  24
#define VOLUME  32760 / VOICES

typedef unsigned short  WORD;
typedef unsigned char   BYTE;
typedef unsigned int    UINT;

extern int  musicPlaying, musicLooping;

extern int  beatTicks, beatTempo;
extern int  musicClock;
extern int  playSamples, playFlag;

extern int  timeTicks;

extern int  numTracks, numTracksEnded;

extern UINT volumeTable[];

void LoadMusTrack(BYTE *);
int LoadMidTracks(int, BYTE *, int);
void InitTracks(void);
void UpdateEvents(void);
void GenerateSample(short *, short);

#endif

// midiplay

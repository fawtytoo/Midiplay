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

typedef void(*function)(void);

extern int  musicSamplerate;
extern int  musicPlaying, musicLooping;

extern int  beatTicks, beatTempo;
extern int  musicClock;
extern int  playSamples, playFlag;

extern int  timeTicks;

extern int  numTracks, numTracksEnded;

void LoadMusTrack(BYTE *);
int LoadMidTracks(int, BYTE *, int);
void InitTracks(void);
void TrackMusEvents(void);
void TrackMidEvents(void);
void UpdateScoreTime(void);
void UpdateVolume(int);
void GenerateSample(short *, short);

#endif

// midiplay

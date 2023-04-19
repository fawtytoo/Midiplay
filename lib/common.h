// midiplay

// Copyright 2022 by Steve Clark

#ifndef __COMMON_H__
#define __COMMON_H__

#define VOICES  24
#define VOLUME  32760 / VOICES

#define ID(i, a)    (i[0] == a[0] && i[1] == a[1] && i[2] == a[2] && i[3] == a[3])

#define LE16(i)     ((i)[0] | ((i)[1] << 8))
#define BE16(i)     (((i)[0] << 8) | (i)[1])
#define LE32(i)     (LE16(i) | (LE16(i + 2) << 16))
#define BE32(i)     ((BE16(i) << 16) | BE16(i + 2))

typedef unsigned short  WORD;
typedef unsigned char   BYTE;
typedef unsigned int    UINT;

extern int  musicPlaying, musicLooping;

extern int  beatTicks, beatTempo;
extern int  musicClock;
extern int  playSamples;

extern int  timeTicks;

extern int  numTracks, numTracksEnded;

extern UINT volumeTable[];

void LoadMusTrack(BYTE *);
int LoadMidTracks(int, BYTE *, int);
void InitTracks(void);
void UpdateEvents(void);

#endif

// midiplay

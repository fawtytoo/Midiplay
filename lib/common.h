// midi play

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

extern int      beatTicks;
extern int      tickTock;
extern float    tickSamples, playSamples;

void loadMusTrack(BYTE *);
int loadMidTracks(int, BYTE *);
void initTracks(void);
void trackMusEvents(void);
void trackMidEvents(void);

void generateSamples(short *, int);

#endif

// midi play

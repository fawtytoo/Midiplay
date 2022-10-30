// midiplay

// Copyright 2022 by Steve Clark

#ifndef __TIMER_H__
#define __TIMER_H__

typedef struct
{
    int rate;
    int acc;
    int remainder;
    int divisor;
} TIMER;

extern TIMER    timerPhase;
extern TIMER    timerSecond, timerBeat;

void SetTimer(TIMER *, int, int);
int UpdateTimer(TIMER *);

#endif

// midiplay

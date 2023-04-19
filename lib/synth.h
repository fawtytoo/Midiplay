// midiplay

// Copyright 2022 by Steve Clark

#ifndef __SYNTH_H__
#define __SYNTH_H__

#include "common.h"

enum
{
    env_none,
    env_attack,
    env_decay,
    env_sustain,
    env_release
};

typedef struct
{
    int     instrument;
    int     volume;
    int     pan;
    int     sustain;
    int     bend;
    int     expression;
} CHANNEL;

typedef struct
{
    CHANNEL *channel;
    int     note;
    int     volume;
    UINT    phase, step;
    int     env_stage;
    short   left, right;
    int     playing; // bit field
} VOICE;

extern VOICE    midVoice[], *voiceHead, *voiceTail;

void Synth_Generate(short *);

#endif

// midiplay

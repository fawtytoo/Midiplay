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

short Synth_GenPhase(UINT, short *);
short Synth_GenEnv(int);

#endif

// midiplay

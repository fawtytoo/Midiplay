// midiplay

// Copyright 2022 by Steve Clark

#include "synth.h"

short Synth_GenPhase(UINT phase, short *neg)
{
    *neg = 0x000;

    if (phase & 0x400)
        *neg = 0xffff;

    return 0x1ff;
}

short Synth_GenEnv(int stage)
{
    short   out = 0x0;

    switch (stage)
    {
      case env_attack:
        out = 0x100;
        break;
    }

    return out;
}

// midiplay

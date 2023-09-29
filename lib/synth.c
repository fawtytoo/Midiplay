// midiplay

// Copyright 2022 by Steve Clark

#include "synth.h"

short Synth_GenPhase(UINT phase)
{
    short   neg = 0;

    if (phase & 0x400)
        neg = -1;

    return 0x1ff ^ neg;
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

void Synth_Generate(short *buffer, int rate)
{
    VOICE   *voice = voiceHead;
    int     index;
    short   left = 0, right = 0;
    short   out;

    for (index = 0; index < VOICES; index++, voice++)
    {
        out = Synth_GenPhase(voice->phase >> 21);
        out = out * Synth_GenEnv(voice->env_stage) >> 8;
        out = out * voice->volume >> 8;

        left += out * voice->left >> 9;
        right += out * voice->right >> 9;

        voice->phase += voice->step * rate;
    }

    *buffer++ = left;
    *buffer = right;
}

// midiplay

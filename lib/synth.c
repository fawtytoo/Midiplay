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

void GenerateSample(short *buffer, short rate)
{
    VOICE   *voice;
    short   left = 0, right = 0;
    short   phase, neg;

    for (voice = voiceHead; voice <= voiceTail; voice++)
    {
        phase = Synth_GenPhase(voice->phase >> 21, &neg);
        phase *= Synth_GenEnv(voice->env_stage) >> 8;

        left += (voice->left * phase >> 9) ^ neg;
        right += (voice->right * phase >> 9) ^ neg;

        voice->phase += voice->step * rate;
    }

    *buffer++ = left;
    *buffer = right;
}

// midiplay

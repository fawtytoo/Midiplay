// midiplay

// Copyright 2022 by Steve Clark

#include "common.h"
#include "synth.h"

#include "event.h"

#define NOTE_PLAY       1
#define NOTE_SUSTAIN    2

// all frequencies are 16.16
UINT    frequencyTable[128] =
{
    0x00082d01, 0x0008a976, 0x00092d51, 0x0009b904, 0x000a4d05, 0x000ae9d3, 0x000b8ff4, 0x000c3ff6, 0x000cfa6f, 0x000dc000, 0x000e914f, 0x000f6f10, 0x00105a02, 0x001152ec, 0x00125aa2, 0x00137208,
    0x00149a0a, 0x0015d3a6, 0x00171fe9, 0x00187fed, 0x0019f4df, 0x001b8000, 0x001d229e, 0x001ede22, 0x0020b404, 0x0022a5d7, 0x0024b545, 0x0026e410, 0x00293414, 0x002ba74d, 0x002e3fd2, 0x0030ffda,
    0x0033e9c0, 0x00370000, 0x003a453d, 0x003dbc44, 0x00416809, 0x00454baf, 0x00496a8b, 0x004dc820, 0x00526829, 0x00574e9b, 0x005c7fa4, 0x0061ffb4, 0x0067d380, 0x006e0000, 0x00748a7b, 0x007b7887,
    0x0082d012, 0x008a9760, 0x0092d516, 0x009b9041, 0x00a4d054, 0x00ae9d36, 0x00b8ff49, 0x00c3ff6a, 0x00cfa6ff, 0x00dc0000, 0x00e914f5, 0x00f6f110, 0x0105a025, 0x01152ec0, 0x0125aa2e, 0x01372082,
    0x0149a0a7, 0x015d3a6d, 0x0171fe92, 0x0187fed4, 0x019f4e00, 0x01b80000, 0x01d229ec, 0x01ede220, 0x020b404a, 0x022a5d82, 0x024b545c, 0x026e4104, 0x0293414e, 0x02ba74db, 0x02e3fd24, 0x030ffda9,
    0x033e9c02, 0x03700000, 0x03a453da, 0x03dbc43e, 0x04168094, 0x0454bb05, 0x0496a8b6, 0x04dc8208, 0x052682a0, 0x0574e9b2, 0x05c7fa49, 0x061ffb56, 0x067d37ff, 0x06e00000, 0x0748a7aa, 0x07b78887,
    0x082d0128, 0x08a975ff, 0x092d517a, 0x09b90410, 0x0a4d0533, 0x0ae9d375, 0x0b8ff493, 0x0c3ff69b, 0x0cfa7011, 0x0dc00000, 0x0e914f54, 0x0f6f110e, 0x105a0250, 0x1152ebfe, 0x125aa2f4, 0x13720820,
    0x149a0a66, 0x15d3a6ea, 0x171fe927, 0x187fed37, 0x19f4e022, 0x1b800000, 0x1d229efa, 0x1ede21c7, 0x20b404a1, 0x22a5d85c, 0x24b54583, 0x26e41040, 0x2934153e, 0x2ba74d5b, 0x2e3fd24f, 0x30ffdaf7
};

UINT    pitchBendTable[64] =
{
    0x0000, 0x02c9, 0x059b, 0x0874, 0x0b55, 0x0e3e, 0x1130, 0x1429, 0x172b, 0x1a35, 0x1d48, 0x2063, 0x2387, 0x26b4, 0x29e9, 0x2d28,
    0x306f, 0x33c0, 0x371a, 0x3a7d, 0x3dea, 0x4160, 0x44e0, 0x486a, 0x4bfd, 0x4f9b, 0x5342, 0x56f4, 0x5ab0, 0x5e76, 0x6247, 0x6623,
    0x6a09, 0x6dfb, 0x71f7, 0x75fe, 0x7a11, 0x7e2f, 0x8258, 0x868d, 0x8ace, 0x8f1a, 0x9373, 0x97d8, 0x9c49, 0xa0c6, 0xa550, 0xa9e6,
    0xae89, 0xb33a, 0xb7f7, 0xbcc1, 0xc199, 0xc67f, 0xcb72, 0xd072, 0xd581, 0xda9e, 0xdfc9, 0xe502, 0xea4a, 0xefa1, 0xf507, 0xfa7c
};

UINT    volumeTable[128] =
{
    0x000, 0x001, 0x002, 0x004, 0x005, 0x007, 0x008, 0x009, 0x00b, 0x00c, 0x00e, 0x00f, 0x011, 0x012, 0x014, 0x015,
    0x017, 0x018, 0x01a, 0x01b, 0x01d, 0x01f, 0x020, 0x022, 0x023, 0x025, 0x027, 0x028, 0x02a, 0x02b, 0x02d, 0x02f,
    0x030, 0x032, 0x034, 0x035, 0x037, 0x039, 0x03b, 0x03c, 0x03e, 0x040, 0x041, 0x043, 0x045, 0x047, 0x049, 0x04a,
    0x04c, 0x04e, 0x050, 0x052, 0x054, 0x055, 0x057, 0x059, 0x05b, 0x05d, 0x05f, 0x061, 0x063, 0x065, 0x067, 0x069,
    0x06b, 0x06d, 0x06f, 0x071, 0x073, 0x075, 0x077, 0x079, 0x07b, 0x07d, 0x07f, 0x081, 0x083, 0x085, 0x087, 0x089,
    0x08c, 0x08e, 0x090, 0x092, 0x094, 0x097, 0x099, 0x09b, 0x09d, 0x0a0, 0x0a2, 0x0a4, 0x0a6, 0x0a9, 0x0ab, 0x0ad,
    0x0b0, 0x0b2, 0x0b5, 0x0b7, 0x0b9, 0x0bc, 0x0be, 0x0c1, 0x0c3, 0x0c6, 0x0c8, 0x0cb, 0x0cd, 0x0d0, 0x0d2, 0x0d5,
    0x0d7, 0x0da, 0x0dc, 0x0df, 0x0e2, 0x0e4, 0x0e7, 0x0ea, 0x0ec, 0x0ef, 0x0f2, 0x0f4, 0x0f7, 0x0fa, 0x0fd, 0x100
};

UINT    panTable[2][128] =
{
    {
        0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100,
        0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100,
        0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100,
        0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100,
        0x100, 0x0fa, 0x0f4, 0x0ef, 0x0e9, 0x0e4, 0x0df, 0x0da, 0x0d4, 0x0cf, 0x0ca, 0x0c5, 0x0c0, 0x0bb, 0x0b6, 0x0b2,
        0x0ad, 0x0a8, 0x0a4, 0x09f, 0x09a, 0x096, 0x091, 0x08d, 0x089, 0x084, 0x080, 0x07c, 0x078, 0x074, 0x070, 0x06c,
        0x068, 0x064, 0x060, 0x05c, 0x058, 0x054, 0x051, 0x04d, 0x049, 0x046, 0x042, 0x03f, 0x03b, 0x038, 0x034, 0x031,
        0x02d, 0x02a, 0x027, 0x024, 0x020, 0x01d, 0x01a, 0x017, 0x014, 0x011, 0x00e, 0x00b, 0x008, 0x005, 0x002, 0x000
    },
    {
        0x000, 0x002, 0x005, 0x008, 0x00b, 0x00e, 0x011, 0x014, 0x017, 0x01a, 0x01d, 0x020, 0x023, 0x026, 0x029, 0x02d,
        0x030, 0x033, 0x037, 0x03a, 0x03d, 0x041, 0x044, 0x048, 0x04b, 0x04f, 0x053, 0x056, 0x05a, 0x05e, 0x062, 0x066,
        0x06a, 0x06d, 0x071, 0x075, 0x07a, 0x07e, 0x082, 0x086, 0x08a, 0x08f, 0x093, 0x097, 0x09c, 0x0a0, 0x0a5, 0x0a9,
        0x0ae, 0x0b3, 0x0b7, 0x0bc, 0x0c1, 0x0c6, 0x0cb, 0x0d0, 0x0d5, 0x0da, 0x0df, 0x0e5, 0x0ea, 0x0ef, 0x0f5, 0x0fa,
        0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100,
        0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100,
        0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100,
        0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100
    }
};

typedef struct
{
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

CHANNEL midChannel[16];
VOICE   midVoice[VOICES], *voiceHead = &midVoice[0], *voiceTail = &midVoice[VOICES - 1];

EVENT   *eventData;

UINT    midVolume = VOLUME;

void ResetChannel(int channel)
{
    midChannel[channel].volume = 100;
    midChannel[channel].pan = 64;
}

void VoiceOff(VOICE *voice, int state)
{
    voice->playing &= state;
    if (voice->playing)
        return;

    voice->env_stage = env_release;
}

void VoiceVolume(VOICE *voice)
{
    UINT    volume;

    volume = midVolume * volumeTable[voice->channel->volume * voice->channel->expression >> 7] * volumeTable[voice->volume] >> 16;

    voice->left = volume * panTable[0][voice->channel->pan] >> 8;
    voice->right = volume * panTable[1][voice->channel->pan] >> 8;
}

void UpdateVolume(int volume)
{
    VOICE   *voice;

    midVolume = volumeTable[volume];

    for (voice = voiceHead; voice <= voiceTail; voice++)
        if (voice->playing)
            VoiceVolume(voice);
}

void ResetVoices()
{
    VOICE   *voice;

    for (voice = voiceHead; voice <= voiceTail; voice++)
        VoiceOff(voice, 0);
}

void ResetControls()
{
    int     channel;

    for (channel = 0; channel < 16; channel++)
    {
        midChannel[channel].sustain = 0;
        midChannel[channel].bend = 128;
        midChannel[channel].expression = 127;
    }
}

void Event_NoteOff()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     note = eventData->data[0];
    //int         volume = eventData->data[1];
    VOICE   *voice;

    for (voice = voiceHead; voice <= voiceTail; voice++)
        if (voice->playing)
            if (voice->channel == channel)
                if (voice->note == note)
                    VoiceOff(voice, NOTE_SUSTAIN);
}

void FrequencyStep(VOICE *voice)
{
    int     note = voice->note;
    int     bend = voice->channel->bend;
    UINT    diff;

    note += (bend >> 6) - 2;
    bend &= 63;

    diff = (frequencyTable[note + 1] - frequencyTable[note]) >> 8;
    voice->step = frequencyTable[note] + ((diff * pitchBendTable[bend]) >> 8);
}

void Event_NoteOn()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     note = eventData->data[0];
    int     volume = eventData->data[1];
    VOICE   *voice;

    for (voice = voiceHead; voice <= voiceTail; voice++)
        if (voice->playing == 0)
        {
            voice->channel = channel;
            voice->note = note;
            voice->volume = volume;
            VoiceVolume(voice);

            FrequencyStep(voice);
            voice->phase = 0;
            voice->env_stage = env_attack;

            voice->playing = NOTE_PLAY | channel->sustain;

            break;
        }
}

void Event_MuteNotes()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    VOICE   *voice;

    for (voice = voiceHead; voice <= voiceTail; voice++)
        if (voice->playing)
            if (voice->channel == channel)
                VoiceOff(voice, NOTE_SUSTAIN);
}

void Event_PitchWheel()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    VOICE   *voice;

    channel->bend = ((eventData->data[0] & 0x7f) | (eventData->data[1] << 7)) >> 6; // 8 bit values

    for (voice = voiceHead; voice <= voiceTail; voice++)
        if (voice->playing)
            if (voice->channel == channel)
                FrequencyStep(voice);
}

void Event_Aftertouch()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     note = eventData->data[0];
    int     volume = eventData->data[1];
    VOICE   *voice;

    for (voice = voiceHead; voice <= voiceTail; voice++)
        if (voice->playing)
            if (voice->channel == channel)
                if (voice->note == note)
                {
                    voice->volume = volume;
                    VoiceVolume(voice);
                }
}

void Event_Sustain()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     sustain = eventData->data[1] >> 6;
    VOICE   *voice;

    // sustain: 0=off, 2=on
    channel->sustain = ((sustain >> 1) | (sustain & 1)) << 1;

    if (channel->sustain != 0)
        return;

    for (voice = voiceHead; voice <= voiceTail; voice++)
        if (voice->playing)
            if (voice->channel == channel)
                VoiceOff(voice, NOTE_PLAY);
}

void Event_ChannelVolume()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     volume = eventData->data[1];
    VOICE   *voice;

    channel->volume = volume & 0x7f;

    for (voice = voiceHead; voice <= voiceTail; voice++)
        if (voice->playing)
            if (voice->channel == channel)
                VoiceVolume(voice);
}

void Event_Pan()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     pan = eventData->data[1] & 0x7f;
    VOICE   *voice;

    channel->pan = pan;

    for (voice = voiceHead; voice <= voiceTail; voice++)
        if (voice->playing)
            if (voice->channel == channel)
                VoiceVolume(voice);
}

void Event_ChannelAftertouch()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     volume = eventData->data[1];
    VOICE   *voice;

    for (voice = voiceHead; voice <= voiceTail; voice++)
        if (voice->playing)
            if (voice->channel == channel)
            {
                voice->volume = volume;
                VoiceVolume(voice);
            }
}

void Event_Expression()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     expression = eventData->data[1] & 0x7f;
    VOICE   *voice;

    channel->expression = expression;

    for (voice = voiceHead; voice <= voiceTail; voice++)
        if (voice->playing)
            if (voice->channel == channel)
                VoiceVolume(voice);
}

void Event_Message()
{
    switch (eventData->data[0])
    {
      case MM_VOLUME:
        Event_ChannelVolume();
        break;

      case MM_PAN:
        Event_Pan();
        break;

      case MM_NOTEOFF:
        Event_MuteNotes();
        break;

      case MM_CTRLOFF:
        ResetControls();
        break;

      case MM_SUSTAIN: // FIXME
        //Event_Sustain();
        break;

      case MM_AFTERTOUCH:
        Event_ChannelAftertouch();
        break;

      case MM_EXPRESS:
        Event_Expression();
        break;

      case MM_SOUNDOFF:
        ResetVoices();
        break;

      case MM_INSTR:
      case MM_MODWHEEL:
      case MM_REG_LSB:
      case MM_REG_MSB:
        break;

      default:
        break;
    }
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

// midiplay

// Copyright 2022 by Steve Clark

#include "common.h"
#include "synth.h"

#include "event.h"

#define NOTE_PLAY       1
#define NOTE_SUSTAIN    2

typedef struct
{
    int     instrument;
    int     volume;
    int     pan;
    int     sustain;
    int     bend;
    int     expression;
}
CHANNEL;

typedef struct
{
    int     index;
    CHANNEL *channel;
    int     note;
    int     volume;
    int     playing; // bit field
}
VOICE;

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

CHANNEL midChannel[16];
VOICE   midVoice[VOICES] =
{
    {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7},
    {8}, {9}, {10}, {11}, {12}, {13}, {14}, {15},
    {16}, {17}, {18}, {19}, {20}, {21}, {22}, {23}
};
VOICE   *voiceHead = &midVoice[0];

EVENT   *eventData;

void ResetChannel(int channel)
{
    midChannel[channel].instrument = 0;
    midChannel[channel].volume = volumeTable[100];
    midChannel[channel].pan = 64;
}

void VoiceOff(VOICE *voice, int state)
{
    voice->playing &= state;
    if (voice->playing)
    {
        return;
    }

    Synth_KeyOff(voice->index);
}

void VoiceVolume(VOICE *voice)
{
    Synth_SetVolume(voice->index, voice->channel->volume * voice->channel->expression * voice->volume >> 16);
}

void ResetVoices()
{
    VOICE   *voice = voiceHead;
    int     index;

    for (index = 0; index < VOICES; index++, voice++)
    {
        VoiceOff(voice, 0);
    }
}

void ResetControls()
{
    int channel;

    for (channel = 0; channel < 16; channel++)
    {
        midChannel[channel].sustain = 0;
        midChannel[channel].bend = 0;
        midChannel[channel].expression = volumeTable[127];
    }
}

void Event_NoteOff()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     note = eventData->data[0];
    //int         volume = eventData->data[1];
    VOICE   *voice = voiceHead;
    int     index;

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel && voice->note == note)
        {
            VoiceOff(voice, NOTE_SUSTAIN);
        }
    }
}

void FrequencyStep(VOICE *voice)
{
    int index, octave;

    octave = voice->note / 12;
    index = (voice->note % 12) * 32 + voice->channel->bend;
    if (index < 0)
    {
        if (octave > 0)
        {
            octave--;
            index += 384;
        }
        else
        {
            index = 0;
        }
    }
    else if (index >= 384)
    {
        octave++;
        index -= 384;
    }

    Synth_SetFrequency(voice->index, index, octave);
}

void Event_NoteOn()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     note = eventData->data[0];
    int     volume = eventData->data[1];
    VOICE   *voice = voiceHead;
    int     index;

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing == 0)
        {
            voice->channel = channel;
            voice->note = note;
            voice->volume = volumeTable[volume];
            VoiceVolume(voice);
            Synth_SetPan(index, channel->pan);

            FrequencyStep(voice);
            Synth_KeyOn(index);

            voice->playing = NOTE_PLAY | channel->sustain;

            break;
        }
    }
}

void Event_MuteNotes()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    VOICE   *voice = voiceHead;
    int     index;

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            VoiceOff(voice, NOTE_SUSTAIN);
        }
    }
}

void Event_PitchWheel()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     bend = ((eventData->data[0] & 0x7f) | (eventData->data[1] << 7));
    VOICE   *voice = voiceHead;
    int     index;

    channel->bend = (bend >> 7) - 64; // 7 bit values

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            FrequencyStep(voice);
        }
    }
}

void Event_Aftertouch()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     note = eventData->data[0];
    int     volume = eventData->data[1];
    VOICE   *voice = voiceHead;
    int     index;

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel && voice->note == note)
        {
            voice->volume = volumeTable[volume];
            VoiceVolume(voice);
        }
    }
}

void Event_Sustain()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     sustain = eventData->data[1] >> 6;
    VOICE   *voice = voiceHead;
    int     index;

    // sustain: 0=off, 2=on
    channel->sustain = ((sustain >> 1) | (sustain & 1)) << 1;

    if (channel->sustain != 0)
    {
        return;
    }

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            VoiceOff(voice, NOTE_PLAY);
        }
    }
}

void Event_ChannelVolume()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     volume = eventData->data[1];
    VOICE   *voice = voiceHead;
    int     index;

    channel->volume = volumeTable[volume & 0x7f];

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            VoiceVolume(voice);
        }
    }
}

void Event_Pan()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     pan = eventData->data[1] & 0x7f;
    VOICE   *voice = voiceHead;
    int     index;

    channel->pan = pan;

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            Synth_SetPan(index, pan);
        }
    }
}

void Event_ChannelAftertouch()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     volume = eventData->data[0];
    VOICE   *voice = voiceHead;
    int     index;

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            voice->volume = volumeTable[volume];
            VoiceVolume(voice);
        }
    }
}

void Event_Expression()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     expression = eventData->data[1] & 0x7f;
    VOICE   *voice = voiceHead;
    int     index;

    channel->expression = volumeTable[expression];

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            VoiceVolume(voice);
        }
    }
}

void Event_ChangeInstrument()
{
    CHANNEL *channel = &midChannel[eventData->channel];

    channel->instrument = eventData->data[1];
}

void Event_Message()
{
    switch (eventData->data[0])
    {
      case CC_07:
        Event_ChannelVolume();
        break;

      case CC_0a:
        Event_Pan();
        break;

      case CC_0b:
        Event_Expression();
        break;

      case CC_40: // FIXME
        //Event_Sustain();
        break;

      case CC_78:
        ResetVoices();
        break;

      case CC_79:
        ResetControls();
        break;

      case CC_7b:
        Event_MuteNotes();
        break;

      case CC_80:
        Event_ChangeInstrument();
        break;

      case CC_01:
      case CC_64:
      case CC_65:
        break;

      default:
        break;
    }
}

// midiplay

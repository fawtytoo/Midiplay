// midiplay

// Copyright 2022 by Steve Clark

#include "common.h"
#include "synth.h"

#include "event.h"

#define NOTE_PLAY       1
#define NOTE_SUSTAIN    2

// all frequencies are 16.16
UINT    frequencyTable[384] =
{
    0x00082d01, 0x000830c9, 0x00083493, 0x0008385e, 0x00083c2b, 0x00083ffa, 0x000843cb, 0x0008479e,
    0x00084b72, 0x00084f48, 0x00085320, 0x000856fa, 0x00085ad5, 0x00085eb3, 0x00086292, 0x00086672,
    0x00086a55, 0x00086e3a, 0x00087220, 0x00087608, 0x000879f2, 0x00087ddd, 0x000881cb, 0x000885ba,
    0x000889ab, 0x00088d9e, 0x00089193, 0x00089589, 0x00089981, 0x00089d7c, 0x0008a178, 0x0008a576,
    0x0008a976, 0x0008ad77, 0x0008b17b, 0x0008b580, 0x0008b987, 0x0008bd90, 0x0008c19b, 0x0008c5a8,
    0x0008c9b6, 0x0008cdc7, 0x0008d1d9, 0x0008d5ee, 0x0008da04, 0x0008de1c, 0x0008e236, 0x0008e652,
    0x0008ea70, 0x0008ee8f, 0x0008f2b1, 0x0008f6d4, 0x0008fafa, 0x0008ff21, 0x0009034a, 0x00090775,
    0x00090ba2, 0x00090fd1, 0x00091402, 0x00091835, 0x00091c6a, 0x000920a1, 0x000924da, 0x00092914,
    0x00092d51, 0x00093190, 0x000935d0, 0x00093a13, 0x00093e57, 0x0009429e, 0x000946e6, 0x00094b31,
    0x00094f7d, 0x000953cb, 0x0009581c, 0x00095c6e, 0x000960c2, 0x00096519, 0x00096971, 0x00096dcc,
    0x00097228, 0x00097686, 0x00097ae7, 0x00097f49, 0x000983ae, 0x00098814, 0x00098c7d, 0x000990e8,
    0x00099554, 0x000999c3, 0x00099e34, 0x0009a2a7, 0x0009a71c, 0x0009ab92, 0x0009b00b, 0x0009b487,
    0x0009b904, 0x0009bd83, 0x0009c204, 0x0009c687, 0x0009cb0d, 0x0009cf95, 0x0009d41e, 0x0009d8aa,
    0x0009dd38, 0x0009e1c8, 0x0009e65a, 0x0009eaee, 0x0009ef84, 0x0009f41d, 0x0009f8b7, 0x0009fd54,
    0x000a01f3, 0x000a0694, 0x000a0b37, 0x000a0fdc, 0x000a1483, 0x000a192d, 0x000a1dd8, 0x000a2286,
    0x000a2736, 0x000a2be8, 0x000a309d, 0x000a3553, 0x000a3a0c, 0x000a3ec7, 0x000a4384, 0x000a4843,
    0x000a4d05, 0x000a51c8, 0x000a568e, 0x000a5b56, 0x000a6021, 0x000a64ed, 0x000a69bc, 0x000a6e8d,
    0x000a7360, 0x000a7835, 0x000a7d0d, 0x000a81e7, 0x000a86c3, 0x000a8ba1, 0x000a9082, 0x000a9565,
    0x000a9a4a, 0x000a9f31, 0x000aa41b, 0x000aa907, 0x000aadf5, 0x000ab2e6, 0x000ab7d9, 0x000abcce,
    0x000ac1c5, 0x000ac6bf, 0x000acbbb, 0x000ad0b9, 0x000ad5b9, 0x000adabc, 0x000adfc2, 0x000ae4c9,
    0x000ae9d3, 0x000aeedf, 0x000af3ee, 0x000af8fe, 0x000afe12, 0x000b0327, 0x000b083f, 0x000b0d59,
    0x000b1276, 0x000b1795, 0x000b1cb6, 0x000b21da, 0x000b2700, 0x000b2c29, 0x000b3154, 0x000b3681,
    0x000b3bb1, 0x000b40e2, 0x000b4617, 0x000b4b4e, 0x000b5087, 0x000b55c3, 0x000b5b01, 0x000b6041,
    0x000b6584, 0x000b6aca, 0x000b7012, 0x000b755c, 0x000b7aa9, 0x000b7ff8, 0x000b8549, 0x000b8a9e,
    0x000b8ff4, 0x000b954d, 0x000b9aa9, 0x000ba007, 0x000ba567, 0x000baaca, 0x000bb02f, 0x000bb597,
    0x000bbb02, 0x000bc06f, 0x000bc5de, 0x000bcb50, 0x000bd0c5, 0x000bd63c, 0x000bdbb5, 0x000be131,
    0x000be6b0, 0x000bec31, 0x000bf1b5, 0x000bf73b, 0x000bfcc4, 0x000c024f, 0x000c07dd, 0x000c0d6e,
    0x000c1300, 0x000c1896, 0x000c1e2e, 0x000c23c9, 0x000c2966, 0x000c2f07, 0x000c34a9, 0x000c3a4e,
    0x000c3ff6, 0x000c45a1, 0x000c4b4e, 0x000c50fd, 0x000c56b0, 0x000c5c64, 0x000c621c, 0x000c67d6,
    0x000c6d93, 0x000c7353, 0x000c7915, 0x000c7eda, 0x000c84a1, 0x000c8a6c, 0x000c9038, 0x000c9608,
    0x000c9bda, 0x000ca1af, 0x000ca787, 0x000cad61, 0x000cb33e, 0x000cb91e, 0x000cbf00, 0x000cc4e5,
    0x000ccace, 0x000cd0b8, 0x000cd6a5, 0x000cdc96, 0x000ce288, 0x000ce87e, 0x000cee77, 0x000cf472,
    0x000cfa6f, 0x000d0070, 0x000d0674, 0x000d0c7a, 0x000d1283, 0x000d188f, 0x000d1e9d, 0x000d24af,
    0x000d2ac3, 0x000d30da, 0x000d36f4, 0x000d3d11, 0x000d4330, 0x000d4952, 0x000d4f78, 0x000d559f,
    0x000d5bca, 0x000d61f8, 0x000d6829, 0x000d6e5c, 0x000d7492, 0x000d7acb, 0x000d8108, 0x000d8747,
    0x000d8d88, 0x000d93cd, 0x000d9a15, 0x000da05f, 0x000da6ad, 0x000dacfd, 0x000db350, 0x000db9a6,
    0x000dc000, 0x000dc65b, 0x000dccbb, 0x000dd31c, 0x000dd981, 0x000ddfe9, 0x000de654, 0x000decc2,
    0x000df333, 0x000df9a6, 0x000e001d, 0x000e0697, 0x000e0d13, 0x000e1393, 0x000e1a16, 0x000e209c,
    0x000e2724, 0x000e2db0, 0x000e343f, 0x000e3ad1, 0x000e4166, 0x000e47fd, 0x000e4e98, 0x000e5537,
    0x000e5bd7, 0x000e627c, 0x000e6923, 0x000e6fcd, 0x000e767a, 0x000e7d2b, 0x000e83de, 0x000e8a95,
    0x000e914f, 0x000e980c, 0x000e9ecc, 0x000ea58f, 0x000eac55, 0x000eb31e, 0x000eb9eb, 0x000ec0bb,
    0x000ec78d, 0x000ece63, 0x000ed53c, 0x000edc19, 0x000ee2f8, 0x000ee9db, 0x000ef0c1, 0x000ef7aa,
    0x000efe96, 0x000f0585, 0x000f0c78, 0x000f136e, 0x000f1a67, 0x000f2163, 0x000f2862, 0x000f2f65,
    0x000f366b, 0x000f3d74, 0x000f4481, 0x000f4b91, 0x000f52a4, 0x000f59ba, 0x000f60d4, 0x000f67f0,
    0x000f6f10, 0x000f7634, 0x000f7d5b, 0x000f8485, 0x000f8bb2, 0x000f92e3, 0x000f9a17, 0x000fa14e,
    0x000fa889, 0x000fafc7, 0x000fb708, 0x000fbe4d, 0x000fc595, 0x000fcce0, 0x000fd42f, 0x000fdb81,
    0x000fe2d7, 0x000fea30, 0x000ff18c, 0x000ff8ec, 0x0010004f, 0x001007b6, 0x00100f20, 0x0010168d,
    0x00101dfe, 0x00102573, 0x00102cea, 0x00103465, 0x00103be4, 0x00104366, 0x00104aec, 0x00105275
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

CHANNEL midChannel[16];
VOICE   midVoice[VOICES], *voiceHead = &midVoice[0], *voiceTail = &midVoice[VOICES - 1];

EVENT   *eventData;

void ResetChannel(int channel)
{
    midChannel[channel].instrument = 0;
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

    volume = VOLUME * volumeTable[voice->channel->volume * voice->channel->expression >> 7] * volumeTable[voice->volume] >> 16;

    voice->left = volume * panTable[0][voice->channel->pan] >> 8;
    voice->right = volume * panTable[1][voice->channel->pan] >> 8;
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
        midChannel[channel].bend = 0;
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
    int     index, octave;

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
            index = 0;
    }
    else if (index >= 384)
    {
        octave++;
        index -= 384;
    }

    voice->step = frequencyTable[index] << octave;
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
    int     bend = ((eventData->data[0] & 0x7f) | (eventData->data[1] << 7));
    VOICE   *voice;

    channel->bend = (bend >> 7) - 64; // 7 bit values

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
    int     volume = eventData->data[0];
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

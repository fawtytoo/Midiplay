// midiplay

// Copyright 2022 by Steve Clark

#include "common.h"
#include "event.h"

#define NOTE_PLAY       1
#define NOTE_SUSTAIN    2

// all frequencies are 16.16
UINT        frequencyTable[128] =
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

UINT        pitchBendTable[64] =
{
    0x0000, 0x02c9, 0x059b, 0x0874, 0x0b55, 0x0e3e, 0x1130, 0x1429, 0x172b, 0x1a35, 0x1d48, 0x2063, 0x2387, 0x26b4, 0x29e9, 0x2d28,
    0x306f, 0x33c0, 0x371a, 0x3a7d, 0x3dea, 0x4160, 0x44e0, 0x486a, 0x4bfd, 0x4f9b, 0x5342, 0x56f4, 0x5ab0, 0x5e76, 0x6247, 0x6623,
    0x6a09, 0x6dfb, 0x71f7, 0x75fe, 0x7a11, 0x7e2f, 0x8258, 0x868d, 0x8ace, 0x8f1a, 0x9373, 0x97d8, 0x9c49, 0xa0c6, 0xa550, 0xa9e6,
    0xae89, 0xb33a, 0xb7f7, 0xbcc1, 0xc199, 0xc67f, 0xcb72, 0xd072, 0xd581, 0xda9e, 0xdfc9, 0xe502, 0xea4a, 0xefa1, 0xf507, 0xfa7c
};

float       volumeTable[128] =
{
    0.000000f, 0.007874f, 0.023622f, 0.039370f, 0.047244f, 0.062992f, 0.078740f, 0.086614f,
    0.102362f, 0.110236f, 0.125984f, 0.133858f, 0.149606f, 0.157480f, 0.173228f, 0.181102f,
    0.196850f, 0.204724f, 0.212598f, 0.228346f, 0.236220f, 0.251969f, 0.259843f, 0.267717f,
    0.283465f, 0.291339f, 0.307087f, 0.322835f, 0.338583f, 0.354331f, 0.370079f, 0.385827f,
    0.393701f, 0.409449f, 0.425197f, 0.433071f, 0.448819f, 0.464567f, 0.472441f, 0.480315f,
    0.496063f, 0.503937f, 0.519685f, 0.527559f, 0.535433f, 0.543307f, 0.559055f, 0.566929f,
    0.574803f, 0.582677f, 0.590551f, 0.598425f, 0.606299f, 0.622047f, 0.629921f, 0.637795f,
    0.645669f, 0.653543f, 0.661417f, 0.661417f, 0.669291f, 0.677165f, 0.685039f, 0.692913f,
    0.700787f, 0.708661f, 0.716535f, 0.724409f, 0.724409f, 0.732283f, 0.740157f, 0.748031f,
    0.755906f, 0.755906f, 0.763780f, 0.771654f, 0.779528f, 0.779528f, 0.787402f, 0.795276f,
    0.795276f, 0.803150f, 0.811024f, 0.811024f, 0.818898f, 0.826772f, 0.826772f, 0.834646f,
    0.842520f, 0.842520f, 0.850394f, 0.858268f, 0.858268f, 0.866142f, 0.866142f, 0.874016f,
    0.881890f, 0.881890f, 0.889764f, 0.889764f, 0.897638f, 0.897638f, 0.905512f, 0.905512f,
    0.913386f, 0.921260f, 0.921260f, 0.929134f, 0.929134f, 0.937008f, 0.937008f, 0.944882f,
    0.944882f, 0.952756f, 0.952756f, 0.960630f, 0.960630f, 0.968504f, 0.968504f, 0.968504f,
    0.976378f, 0.976378f, 0.984252f, 0.984252f, 0.992126f, 0.992126f, 1.000000f, 1.000000f
};

float       panTable[128][2] =
{
    {1.0f, 0.000000f}, {1.0f, 0.000122f}, {1.0f, 0.000488f}, {1.0f, 0.001099f},
    {1.0f, 0.001955f}, {1.0f, 0.003056f}, {1.0f, 0.004404f}, {1.0f, 0.005999f},
    {1.0f, 0.007843f}, {1.0f, 0.009937f}, {1.0f, 0.012282f}, {1.0f, 0.014881f},
    {1.0f, 0.017735f}, {1.0f, 0.020847f}, {1.0f, 0.024219f}, {1.0f, 0.027854f},
    {1.0f, 0.031754f}, {1.0f, 0.035924f}, {1.0f, 0.040365f}, {1.0f, 0.045084f},
    {1.0f, 0.050082f}, {1.0f, 0.055366f}, {1.0f, 0.060939f}, {1.0f, 0.066807f},
    {1.0f, 0.072975f}, {1.0f, 0.079450f}, {1.0f, 0.086238f}, {1.0f, 0.093346f},
    {1.0f, 0.100782f}, {1.0f, 0.108553f}, {1.0f, 0.116669f}, {1.0f, 0.125140f},
    {1.0f, 0.133975f}, {1.0f, 0.143186f}, {1.0f, 0.152785f}, {1.0f, 0.162786f},
    {1.0f, 0.173203f}, {1.0f, 0.184052f}, {1.0f, 0.195350f}, {1.0f, 0.207118f},
    {1.0f, 0.219375f}, {1.0f, 0.232146f}, {1.0f, 0.245456f}, {1.0f, 0.259335f},
    {1.0f, 0.273816f}, {1.0f, 0.288934f}, {1.0f, 0.304731f}, {1.0f, 0.321256f},
    {1.0f, 0.338562f}, {1.0f, 0.356713f}, {1.0f, 0.375782f}, {1.0f, 0.395856f},
    {1.0f, 0.417039f}, {1.0f, 0.439457f}, {1.0f, 0.463264f}, {1.0f, 0.488654f},
    {1.0f, 0.515877f}, {1.0f, 0.545261f}, {1.0f, 0.577258f}, {1.0f, 0.612513f},
    {1.0f, 0.652015f}, {1.0f, 0.697423f}, {1.0f, 0.751961f}, {1.0f, 0.823915f},
    {1.0f, 1.0f}, {0.822534f, 1.0f}, {0.750031f, 1.0f}, {0.695089f, 1.0f},
    {0.649354f, 1.0f}, {0.609575f, 1.0f}, {0.574082f, 1.0f}, {0.541877f, 1.0f},
    {0.512308f, 1.0f}, {0.484921f, 1.0f}, {0.459385f, 1.0f}, {0.435447f, 1.0f},
    {0.412913f, 1.0f}, {0.391626f, 1.0f}, {0.371461f, 1.0f}, {0.352311f, 1.0f},
    {0.334090f, 1.0f}, {0.316722f, 1.0f}, {0.300146f, 1.0f}, {0.284305f, 1.0f},
    {0.269151f, 1.0f}, {0.254644f, 1.0f}, {0.240745f, 1.0f}, {0.227423f, 1.0f},
    {0.214647f, 1.0f}, {0.202391f, 1.0f}, {0.190632f, 1.0f}, {0.179348f, 1.0f},
    {0.168521f, 1.0f}, {0.158131f, 1.0f}, {0.148165f, 1.0f}, {0.138605f, 1.0f},
    {0.129441f, 1.0f}, {0.120658f, 1.0f}, {0.112246f, 1.0f}, {0.104194f, 1.0f},
    {0.096492f, 1.0f}, {0.089132f, 1.0f}, {0.082106f, 1.0f}, {0.075405f, 1.0f},
    {0.069024f, 1.0f}, {0.062954f, 1.0f}, {0.057191f, 1.0f}, {0.051728f, 1.0f},
    {0.046561f, 1.0f}, {0.041685f, 1.0f}, {0.037095f, 1.0f}, {0.032787f, 1.0f},
    {0.028758f, 1.0f}, {0.025004f, 1.0f}, {0.021522f, 1.0f}, {0.018308f, 1.0f},
    {0.015361f, 1.0f}, {0.012678f, 1.0f}, {0.010257f, 1.0f}, {0.008095f, 1.0f},
    {0.006192f, 1.0f}, {0.004545f, 1.0f}, {0.003154f, 1.0f}, {0.002018f, 1.0f},
    {0.001134f, 1.0f}, {0.000504f, 1.0f}, {0.000126f, 1.0f}, {0.000000f, 1.0f}
};

typedef struct
{
    int         volume;
    int         pan;
    int         sustain;
    int         bend;
} CHANNEL;

typedef struct
{
    CHANNEL     *channel;
    int         note;
    int         volume;
    UINT        phase, step;
    short       stereo[2];
    int         playing; // bit field
} VOICE;

CHANNEL     midChannel[16];
VOICE       midVoice[VOICES];

EVENT       *eventData;

int         midVolume = VOLUME;

int         rateAcc;

void voiceVolume(VOICE *voice)
{
    float       volume;

    volume = midVolume * volumeTable[voice->channel->volume];
    volume *= volumeTable[voice->volume];

    voice->stereo[0] = volume * panTable[voice->channel->pan][0];
    voice->stereo[1] = volume * panTable[voice->channel->pan][1];
}

void updateVolume(int volume)
{
    int         voice;

    midVolume = VOLUME * volume / 127;

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].playing)
            voiceVolume(&midVoice[voice]);
}

void resetVoices()
{
    int         voice;

    for (voice = 0; voice < VOICES; voice++)
        midVoice[voice].playing = 0;
}

void resetControls()
{
    int         channel;

    for (channel = 0; channel < 16; channel++)
    {
        midChannel[channel].volume = 100;
        midChannel[channel].pan = 64;
        midChannel[channel].sustain = 0;
        midChannel[channel].bend = 128;
    }
}

void eventNoteOff()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         note = eventData->data[0];
    //int         volume = eventData->data[1];
    int         voice;

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].playing)
            if (midVoice[voice].channel == channel)
                if (midVoice[voice].note == note)
                    midVoice[voice].playing &= NOTE_SUSTAIN;
}

void frequencyStep(VOICE *voice)
{
    int         note = voice->note;
    int         bend = voice->channel->bend;
    UINT        diff;

    note += (bend >> 6) - 2;
    bend &= 63;

    diff = (frequencyTable[note + 1] - frequencyTable[note]) >> 16;

    voice->step = frequencyTable[note] + ((diff * pitchBendTable[bend]));
}

void eventNoteOn()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         note = eventData->data[0];
    int         volume = eventData->data[1];
    int         voice;

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].playing == 0)
        {
            midVoice[voice].channel = channel;
            midVoice[voice].note = note;
            midVoice[voice].volume = volume;
            voiceVolume(&midVoice[voice]);

            frequencyStep(&midVoice[voice]);
            midVoice[voice].phase = 0;

            midVoice[voice].playing = NOTE_PLAY | channel->sustain;

            break;
        }
}

void eventMuteNotes()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         voice;

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].playing)
            if (midVoice[voice].channel == channel)
                midVoice[voice].playing &= NOTE_SUSTAIN;
}

void eventPitchWheel()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         voice;

    channel->bend = ((eventData->data[0] & 0x7f) | (eventData->data[1] << 7)) >> 6; // 8 bit values

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].playing)
            if (midVoice[voice].channel == channel)
                frequencyStep(&midVoice[voice]);
}

void eventAftertouch()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         note = eventData->data[0];
    int         volume = eventData->data[1];
    int         voice;

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].playing)
            if (midVoice[voice].channel == channel)
                if (midVoice[voice].note == note)
                {
                    midVoice[voice].volume = volume;
                    voiceVolume(&midVoice[voice]);
                }
}

void eventSustain()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         sustain = eventData->data[1] >> 6;
    int         voice;

    // sustain: 0=off, 2=on
    channel->sustain = ((sustain >> 1) | (sustain & 1)) << 1;

    if (sustain != 0)
        return;

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].playing)
            if (midVoice[voice].channel == channel)
                midVoice[voice].playing &= NOTE_PLAY;
}

void eventChannel()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         volume = eventData->data[1];
    int         voice;

    channel->volume = volume & 0x7f;

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].playing)
            if (midVoice[voice].channel == channel)
                voiceVolume(&midVoice[voice]);
}

void eventPan()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         pan = eventData->data[1] & 0x7f;
    int         voice;

    channel->pan = pan;

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].playing)
            if (midVoice[voice].channel == channel)
                voiceVolume(&midVoice[voice]);
}

void eventChannelAftertouch()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         volume = eventData->data[1];
    int         voice;

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].playing)
            if (midVoice[voice].channel == channel)
            {
                midVoice[voice].volume = volume;
                voiceVolume(&midVoice[voice]);
            }
}

void eventMessage()
{
    int         message = eventData->data[0];

    switch (message)
    {
      case MM_VOLUME:
        eventChannel();
        break;

      case MM_PAN:
        eventPan();
        break;

      case MM_NOTEOFF:
        eventMuteNotes();
        break;

      case MM_CTRLOFF:
        resetControls();
        break;

      case MM_SUSTAIN: // FIXME
        //eventSustain();
        break;

      case MM_AFTERTOUCH:
        eventChannelAftertouch();
        break;

      case MM_INSTR:
      case MM_EXPRESS:
      case MM_MODWHEEL:
      case MM_REG_LSB:
      case MM_REG_MSB:
        break;

      default:
        break;
    }
}

void generateSample(short *buffer)
{
    int         voice;
    short       left = 0, right = 0;

    if (musicPlaying)
        while (rateAcc >= musicSamplerate)
        {
            left = right = 0;

            for (voice = 0; voice < VOICES; voice++)
            {
                if (midVoice[voice].playing == 0)
                    continue;

                left += midVoice[voice].stereo[0] * (((midVoice[voice].phase >> 31) << 1) - 1);
                right += midVoice[voice].stereo[1] * (((midVoice[voice].phase >> 31) << 1) - 1);

                midVoice[voice].phase += midVoice[voice].step;
            }

            rateAcc -= musicSamplerate;
        }

    *buffer++ = left;
    *buffer = right;

    rateAcc += 65536;
}

// midiplay

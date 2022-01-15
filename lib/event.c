// midiplay

// Copyright 2022 by Steve Clark

#include "common.h"
#include "event.h"

float       frequencyTable[128] =
{
    8.175799f, 8.661958f, 9.177023f, 9.722718f, 10.300862f, 10.913381f, 11.562326f, 12.249859f,
    12.978270f, 13.750000f, 14.567619f, 15.433851f, 16.351598f, 17.323916f, 18.354046f, 19.445436f,
    20.601725f, 21.826762f, 23.124651f, 24.499717f, 25.956541f, 27.500000f, 29.135233f, 30.867708f,
    32.703196f, 34.647827f, 36.708098f, 38.890873f, 41.203442f, 43.653531f, 46.249303f, 48.999427f,
    51.913090f, 55.000000f, 58.270467f, 61.735416f, 65.406391f, 69.295654f, 73.416196f, 77.781746f,
    82.406885f, 87.307063f, 92.498606f, 97.998854f, 103.826180f, 110.000000f, 116.540944f, 123.470822f,
    130.812783f, 138.591319f, 146.832380f, 155.563492f, 164.813783f, 174.614111f, 184.997211f, 195.997723f,
    207.652343f, 220.000000f, 233.081878f, 246.941654f, 261.625565f, 277.182627f, 293.664772f, 311.126984f,
    329.627559f, 349.228229f, 369.994423f, 391.995435f, 415.304697f, 440.000000f, 466.163762f, 493.883303f,
    523.251131f, 554.365266f, 587.329532f, 622.253967f, 659.255105f, 698.456472f, 739.988845f, 783.990861f,
    830.609407f, 880.000000f, 932.327549f, 987.766575f, 1046.502261f, 1108.730554f, 1174.659039f, 1244.507935f,
    1318.510264f, 1396.912887f, 1479.977691f, 1567.981787f, 1661.218745f, 1760.000000f, 1864.654943f, 1975.533314f,
    2093.004522f, 2217.460926f, 2349.318273f, 2489.015870f, 2637.020310f, 2793.826005f, 2959.955382f, 3135.963315f,
    3322.437764f, 3520.000000f, 3729.309887f, 3951.066628f, 4186.009045f, 4434.921851f, 4698.636546f, 4978.031740f,
    5274.040620f, 5587.652011f, 5919.910763f, 6271.926630f, 6644.875527f, 7040.000000f, 7458.621006f, 7902.131949f,
    8372.018090f, 8869.845168f, 9397.271538f, 9956.063479f, 10548.082983f, 11175.302175f, 11839.821527f, 12543.855333f
};

float       pitchBendTable[64] =
{
    0.000000f, 0.010889f, 0.021897f, 0.033025f, 0.044274f, 0.055645f, 0.067140f, 0.078761f,
    0.090508f, 0.102383f, 0.114387f, 0.126522f, 0.138789f, 0.151189f, 0.163725f, 0.176397f,
    0.189207f, 0.202157f, 0.215247f, 0.228481f, 0.241858f, 0.255381f, 0.269051f, 0.282870f,
    0.296840f, 0.310961f, 0.325237f, 0.339668f, 0.354256f, 0.369002f, 0.383910f, 0.398980f,
    0.414214f, 0.429613f, 0.445181f, 0.460918f, 0.476826f, 0.492908f, 0.509164f, 0.525598f,
    0.542211f, 0.559004f, 0.575981f, 0.593142f, 0.610490f, 0.628027f, 0.645755f, 0.663677f,
    0.681793f, 0.700106f, 0.718619f, 0.737334f, 0.756252f, 0.775376f, 0.794709f, 0.814252f,
    0.834008f, 0.853979f, 0.874168f, 0.894576f, 0.915207f, 0.936062f, 0.957144f, 0.978456f
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
} CHANNEL;

typedef struct
{
    CHANNEL     *channel;
    int         note;
    int         volume; // midi specific
    int         playing, sustain;
    float       width, sample;
} VOICE;

CHANNEL     midChannel[16];
VOICE       midVoice[VOICES];

EVENT       *eventData;

void resetVoices()
{
    int         voice;

    for (voice = 0; voice < VOICES; voice++)
        midVoice[voice].playing = midVoice[voice].sustain = 0;
}

void resetControls()
{
    int         channel;

    for (channel = 0; channel < 16; channel++)
    {
        midChannel[channel].volume = 100;
        midChannel[channel].pan = 64;
    }
}

void eventNoteOff()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         note = eventData->data[0];
    //int         volume = curTrack->event.data[1];
    int         voice;

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].channel == channel && midVoice[voice].note == note)
            midVoice[voice].playing = 0;
}

void eventNoteOn()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         note = eventData->data[0];
    int         volume = eventData->data[1];
    int         voice;

    if (volume == 0)
    {
        eventNoteOff();
        return;
    }

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].playing == 0 && midVoice[voice].sustain == 0)
            break;

    if (voice == VOICES)
        return; // 4:0 Out of memory

    if (volume < 128)
        channel->volume = volume;

    midVoice[voice].channel = channel;

    midVoice[voice].width = (float)musicSamplerate / frequencyTable[note];
    midVoice[voice].sample = midVoice[voice].width;

    midVoice[voice].note = note;
    midVoice[voice].volume = 127;
    midVoice[voice].playing = 1;
}

void eventMuteNotes()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         voice;

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].channel == channel)
            midVoice[voice].playing = 0;
}

void eventPitchWheel()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         bend = ((eventData->data[0] & 0x7f) | (eventData->data[1] << 7)) >> 6; // 8 bit values
    int         voice;
    int         note;
    float       diff;

    for (voice = 0; voice < VOICES; voice++)
    {
        if (midVoice[voice].channel != channel)
            continue;

        note = midVoice[voice].note;
        if (bend < 64)
        {
            note -= 2;
        }
        else if (bend < 128)
        {
            note -= 1;
            bend -= 64;
        }
        else if (bend > 191)
        {
            note += 1;
            bend -= 192;
        }
        else
            bend -= 128;

        diff = frequencyTable[note + 1] - frequencyTable[note];

        midVoice[voice].width = (float)musicSamplerate / (frequencyTable[note] + diff * pitchBendTable[bend]);

        if (midVoice[voice].sample > midVoice[voice].width)
            midVoice[voice].sample = midVoice[voice].width;
    }
}

void eventAftertouch()
{
    CHANNEL     *channel = &midChannel[eventData->channel];
    int         note = eventData->data[0];
    int         volume = eventData->data[1];
    int         voice;

    for (voice = 0; voice < VOICES; voice++)
        if (midVoice[voice].note == note && midVoice[voice].channel == channel)
            midVoice[voice].volume = volume;
}

void eventSustain(int sustain)
{
    int         voice;

    for (voice = 0; voice < VOICES; voice++)
        if (sustain)
        {
            if (midVoice[voice].playing)
                midVoice[voice].sustain = 1;
        }
        else
            midVoice[voice].sustain = 0;
}

void eventMessage()
{
    int         channel = eventData->channel;
    int         message = eventData->data[0];
    int         value = eventData->data[1];

    switch (message)
    {
      case MM_VOLUME:
        midChannel[channel].volume = value & 0x7f;
        break;

      case MM_PAN:
        midChannel[channel].pan = value & 0x7f;
        break;

      case MM_NOTEOFF:
        eventMuteNotes();
        break;

      case MM_CTRLOFF:
        resetControls();
        break;

      case MM_SUSTAIN:
        eventSustain((value & 192) >> 6);
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

void generateSamples(short *buffer, int samples)
{
    int         voice, count;
    float       volume;
    short       *out, left, right;

    for (voice = 0; voice < VOICES; voice++)
    {
        if (midVoice[voice].playing == 0 && midVoice[voice].sustain == 0)
            continue;

        volume = musicVolume * volumeTable[midVoice[voice].channel->volume];
        volume *= volumeTable[midVoice[voice].volume];
        left = volume * panTable[midVoice[voice].channel->pan][0];
        right = volume * panTable[midVoice[voice].channel->pan][1];

        out = buffer;

        for (count = 0; count < samples; count++)
        {
            if (midVoice[voice].sample < midVoice[voice].width / 2.0f)
            {
                *out++ += left;
                *out++ += right;
            }
            else
            {
                *out++ -= left;
                *out++ -= right;
            }

            if (--midVoice[voice].sample < 1.0f)
                midVoice[voice].sample += midVoice[voice].width;
        }
    }
}

// midiplay

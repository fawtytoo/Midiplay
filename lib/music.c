// midi play

#include "common.h"

typedef struct
{
    char        id[4];
    WORD        scoreLen;
    WORD        scoreStart;
    // there is more to a MUS header but this is all we need
} __attribute__ ((packed)) HDR_MUS;

typedef struct
{
    char        id[4];
    int         length;
    WORD        type;
    WORD        ntracks;
    WORD        ticks;
} __attribute__ ((packed)) HDR_MID;

function    musicEvents;

int         musicInit = 0;

int         musicSamplerate;
int         musicLooping;
int         musicPlaying = 0;

int         musicVolume = VOLUME;

void MUSIC_Init(int samplerate)
{
    musicSamplerate = samplerate;

    musicInit = 1;
}

void MUSIC_Play(void *data, int size, int looping)
{
    HDR_MUS     *hdrMus;
    HDR_MID     *hdrMid;

    if (musicInit == 0)
        return;

    musicInit = 1;
    musicPlaying = 0;

    hdrMus = (HDR_MUS *)data;
    hdrMid = (HDR_MID *)data;

    if (*(UINT *)hdrMus->id == ('M' | ('U' << 8) | ('S' << 16) | (0x1a << 24)))
    {
        if (size != hdrMus->scoreStart + hdrMus->scoreLen)
            return;

        loadMusTrack(data + hdrMus->scoreStart);
        musicEvents = trackMusEvents;
        tickSamples = (float)musicSamplerate / 140.0f;
    }
    else if (*(UINT *)hdrMid->id == ('M' | ('T' << 8) | ('h' << 16) | ('d' << 24)))
    {
        if (loadMidTracks(__builtin_bswap16(hdrMid->ntracks), data + sizeof(HDR_MID)) == 1)
            return; // track header failed

        beatTicks = __builtin_bswap16(hdrMid->ticks);
        musicEvents = trackMidEvents;
    }
    else
        return;

    initTracks();
    musicLooping = looping;
    musicPlaying = 1;
    musicInit = 2;
}

void MUSIC_Pause(int playing)
{
    // if there's nothing to play, don't force it
    if (musicInit < 2)
        return;

    musicPlaying = playing;
}

void MUSIC_SetVolume(int volume)
{
    if (volume < 0)
        volume = 0;
    else if (volume > 127)
        volume = 127;

    musicVolume = VOLUME * volume / 127;
}

int MUSIC_IsPlaying()
{
    return musicPlaying;
}

void MUSIC_Output(short *buffer, int length)
{
    int         samples;

    for (samples = 0; samples < length; samples++)
        *(buffer + samples) = 0;

    length /= 2; // 1 sample = left + right

    while (length)
    {
        // fill with silence if we're not playing
        if (musicPlaying == 0)
        {
            length--;
            continue;
        }

        if (playSamples < 1.0f)
        {
            playSamples += tickSamples;
            musicEvents();
            tickTock++;
        }

        // how many samples should we play?
        samples = (int)playSamples;
        if (samples > length)
            samples = length;

        playSamples -= samples;
        length -= samples;

        generateSamples(buffer, samples);
        buffer += samples * 2;
    }
}

// midi play

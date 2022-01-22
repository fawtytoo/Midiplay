// midiplay

// Copyright 2022 by Steve Clark

#include "common.h"

typedef struct
{
    char        id[4];
    WORD        scoreLen;
    WORD        scoreStart;
    WORD        priChannels;
    WORD        secChannels;
    WORD        numInstruments;
    WORD        ppqn; // we're going to use this to specify ppqn, default 70
} __attribute__ ((packed)) HDR_MUS;

typedef struct
{
    char        id[4];
    int         length;
    WORD        type;
    WORD        ntracks;
    WORD        ppqn;
} __attribute__ ((packed)) HDR_MID;

function    musicEvents;

int         musicInit = 0;

int         musicSamplerate;
int         musicLooping;
int         musicPlaying = 0;

int         musicVolume = VOLUME;

void Midiplay_Init(int samplerate)
{
    musicSamplerate = samplerate;

    musicInit = 1;
}

int Midiplay_Load(void *data, int size, int looping)
{
    HDR_MUS     *hdrMus;
    HDR_MID     *hdrMid;

    if (musicInit == 0)
        return 0;

    musicInit = 1;
    musicPlaying = 0;

    if (size < 4)
        return 0;

    hdrMus = (HDR_MUS *)data;
    hdrMid = (HDR_MID *)data;

    if (*(UINT *)hdrMus->id == ('M' | ('U' << 8) | ('S' << 16) | (0x1a << 24)))
    {
        if (size < sizeof(HDR_MUS))
            return 0;

        if (size < hdrMus->scoreStart + hdrMus->scoreLen)
            return 0;

        loadMusTrack(data + hdrMus->scoreStart);

        beatTicks = 70;
        if (hdrMus->ppqn > 0)
            beatTicks = hdrMus->ppqn;

        musicEvents = trackMusEvents;
    }
    else if (*(UINT *)hdrMid->id == ('M' | ('T' << 8) | ('h' << 16) | ('d' << 24)))
    {
        if (size < sizeof(HDR_MID))
            return 0;

        if (loadMidTracks(__builtin_bswap16(hdrMid->ntracks), data + sizeof(HDR_MID), size - sizeof(HDR_MID)) == 1)
            return 0; // track header failed

        beatTicks = __builtin_bswap16(hdrMid->ppqn);
        musicEvents = trackMidEvents;
    }
    else
        return 0;

    initTracks();
    musicLooping = 0;

    while (numTracksEnded < numTracks)
    {
        updateTime();
        musicEvents();
        tickTock++;
    }

    musicLooping = looping;
    musicInit = 2;

    return 1;
}

void Midiplay_Play(int playing)
{
    // if there's nothing to play, don't force it
    if (musicInit < 2)
        return;

    if (numTracksEnded == numTracks)
        initTracks();

    musicPlaying = playing;
}

void Midiplay_SetVolume(int volume)
{
    if (volume < 0)
        volume = 0;
    else if (volume > 127)
        volume = 127;

    musicVolume = VOLUME * volume / 127;
}

int Midiplay_IsPlaying()
{
    if (musicInit < 2)
        return 0;

    if (numTracksEnded < numTracks)
        return 1;

    return 0;
}

void Midiplay_Output(short *buffer, int length)
{
    int         samples;

    for (samples = 0; samples < length; samples++)
        *(buffer + samples) = 0;

    length /= 2; // 1 sample = left + right

    while (length)
    {
        // fill with silence if we're not playing or not initialised
        if (musicPlaying == 0 || musicInit < 2)
        {
            length--;
            continue;
        }

        if (playSamples < 1.0f)
        {
            updateTime();
            musicEvents();
            playSamples += tickSamples;
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

int Midiplay_Time()
{
    if (musicInit < 2)
        return 0;

    return timeTicks / beatTicks;
}

void Midiplay_Loop(int looping)
{
    musicLooping = looping;
}

void Midiplay_Restart()
{
    initTracks();
}

// midiplay

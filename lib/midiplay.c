// midiplay

// Copyright 2022 by Steve Clark

#include "common.h"
#include "timer.h"

typedef struct
{
    char    id[4];
    WORD    scoreLen;
    WORD    scoreStart;
    WORD    priChannels;
    WORD    secChannels;
    WORD    numInstruments;
    WORD    ppqn; // we're going to use this to specify ppqn, default 70
} __attribute__ ((packed)) HDR_MUS;

typedef struct
{
    char    id[4];
    int     length;
    WORD    type;
    WORD    ntracks;
    WORD    ppqn;
} __attribute__ ((packed)) HDR_MID;

int         musicInit = 0;

int         musicLooping;
int         musicPlaying = 0;
int         musicVolume = 0x100;

TIMER       phaseRate;

void Midiplay_Init(int samplerate)
{
    SetTimer(&phaseRate, 65536, samplerate);
    SetTimer(&timerSecond, 1000000, samplerate);

    musicInit = 1;
}

int Midiplay_Load(void *data, int size, int looping)
{
    HDR_MUS *hdrMus;
    HDR_MID *hdrMid;

    if (musicInit == 0)
        return 0;

    musicInit = 1;
    musicPlaying = 0;

    if (size < 4)
        return 0;

    hdrMus = (HDR_MUS *)data;
    hdrMid = (HDR_MID *)data;

    if (hdrMus->id[0] == 'M' && hdrMus->id[1] == 'U' && hdrMus->id[2] == 'S' && hdrMus->id[3] == 0x1a)
    {
        if (size < sizeof(HDR_MUS))
            return 0;

        if (size < hdrMus->scoreStart + hdrMus->scoreLen)
            return 0;

        LoadMusTrack(data + hdrMus->scoreStart);

        beatTicks = 70;
        if (hdrMus->ppqn > 0)
            beatTicks = hdrMus->ppqn;
    }
    else if (hdrMid->id[0] == 'M' && hdrMid->id[1] == 'T' && hdrMid->id[2] == 'h' && hdrMid->id[3] == 'd')
    {
        if (size < sizeof(HDR_MID))
            return 0;

        beatTicks = __builtin_bswap16(hdrMid->ppqn);
        if (beatTicks < 0)
            return 0; // no support for SMPTE format yet

        if (LoadMidTracks(__builtin_bswap16(hdrMid->ntracks), data + sizeof(HDR_MID), size - sizeof(HDR_MID)) == 1)
            return 0; // track header failed
    }
    else
        return 0;

    InitTracks();
    musicLooping = 0;

    while (numTracksEnded < numTracks)
        UpdateEvents();

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
        InitTracks();

    musicPlaying = playing;
}

void Midiplay_SetVolume(int volume)
{
    if (volume < 0)
        volume = 0;
    else if (volume > 127)
        volume = 127;

    musicVolume = volumeTable[volume];
}

int Midiplay_IsPlaying()
{
    if (musicInit < 2)
        return 0;

    if (numTracksEnded < numTracks)
        return 1;

    return 0;
}

void Midiplay_Output(short *output, int length)
{
    short   buffer[2];
    int     rate;

    length /= 2; // 1 sample = left + right

    while (length)
    {
        if (musicPlaying)
        {
            if (playFlag)
            {
                UpdateEvents();
                playSamples += UpdateTimer(&timerBeat);
                playFlag = 0;
            }

            while (length)
            {
                rate = UpdateTimer(&timerSecond);
                if (playSamples < rate)
                {
                    // is this a FIXME or TODO or just a NOTE?
                    // there is the potential of lost/gained time here due
                    //  to the rate returned by UpdateTimer being unused
                    // however, any time discrepancy should be minimal
                    // ideally, the value should be re-used
                    playFlag = 1;
                    break;
                }
                playSamples -= rate;

                GenerateSample(buffer, UpdateTimer(&phaseRate));
                output[0] = buffer[0] * musicVolume >> 8;
                output[1] = buffer[1] * musicVolume >> 8;
                output += 2;
                length--;
            }
        }
        else
        {
            *output++ = 0;
            *output++ = 0;
            length--;
        }
    }
}

int Midiplay_Time()
{
    if (musicInit < 2)
        return 0;

    return timeTicks * 10 / beatTicks;
}

void Midiplay_Loop(int looping)
{
    musicLooping = looping;
}

void Midiplay_Restart()
{
    InitTracks();
}

// midiplay

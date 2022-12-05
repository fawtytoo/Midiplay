// midiplay

// Copyright 2022 by Steve Clark

#include "common.h"
#include "timer.h"

// header sizes in bytes
#define MUS_HDRSIZE 16
#define MID_HDRSIZE 14

#define LE16(a, b)  (a | (b << 8))
#define BE16(a, b)  ((a << 8) | b)

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

int Midiplay_Load(void *data, int size)
{
    BYTE    *hdr = (BYTE *)data;

    if (musicInit == 0)
        return 0;

    musicInit = 1;
    musicPlaying = 0;

    if (size < 4)
        return 0;

    if (hdr[0] == 'M' && hdr[1] == 'U' && hdr[2] == 'S' && hdr[3] == 0x1a)
    {
        if (size < MUS_HDRSIZE)
            return 0;

        if (size < LE16(hdr[6], hdr[7]) + LE16(hdr[4], hdr[5]))
            return 0;

        beatTicks = LE16(hdr[14], hdr[15]);
        if (beatTicks == 0)
            beatTicks = 70;

        LoadMusTrack(hdr + LE16(hdr[6], hdr[7]));
    }
    else if (hdr[0] == 'M' && hdr[1] == 'T' && hdr[2] == 'h' && hdr[3] == 'd')
    {
        if (size < MID_HDRSIZE)
            return 0;

        beatTicks = BE16(hdr[12], hdr[13]);
        if (beatTicks < 0)
            return 0; // no support for SMPTE format yet

        size -= MID_HDRSIZE;
        if (LoadMidTracks(BE16(hdr[10], hdr[11]), &hdr[MID_HDRSIZE], size) == 1)
            return 0; // track header failed
    }
    else
        return 0;

    InitTracks();
    musicLooping = 0;

    while (numTracksEnded < numTracks)
        UpdateEvents();

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

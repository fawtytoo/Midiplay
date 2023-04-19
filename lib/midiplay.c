// midiplay

// Copyright 2022 by Steve Clark

#include "common.h"
#include "timer.h"
#include "synth.h"

// header sizes in bytes
#define RMI_HDRSIZE     20
#define MUS_HDRSIZE     16
#define MID_HDRSIZE     14

int         musicInit = 0;

int         musicLooping;
int         musicPlaying = 0;
int         musicVolume = 0x100;

void Midiplay_Init(int samplerate)
{
    SetTimer(&timerPhase, 65536, samplerate);
    SetTimer(&timerSecond, 1000000, samplerate);

    musicInit = 1;
}

int Midiplay_Load(void *data, int size)
{
    BYTE    *byte = (BYTE *)data;

    if (musicInit == 0)
        return 0;

    musicInit = 1;
    musicPlaying = 0;

    if (size < 4)
        return 0;

    if (ID(byte, "RIFF"))
    {
        if (size < RMI_HDRSIZE)
        {
            return 0;
        }

        if (LE32(byte + 4) != size - 8)
        {
            return 0;
        }

        if (!ID((byte + 8), "RMID"))
        {
            return 0;
        }

        if (!ID((byte + 12), "data"))
        {
            return 0;
        }

        if (size - RMI_HDRSIZE < LE32(byte + 16))
        {
            return 0;
        }
        // adjust the size to the midi data chunk
        size = LE32(byte + 16);
        byte += RMI_HDRSIZE;
    }

    if (ID(byte, "MUS" "\x1a"))
    {
        if (size < MUS_HDRSIZE)
            return 0;

        if (size < LE16(byte + 6) + LE16(byte + 4))
            return 0;

        beatTicks = LE16(byte + 14);
        if (beatTicks == 0)
            beatTicks = 70;

        LoadMusTrack(byte + LE16(byte + 6));
    }
    else if (ID(byte, "MThd"))
    {
        if (size < MID_HDRSIZE)
            return 0;

        beatTicks = BE16(byte + 12);
        if (beatTicks < 0)
            return 0; // no support for SMPTE format yet

        size -= MID_HDRSIZE;
        if (LoadMidTracks(BE16(byte + 10), byte + MID_HDRSIZE, size) == 1)
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

    while (length)
    {
        if (musicPlaying)
        {
            playSamples -= UpdateTimer(&timerSecond);
            if (playSamples < 0)
            {
                UpdateEvents();
                playSamples += UpdateTimer(&timerBeat);
            }

            Synth_Generate(buffer);
            output[0] = buffer[0] * musicVolume >> 8;
            output[1] = buffer[1] * musicVolume >> 8;
        }
        else
        {
            output[0] = 0;
            output[1] = 0;
        }

        output += 2;
        length -= 2;
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

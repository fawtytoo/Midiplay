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
    WORD        ticks; // we're going to use this to specify ppqn, default 70
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

// MUSIC_Init
// samplerate   The samplerate of the audio output stream
void MUSIC_Init(int samplerate)
{
    musicSamplerate = samplerate;

    musicInit = 1;
}

// MUSIC_Load
// data         Memory pointer to the music data
// size         Size of the music data in bytes
// looping      Whether the music will repeat (0 = once, 1 = repeating)

// Returns 1 on success, 0 on failure
int MUSIC_Load(void *data, int size, int looping)
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
        if (hdrMus->ticks > 0)
            beatTicks = hdrMus->ticks;

        musicEvents = trackMusEvents;
    }
    else if (*(UINT *)hdrMid->id == ('M' | ('T' << 8) | ('h' << 16) | ('d' << 24)))
    {
        if (size < sizeof(HDR_MID))
            return 0;

        if (loadMidTracks(__builtin_bswap16(hdrMid->ntracks), data + sizeof(HDR_MID), size - sizeof(HDR_MID)) == 1)
            return 0; // track header failed

        beatTicks = __builtin_bswap16(hdrMid->ticks);
        musicEvents = trackMidEvents;
    }
    else
        return 0;

    initTracks();
    musicLooping = 0;
    musicPlaying = 1;

    while (musicPlaying)
    {
        updateTime();
        musicEvents();
        tickTock++;
    }

    musicLooping = looping;
    musicInit = 2;

    return 1;
}

// MUSIC_Play
// playing      0 = pause, 1 = play
void MUSIC_Play(int playing)
{
    // if there's nothing to play, don't force it
    if (musicInit < 2)
        return;

    if (playing == 1 && numTracksEnded == numTracks)
        initTracks();

    musicPlaying = playing;
}

// MUSIC_Volume
// volume       Range 0 to 127, 0 = minimum, 127 = maximum
void MUSIC_SetVolume(int volume)
{
    if (volume < 0)
        volume = 0;
    else if (volume > 127)
        volume = 127;

    musicVolume = VOLUME * volume / 127;
}

// MUSIC_IsPlaying
// Returns 0 if not playing
// Paused/looping music will still return 1
int MUSIC_IsPlaying()
{
    if (musicInit < 2)
        return 0;

    if (numTracksEnded < numTracks)
        return 1;

    return 0;
}

// MUSIC_Output
// buffer       Pointer to pre-allocated buffer
// length       Length of buffer

// Music is rendered as stereo 16bit signed samples
void MUSIC_Output(short *buffer, int length)
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
            playSamples += tickSamples;
            updateTime();
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

// MUSIC_Time
// Returns time in seconds

// Calling this immediately after MUSIC_Load returns the length of the music
// Calling this while MUSIC_IsPlaying returns the current position
int MUSIC_Time()
{
    return timeLast / beatTicks;
}

// midiplay

// midiplay

// Copyright 2022 by Steve Clark

#include "common.h"
#include "event.h"

#define TICKSAMPLES ((float)musicSamplerate * (float)beatTempo / (float)beatTicks) / 1000000.0f

int         controllerMap[2][128] =
{
    {   // mus
        MM_INSTR, MM_NONE, MM_MODWHEEL, MM_VOLUME, MM_PAN, MM_EXPRESS, MM_NONE, MM_NONE,
        MM_SUSTAIN, MM_NONE, MM_NOTEOFF, MM_NOTEOFF, MM_NONE, MM_NONE, MM_CTRLOFF, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE
    },
    {   // midi
        MM_NONE, MM_MODWHEEL, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_VOLUME,
        MM_NONE, MM_NONE, MM_PAN, MM_EXPRESS, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_SUSTAIN, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_REG_LSB, MM_REG_MSB, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE, MM_NONE,
        MM_CTRLOFF, MM_NONE, MM_NONE, MM_NOTEOFF, MM_NONE, MM_NONE, MM_NONE, MM_NONE
    }
};

typedef struct
{
    BYTE        *data, *pos;
    int         time; // tick accumulator
    struct
    {
        BYTE        running;
        function    doEvent;
    } midi;
    EVENT       event;
    int         done;
} TRACK;

TRACK       midTrack[65536], *curTrack;
int         numTracks, numTracksEnded;

int         beatTicks, beatTempo;
float       tickSamples, playSamples;
int         tickTock;

int         timeTicks;
int         timeTempo;

int         oldTrack = 0;

void updateTime()
{
    timeTempo += beatTempo;
    timeTicks += (timeTempo / 1000000);
    timeTempo %= 1000000;
}

void initTracks()
{
    int         track;

    for (track = 0; track < numTracks; track++)
    {
        midTrack[track].pos = midTrack[track].data;
        midTrack[track].done = 0;
        midTrack[track].time = 0;
        midTrack[track].midi.doEvent = NULL;
    }

    tickTock = 0;
    playSamples = 0.0f;

    beatTempo = 500000;
    tickSamples = TICKSAMPLES;

    resetControls();
    resetVoices();

    numTracksEnded = 0;

    timeTicks = timeTempo = 0;
}

void setTempo()
{
    beatTempo = (curTrack->event.data[0] << 16) | (curTrack->event.data[1] << 8) | curTrack->event.data[2];

    tickSamples = TICKSAMPLES;
}

void endOfTrack()
{
    curTrack->done = 1;
    numTracksEnded++;
}

void endOfMidiTrack()
{
    endOfTrack();
    curTrack->midi.doEvent = NULL;
    if (numTracksEnded < numTracks)
        return;

    if (musicLooping == 0)
    {
        musicPlaying = 0;
        return;
    }

    initTracks();
    // at this point we need to parse a new event from track 0
    // but we may be in the middle of several tracks
    // this is some hack! but it's more straight forward
    oldTrack = 0;
    curTrack = &midTrack[0];
}

UINT getLength()
{
    UINT        length = 0;
    BYTE        data;

    do
    {
        data = *curTrack->pos++;
        length = (length << 7) | (data & 127);
    } while (data & 128);

    return length;
}

int getMusEvent(int *time)
{
    BYTE        data, last;

    data = *curTrack->pos++;
    curTrack->event.channel = data & 0x0f;
    last = data & 0x80;

    *time = 0;

    switch (data & 0x70)
    {
      case 0x00: // release note
        curTrack->event.data[0] = (*curTrack->pos++ & 0x7f);
        eventNoteOff();
        break;

      case 0x10: // play note
        data = *curTrack->pos++;
        curTrack->event.data[0] = (data & 0x7f);
        curTrack->event.data[1] = 0x80;
        if (data & 0x80)
            curTrack->event.data[1] = (*curTrack->pos++ & 0x7f);

        eventNoteOn();
        break;

      case 0x20: // pitch wheel (adjusted to 14 bit value)
        curTrack->event.data[0] = (*curTrack->pos & 0x1) << 6;
        curTrack->event.data[1] = *curTrack->pos++ >> 1;
        eventPitchWheel();
        break;

      case 0x30: // system event
        curTrack->event.data[0] = controllerMap[0][*curTrack->pos++];
        eventMessage();
        break;

      case 0x40: // change controller
        curTrack->event.data[0] = controllerMap[0][*curTrack->pos++];
        curTrack->event.data[1] = *curTrack->pos++;
        eventMessage();
        break;

      case 0x50: // end of measure?
        break;

      case 0x60: // score end
        if (musicLooping)
            initTracks();
        else
            endOfTrack();

        return 1 - musicLooping;

      case 0x70:
        // requires one data byte but is unused
        curTrack->event.data[0] = *curTrack->pos++;
        break;

    }

    if (last)
        *time = getLength();

    return last;
}

void getMidEvent()
{
    BYTE        data, event = 0x0;

    curTrack->midi.doEvent = NULL;

    data = *curTrack->pos;

    if (data & 0x80)
        event = *curTrack->pos++;
    else
        data = curTrack->midi.running;

    curTrack->event.channel = data & 0x0f;

    switch (data & 0xf0)
    {
      case 0x80:
        curTrack->event.data[0] = *curTrack->pos++;
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->midi.doEvent = eventNoteOff;
        break;

      case 0x90:
        curTrack->event.data[0] = *curTrack->pos++;
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->midi.doEvent = eventNoteOn;
        break;

      case 0xa0:
        curTrack->event.data[0] = *curTrack->pos++;
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->midi.doEvent = eventAftertouch;
        break;

      case 0xb0: // controller message
        curTrack->event.data[0] = controllerMap[1][*curTrack->pos++];
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->midi.doEvent = eventMessage;
        break;

      case 0xc0:
        curTrack->event.data[0] = MM_INSTR;
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->midi.doEvent = eventMessage;
        break;

      case 0xd0:
        curTrack->event.data[0] = MM_VOLUME;
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->midi.doEvent = eventMessage;
        break;

      case 0xe0:
        curTrack->event.data[0] = *curTrack->pos++;
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->midi.doEvent = eventPitchWheel;
        break;

      case 0xf0:
        if ((data & 0x0f) == 0xf) // meta event
        {
            data = *curTrack->pos++;
            if (data == 0x2f) // end of track
            {
                curTrack->midi.doEvent = endOfMidiTrack;
                return;
            }
            else if (data == 0x51 && *curTrack->pos == 3)
            {
                curTrack->event.data[0] = curTrack->pos[1];
                curTrack->event.data[1] = curTrack->pos[2];
                curTrack->event.data[2] = curTrack->pos[3];
                curTrack->midi.doEvent = setTempo;
            }
        }

        curTrack->pos += getLength();
        return; // don't want to affect running events
    }

    if (event & 0x80)
        curTrack->midi.running = event;
}

void trackMusEvents()
{
    int         time;

    if (curTrack->done)
        return;

    if (curTrack->time > tickTock)
        return;

    while (getMusEvent(&time) == 0);

    curTrack->time += time;
}

// a bit of hacking in here to avoid a while loop
void trackMidEvents()
{
    int         track, ticks;

    for (track = 0; track < numTracks; track++)
    {
        curTrack = &midTrack[track];
        eventData = &curTrack->event;
        if (curTrack->time > tickTock)
            continue;

        oldTrack = track;

        if (curTrack->midi.doEvent)
            curTrack->midi.doEvent();

        // when all tracks have ended, this will be set to track 0
        track = oldTrack;

        if (curTrack->done)
            continue;

        ticks = getLength();
        curTrack->time += ticks;
        getMidEvent();

        if (ticks == 0)
            track--;
    }
}

void loadMusTrack(BYTE *data)
{
    midTrack[0].data = data;
    curTrack = &midTrack[0];
    eventData = &curTrack->event;

    numTracks = 1;
}

int loadMidTracks(int count, BYTE *data, int size)
{
    int         track;
    int         length;

    for (track = 0; track < count; track++)
    {
        if (size < 8)
            return 1;

        if (*(UINT *)data != ('M' | ('T' << 8) | ('r' << 16) | ('k' << 24)))
            return 1; // invalid header

        data += 4;
        length = __builtin_bswap32(*(UINT *)data);
        data += 4;

        size -= 8;
        if (size < length)
            return 1;

        midTrack[track].data = data;
        data += length;
        size -= length;
    }

    numTracks = count;

    return 0;
}

// midiplay

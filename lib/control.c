// midiplay

// Copyright 2022 by Steve Clark

#include "common.h"
#include "event.h"
#include "timer.h"

#define MICROSEC    1000000

// controller map for MUS
int     controllerMap[128] =
{
    CC_80, CC_NO, CC_01, CC_07, CC_0a, CC_0b, CC_NO, CC_NO,
    CC_40, CC_NO, CC_78, CC_7b, CC_NO, CC_NO, CC_79, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO,
    CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO, CC_NO
};

typedef void (*DOEVENT)(void);

typedef struct
{
    BYTE    *data, *pos;
    int     clock; // accumulator
    BYTE    running;
    DOEVENT DoEvent;
    EVENT   event;
    int     done;
} TRACK;

TRACK   midTrack[65536], *curTrack, *endTrack;
int     numTracks, numTracksEnded;

BYTE    prevVolume[16]; // last known note volume on channel

int     beatTicks = 96, beatTempo = 500000;
int     playSamples;
int     musicClock;

int     timeTicks, timeTempo;

DOEVENT MusicEvents;

void UpdateScoreTime()
{
    timeTempo += beatTempo;
    timeTicks += (timeTempo / MICROSEC);
    timeTempo %= MICROSEC;
}

// sometimes, you just want nothing to happen
void DoNothing()
{
}

void InitTracks()
{
    int     track, channel;

    for (track = 0; track < numTracks; track++)
    {
        midTrack[track].pos = midTrack[track].data;
        midTrack[track].done = 0;
        midTrack[track].clock = 0;
        midTrack[track].DoEvent = DoNothing;
    }

    for (channel = 0; channel < 16; channel++)
    {
        ResetChannel(channel);
        prevVolume[channel] = 0;
    }

    musicClock = 0;
    playSamples = 0;

    beatTempo = 500000;
    SetTimer(&timerBeat, beatTempo, beatTicks);

    ResetControls();
    ResetVoices();

    numTracksEnded = 0;

    timeTicks = timeTempo = 0;
}

void SetTempo()
{
    // if the tempo changes, should playSamples be reset?
    beatTempo = (curTrack->event.data[0] << 16) | (curTrack->event.data[1] << 8) | curTrack->event.data[2];
    SetTimer(&timerBeat, beatTempo, beatTicks);
}

void EndOfTrack()
{
    curTrack->done = 1;
    numTracksEnded++;
}

void EndOfMidiTrack()
{
    EndOfTrack();
    curTrack->DoEvent = DoNothing;
    if (numTracksEnded < numTracks)
        return;

    if (musicLooping == 0)
    {
        musicPlaying = 0;
        return;
    }

    InitTracks();
    // at this point we need to parse a new event from track 0
    curTrack = &midTrack[0];
    // eventData does not need to be set
}

UINT GetLength()
{
    UINT    length;
    BYTE    data;

    // unrolled
    data = *curTrack->pos++;
    length = (data & 127);
    if (data & 128)
    {
        data = *curTrack->pos++;
        length = (length << 7) | (data & 127);
        if (data & 128)
        {
            data = *curTrack->pos++;
            length = (length << 7) | (data & 127);
            if (data & 128)
            {
                data = *curTrack->pos++;
                length = (length << 7) | (data & 127);
            }
        }
    }

    return length;
}

int GetMusEvent(int *time)
{
    BYTE    data, last;

    curTrack->DoEvent = DoNothing;

    data = *curTrack->pos++;
    curTrack->event.channel = data & 0x0f;
    last = data & 0x80;

    *time = 0;

    switch (data & 0x70)
    {
      case 0x00: // release note
        curTrack->event.data[0] = (*curTrack->pos++ & 0x7f);
        curTrack->DoEvent = Event_NoteOff;
        break;

      case 0x10: // play note
        data = *curTrack->pos++;
        curTrack->event.data[0] = (data & 0x7f);
        if (data & 0x80)
        {
            curTrack->event.data[1] = *curTrack->pos++ & 0x7f;
            // should the volume be saved if the value is 0?
            prevVolume[curTrack->event.channel] = curTrack->event.data[1];
        }
        else
        {
            curTrack->event.data[1] = prevVolume[curTrack->event.channel];
        }
        if (curTrack->event.data[1] == 0)
        {
            curTrack->DoEvent = Event_NoteOff;
        }
        else
        {
            curTrack->DoEvent = Event_NoteOn;
        }
        break;

      case 0x20: // pitch wheel (adjusted to 14 bit value)
        curTrack->event.data[0] = (*curTrack->pos & 0x1) << 6;
        curTrack->event.data[1] = *curTrack->pos++ >> 1;
        curTrack->DoEvent = Event_PitchWheel;
        break;

      case 0x30: // system event
        curTrack->event.data[0] = controllerMap[*curTrack->pos++];
        curTrack->DoEvent = Event_Message;
        break;

      case 0x40: // change controller
        curTrack->event.data[0] = controllerMap[*curTrack->pos++];
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->DoEvent = Event_Message;
        break;

      case 0x50: // end of measure?
        break;

      case 0x60: // score end
        if (musicLooping)
        {
            curTrack->DoEvent = InitTracks;
        }
        else
        {
            curTrack->DoEvent = EndOfTrack;
        }
        last = 1 - musicLooping;
        break;

      case 0x70:
        // requires one data byte but is unused
        curTrack->event.data[0] = *curTrack->pos++;
        break;
    }

    curTrack->DoEvent();

    if (last & 0x80)
        *time = GetLength();

    return last;
}

void GetMidEvent()
{
    BYTE    data, event = 0x0;
    UINT    length;

    curTrack->DoEvent = DoNothing;

    data = *curTrack->pos;

    if (data & 0x80)
        event = *curTrack->pos++;
    else
        data = curTrack->running;

    curTrack->event.channel = data & 0x0f;

    switch (data & 0xf0)
    {
      case 0x80:
        curTrack->event.data[0] = *curTrack->pos++;
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->DoEvent = Event_NoteOff;
        break;

      case 0x90:
        curTrack->event.data[0] = *curTrack->pos++;
        curTrack->event.data[1] = *curTrack->pos++;
        if (curTrack->event.data[1] == 0)
        {
            curTrack->DoEvent = Event_NoteOff;
        }
        else
        {
            curTrack->DoEvent = Event_NoteOn;
        }
        break;

      case 0xa0:
        curTrack->event.data[0] = *curTrack->pos++;
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->DoEvent = Event_Aftertouch;
        break;

      case 0xb0: // controller message
        curTrack->event.data[0] = *curTrack->pos++;
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->DoEvent = Event_Message;
        break;

      case 0xc0:
        // instrument number must be in 2nd byte of event.data
        //  as that's where MUS puts it
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->DoEvent = Event_ChangeInstrument;
        break;

      case 0xd0:
        curTrack->event.data[0] = *curTrack->pos++;
        curTrack->DoEvent = Event_ChannelAftertouch;
        break;

      case 0xe0:
        curTrack->event.data[0] = *curTrack->pos++;
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->DoEvent = Event_PitchWheel;
        break;

      case 0xf0:
        if ((data & 0x0f) == 0xf) // meta event
        {
            data = *curTrack->pos++;
            if (data == 0x2f) // end of track
            {
                curTrack->DoEvent = EndOfMidiTrack;
                return;
            }
            else if (data == 0x51 && *curTrack->pos == 3)
            {
                curTrack->event.data[0] = curTrack->pos[1];
                curTrack->event.data[1] = curTrack->pos[2];
                curTrack->event.data[2] = curTrack->pos[3];
                curTrack->DoEvent = SetTempo;
            }
        }

        length = GetLength();
        curTrack->pos += length;
        return; // don't want to affect running events
    }

    if (event & 0x80)
        curTrack->running = event;
}

void TrackMusEvents()
{
    int     ticks;

    if (curTrack->done)
        return;

    if (curTrack->clock > musicClock)
        return;

    while (GetMusEvent(&ticks) == 0);

    curTrack->clock += ticks;
}

void TrackMidEvents()
{
    int     ticks;

    curTrack = &midTrack[0];

    do
    {
        if (curTrack->clock == musicClock)
        {
            do
            {
                eventData = &curTrack->event;

                curTrack->DoEvent();

                if (curTrack->done)
                    break;

                ticks = GetLength();
                curTrack->clock += ticks;
                GetMidEvent();
            }
            while (ticks == 0);
        }

        curTrack++;
    }
    while (curTrack <= endTrack);
}

void UpdateEvents()
{
    UpdateScoreTime();
    MusicEvents();
    musicClock++;
}

void LoadMusTrack(BYTE *data)
{
    midTrack[0].data = data;
    curTrack = &midTrack[0];
    eventData = &curTrack->event;

    numTracks = 1;

    MusicEvents = TrackMusEvents;
}

int LoadMidTracks(int count, BYTE *data, int size)
{
    int     track;
    int     length;

    for (track = 0; track < count; track++)
    {
        if (size < 8)
            return 1;

        if (*data++ != 'M')
            return 1;

        if (*data++ != 'T')
            return 1;

        if (*data++ != 'r')
            return 1;

        if (*data++ != 'k')
            return 1;

        length = *data++;
        length = (length << 8) | *data++;
        length = (length << 8) | *data++;
        length = (length << 8) | *data++;

        size -= 8;
        if (size < length)
            return 1;

        midTrack[track].data = data;
        data += length;
        size -= length;
    }

    numTracks = count;
    endTrack = &midTrack[numTracks - 1];

    MusicEvents = TrackMidEvents;

    return 0;
}

// midiplay

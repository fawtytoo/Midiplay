// midiplay

// Copyright 2022 by Steve Clark

#include "common.h"
#include "event.h"

int         controllerMap[2][128] =
{
    {   // mus
        MM_INSTR, MM_NONE, MM_MODWHEEL, MM_VOLUME, MM_PAN, MM_EXPRESS, MM_NONE, MM_NONE,
        MM_SUSTAIN, MM_NONE, MM_SOUNDOFF, MM_NOTEOFF, MM_NONE, MM_NONE, MM_CTRLOFF, MM_NONE,
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
        MM_SOUNDOFF, MM_CTRLOFF, MM_NONE, MM_NOTEOFF, MM_NONE, MM_NONE, MM_NONE, MM_NONE
    }
};

typedef struct
{
    BYTE        *data, *pos;
    int         clock; // accumulator
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

BYTE        prevVolume[16]; // last known note volume on channel

int         beatTicks = 96, beatTempo = 500000;
int         playSamples;
int         musicClock;

int         timeTicks;
int         timeTempo;

int         oldTrack = 0;

UINT        timerSecondAcc = 1000000;
UINT        timerBeatAcc = 500000;
UINT        timerRemainder = 0;

int tickSamples()
{
    int         rateSec, rateBeat, rateTick;

    rateSec = timerSecondAcc / musicSamplerate;
    timerSecondAcc -= musicSamplerate * rateSec;
    timerSecondAcc += 1000000;

    rateBeat = timerBeatAcc / beatTicks;
    timerBeatAcc -= beatTicks * rateBeat;
    timerBeatAcc += beatTempo;

    rateBeat += timerRemainder;
    rateTick = rateBeat / rateSec;
    timerRemainder = rateBeat - rateSec * rateTick;

    return rateTick;
}

void updateTime()
{
    timeTempo += beatTempo;
    timeTicks += (timeTempo / 1000000);
    timeTempo %= 1000000;
}

void eventNull() {}

void initTracks()
{
    int         track, channel;

    for (track = 0; track < numTracks; track++)
    {
        midTrack[track].pos = midTrack[track].data;
        midTrack[track].done = 0;
        midTrack[track].clock = 0;
        midTrack[track].midi.doEvent = eventNull;
    }

    for (channel = 0; channel < 16; channel++)
    {
        resetChannel(channel);
        prevVolume[channel] = 0;
    }

    musicClock = 0;
    playSamples = 0;

    beatTempo = 500000;

    // reset the timer accumulators
    timerSecondAcc = 1000000;
    timerBeatAcc = beatTempo;
    timerRemainder = 0;

    resetControls();
    resetVoices();

    numTracksEnded = 0;

    timeTicks = timeTempo = 0;
}

void setTempo()
{
    beatTempo = (curTrack->event.data[0] << 16) | (curTrack->event.data[1] << 8) | curTrack->event.data[2];
}

void endOfTrack()
{
    curTrack->done = 1;
    numTracksEnded++;
}

void endOfMidiTrack()
{
    endOfTrack();
    curTrack->midi.doEvent = eventNull;
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
    // eventData does not need to be set
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

    curTrack->midi.doEvent = eventNull;

    data = *curTrack->pos++;
    curTrack->event.channel = data & 0x0f;
    last = data & 0x80;

    *time = 0;

    switch (data & 0x70)
    {
      case 0x00: // release note
        curTrack->event.data[0] = (*curTrack->pos++ & 0x7f);
        curTrack->midi.doEvent = eventNoteOff;
        break;

      case 0x10: // play note
        data = *curTrack->pos++;
        curTrack->event.data[0] = (data & 0x7f);
        curTrack->event.data[1] = data & 0x80 ? (*curTrack->pos++ & 0x7f) : prevVolume[curTrack->event.channel];
        // should the volume be saved if the value is 0?
        prevVolume[curTrack->event.channel] = curTrack->event.data[1];
        curTrack->midi.doEvent = curTrack->event.data[1] == 0 ? eventNoteOff : eventNoteOn;
        break;

      case 0x20: // pitch wheel (adjusted to 14 bit value)
        curTrack->event.data[0] = (*curTrack->pos & 0x1) << 6;
        curTrack->event.data[1] = *curTrack->pos++ >> 1;
        curTrack->midi.doEvent = eventPitchWheel;
        break;

      case 0x30: // system event
        curTrack->event.data[0] = controllerMap[0][*curTrack->pos++];
        curTrack->midi.doEvent = eventMessage;
        break;

      case 0x40: // change controller
        curTrack->event.data[0] = controllerMap[0][*curTrack->pos++];
        curTrack->event.data[1] = *curTrack->pos++;
        curTrack->midi.doEvent = eventMessage;
        break;

      case 0x50: // end of measure?
        break;

      case 0x60: // score end
        curTrack->midi.doEvent = musicLooping ? initTracks : endOfTrack;
        last = 1 - musicLooping;
        break;

      case 0x70:
        // requires one data byte but is unused
        curTrack->event.data[0] = *curTrack->pos++;
        break;
    }

    curTrack->midi.doEvent();

    if (last & 0x80)
        *time = getLength();

    return last;
}

void getMidEvent()
{
    BYTE        data, event = 0x0;

    curTrack->midi.doEvent = eventNull;

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
        curTrack->midi.doEvent = curTrack->event.data[1] == 0 ? eventNoteOff : eventNoteOn;
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
        curTrack->event.data[0] = MM_AFTERTOUCH;
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
    int         ticks;

    if (curTrack->done)
        return;

    if (curTrack->clock > musicClock)
        return;

    while (getMusEvent(&ticks) == 0);

    curTrack->clock += ticks;
}

void trackMidEvents()
{
    int         track, ticks;

    for (track = 0; track < numTracks; track++)
    {
        curTrack = &midTrack[track];

        if (curTrack->clock > musicClock)
            continue;

        eventData = &curTrack->event;
        oldTrack = track;

        curTrack->midi.doEvent();

        // when all tracks have ended, this will be set to track 0
        track = oldTrack;

        if (curTrack->done)
            continue;

        ticks = getLength();
        curTrack->clock += ticks;
        getMidEvent();

        // a bit of hacking here to avoid a while loop
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

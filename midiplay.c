// midiplay

// Copyright 2022 by Steve Clark

#include "synth.h"

#define ID(i, a)    (i[0] == a[0] && i[1] == a[1] && i[2] == a[2] && i[3] == a[3])

#define LE16(i)     ((i)[0] | ((i)[1] << 8))
#define BE16(i)     (((i)[0] << 8) | (i)[1])
#define LE32(i)     (LE16(i) | (LE16(i + 2) << 16))
#define BE32(i)     ((BE16(i) << 16) | BE16(i + 2))

// timer -----------------------------------------------------------------------
typedef struct
{
    int rate;
    int acc;
    int remainder;
    int divisor;
}
TIMER;

TIMER   timerPhase, timerSecond, timerBeat;

// event -----------------------------------------------------------------------
#define NOTE_PLAY       1
#define NOTE_SUSTAIN    2

enum
{
    CC_NO = 255,
    CC_01 = 1,   // mod wheel
    CC_07 = 7,   // volume
    CC_0a = 10,  // pan
    CC_0b = 11,  // expression
    CC_40 = 64,  // sustain
    CC_64 = 100, // reg lsb
    CC_65 = 101, // reg msb
    CC_78 = 120, // all sounds off
    CC_79 = 121, // reset controllers
    CC_7b = 123, // all notes off
    // the following message values are MUS only and have bit 7 set
    CC_80 = 128  // MUS instrument change
};

typedef struct
{
    BYTE    channel;
    BYTE    data[3];
} EVENT;

typedef struct
{
    int     instrument;
    int     volume;
    int     pan;
    int     sustain;
    int     bend;
    int     expression;
}
CHANNEL;

typedef struct
{
    int     index;
    CHANNEL *channel;
    int     note;
    int     volume;
    int     playing; // bit field
}
VOICE;

UINT    volumeTable[128] =
{
    0x000, 0x001, 0x002, 0x004, 0x005, 0x007, 0x008, 0x009, 0x00b, 0x00c, 0x00e, 0x00f, 0x011, 0x012, 0x014, 0x015,
    0x017, 0x018, 0x01a, 0x01b, 0x01d, 0x01f, 0x020, 0x022, 0x023, 0x025, 0x027, 0x028, 0x02a, 0x02b, 0x02d, 0x02f,
    0x030, 0x032, 0x034, 0x035, 0x037, 0x039, 0x03b, 0x03c, 0x03e, 0x040, 0x041, 0x043, 0x045, 0x047, 0x049, 0x04a,
    0x04c, 0x04e, 0x050, 0x052, 0x054, 0x055, 0x057, 0x059, 0x05b, 0x05d, 0x05f, 0x061, 0x063, 0x065, 0x067, 0x069,
    0x06b, 0x06d, 0x06f, 0x071, 0x073, 0x075, 0x077, 0x079, 0x07b, 0x07d, 0x07f, 0x081, 0x083, 0x085, 0x087, 0x089,
    0x08c, 0x08e, 0x090, 0x092, 0x094, 0x097, 0x099, 0x09b, 0x09d, 0x0a0, 0x0a2, 0x0a4, 0x0a6, 0x0a9, 0x0ab, 0x0ad,
    0x0b0, 0x0b2, 0x0b5, 0x0b7, 0x0b9, 0x0bc, 0x0be, 0x0c1, 0x0c3, 0x0c6, 0x0c8, 0x0cb, 0x0cd, 0x0d0, 0x0d2, 0x0d5,
    0x0d7, 0x0da, 0x0dc, 0x0df, 0x0e2, 0x0e4, 0x0e7, 0x0ea, 0x0ec, 0x0ef, 0x0f2, 0x0f4, 0x0f7, 0x0fa, 0x0fd, 0x100
};

CHANNEL midChannel[16];
VOICE   midVoice[VOICES] =
{
    {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7},
    {8}, {9}, {10}, {11}, {12}, {13}, {14}, {15},
    {16}, {17}, {18}, {19}, {20}, {21}, {22}, {23}
};
VOICE   *voiceHead = &midVoice[0];

EVENT   *eventData;

// control ---------------------------------------------------------------------
#define MICROSEC    1000000

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

// midiplay --------------------------------------------------------------------
#define RMI_HDRSIZE     20
#define MUS_HDRSIZE     16
#define MID_HDRSIZE     14

int         musicInit = 0;

int         musicLooping;
int         musicPlaying = 0;
int         musicVolume = 0x100;

// timer -----------------------------------------------------------------------
int UpdateTimer(TIMER *timer)
{
    timer->acc += timer->remainder;
    if (timer->acc < timer->divisor)
        return timer->rate;

    timer->acc -= timer->divisor;

    return timer->rate + 1;
}

void SetTimer(TIMER *timer, int numerator, int divisor)
{
    timer->acc = 0;
    timer->rate = numerator / divisor;
    timer->remainder = numerator - timer->rate * divisor;
    timer->divisor = divisor;
}

// event -----------------------------------------------------------------------
void ResetChannel(int channel)
{
    midChannel[channel].instrument = 0;
    midChannel[channel].volume = volumeTable[100];
    midChannel[channel].pan = 64;
}

void VoiceOff(VOICE *voice, int state)
{
    voice->playing &= state;
    if (voice->playing)
    {
        return;
    }

    Synth_KeyOff(voice->index);
}

void VoiceVolume(VOICE *voice)
{
    Synth_SetVolume(voice->index, voice->channel->volume * voice->channel->expression * voice->volume >> 16);
}

void ResetVoices()
{
    VOICE   *voice = voiceHead;
    int     index;

    for (index = 0; index < VOICES; index++, voice++)
    {
        VoiceOff(voice, 0);
    }
}

void ResetControls()
{
    int channel;

    for (channel = 0; channel < 16; channel++)
    {
        midChannel[channel].sustain = 0;
        midChannel[channel].bend = 0;
        midChannel[channel].expression = volumeTable[127];
    }
}

void Event_NoteOff()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     note = eventData->data[0];
    //int         volume = eventData->data[1];
    VOICE   *voice = voiceHead;
    int     index;

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel && voice->note == note)
        {
            VoiceOff(voice, NOTE_SUSTAIN);
        }
    }
}

void FrequencyStep(VOICE *voice)
{
    int index, octave;

    octave = voice->note / 12;
    index = (voice->note % 12) * 32 + voice->channel->bend;
    if (index < 0)
    {
        if (octave > 0)
        {
            octave--;
            index += 384;
        }
        else
        {
            index = 0;
        }
    }
    else if (index >= 384)
    {
        octave++;
        index -= 384;
    }

    Synth_SetFrequency(voice->index, index, octave);
}

void Event_NoteOn()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     note = eventData->data[0];
    int     volume = eventData->data[1];
    VOICE   *voice = voiceHead;
    int     index;

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing == 0)
        {
            voice->channel = channel;
            voice->note = note;
            voice->volume = volumeTable[volume];
            VoiceVolume(voice);
            Synth_SetPan(index, channel->pan);

            FrequencyStep(voice);
            Synth_KeyOn(index);

            voice->playing = NOTE_PLAY | channel->sustain;

            break;
        }
    }
}

void Event_MuteNotes()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    VOICE   *voice = voiceHead;
    int     index;

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            VoiceOff(voice, NOTE_SUSTAIN);
        }
    }
}

void Event_PitchWheel()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     bend = ((eventData->data[0] & 0x7f) | (eventData->data[1] << 7));
    VOICE   *voice = voiceHead;
    int     index;

    channel->bend = (bend >> 7) - 64; // 7 bit values

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            FrequencyStep(voice);
        }
    }
}

void Event_Aftertouch()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     note = eventData->data[0];
    int     volume = eventData->data[1];
    VOICE   *voice = voiceHead;
    int     index;

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel && voice->note == note)
        {
            voice->volume = volumeTable[volume];
            VoiceVolume(voice);
        }
    }
}

void Event_Sustain()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     sustain = eventData->data[1] >> 6;
    VOICE   *voice = voiceHead;
    int     index;

    // sustain: 0=off, 2=on
    channel->sustain = ((sustain >> 1) | (sustain & 1)) << 1;

    if (channel->sustain != 0)
    {
        return;
    }

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            VoiceOff(voice, NOTE_PLAY);
        }
    }
}

void Event_ChannelVolume()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     volume = eventData->data[1];
    VOICE   *voice = voiceHead;
    int     index;

    channel->volume = volumeTable[volume & 0x7f];

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            VoiceVolume(voice);
        }
    }
}

void Event_Pan()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     pan = eventData->data[1] & 0x7f;
    VOICE   *voice = voiceHead;
    int     index;

    channel->pan = pan;

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            Synth_SetPan(index, pan);
        }
    }
}

void Event_ChannelAftertouch()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     volume = eventData->data[0];
    VOICE   *voice = voiceHead;
    int     index;

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            voice->volume = volumeTable[volume];
            VoiceVolume(voice);
        }
    }
}

void Event_Expression()
{
    CHANNEL *channel = &midChannel[eventData->channel];
    int     expression = eventData->data[1] & 0x7f;
    VOICE   *voice = voiceHead;
    int     index;

    channel->expression = volumeTable[expression];

    for (index = 0; index < VOICES; index++, voice++)
    {
        if (voice->playing && voice->channel == channel)
        {
            VoiceVolume(voice);
        }
    }
}

void Event_ChangeInstrument()
{
    CHANNEL *channel = &midChannel[eventData->channel];

    channel->instrument = eventData->data[1];
}

void Event_Message()
{
    switch (eventData->data[0])
    {
      case CC_07:
        Event_ChannelVolume();
        break;

      case CC_0a:
        Event_Pan();
        break;

      case CC_0b:
        Event_Expression();
        break;

      case CC_40: // FIXME
        //Event_Sustain();
        break;

      case CC_78:
        ResetVoices();
        break;

      case CC_79:
        ResetControls();
        break;

      case CC_7b:
        Event_MuteNotes();
        break;

      case CC_80:
        Event_ChangeInstrument();
        break;

      case CC_01:
      case CC_64:
      case CC_65:
        break;

      default:
        break;
    }
}

// control ---------------------------------------------------------------------
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

        if (!ID(data, "MTrk"))
            return 1;

        length = BE32(data + 4);

        data += 8;
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

// midiplay --------------------------------------------------------------------
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

            Synth_Generate(buffer, UpdateTimer(&timerPhase));
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
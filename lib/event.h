// midiplay

// Copyright 2022 by Steve Clark

#ifndef __EVENT_H__
#define __EVENT_H__

typedef struct
{
    BYTE    channel;
    BYTE    data[3];
} EVENT;

extern EVENT    *eventData;

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

void ResetChannel(int);

void ResetControls(void);
void ResetVoices(void);

void Event_NoteOff(void);
void Event_NoteOn(void);
void Event_Aftertouch(void);
void Event_Message(void);
void Event_PitchWheel(void);
void Event_ChannelAftertouch(void);
void Event_ChangeInstrument(void);

#endif

// midiplay

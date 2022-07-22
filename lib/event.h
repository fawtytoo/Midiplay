// midiplay

// Copyright 2022 by Steve Clark

#ifndef __EVENT_H__
#define __EVENT_H__

typedef struct
{
    BYTE        channel;
    BYTE        data[3];
} EVENT;

extern EVENT        *eventData;

enum
{
    MM_NONE,
    MM_VOLUME,
    MM_PAN,
    MM_INSTR,
    MM_NOTEOFF,
    MM_MODWHEEL,
    MM_EXPRESS,
    MM_SUSTAIN,
    MM_REG_LSB,
    MM_REG_MSB,
    MM_CTRLOFF,
    MM_AFTERTOUCH,
    MM_SOUNDOFF
};

void resetChannel(int);

void resetControls(void);
void resetVoices(void);

void eventNoteOff(void);
void eventNoteOn(void);
void eventAftertouch(void);
void eventMessage(void);
void eventPitchWheel(void);

#endif

// midiplay

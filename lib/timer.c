// midiplay

// Copyright 2022 by Steve Clark

#include "timer.h"

TIMER   timerPhase;
TIMER   timerSecond, timerBeat;

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

// midiplay

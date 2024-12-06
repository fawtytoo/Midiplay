//  Copyright 2021-2024 by Steve Clark

//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.

//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:

//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required. 
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.

#ifndef __MIDIPLAY_H__
#define __MIDIPLAY_H__

// comment this line if the score time is not required
#define MP_TIME

// file size of genmidi data
#define GENMIDI_SIZE    11908

// Midiplay_Init(samplerate, genmidi)
// samplerate   The samplerate of the audio output stream
// genmidi      GENMIDI instrument data
// Return 1 on success, 0 on failure
int Midiplay_Init(int, char *);

// Midiplay_Load(data, size)
// data         Memory pointer to the music data
// size         Size of the music data in bytes
// Returns 0 on success, 1 on failure or 2 if unsupported
int Midiplay_Load(void *, int);

// Midiplay_Play(playing)
// playing      0 = pause, 1 = play
void Midiplay_Play(int);

// Midiplay_Volume(volume)
// volume       Range 0 (minimum) to 127 (maximum)
void Midiplay_SetVolume(int);

// Midiplay_Output(buffer)
// buffer       Pointer to buffer sample
// Audio is rendered as a stereo 16bit signed sample
void Midiplay_Output(short [2]);

// Midiplay_IsPlaying
// Returns 0 if not playing
// Paused/looping music will still return 1
int Midiplay_IsPlaying(void);

// Midiplay_Time
// Returns time in seconds
// Calling this immediately after Midiplay_Load returns the length of the score
// Calling this while Midiplay_IsPlaying returns the current position
int Midiplay_Time(void);

// Midiplay_Loop(looping)
// Changes looping mode
// looping      Whether the music will repeat (0 = off, 1 = repeating)
void Midiplay_Loop(int);

// Midiplay_Replay
// Replays score from beginning
void Midiplay_Replay(void);

#endif

// midiplay

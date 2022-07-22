# Midiplay

## A functional MUS/MIDI library/player.
Currently for Linux only.
The library is written in pure C with no dependencies.
The player uses the SDL2 library for audio output.

This project can play both MUS and MIDI files due to their similar structure. MIDI type 0 & 1 files are supported; type 2 is unknown due to lack of suitable files to test with. Similarly, Midiplay does not handle the SMPTE format.

### Inspiration
The inspiration for this project came from the original DOOM source code.

The DOOM source code did not come with the ability to play music. DOOM music is in the MUS form; a derivative of MIDI. It has the same basic event/delta-time structure as MIDI.

### Code
The project has been created as a stand alone library. It has an external main.c for demonstration purposes, so can be used as a functional player from the command line. Alternatively, it can be fully integrated into any other project.

Playback is done using a square wave, because this is the simplest wave to create.

- 16 Channels (instruments not currently supported).
- 24 Voices (minimum for General MIDI Level 1 Spec).
- Full stereo sound

### Focus
The project is meant for demonstration purposes only, but is functional as a player in itself. However, its focus is on event parsing and timing accuracy, rather than dealing with all possible events and instrument sounds.

This project deals with timing somewhat differently than many other MIDI projects. The timing is done using the delta-time values (ticks) only. The tick values and samples per tick are kept separate. Samples per tick are solely used for playback. This gives 2 advantages: ensures synchronisation across tracks, and less time is spent checking for new events. Incidentally, this is the reason why the SMPTE format is not supported.

### Events
The library handles only a few basic events, but enough that would satisfy most MIDI/MUS data.
- Note On
- Note Off
- Pitch Wheel (fixed 2 semitones up/down)
- After Touch (key pressure)
- After Touch (channel pressure)
- Sustain (currently disabled)
- Tempo (MIDI only)
- Channel Volume
- Panning
- All Notes Off
- All Sounds Off
- Expression

## Sources
- [MUS format](https://moddingwiki.shikadi.net/wiki/MUS_Format)
- [MID format](https://moddingwiki.shikadi.net/wiki/MID_Format)
- [Technical docs](http://midi.teragonaudio.com/)
- [Standard MIDI-File Format Spec 1.1](http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html)
- [Official MIDI Specifications](https://www.midi.org/specifications)

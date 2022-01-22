# Midiplay

## A functional MUS/MIDI library/player.
Currently for Linux only.
The library is written in pure C with no dependencies.
The player uses the SDL2 library for audio output.

### Inspiration
The inspiration for this project came from the original DOOM source code.

The DOOM source code did not come with the ability to play music. DOOM music is in the MUS form; a derivative of MIDI. It has the same basic event/delta-time structure as MIDI.

This project can play both MUS and MIDI files due to their similar structure. MIDI type 0 & 1 files are supported; type 2 is unknown due to lack of suitable files to test with.

### Code
The project has been created as a stand alone library. It has an external main.c for demonstration purposes, so can be used as a functional player from the command line. Alternatively, it can be fully integrated into any other project.

Playback is done using a square wave, because this is the simplest wave to create.

- 16 Channels (instruments not supported).
- 24 Voices (minimum for General MIDI Level 1 Spec).

### Focus
The project is meant for demonstration purposes only, but is functional as a player in itself. However, its focus is on event parsing and timing accuracy, rather than dealing with all possible events and instrument sounds.

This project deals with timing somewhat differently than many other MIDI projects. The timing is done using the delta-time values (ticks) only. The tick values and samples per tick are kept separate. Samples per tick are solely used for playback. This gives 2 advantages: ensures synchronisation across tracks, and less time is spent checking for new events.

### Events
The library handles only a few basic events, but enough that would satisfy most MIDI/MUS data.
- Note On
- Note Off
- Pitch Wheel (fixed 2 semitones up/down)
- After Touch
- Sustain (handled as Sostenuto)
- Tempo (MIDI only)
- Channel Volume
- Panning
- All Notes Off

### MUS and Raptor
The 1994 game Raptor also used MUS files, but were played at a different rate than DOOM music.

In order to accommodate different tick rates (Pulses Per Quarter Note) that may arise in MUS data, Midiplay utilises unused bytes in the MUS header.

Although MUS documentation suggests a tick rate per second for MUS (DOOM = 140Hz, Raptor = 70Hz), it seemed better to translate these rates to fall in line with MIDI specification which uses a default beat (A.K.A. quarter note) tempo of 500,000µs (120 beats per minute), thus MUS data can be played using PPQN. This has the advantages of having more shared code within Midiplay for MUS and MIDI, but also not needing to change the MUS file delta-times to suit the player.

The MUS header is 16 bytes long, and has 2 unused/reserved bytes at the end which can specify the PPQN. A zero value will default to 70ppqn, and any non-zero value will be used instead of the default. For example, changing the value of these 2 bytes to 35 would be suitable for Raptor music. Either 70 or 0, would be suitable for DOOM music. Therefore, 70ppqn = 140Hz, 35ppqn = 70Hz.

Advantages of utilising this method is it standardises MUS files, and preserves the track data.

## Sources
- [MUS format](https://moddingwiki.shikadi.net/wiki/MUS_Format)
- [MID format](https://moddingwiki.shikadi.net/wiki/MID_Format)
- [Technical docs](http://midi.teragonaudio.com/)
- [Standard MIDI-File Format Spec 1.1](http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html)

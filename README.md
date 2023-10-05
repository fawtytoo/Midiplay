# Midiplay

## A functional MUS/MIDI player.
Midiplay is a functional command line player. It can also be used as a stand alone library or, alternatively, it can be integrated directly into any other project.

Midiplay is written in pure C with no dependencies and uses SDL for audio output.

Although written for Linux, could be ported easily to other Operating Systems.

This project can play both MUS and MIDI files due to their similar structure. MIDI type 0 & 1 files are supported; type 2 is unknown due to lack of suitable files to test with. Similarly, Midiplay does not handle the SMPTE format as it's not widely used, if at all.

Also supports the RMI file format. RMI is a RIFF container for MIDI.

- 16 Channels (instruments not currently supported).
- 24 Voices (minimum for General MIDI Level 1 Spec).
- Full stereo panning

### Inspiration
The inspiration for this project came from the original DOOM source code.

The DOOM source code did not come with the ability to play music. DOOM music is in the MUS form; a derivative of MIDI. It has the same basic event/delta-time structure as MIDI.

### Code
Playback is done using a square wave, because this is the simplest wave to create. Although the synthesiser is basic, it can be swapped for another (such as Nuked OPL) with minimal code modification.

The code is written to be as efficient as possible using function pointers to handle events, eliminating unnecessary conditional checks. MIDI data is assumed to be completely valid, and only basic checks are made on loading the data into memory.

### Focus
The project is meant for educational purposes, and its focus is on event parsing and timing accuracy, rather than dealing with all possible events and instrument sounds. This makes Midiplay a very solid MIDI player.

This project deals with timing somewhat differently than many other MIDI projects. The timing is done using the delta-time values (ticks) only. The tick values and samples per tick are kept separate. Samples per tick are solely used for playback. This gives 2 advantages: ensures synchronisation across tracks, and less time is spent checking for new events. Incidentally, this is the reason why the SMPTE format is not supported.

### Events
Midiplay can handle enough events that should satisfy most MIDI/MUS data.
- MUS/MIDI
  - Note On / Off
  - Channel Volume & Expression
  - Panning
  - All Notes Off
  - All Sounds Off
  - Pitch Wheel (fixed 2 semitones up/down)
  - Change Instrument
- MIDI
  - After Touch (key pressure)
  - After Touch (channel pressure)
  - Tempo
  - Registered Parameter Number
  - Data Entry

## Sources
- [MUS format](https://moddingwiki.shikadi.net/wiki/MUS_Format)
- [MID format](https://moddingwiki.shikadi.net/wiki/MID_Format)
- [Technical docs](http://midi.teragonaudio.com/)
- [Standard MIDI-File Format Spec 1.1](http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html)
- [Official MIDI Specifications](https://www.midi.org/specifications)


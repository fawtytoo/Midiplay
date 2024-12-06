# Midiplay

## A MIDI/MUS player
Midiplay is a very robust and fully functional command line player for MIDI/MUS using OPL audio synthesis. Also supports the RMI file format; a RIFF container for MIDI.

Midiplay can play both MUS and MIDI files due to their similar structure. MIDI type 0 & 1 files are supported; type 2 is unknown due to lack of suitable files to test with. Similarly, Midiplay does not handle the SMPTE format as it's not widely used, if at all.

Written for Linux in C using SDL for audio output.

Midiplay can be integrated directly into any other project by simply omitting `main.c` and using `midiplay.h` for the API calls.

#### Features
- 24 Voices (minimum for General MIDI Level 1 Spec)
- OPL synthesis of instruments
- Accurate event parsing and timing

### Inspiration
The inspiration for this project came from the original DOOM source code.

The DOOM source code did not come with the ability to play music. DOOM music is in the MUS form; a derivative of MIDI. It has the same basic event/delta-time structure as MIDI.

### Code
The MIDI/MUS parser is written to be as efficient as possible using function pointers to handle events, eliminating unnecessary conditional checks. File data is assumed to be completely valid, and only basic checks are made on loading the data into memory.

Audio is generated using a modified version of Nuked OPL3. Strictly speaking, it is no longer an OPL emulator but just harnesses the OPL synthesizer using more direct function calls.
- 2 operator mode (no percussion modes)
- Increased note range
- Single "chip" implementation capable of 128 voices[^1]
- Full stereo panning
- Post processed operator volume[^2]

[^1]: Change NVOICES in `opl.h` to however many voices are required.
[^2]: The volume of each group of operators (modulator and carrier) are generated before having any volume change applied.

To take advantage of OPL, you will need a GENMIDI (General Midi instrument data file). This is the same GENMIDI as found in DOOM. To hear the audio at its best, it's recommended to use DMXOPL.

[DMXOPL - YMF262-enhanced FM patch set for Doom and source ports](https://github.com/sneakernets/DMXOPL)

### Events
Midiplay can handle enough events that should satisfy most MIDI/MUS data.
- MIDI/MUS
  - Note On / Off
  - Channel Volume & Expression
  - Panning
  - All Notes Off
  - All Sounds Off
  - Pitch Bend
  - Change Instrument
  - Sustain
- MIDI
  - After Touch (key pressure)
  - After Touch (channel pressure)
  - Tempo
  - Registered Parameter Number
    - Pitch Bend Range
  - Data Entry

## Sources
- [MID format](https://moddingwiki.shikadi.net/wiki/MID_Format)
- [MUS format](https://moddingwiki.shikadi.net/wiki/MUS_Format)
- [Technical docs](http://midi.teragonaudio.com/)
- [Standard MIDI-File Format Spec 1.1](http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html)
- [Official MIDI Specifications](https://www.midi.org/specifications)
- [NUKED-OPL3](https://github.com/nukeykt/Nuked-OPL3)

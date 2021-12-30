# Midiplay

## A functional MUS/MIDI library/player.

### Inspiration
The inspiration for this project came from the original DOOM source code.

The DOOM source code did not come with the ability to play music. DOOM music is in the MUS form; a derivitive of MIDI. It has the same basic event/time delay structure as MIDI.

This project can play both MUS and MIDI files due to their similar structure. MIDI type 1 files are supported; type 0 & 2 are unknown due to lack of suitable files to test with.

### Code
The project has been created as a stand alone library. It has an external main.c for demonstration purposes, so can be used as a functional player from the command line. Alternatively, it can be fully integrated into any other project.

Playback is done using a square wave, because this is the simplest wave to create.

### Focus
The project is meant for demonstration purposes only, but is functional as a player in itself. However, its focus is on event parsing and timing accuracy, rather than dealing with all possible events and instrument sounds.

This project deals with timing somewhat differently than many other MIDI projects. The timing here is done using the tick count used in time delay values only. It does not make further calculations on samples per tick. The time delay values and samples per tick are kept separate. Samples per tick are solely used for playback. This gives 2 advantages: ensures synchronisation across tracks, and less time is spent checking for new events.

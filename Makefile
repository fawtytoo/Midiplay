# Midiplay

# Copyright (C) 2022 by Steve Clark

TARGET = midiplay

CC = gcc

CFLAGS = -O -Wall -MMD
LDFLAGS = -lSDL

SOURCE = main.o lib/midiplay.o lib/control.o lib/event.o lib/timer.o lib/synth.o

all:	$(SOURCE)
	$(CC) $(SOURCE) -o $(TARGET) $(LDFLAGS)

%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SOURCE) $(TARGET)
	rm -f lib/*.d *.d

install:	all
	cp $(TARGET) ~/.local/bin

-include lib/*.d

# Midiplay

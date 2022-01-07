# Midiplay

# Copyright (C) 2022 by Steve Clark

TARGET = midiplay

CC = gcc

CFLAGS = -O -Wall
LDFLAGS = -lSDL2

SOURCE = main.o lib/music.o lib/control.o

all:	$(SOURCE)
	$(CC) $(SOURCE) -o $(TARGET) $(LDFLAGS)

%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SOURCE) $(TARGET)

install:	all
	cp $(TARGET) ~/.local/bin

# Midiplay

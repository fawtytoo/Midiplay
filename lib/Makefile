# Midiplay

# Copyright 2022 by Steve Clark

TARGET = libmidiplay.so

CC = gcc

CFLAGS = -O2 -Wall -fPIC
LDFLAGS =

SOURCE = midiplay.o control.o

all:	 $(SOURCE)
	$(CC) -shared $(SOURCE) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(SOURCE)

%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@

install:	all
	cp $(TARGET) ~/.local/lib

# Midiplay

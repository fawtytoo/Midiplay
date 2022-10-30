# Midiplay

# Copyright 2022 by Steve Clark

TARGET = libmidiplay.so

CC = gcc

CFLAGS = -O2 -Wall -fPIC -MMD
LDFLAGS =

SOURCE = midiplay.o control.o event.o timer.o

all:	 $(SOURCE)
	$(CC) -shared $(SOURCE) -o $(TARGET) $(LDFLAGS)

%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(SOURCE)
	rm -f *.d

install:	all
	cp $(TARGET) ~/.local/lib
	cp midiplay.h ~/.local/include

-include *.d

# Midiplay

# MIDI player

TARGET = libmusic.so

CC = gcc

CFLAGS = -O2 -Wall -fPIC
LDFLAGS =

SOURCE = music.o control.o

all:	 $(SOURCE)
	$(CC) -shared $(SOURCE) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(SOURCE)

%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@

install:	all
	cp $(TARGET) ~/.local/lib

# MIDI

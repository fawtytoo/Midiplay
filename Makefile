# doom music

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

# doom music

// midiplay

// Copyright 2022 by Steve Clark

#include <SDL2/SDL.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "lib/music.h"

#define ERROR           printf("%s\n\t%s\n", argv[arg], strerror(errno))

#define SAMPLERATE      44100
#define SAMPLECOUNT     2048

typedef void (*cb_t)(short *, int);

void sdlCallback(void *userdata, Uint8 *buffer, int length)
{
    cb_t        cb = userdata;

    cb((short *)buffer, length / 2);
}
 
int digits(int number, int count)
{
    if (number > 9)
        count = digits(number / 10, count + 1);

    return count;
}

int main(int argc, char **argv)
{
    SDL_AudioDeviceID   sdlAudio;
    SDL_AudioSpec       want;

    struct stat         status;
    FILE                *file;
    char                *buffer;

    int                 arg;
    char                *name;

    int                 length, count;

    if (argc < 2)
        return 1;

    SDL_Init(SDL_INIT_AUDIO);

    want.freq = SAMPLERATE;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = SAMPLECOUNT;
    want.callback = sdlCallback;
    want.userdata = MUSIC_Output;

    sdlAudio = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    SDL_PauseAudioDevice(sdlAudio, 0);

    MUSIC_Init(SAMPLERATE);

    for (arg = 1; arg < argc; arg++)
    {
        if (stat(argv[arg], &status) < 0)
        {
            ERROR;
            continue;
        }

        if (S_ISDIR(status.st_mode))
        {
            errno = EISDIR; // needs to be set explicitly
            ERROR;
            continue;
        }

        if ((file = fopen(argv[arg], "rb")) == NULL)
        {
            ERROR;
            continue;
        }

        buffer = malloc(status.st_size);
        fread(buffer, status.st_size, 1, file);
        fclose(file);

        if (MUSIC_Load(buffer, status.st_size, MUSIC_PLAYONCE))
        {
            if ((name = strrchr(argv[arg], '/')) == NULL)
                name = argv[arg];
            else
                name++;

            printf("Playing: %s\n", name);

            length = MUSIC_Time();
            count = digits(length / 60, 1);
            printf("\t %*s / %i:%02i\033[1A\n", count + 3, " ", length / 60, length % 60);

            MUSIC_Play(MUSIC_PLAY);

            while (MUSIC_IsPlaying())
            {
                SDL_Delay(1);
                length = MUSIC_Time();
                printf("\t %*i:%02i\033[1A\n", count, length / 60, length % 60);
            }

            printf("\033[K");
        }
        else
            printf("Invalid file: %s\n", argv[arg]);

        free(buffer);
    }

    SDL_Quit();

    return 0;
}

// midiplay

// midi play

#include <SDL2/SDL.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib/music.h"

#define SAMPLERATE      44100
#define SAMPLECOUNT     2048

typedef void (*cb_t)(short *, int);

void sdlCallback(void *userdata, Uint8 *buffer, int length)
{
    cb_t        cb = userdata;

    cb((short *)buffer, length / 2);
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
            printf("Error checking %s\n", argv[arg]);
            continue;
        }

        if ((file = fopen(argv[arg], "rb")) == NULL)
        {
            printf("Error opening %s\n", argv[arg]);
            continue;
        }

        if ((name = strrchr(argv[arg], '/')) == NULL)
            name = argv[arg];
        else
            name++;

        printf("Playing: %s\n", name);

        buffer = malloc(status.st_size);
        fread(buffer, status.st_size, 1, file);
        fclose(file);

        MUSIC_Play(buffer, status.st_size, 0);

        while (MUSIC_IsPlaying())
            SDL_Delay(1);

        free(buffer);
    }

    SDL_Quit();

    return 0;
}

// midi play

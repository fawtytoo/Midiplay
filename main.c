// midiplay

// Copyright 2022 by Steve Clark

#include <SDL2/SDL.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include "lib/midiplay.h"

#define ERROR           printf("%s %s\n\r", argv[arg], strerror(errno))

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

    struct termios      termAttr;
    tcflag_t            localMode, outputMode;
    int                 blockMode;

    struct stat         status;
    FILE                *file;
    char                *buffer;

    int                 arg;
    char                *name;

    int                 length, count;

    int                 looping = 0;
    int                 playing = 1;
    int                 volume = 100;

    char                key;

    if (argc < 2)
    {
        printf("Usage: %s [FILES]\n\tSpecify one or more FILES to play\n", argv[0]);
        return 1;
    }

    printf("P = Play/Pause | N = Next | R = Restart | L = Loop | +/- Volume | Q = Quit\n");

    SDL_Init(SDL_INIT_AUDIO);

    want.freq = SAMPLERATE;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = SAMPLECOUNT;
    want.callback = sdlCallback;
    want.userdata = Midiplay_Output;

    sdlAudio = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    SDL_PauseAudioDevice(sdlAudio, 0);

    Midiplay_Init(SAMPLERATE);

    blockMode = fcntl(STDIN_FILENO, F_GETFL, FNONBLOCK);
    tcgetattr(STDIN_FILENO, &termAttr);
    localMode = termAttr.c_lflag;
    outputMode = termAttr.c_oflag;
    termAttr.c_lflag &= ~(ICANON | ECHO | ISIG);
    termAttr.c_oflag &= ~ONLCR;
    tcsetattr(STDIN_FILENO, TCSANOW, &termAttr);
    fcntl(STDIN_FILENO, F_SETFL, FNONBLOCK);

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

        if (Midiplay_Load(buffer, status.st_size, looping))
        {
            if ((name = strrchr(argv[arg], '/')) == NULL)
                name = argv[arg];
            else
                name++;

            count = digits(argc - 1, 1);
            printf("Playing %*i/%i: %s\r\n", count, arg, argc - 1, name);

            length = Midiplay_Time();
            count = digits(length / 60, 1);
            printf("[  ] [    ] %*s / %i:%02i\r", count + 3, " ", length / 60, length % 60);

            Midiplay_Play(playing);

            while (Midiplay_IsPlaying())
            {
                SDL_Delay(1);

                key = getc(stdin);
                if (key == 'l')
                {
                    looping = 1 - looping;
                    Midiplay_Loop(looping);
                }
                else if (key == 'p')
                {
                    playing = 1 - playing;
                    Midiplay_Play(playing);
                }
                else if (key == 'n')
                    break;
                else if (key == 'q')
                {
                    arg = argc;
                    break;
                }
                else if (key == '-')
                {
                    if (volume > 0)
                    {
                        volume--;
                        Midiplay_SetVolume(volume * 127 / 100);
                    }
                }
                else if (key == '=')
                {
                    if (volume < 100)
                    {
                        volume++;
                        Midiplay_SetVolume(volume * 127 / 100);
                    }
                }
                else if (key == 'r')
                    Midiplay_Restart();

                length = Midiplay_Time();
                printf("[%s%s] [%3i%%] %*i:%02i\r", playing ? " " : "P", looping ? "L" : " ", volume, count, length / 60, length % 60);
            }
        }
        else
            printf("Invalid file: %s\n\r", argv[arg]);

        free(buffer);
    }

    termAttr.c_lflag = localMode;
    termAttr.c_oflag = outputMode;
    tcsetattr(STDIN_FILENO, TCSANOW, &termAttr);
    fcntl(STDIN_FILENO, F_SETFL, blockMode);

    SDL_Quit();

    return 0;
}

// midiplay

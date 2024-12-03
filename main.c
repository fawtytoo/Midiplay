// midiplay

// Copyright 2022 by Steve Clark

#include <SDL/SDL.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include "midiplay.h"

#define ERROR           printf("%s %s\n\r", argv[arg], strerror(errno))
#define INVALID_FILE    printf("Invalid file: %s\n\r", argv[arg])

#define SAMPLERATE      44100
#define SAMPLECOUNT     2048

#define RMI_HDRSIZE     20

typedef unsigned int    UINT;

void SdlCallback(void *unused, Uint8 *buffer, int length)
{
    (void)unused;

    Midiplay_Output((short *)buffer, length / 2);
}
 
int DoDigits(int number, int count)
{
    if (number > 9)
    {
        count = DoDigits(number / 10, count + 1);
    }

    return count;
}

int main(int argc, char **argv)
{
    SDL_AudioSpec   want;

    struct termios  termAttr;
    tcflag_t        localMode, outputMode;
    int             blockMode;

    struct stat     status;
    FILE            *file;
    char            *buffer, *data;
    int             size;

    int             arg;
    char            *name;
#ifdef MP_TIME
    int             time;
#endif
    int             count;

    int             looping = 0;
    int             playing = 0;
    int             volume = 100;

    char            key;

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
    want.callback = SdlCallback;

    SDL_OpenAudio(&want, NULL);
    SDL_PauseAudio(SDL_TRUE);

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

        size = status.st_size;
        buffer = malloc(size);
        fread(buffer, size, 1, file);
        fclose(file);

        data = buffer;
        if (strncmp(data, "RIFF", 4) == 0)
        {
            if (size < RMI_HDRSIZE)
            {
                INVALID_FILE;
                continue;
            }

            if (*(UINT *)(data + 4) != size - 8)
            {
                INVALID_FILE;
                continue;
            }

            if (strncmp((data + 8), "RMIDdata", 8) != 0)
            {
                INVALID_FILE;
                continue;
            }

            if (size - RMI_HDRSIZE < *(UINT *)(data + 16))
            {
                INVALID_FILE;
                continue;
            }
            // adjust the size to the midi data chunk
            size = *(UINT *)(data + 16);
            data += RMI_HDRSIZE;
        }

        if (Midiplay_Load(data, size))
        {
            if ((name = strrchr(argv[arg], '/')) == NULL)
            {
                name = argv[arg];
            }
            else
            {
                name++;
            }

            count = DoDigits(argc - 1, 1);
            printf("Playing %*i/%i: %s\r\n", count, arg, argc - 1, name);
#ifdef MP_TIME
            time = Midiplay_Time();
            count = DoDigits(time / 600, 1);
            printf("[  ] [    ] %*s / %i:%02i.%i\r", count + 5, " ", time / 600, (time / 10) % 60, time % 10);
#else
            printf("[  ] [    ]\r");
#endif
            Midiplay_Play(1);

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
                    SDL_PauseAudio(playing ? SDL_FALSE : SDL_TRUE);
                }
                else if (key == 'n')
                {
                    break;
                }
                else if (key == 'q')
                {
                    arg = argc;
                    break;
                }
                else if (key == '-' && volume > 0)
                {
                    volume--;
                    Midiplay_SetVolume(volume * 127 / 100);
                }
                else if (key == '=' && volume < 100)
                {
                    volume++;
                    Midiplay_SetVolume(volume * 127 / 100);
                }
                else if (key == 'r')
                {
                    Midiplay_Restart();
                }
#ifdef MP_TIME
                time = Midiplay_Time();
                printf("[%s%s] [%3i%%] %*i:%02i.%i\r", playing ? " " : "P", looping ? "L" : " ", volume, count, time / 600, (time / 10) % 60, time % 10);
#else
                printf("[%s%s] [%3i%%]\r", playing ? " " : "P", looping ? "L" : " ", volume);
#endif
            }

            printf("\33[K\r");
        }
        else
        {
            INVALID_FILE;
        }

        Midiplay_Play(0);

        free(buffer);
    }

    termAttr.c_lflag = localMode;
    termAttr.c_oflag = outputMode;
    tcsetattr(STDIN_FILENO, TCSANOW, &termAttr);
    fcntl(STDIN_FILENO, F_SETFL, blockMode);

    SDL_CloseAudio();

    SDL_Quit();

    return 0;
}

// midiplay

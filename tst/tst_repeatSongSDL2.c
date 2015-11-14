/**
 * Simple test to render a song in a single buffer and play it repeatedly (using
 * SDL2 as the backend)
 * 
 * @file tst/tst_repeatSongSDL2.c
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#include <c_synth/synth.h>
#include <c_synth/synth_assert.h>
#include <c_synth/synth_errors.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Structure to hold every track pointer as well as how many samples/bytes were
 * played */
struct stSharedData {
    /** Array with tracks' data */
    char *pBuf;
    /** Length of the track, in samples */
    int bufLen;
    /* Whether the song finished playing */
    int didFinish;
    /* Whether the song actually loops or not */
    int doLoop;
    /* Sample to which the song should jump */
    int loopPos;
    /** Number of played samples */
    int pos;
    /** Number of played bytes */
    int posBytes;
    /** Current mode being played */
    synthBufMode mode;
};

static void audioCallback(void *pArg, Uint8 *pStream, int len) {
    int i, j;
    struct stSharedData *pData;
    synthBufMode mode;

    /* Retrieve the data */
    pData = (struct stSharedData*)pArg;
    mode = pData->mode;

    /* Clear the buffer (so there's no extra noise) */
    memset(pStream, 0x0, len);

    /* Check the buffer finished playing */
    if (pData->pos >= pData->bufLen) {
        if (pData->doLoop) {
            pData->pos = pData->loopPos;
            pData->posBytes = pData->pos;
            if (mode & SYNTH_2CHAN) {
                pData->posBytes *= 2;
            }
            if (mode & SYNTH_16BITS) {
                pData->posBytes *= 2;
            }
        }
        else {
            if (!pData->didFinish) {
                pData->didFinish = 1;
            }
            return;
        }
    }

    /* Output the track data */
    do {
        char *pBuf;
        int bufLen;

        /* Retrieve the current buffer */
        pBuf = pData->pBuf;
        bufLen = pData->bufLen;

        /* Loop through the track according with the mode */
        i = 0;
        j = 0;
        while (j < len) {
            int pos;

            if (pData->pos + i >= bufLen) {
                if (pData->doLoop) {
                    pData->pos = pData->loopPos;
                    pData->posBytes = pData->pos;
                    if (mode & SYNTH_2CHAN) {
                        pData->posBytes *= 2;
                    }
                    if (mode & SYNTH_16BITS) {
                        pData->posBytes *= 2;
                    }
                    len -= j;
                    pStream += j;
                    j = 0;
                    i = 0;
                }
                else {
                    pData->pos += i;
                    pData->posBytes += j;
                    if (!pData->didFinish) {
                        pData->didFinish = 1;
                    }
                    return;
                }
            }

            pos = pData->posBytes + j;

            if ((mode & SYNTH_1CHAN) && (mode & SYNTH_8BITS) &&
                    (mode & SYNTH_UNSIGNED)) {
                Uint8 chan, dst;

                /* Retrieve the current stream and the channel data */
                chan = pBuf[pos] & 0x00ff;
                dst  = pStream[j] & 0x00ff;

                /* 'Mix' both */
                dst += chan;

                /* Return it to the stream */
                pStream[j] = dst & 0x00ff;

                j++;
            }
            else if ((mode & SYNTH_1CHAN) && (mode & SYNTH_8BITS) &&
                    (mode & SYNTH_SIGNED)) {
                Sint8 chan, dst;

                /* Retrieve the current stream and the channel data */
                chan = pBuf[pos] & 0x00ff;
                dst  = pStream[j] & 0x00ff;

                /* 'Mix' both */
                dst += chan;

                /* Return it to the stream */
                pStream[j] = dst & 0x00ff;

                j++;
            }
            else if ((mode & SYNTH_1CHAN) && (mode & SYNTH_16BITS) &&
                    (mode & SYNTH_UNSIGNED)) {
                Uint16 chan, dst;

                /* Retrieve the current stream and the channel data */
                chan = (pBuf[pos] & 0x00ff) |
                    ((pBuf[pos + 1] << 8) & 0xff00);
                dst  = (pStream[j] & 0x00ff) | ((pStream[j + 1] << 8) & 0xff00);

                /* 'Mix' both */
                dst += chan;

                /* Return it to the stream */
                pStream[j] = dst & 0x00ff;
                pStream[j + 1] = (dst >> 8) & 0x00ff;

                j += 2;
            }
            else if ((mode & SYNTH_1CHAN) && (mode & SYNTH_16BITS) &&
                    (mode & SYNTH_SIGNED)) {
                Sint16 chan, dst;

                /* Retrieve the current stream and the channel data */
                chan = (pBuf[pos] & 0x00ff) |
                    ((pBuf[pos + 1] << 8) & 0xff00);
                dst  = (pStream[j] & 0x00ff) | ((pStream[j + 1] << 8) & 0xff00);

                /* 'Mix' both */
                dst += chan;

                /* Return it to the stream */
                pStream[j] = dst & 0x00ff;
                pStream[j + 1] = (dst >> 8) & 0x00ff;

                j += 2;
            }
            else if ((mode & SYNTH_2CHAN) && (mode & SYNTH_8BITS) &&
                    (mode & SYNTH_UNSIGNED)) {
                Uint8 chan1, chan2, dst1, dst2;

                /* Retrieve the current stream and the channel data */
                chan1 = pBuf[pos] & 0x00ff;
                chan2 = pBuf[pos + 1] & 0x00ff;
                dst1  = pStream[j] & 0x00ff;
                dst2  = pStream[j + 1] & 0x00ff;

                /* 'Mix' both */
                dst1 += chan1;
                dst2 += chan2;

                /* Return it to the stream */
                pStream[j] = dst1 & 0x00ff;
                pStream[j + 1] = dst2 & 0x00ff;

                j += 2;
            }
            else if ((mode & SYNTH_2CHAN) && (mode & SYNTH_8BITS) &&
                    (mode & SYNTH_SIGNED)) {
                Sint8 chan1, chan2, dst1, dst2;

                /* Retrieve the current stream and the channel data */
                chan1 = pBuf[pos] & 0x00ff;
                chan2 = pBuf[pos + 1] & 0x00ff;
                dst1  = pStream[j] & 0x00ff;
                dst2  = pStream[j + 1] & 0x00ff;

                /* 'Mix' both */
                dst1 += chan1;
                dst2 += chan2;

                /* Return it to the stream */
                pStream[j] = dst1 & 0x00ff;
                pStream[j + 1] = dst2 & 0x00ff;

                j += 2;
            }
            else if ((mode & SYNTH_2CHAN) && (mode & SYNTH_16BITS) &&
                    (mode & SYNTH_UNSIGNED)) {
                Uint16 chan1, chan2, dst1, dst2;

                /* Retrieve the current stream and the channel data */
                chan1 = (pBuf[pos] & 0x00ff) |
                    ((pBuf[pos + 1] << 8) & 0xff00);
                chan2 = (pBuf[pos + 2] & 0x00ff) |
                    ((pBuf[pos + 3] << 8) & 0xff00);
                dst1  = (pStream[j] & 0x00ff) |
                        ((pStream[j + 1] << 8) & 0xff00);
                dst2  = (pStream[j + 2] & 0x00ff) |
                        ((pStream[j + 3] << 8) & 0xff00);

                /* 'Mix' both */
                dst1 += chan1;
                dst2 += chan2;

                /* Return it to the stream */
                pStream[j] = dst1 & 0x00ff;
                pStream[j + 1] = (dst1 >> 8) & 0x00ff;
                pStream[j + 2] = dst2 & 0x00ff;
                pStream[j + 3] = (dst2 >> 8) & 0x00ff;

                j += 4;
            }
            else if ((mode & SYNTH_2CHAN) && (mode & SYNTH_16BITS) &&
                    (mode & SYNTH_SIGNED)) {
                Sint16 chan1, chan2, dst1, dst2;

                /* Retrieve the current stream and the channel data */
                chan1 = (pBuf[pos] & 0x00ff) |
                    ((pBuf[pos + 1] << 8) & 0xff00);
                chan2 = (pBuf[pos + 2] & 0x00ff) |
                    ((pBuf[pos + 3] << 8) & 0xff00);
                dst1  = (pStream[j] & 0x00ff) |
                        ((pStream[j + 1] << 8) & 0xff00);
                dst2  = (pStream[j + 2] & 0x00ff) |
                        ((pStream[j + 3] << 8) & 0xff00);

                /* 'Mix' both */
                dst1 += chan1;
                dst2 += chan2;

                /* Return it to the stream */
                pStream[j] = dst1 & 0x00ff;
                pStream[j + 1] = (dst1 >> 8) & 0x00ff;
                pStream[j + 2] = dst2 & 0x00ff;
                pStream[j + 3] = (dst2 >> 8) & 0x00ff;

                j += 4;
            }
            i++;
        }
    } while (0);

    pData->pos += i;
    pData->posBytes += j;
}

/* Simple test song */
static char __song[] = "MML t90 l16 o5 e e8 e r c e r g4 > g4 <";

/**
 * Entry point
 * 
 * @param  [ in]argc Number of arguments
 * @param  [ in]argv List of arguments
 * @return           The exit code
 */
int main(int argc, char *argv[]) {
    char *pBuf, *pSrc, *pTmp;
    int bufLen, doLoop, didInitSDL, freq, handle, isFile, irv, len, loopPos,
        numBytes;
    SDL_AudioDeviceID dev;
    SDL_AudioSpec wanted, specs;
    struct stSharedData data;
    synthBufMode mode;
    synthCtx *pCtx;
    synth_err rv;

    /* Clean the context, so it's not freed on error */
    didInitSDL = 0;
    pCtx = 0;
    pBuf = 0;
    pTmp = 0;
    dev = 0;

    /* Store the default frequency */
    freq = 44100;
    /* Store the default mode */
    mode = SYNTH_1CHAN_U8BITS;
    numBytes = 1;
    isFile = 0;
    pSrc = 0;
    len = 0;
    /* Check argc/argv */
    if (argc > 1) {
        int i;

        i = 1;
        while (i < argc) {
#define IS_PARAM(l_cmd, s_cmd) \
  if (strcmp(argv[i], l_cmd) == 0 || strcmp(argv[i], s_cmd) == 0)
            IS_PARAM("--string", "-s") {
                if (argc <= i + 1) {
                    printf("Expected parameter but got nothing! Run "
                            "'tst_repeatSongSDL2 --help' for usage!\n");
                    return 1;
                }

                /* Store the string and retrieve its length */
                pSrc = argv[i + 1];
                isFile = 0;
                len = strlen(argv[i + 1]);
            }
            IS_PARAM("--file", "-f") {
                if (argc <= i + 1) {
                    printf("Expected parameter but got nothing! Run "
                            "'tst_repeatSongSDL2 --help' for usage!\n");
                    return 1;
                }

                /* Store the filename */
                pSrc = argv[i + 1];
                isFile = 1;
            }
            IS_PARAM("--frequency", "-F") {
                char *pNum;
                int tmp;

                if (argc <= i + 1) {
                    printf("Expected parameter but got nothing! Run "
                            "'tst_repeatSongSDL2 --help' for usage!\n");
                    return 1;
                }

                pNum = argv[i + 1];

                tmp = 0;

                while (*pNum != '\0') {
                    tmp = tmp * 10 + (*pNum) - '0';
                    pNum++;
                }

                freq = tmp;
            }
            IS_PARAM("--mode", "-m") {
                char *pMode;

                if (argc <= i + 1) {
                    printf("Expected parameter but got nothing! Run "
                            "'tst_repeatSongSDL2 --help' for usage!\n");
                    return 1;
                }

                pMode = argv[i + 1];

                if (strcmp(pMode, "1chan-u8") == 0) {
                    mode = SYNTH_1CHAN_U8BITS;
                    numBytes = 1;
                }
                else if (strcmp(pMode, "1chan-8") == 0) {
                    mode = SYNTH_1CHAN_8BITS;
                    numBytes = 1;
                }
                else if (strcmp(pMode, "1chan-u16") == 0) {
                    mode = SYNTH_1CHAN_U16BITS;
                    numBytes = 2;
                }
                else if (strcmp(pMode, "1chan-16") == 0) {
                    mode = SYNTH_1CHAN_16BITS;
                    numBytes = 2;
                }
                else if (strcmp(pMode, "2chan-u8") == 0) {
                    mode = SYNTH_2CHAN_U8BITS;
                    numBytes = 2;
                }
                else if (strcmp(pMode, "2chan-8") == 0) {
                    mode = SYNTH_2CHAN_8BITS;
                    numBytes = 2;
                }
                else if (strcmp(pMode, "2chan-u16") == 0) {
                    mode = SYNTH_2CHAN_U16BITS;
                    numBytes = 4;
                }
                else if (strcmp(pMode, "2chan-16") == 0) {
                    mode = SYNTH_2CHAN_16BITS;
                    numBytes = 4;
                }
                else {
                    printf("Invalid mode! Run 'tst_repeatSongSDL2 --help' to "
                            "check the usage!\n");
                    return 1;
                }
            }
            IS_PARAM("--help", "-h") {
                printf("A simple test for the c_synth library\n"
                        "\n"
                        "Usage: tst_repeatSongSDL2 [--string | -s "
                            "\"the song\"] [--file | -f <file>]\n"
                        "                       [--frequency | -F <freq>] "
                            "[--mode | -m <mode>]\n"
                        "                       [--help | -h]\n"
                        "\n"
                        "Compiles a single song and the plays it repeatedly.\n"
                        "\n"
                        "If the song doesn't have a loop poin, it will play "
                            "only once.\n"
                        "\n"
                        "'<mode>' must be one of the following:\n"
                        "  1chan-u8 : 1 channel, unsigned  8 bits samples\n"
                        "  1chan-8  : 1 channel,   signed  8 bits samples\n"
                        "  1chan-u16: 1 channel, unsigned 16 bits samples\n"
                        "  1chan-16 : 1 channel,   signed 16 bits samples\n"
                        "  2chan-u8 : 2 channel, unsigned  8 bits samples\n"
                        "  2chan-8  : 2 channel,   signed  8 bits samples\n"
                        "  2chan-u16: 2 channel, unsigned 16 bits samples\n"
                        "  2chan-16 : 2 channel,   signed 16 bits samples\n"
                        "\n"
                        "If no argument is passed, it will compile a simple "
                            "test song.\n");
                return 0;
            }

            i += 2;
#undef IS_PARAM
        }
    }

    /* Initialize it */
    printf("Initialize the synthesizer...\n");
    rv = synth_init(&pCtx, freq);
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Compile a song */
    if (pSrc != 0) {
        if (isFile) {
            printf("Compiling song from file '%s'...\n", pSrc);
            rv = synth_compileSongFromFile(&handle, pCtx, pSrc);
        }
        else {
            printf("Compiling song '%s'...\n", pSrc);
            rv = synth_compileSongFromString(&handle, pCtx, pSrc, len);
        }
    }
    else {
        printf("Compiling static song '%s'...\n", __song);
        rv = synth_compileSongFromStringStatic(&handle, pCtx, __song);
    }

    /* Check if it compiled successfully */
    if (rv != SYNTH_OK) {
        char *pError;
        synth_err irv;

        /* Retrieve and print the error */
        irv = synth_getCompilerErrorString(&pError, pCtx);
        SYNTH_ASSERT_ERR(irv == SYNTH_OK, irv);

        printf("%s", pError);
    }
    else {
        printf("Song compiled successfully!\n");
    }
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Get the length of the song */
    rv = synth_getSongLength(&bufLen, pCtx, handle);
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Alloc the buffer to store the song */
    pBuf = (char*)malloc(bufLen * numBytes * sizeof(char));
    SYNTH_ASSERT_ERR(pBuf, SYNTH_MEM_ERR);

    /* Alloc the temporary buffer to store each track */
    pTmp = (char*)malloc(bufLen * numBytes * sizeof(char));
    SYNTH_ASSERT_ERR(pTmp, SYNTH_MEM_ERR);

    /* Render the song */
    rv = synth_renderSong(pBuf, pCtx, handle, mode, pTmp);
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Check if it actually loops */
    rv = synth_canSongLoop(pCtx, handle);
    SYNTH_ASSERT(rv == SYNTH_OK || rv == SYNTH_NOT_LOOPABLE);
    if (rv == SYNTH_OK) {
        /* Retrieve the loop position */
        rv = synth_getSongIntroLength(&loopPos, pCtx, handle);
        SYNTH_ASSERT(rv == SYNTH_OK);

        doLoop = 1;
    }
    else {
        loopPos = 0;
        doLoop = 0;
    }


    /* Set the data object, so the tracks may be sent to the audio callback */
    memset(&data, 0x0, sizeof(struct stSharedData));
    data.bufLen = bufLen;
    data.doLoop = doLoop;
    data.loopPos = loopPos;
    data.mode = mode;
    data.pBuf = pBuf;

    /* Initialize SDL so it can play the song */
    irv = SDL_Init(SDL_INIT_AUDIO);
    SYNTH_ASSERT_ERR(irv >= 0, SYNTH_INTERNAL_ERR);
    didInitSDL = 1;

    /* Set the audio specs according to the requested ones */
    wanted.freq = freq;
    wanted.samples = 4096;

    if (mode & SYNTH_1CHAN) {
        wanted.channels = 1;
    }
    else if (mode & SYNTH_2CHAN) {
        wanted.channels = 2;
    }

    wanted.samples *= wanted.channels;

    if ((mode & SYNTH_8BITS) && (mode & SYNTH_UNSIGNED)) {
        wanted.format = AUDIO_U8;
    }
    else if ((mode & SYNTH_8BITS) && (mode & SYNTH_SIGNED)) {
        wanted.format = AUDIO_S8;
    }
    else if ((mode & SYNTH_16BITS) && (mode & SYNTH_UNSIGNED)) {
        wanted.format = AUDIO_U16LSB;
    }
    else if ((mode & SYNTH_16BITS) && (mode & SYNTH_SIGNED)) {
        wanted.format = AUDIO_S16LSB;
    }

    wanted.callback = audioCallback;
    wanted.userdata = (void*)&data;

    /* Open the device, so the song may be played */
    dev = SDL_OpenAudioDevice(0, 0, &wanted, &specs, 0);
    SYNTH_ASSERT_ERR(dev != 0, SYNTH_INTERNAL_ERR);

    /* Play the song */
    SDL_PauseAudioDevice(dev, 0);

    /* Wait until either the song ends or a keyboard key is pressed */
    while (!data.didFinish) {
        SDL_Delay(1000);
    }

    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK) {
        printf("An error happened!\n");
    }

    if (dev != 0) {
        SDL_PauseAudioDevice(dev, 1);
        SDL_CloseAudioDevice(dev);
    }

    if (didInitSDL) {
        SDL_Quit();
    }

    if (pCtx) {
        printf("Releasing resources used by the lib...\n");
        synth_free(&pCtx);
    }

    /* Release every buffer */
    if (pBuf) {
        free(pBuf);
    }
    if (pTmp) {
        free(pTmp);
    }

    printf("Exiting...\n");
    return rv;
}


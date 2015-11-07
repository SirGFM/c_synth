/**
 * Simple test to render a song and play it (using SDL2 as the backend)
 * 
 * @file tst/tst_playSongSDL2.c
 */
#include <SDL2/SDL.h>

#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Structure to hold every track pointer as well as how many samples/bytes were
 * played */
struct stSharedData {
    /** Array with tracks' data */
    char **ppBufs;
    /** Length of each track */
    int *pBufLens;
    /** Number of tracks */
    int numBufs;
    /** Number of played samples/bytes */
    int pos;
    /** Current mode being played */
    synthBufMode mode;
};

static void audioCallback(void *pArg, Uint8 *pStream, int len) {
    int i;
    struct stSharedData *pData;
    synthBufMode mode;

    /* Retrieve the data */
    pData = (struct stSharedData*)pArg;
    mode = pData->mode;

    /* Clear the buffer (so there's no extra noise) */
    memset(pStream, 0x0, len);

    /* Mix each track data */
    i = 0;
    while (i < pData->numBufs) {
        char *pBuf;
        int j, bufLen;

        /* Retrieve the current buffer */
        pBuf = pData->ppBufs[i];
        bufLen = pData->pBufLens[i];

        /* Already increase the counter, so we can loop early */
        i++;
        if (pData->pos >= bufLen) {
            continue;
        }

        /* Loop through the track according with the mode */
        j = 0;
        while (j < len) {
            if (pData->pos + j >= bufLen) {
                break;
            }

            if ((mode & SYNTH_1CHAN) && (mode & SYNTH_8BITS) &&
                    (mode & SYNTH_UNSIGNED)) {
                Uint8 chan, dst;

                /* Retrieve the current stream and the channel data */
                chan = pBuf[pData->pos + j] & 0x00ff;
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
                chan = pBuf[pData->pos + j] & 0x00ff;
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
                chan = (pBuf[pData->pos + j] & 0x00ff) |
                    ((pBuf[pData->pos + j + 1] << 8) & 0xff00);
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
                chan = (pBuf[pData->pos + j] & 0x00ff) |
                    ((pBuf[pData->pos + j + 1] << 8) & 0xff00);
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
                chan1 = pBuf[pData->pos + j] & 0x00ff;
                chan2 = pBuf[pData->pos + j + 1] & 0x00ff;
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
                chan1 = pBuf[pData->pos + j] & 0x00ff;
                chan2 = pBuf[pData->pos + j + 1] & 0x00ff;
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
                chan1 = (pBuf[pData->pos + j] & 0x00ff) |
                    ((pBuf[pData->pos + j + 1] << 8) & 0xff00);
                chan2 = (pBuf[pData->pos + j + 2] & 0x00ff) |
                    ((pBuf[pData->pos + j + 3] << 8) & 0xff00);
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
                chan1 = (pBuf[pData->pos + j] & 0x00ff) |
                    ((pBuf[pData->pos + j + 1] << 8) & 0xff00);
                chan2 = (pBuf[pData->pos + j + 2] & 0x00ff) |
                    ((pBuf[pData->pos + j + 3] << 8) & 0xff00);
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
        }
    }

    pData->pos += len;
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
    char **ppBufs, *pSrc;
    int didInitSDL, freq, handle, i, isFile, irv, len, numTracks, maxLen,
            *pBufLens;
    SDL_AudioDeviceID dev;
    SDL_AudioSpec wanted, specs;
    struct stSharedData data;
    synthBufMode mode;
    synthCtx *pCtx;
    synth_err rv;

    /* Clean the context, so it's not freed on error */
    didInitSDL = 0;
    pCtx = 0;
    ppBufs = 0;
    pBufLens = 0;
    dev = 0;

    /* Store the default frequency */
    freq = 44100;
    /* Store the default mode */
    mode = SYNTH_1CHAN_U8BITS;
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
                            "'tst_playSongSDL2 --help' for usage!\n");
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
                            "'tst_playSongSDL2 --help' for usage!\n");
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
                            "'tst_playSongSDL2 --help' for usage!\n");
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
                            "'tst_playSongSDL2 --help' for usage!\n");
                    return 1;
                }

                pMode = argv[i + 1];

                if (strcmp(pMode, "1chan-u8") == 0) {
                    mode = SYNTH_1CHAN_U8BITS;
                }
                else if (strcmp(pMode, "1chan-8") == 0) {
                    mode = SYNTH_1CHAN_8BITS;
                }
                else if (strcmp(pMode, "1chan-u16") == 0) {
                    mode = SYNTH_1CHAN_U16BITS;
                }
                else if (strcmp(pMode, "1chan-16") == 0) {
                    mode = SYNTH_1CHAN_16BITS;
                }
                else if (strcmp(pMode, "2chan-u8") == 0) {
                    mode = SYNTH_2CHAN_U8BITS;
                }
                else if (strcmp(pMode, "2chan-8") == 0) {
                    mode = SYNTH_2CHAN_8BITS;
                }
                else if (strcmp(pMode, "2chan-u16") == 0) {
                    mode = SYNTH_2CHAN_U16BITS;
                }
                else if (strcmp(pMode, "2chan-16") == 0) {
                    mode = SYNTH_2CHAN_16BITS;
                }
                else {
                    printf("Invalid mode! Run 'tst_playSongSDL2 --help' to "
                            "check the usage!\n");
                    return 1;
                }
            }
            IS_PARAM("--help", "-h") {
                printf("A simple test for the c_synth library\n"
                        "\n"
                        "Usage: tst_playSongSDL2 [--string | -s \"the song\"] "
                            "[--file | -f <file>]\n"
                        "                       [--frequency | -F <freq>] "
                            "[--mode | -m <mode>]\n"
                        "                       [--help | -h]\n"
                        "\n"
                        "Compiles a single song and the plays it once.\n"
                        "\n"
                        "Note that this test ignores a song loop point and "
                            "tracks with different\n"
                        "lengths.\n"
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

    /* Get the number of tracks in the song */
    printf("Retrieving the number of tracks in the song...\n");
    rv = synth_getAudioTrackCount(&numTracks, pCtx, handle);
    SYNTH_ASSERT(rv == SYNTH_OK);
    printf("Found %i tracks\n", numTracks);

    /* Create arrays for storing the buffers and lengths of each song */
    ppBufs = (char**)malloc(sizeof(char*) * numTracks);
    SYNTH_ASSERT_ERR(ppBufs, SYNTH_MEM_ERR);
    memset(ppBufs, 0x0, sizeof(char*) * numTracks);

    pBufLens = (int*)malloc(sizeof(int) * numTracks);
    SYNTH_ASSERT_ERR(pBufLens, SYNTH_MEM_ERR);

    /* Get the number of samples in each track */
    printf("Rendering each of the song's tracks...\n");
    i = 0;
    maxLen = 0;
    while (i < numTracks) {
        char *pBuf;
        int intro, len, reqLen;

        /* Retrieve the length of the track, in samples */
        rv = synth_getTrackIntroLength(&intro, pCtx, handle, i);
        SYNTH_ASSERT(rv == SYNTH_OK);
        rv = synth_getTrackLength(&len, pCtx, handle, i);
        SYNTH_ASSERT(rv == SYNTH_OK);

        printf("Track %i requires %i samples and loops at %i\n", i + 1, len,
                intro);

        /* Find the longest track, in samples */
        if (len > maxLen) {
            maxLen = len;
        }

        /* Retrieve the number of bytes required */
        reqLen = len;
        if (mode & SYNTH_16BITS) {
            reqLen *= 2;
        }
        if (mode & SYNTH_2CHAN) {
            reqLen *= 2;
        }

        printf("Track %i requires %i bytes (%i KB, %i MB)\n", i + 1, reqLen,
                reqLen >> 10, reqLen >> 20);

        /* Alloc the track's buffer... */
        pBuf = (char*)malloc(reqLen);
        SYNTH_ASSERT_ERR(pBuf, SYNTH_MEM_ERR);

        /* Store the buffer's pointer and length to play it later */
        ppBufs[i] = pBuf;
        pBufLens[i] = reqLen;

        /* Render the track */
        rv = synth_renderTrack(pBuf, pCtx, handle, i, mode);
        SYNTH_ASSERT(rv == SYNTH_OK);

        i++;
    }

    /* Set the data object, so the tracks may be sent to the audio callback */
    data.ppBufs = ppBufs;
    data.pBufLens = pBufLens;
    data.numBufs = numTracks;
    data.pos = 0;
    data.mode = mode;

    /* Initialize SDL so it can play the song */
    irv = SDL_Init(SDL_INIT_AUDIO);
    SYNTH_ASSERT_ERR(irv >= 0, SYNTH_INTERNAL_ERR);
    didInitSDL = 1;

    /* Set the audio specs according to the requested ones */
    wanted.freq = freq;

    if (mode & SYNTH_1CHAN) {
        wanted.channels = 1;
    }
    else if (mode & SYNTH_2CHAN) {
        wanted.channels = 2;
    }

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

    /* Wait long enough, until the audio is played */
    printf("Sleeping for %ums, while the song plays...\n",
            (unsigned int)maxLen * 1000 / freq + 1);
    SDL_Delay((unsigned int)maxLen * 1000 / freq + 1);

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
    if (ppBufs) {
        int i;

        while (i < numTracks) {
            if (ppBufs[i]) {
                free(ppBufs[i]);
            }

            i++;
        }
        free(ppBufs);
    }

    printf("Exiting...\n");
    return rv;
}


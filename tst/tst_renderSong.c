/**
 * Simple test to render a song into a raw file
 * 
 * @file tst/tst_renderSong.c
 */
#include <SDL2/SDL.h>

#include <c_synth/synth.h>
#include <c_synth/synth_assert.h>
#include <c_synth/synth_errors.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
    FILE *fOut;
    char **ppBufs, *pSrc, *pOut;
    int freq, didWrite, handle, i, isFile, len, numTracks, maxLen,
            *pBufLens;
    synthBufMode mode;
    synthCtx *pCtx;
    synth_err rv;

    /* Clean the context, so it's not freed on error */
    pCtx = 0;
    ppBufs = 0;
    pBufLens = 0;
    pOut = 0;
    fOut = 0;

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
                            "'tst_renderSong --help' for usage!\n");
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
                            "'tst_renderSong --help' for usage!\n");
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
                            "'tst_renderSong --help' for usage!\n");
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
                            "'tst_renderSong --help' for usage!\n");
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
                    printf("Invalid mode! Run 'tst_renderSong --help' to "
                            "check the usage!\n");
                    return 1;
                }
            }
            IS_PARAM("--help", "-h") {
                printf("A simple test for the c_synth library\n"
                        "\n"
                        "Usage: tst_renderSong [--string | -s \"the song\"] "
                            "[--file | -f <file>]\n"
                        "                       [--frequency | -F <freq>] "
                            "[--mode | -m <mode>]\n"
                        "                       [--help | -h] "
                            "[--out | -o <file>]\n"
                        "\n"
                        "Compiles a single song and save it to a file.\n"
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
            IS_PARAM("--out", "-o") {
                if (argc <= i + 1) {
                    printf("Expected parameter but got nothing! Run "
                            "'tst_renderSong --help' for usage!\n");
                    return 1;
                }

                /* Store the filename */
                pOut = argv[i + 1];
            }

            i += 2;
#undef IS_PARAM
        }
    }

    if (pOut == 0) {
        printf("No output file supplied!\n");
        SYNTH_ASSERT(0);
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

    /* Loop through every track, mixing them and saving them to the file */
    fOut = fopen(pOut, "wb");
    if (!fOut) {
        printf("Failed to open output file '%s'!\n", pOut);
        SYNTH_ASSERT(0);
    }

    i = 0;
    do {
        uint8_t data[4];
        int j;

        didWrite = 0;
        memset(data, 0, sizeof(data));
        for (j = 0; j < numTracks; j++) {
            uint8_t *pBuf;

            if (i >= pBufLens[j]) {
                continue;
            }

            pBuf = (uint8_t*)ppBufs[j];

            switch (mode) {
            case SYNTH_1CHAN_U8BITS: {
                uint8_t chan, dst;

                chan = pBuf[i] & 0x00ff;
                dst  = data[0] & 0x00ff;

                data[0] = (dst + chan) & 0x00ff;
            } break;
            case SYNTH_1CHAN_8BITS: {
                int8_t chan, dst;

                chan = pBuf[i] & 0x00ff;
                dst  = data[0] & 0x00ff;

                data[0] = (dst + chan) & 0x00ff;
            } break;
            case SYNTH_1CHAN_U16BITS: {
                uint16_t chan, dst;

                chan = (pBuf[i] & 0x00ff) | ((pBuf[i + 1] << 8) & 0xff00);
                dst  = (data[0] & 0x00ff) | ((data[1] << 8) & 0xff00);

                dst += chan;

                data[0] = dst & 0x00ff;
                data[1] = (dst >> 8) & 0x00ff;
            } break;
            case SYNTH_1CHAN_16BITS: {
                int16_t chan, dst;

                chan = (pBuf[i] & 0x00ff) | ((pBuf[i + 1] << 8) & 0xff00);
                dst  = (data[0] & 0x00ff) | ((data[1] << 8) & 0xff00);

                dst += chan;

                data[0] = dst & 0x00ff;
                data[1] = (dst >> 8) & 0x00ff;
            } break;
            case SYNTH_2CHAN_U8BITS: {
                uint8_t chan1, chan2, dst1, dst2;

                chan1 = pBuf[i] & 0x00ff;
                chan2 = pBuf[i + 1] & 0x00ff;
                dst1  = data[0] & 0x00ff;
                dst2  = data[1] & 0x00ff;

                data[0] = (chan1 + dst1) & 0x00ff;
                data[1] = (chan2 + dst2) & 0x00ff;
            } break;
            case SYNTH_2CHAN_8BITS: {
                int8_t chan1, chan2, dst1, dst2;

                chan1 = pBuf[i] & 0x00ff;
                chan2 = pBuf[i + 1] & 0x00ff;
                dst1  = data[0] & 0x00ff;
                dst2  = data[1] & 0x00ff;

                data[0] = (chan1 + dst1) & 0x00ff;
                data[1] = (chan2 + dst2) & 0x00ff;
            } break;
            case SYNTH_2CHAN_U16BITS: {
                uint16_t chan1, chan2, dst1, dst2;

                chan1 = (pBuf[i] & 0x00ff) | ((pBuf[i + 1] << 8) & 0xff00);
                chan2 = (pBuf[i + 2] & 0x00ff) | ((pBuf[i + 3] << 8) & 0xff00);
                dst1  = (data[0] & 0x00ff) | ((data[1] << 8) & 0xff00);
                dst2  = (data[2] & 0x00ff) | ((data[3] << 8) & 0xff00);

                dst1 += chan1;
                dst2 += chan2;

                data[0] = dst1 & 0x00ff;
                data[1] = (dst1 >> 8) & 0x00ff;
                data[2] = dst2 & 0x00ff;
                data[3] = (dst2 >> 8) & 0x00ff;
            } break;
            case SYNTH_2CHAN_16BITS: {
                int16_t chan1, chan2, dst1, dst2;

                chan1 = (pBuf[i] & 0x00ff) | ((pBuf[i + 1] << 8) & 0xff00);
                chan2 = (pBuf[i + 2] & 0x00ff) | ((pBuf[i + 3] << 8) & 0xff00);
                dst1  = (data[0] & 0x00ff) | ((data[1] << 8) & 0xff00);
                dst2  = (data[2] & 0x00ff) | ((data[3] << 8) & 0xff00);

                dst1 += chan1;
                dst2 += chan2;

                data[0] = dst1 & 0x00ff;
                data[1] = (dst1 >> 8) & 0x00ff;
                data[2] = dst2 & 0x00ff;
                data[3] = (dst2 >> 8) & 0x00ff;
            } break;
            default: { /* Shouldn't ever happen; avoid warning */ }
            }

            didWrite = 1;
        }

        if (didWrite) {
            size_t numIn, numOut;

            switch (mode &
                    (SYNTH_1CHAN | SYNTH_8BITS | SYNTH_2CHAN | SYNTH_16BITS)) {
            case (SYNTH_1CHAN | SYNTH_8BITS): {
                numIn = 1;
            } break;
            case (SYNTH_1CHAN | SYNTH_16BITS):
            case (SYNTH_2CHAN | SYNTH_8BITS): {
                numIn = 2;
            } break;
            case (SYNTH_2CHAN | SYNTH_16BITS): {
                numIn = 4;
            } break;
            }

            numOut = fwrite(data, sizeof(uint8_t), numIn, fOut);
            if (numIn != numOut) {
                printf("Failed to write data\n");
                SYNTH_ASSERT(0);
            }
        }

        switch (mode &
                (SYNTH_1CHAN | SYNTH_8BITS | SYNTH_2CHAN | SYNTH_16BITS)) {
        case (SYNTH_1CHAN | SYNTH_8BITS): {
            i++;
        } break;
        case (SYNTH_1CHAN | SYNTH_16BITS):
        case (SYNTH_2CHAN | SYNTH_8BITS): {
            i += 2;
        } break;
        case (SYNTH_2CHAN | SYNTH_16BITS): {
            i += 4;
        } break;
        }

    } while (didWrite);

    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK) {
        printf("An error happened!\n");
    }

    if (pCtx) {
        printf("Releasing resources used by the lib...\n");
        synth_free(&pCtx);
    }

    /* Release every buffer */
    if (ppBufs) {
        int i;

        i = 0;
        while (i < numTracks) {
            if (ppBufs[i]) {
                free(ppBufs[i]);
            }

            i++;
        }
        free(ppBufs);
    }

    if (pBufLens) {
        free(pBufLens);
    }

    if (fOut) {
        fclose(fOut);
    }

    printf("Exiting...\n");
    return rv;
}


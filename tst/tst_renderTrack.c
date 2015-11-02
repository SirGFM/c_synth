/**
 * Simple test to render each track of a song
 * 
 * @file tst/tst_renderTrack.c
 */
#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    char *pBuf, *pSrc;
    int bufLen, freq, handle, i, isFile, len, num;
    synthBufMode mode;
    synthCtx *pCtx;
    synth_err rv;

    /* Clean the context, so it's not freed on error */
    pCtx = 0;
    pBuf = 0;
    bufLen = 0;

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
                            "'tst_renderTrack --help' for usage!\n");
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
                            "'tst_renderTrack --help' for usage!\n");
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
                            "'tst_renderTrack --help' for usage!\n");
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
                            "'tst_renderTrack --help' for usage!\n");
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
                    printf("Invalid mode! Run 'tst_renderTrack --help' to "
                            "check the usage!\n");
                    return 1;
                }
            }
            IS_PARAM("--help", "-h") {
                printf("A simple test for the c_synth library\n"
                        "\n"
                        "Usage: tst_renderTrack [--string | -s \"the song\"] "
                            "[--file | -f <file>]\n"
                        "                       [--frequency | -F <freq>] "
                            "[--mode | -m <mode>] \n"
                        "                       [--help | -h]\n"
                        "\n"
                        "Compiles a single song and then render each of its "
                            "tracks.\n"
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
    rv = synth_getAudioTrackCount(&num, pCtx, handle);
    SYNTH_ASSERT(rv == SYNTH_OK);
    printf("Found %i tracks\n", num);

    /* Get the number of samples in each track */
    printf("Rendering each of the song's tracks...\n");
    i = 0;
    while (i < num) {
        int intro, len, reqLen;

        /* Retrieve the length of the track, in samples */
        rv = synth_getTrackIntroLength(&intro, pCtx, handle, i);
        SYNTH_ASSERT(rv == SYNTH_OK);
        rv = synth_getTrackLength(&len, pCtx, handle, i);
        SYNTH_ASSERT(rv == SYNTH_OK);

        printf("Track %i requires %i samples and loops at %i\n", i + 1, len,
                intro);

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

        /* Check if the buffer must be expanded... */
        if (bufLen < reqLen) {
            printf("Expanding the buffer from %i bytes to %i bytes...\n",
                    bufLen, reqLen);

            pBuf = (char*)realloc(pBuf, reqLen);
            SYNTH_ASSERT_ERR(pBuf, SYNTH_MEM_ERR);

            bufLen = reqLen;
        }

        /* Render the track */
        rv = synth_renderTrack(pBuf, pCtx, handle, i, mode);
        SYNTH_ASSERT(rv == SYNTH_OK);

        i++;
    }

    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK) {
        printf("An error happened!\n");
    }

    if (pCtx) {
        printf("Releasing resources used by the lib...\n");
        synth_free(&pCtx);
    }

    if (pBuf) {
        free(pBuf);
    }

    printf("Exiting...\n");
    return rv;
}


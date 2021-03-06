/**
 * Simple test to compile a song either from the command line or from a file
 * 
 * If no song is passed, it will use a default, static, songs
 * 
 * @file tst/tst_compileSong.c
 */
#include <c_synth/synth.h>
#include <c_synth/synth_assert.h>
#include <c_synth/synth_errors.h>

#include <stdio.h>
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
    char *pSrc;
    int freq, handle, isFile, len;
    synthCtx *pCtx;
    synth_err rv;

    /* Clean the context, so it's not freed on error */
    pCtx = 0;

    /* Store the default frequency */
    freq = 44100;
    isFile = 0;
    pSrc = 0;
    len = 0;
    /* TODO Check argc/argv */
    if (argc > 1) {
        int i;

        i = 1;
        while (i < argc) {
#define IS_PARAM(l_cmd, s_cmd) \
  if (strcmp(argv[i], l_cmd) == 0 || strcmp(argv[i], s_cmd) == 0)
            IS_PARAM("--string", "-s") {
                if (argc <= i + 1) {
                    printf("Expected parameter but got nothing! Run "
                            "'tst_compileSong --help' for usage!\n");
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
                            "'tst_compileSong --help' for usage!\n");
                    return 1;
                }

                /* Store the filename */
                pSrc = argv[i + 1];
                isFile = 1;
            }
            IS_PARAM("--help", "-h") {
                printf("A simple test for the c_synth library\n"
                        "\n"
                        "Usage: tst_compileSong [--string | -s \"the song\"] "
                            "[--file | -f <file>]\n"
                        "                       [--help | -h]\n"
                        "\n"
                        "Only one song can be compiled at a time, and this "
                            "program simply checks if it \n"
                        "compiles successfully or not (no output is "
                            "generated).\n"
                        "On error, however, this program does display the "
                            "cause and position of the \n"
                        "error.\n"
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

    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK) {
        printf("An error happened!\n");
    }

    if (pCtx) {
        printf("Releasing resources used by the lib...\n");
        synth_free(&pCtx);
    }

    printf("Exiting...\n");
    return rv;
}


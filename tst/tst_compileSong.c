/**
 * Simple test to compile a song either from the command line or from a file
 * 
 * If no song is passed, it will use a default, static, songs
 * 
 * @file tst/tst_compileSong.c
 */
#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>

#include <stdio.h>

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
    int freq, handle;
    synthCtx *pCtx;
    synth_err rv;

    /* Clean the context, so it's not freed on error */
    pCtx = 0;

    /* TODO Check argc/argv */
    freq = 44100;

    /* Initialize it */
    rv = synth_init(&pCtx, freq);
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Compile a song */
    rv = synth_compileSongFromStringStatic(&handle, pCtx, __song);
    if (rv != SYNTH_OK) {
        char *pError;
        synth_err irv;

        /* Retrieve and print the error */
        irv = synth_getCompilerErrorString(&pError, pCtx);
        SYNTH_ASSERT_ERR(irv == SYNTH_OK, irv);

        printf("%s", pError);
    }
    SYNTH_ASSERT(rv == SYNTH_OK);

    rv = SYNTH_OK;
__err:
    if (pCtx) {
        synth_free(&pCtx);
    }

    return rv;
}


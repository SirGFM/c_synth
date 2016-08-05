/**
 * @project   c_synth
 * @license   zlib license
 * @file      src/synth_error.c
 *
 * @summary   Internal error handling
 *
 * While c_synth/synth_error.h enumerates and handles errors caused by
 * user input, this header files expands that by implementing macros to
 * check programming errors. Those are only enabled on the debug build,
 * and they are designed to aid finding runtime bugs, or "simple
 * programming errors".
 */
#include <c_synth_internal/synth_error.h>

#include <stdio.h>
#include <stdlib.h>

/**
 * Prints an asserted statement and the current call stack (if possible)
 * and then exits the program.
 *
 * If printing the call stack isn't implemented for a given platform, at
 * least the source of the asserted statement is logged.
 *
 * @param  [ in]pAsserted The asserted statement
 * @param  [ in]pFile     The source file
 * @param  [ in]pFunction The currently executing function
 * @param  [ in]line      The line number of the asserted statement
 */
void synth_throwError(const char *pAsserted, const char *pFile
        , const char *pFunction, const int line) {
    /* TODO Fix logging!!! */
    printf("Asserted statement: %s\n", pAsserted);
    printf("Asserted position: %s:%d (%s)\n", pFile, line, pFunction);
    /* TODO Print callback */
    exit(-1);
}


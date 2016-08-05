/**
 * @project   c_synth
 * @license   zlib license
 * @file      src/include/c_synth_internal/synth_error.h
 *
 * @summary   Internal error handling
 *
 * While c_synth/synth_error.h enumerates and handles errors caused by
 * user input, this header files expands that by implementing macros to
 * check programming errors. Those are only enabled on the debug build,
 * and they are designed to aid finding runtime bugs, or "simple
 * programming errors".
 */
#ifndef __SYNTH_INTERNAL_ERROR_H__
#define __SYNTH_INTERNAL_ERROR_H__

/**
 * Check if a statement is false and exit the program with an error
 * code.
 *
 * If available on the platform, the current stack strace (as well
 * as the failed statement) is logged, to help debugging.
 *
 * This functionality is always enabled on debug builds and is disabled
 * on release ones (unless requested through a 'STRICT' define).
 */
#if defined(DEBUG) || defined(STRICT)
# define synth_assert(stmt) \
    do { \
        if (!(stmt)) { \
            synth_throwError(#stmt, __FILE__, __FUNCTION__, __LINE__); \
        } \
    } while (0)
#else
# define synth_assert(stmt)
#endif

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
        , const char *pFunction, const int line);

#endif /* __SYNTH_INTERNAL_ERROR_H__ */


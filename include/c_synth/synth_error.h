/**
 * @project   c_synth
 * @license   zlib license
 * @file      include/c_synth/synth_error.h
 *
 * @summary   The language's lexer.
 *
 * @typedef   synth_error Error code returned by the synth
 * 
 * List all errors. They are mapped into an enumeration that (hopefully)
 * covers all cases and, therefore, will never required a generic
 * "function failed".
 *
 * A human readable error string may be retrieved from the function
 * 'synth_getErrorString'.
 */
#ifndef __SYNTH_ERRORS_H__
#define __SYNTH_ERRORS_H__

typedef enum {
    SYNTH_OK = 0,
    SYNTH_FUNCTION_NOT_IMPLEMENTED,
    SYNTH_MAX_ERR
} synth_err;

/**
 * Returns a human readable string for a given error.
 *
 * @param  [ in]rv The error code
 * @return         Error description
 */
const char* synth_getErrorString(synth_err rv);

#endif /* __SYNTH_ERRORS_H__ */

#ifndef __SYNTH_ERRORS_H__

typedef enum {
    SYNTH_OK = 0,
    SYNTH_ALREADY_STARTED,
    SYNTH_INTERNAL_ERR,
    SYNTH_MEM_ERR,
    SYNTH_OPEN_FILE_ERR,
    SYNTH_FUNCTION_NOT_IMPLEMENTED,
    SYNTH_INVALID_TOKEN,
    SYNTH_EOF,
    SYNTH_EOS,
    SYNTH_UNEXPECTED_TOKEN,
    SYNTH_EMPTY_SEQUENCE,
    SYNTH_INVALID_WAVE,
    SYNTH_NOT_INITIALIZED,
    SYNTH_ALREADY_INITIALIZED,
    SYNTH_THREAD_ALREADY_INITIALIZED,
    SYNTH_BAD_PARAM_ERR,
    SYNTH_THREAD_INIT_FAILED,
    SYNTH_BUFFER_ALREADY_INITIALIZED,
    SYNTH_BUFFER_NOT_ENOUGH_SAMPLES,
    SYNTH_COULDNT_LOCK,
    SYNTH_NO_ERRORS,
    SYNTH_INVALID_INDEX,
    SYNTH_COMPLEX_LOOPPOINT,
    SYNTH_NOT_LOOPABLE,
    SYNTH_COMPASS_OVERFLOW,
    SYNTH_BAD_LOOP_START,
    SYNTH_BAD_LOOP_END,
    SYNTH_BAD_LOOP_POINT,
    SYNTH_MAX_ERR
} synth_err;

#endif /* __SYNTH_ERRORS_H__ */


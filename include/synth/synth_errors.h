/**
 * @file include/synth/synth_errors.h
 * 
 * Enumeration of possible errors
 */
#ifndef __SYNTH_ERRORS_H
#define __SYNTH_ERRORS_H

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
    SYNTH_MAX_ERR
} synth_err;

#endif


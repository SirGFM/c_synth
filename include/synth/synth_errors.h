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
    SYNTH_EOF_ERR,
    SYNTH_EOS_ERR,
//    SYNTH_UNEXPECTED_TOKEN,
    SYNTH_MAX_ERR
} synth_err;

#endif


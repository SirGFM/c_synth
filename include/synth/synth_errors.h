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
    SYNTH_MAX_ERR
} synth_err;

#endif


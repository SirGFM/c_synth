/**
 * @file include/synth/synth_types.h
 */
#ifndef __SYNTH_TYPES_H_
#define __SYNTH_TYPES_H_

#include <stdint.h>

typedef enum {
    SYNTH_TRUE = 0,
    SYNTH_FALSE
} synth_bool;

typedef enum {
    SYNTH_MAX_PARAM = 0
} synth_param;

typedef enum {
    W_SQUARE = 0,
    W_PULSE_12_5,
    W_PULSE_25,
    W_PULSE_75,
    W_TRIANGLE,
    W_NOISE,
    SYNTH_MAX_WAVE
} synth_wave;

typedef enum {
    N_C = 0,
    N_CS,
    N_D,
    N_DS,
    N_E,
    N_F,
    N_FS,
    N_G,
    N_GS,
    N_A,
    N_AS,
    N_B,
    N_REST,
    N_LOOP,
} synth_note;

#endif


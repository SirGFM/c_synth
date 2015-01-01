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
    N_CB = 0,   // Required for decreasing octave
    N_C,
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
    N_BS,   // Required for increasing octave
    N_REST,
    N_LOOP,
} synth_note;

typedef enum {
    T_SET_BPM = 0,
    T_SET_DURATION,
    T_SET_OCTAVE,
    T_SET_REL_OCTAVE,
    T_SET_LOOPPOINT,
    T_END_OF_TRACK,
    T_SET_VOLUME,
    T_SET_REL_VOLUME,
    T_SET_KEYOFF,
    T_SET_PAN,
    T_SET_LOOP_START,
    T_SET_LOOP_END,
    T_SET_WAVE,
    T_NOTE,
    T_DURATION,
    T_NUMBER,
    T_DONE,
    TK_MAX
} synth_token;

#endif


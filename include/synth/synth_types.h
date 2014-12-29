/**
 * @file include/synth/synth_types.h
 */
#ifndef __SYNTH_TYPES_H_
#define __SYNTH_TYPES_H_

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
    N_Cs,
    N_D,
    N_Ds,
    N_E,
    N_F,
    N_Fs
    N_G,
    N_Gs,
    N_A,
    N_As,
    N_B,
} synth_note;

#endif


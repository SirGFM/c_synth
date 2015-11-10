/**
 * 
 * @file src/synth_note.c
 */
#include <c_synth/synth_assert.h>
#include <c_synth/synth_errors.h>

#include <c_synth_internal/synth_note.h>
#include <c_synth_internal/synth_prng.h>
#include <c_synth_internal/synth_types.h>
#include <c_synth_internal/synth_volume.h>

#include <stdlib.h>
#include <string.h>

/**
 * Static array with note frequencies; to get lower octaves one, right-shift
 * by the number of octaves going down
 */
static int __synthNote_frequency[] = {
/* 'Cb' 8 */0xffff,
/*  C   8 */  4186,
/*  C#  8 */  4435,
/*  D   8 */  4699,
/*  D#  8 */  4978,
/*  E   8 */  5274,
/*  F   8 */  5588,
/*  F#  8 */  5920,
/*  G   8 */  6272,
/*  G#  8 */  6645,
/*  A   8 */  7040,
/*  A#  8 */  7459,
/*  B   8 */  7902,
/* 'B#' 8 */0xffff
};

/**
 * Retrieve a new note pointer, so it can be later initialized
 * 
 * @param  [out]pNote The new note
 * @param  [ in]pCtx  The synthesizer context
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthNote_init(synthNote **ppNote, synthCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(ppNote, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    /* Make sure there's enough space for another note */
    SYNTH_ASSERT_ERR(pCtx->notes.max == 0 || pCtx->notes.used < pCtx->notes.max,
            SYNTH_MEM_ERR);

    /* Retrieve the note to be used */
    if (pCtx->notes.used >= pCtx->notes.len) {
        /* 'Double' the current buffer; Note that this will never be called if
         * the context was pre-alloc'ed, since 'max' will be set; The '+1' is
         * for the first note, in which len will be 0 */
        pCtx->notes.buf.pNotes = (synthNote*)realloc(pCtx->notes.buf.pNotes,
                (1 + pCtx->notes.len * 2) * sizeof(synthNote));
        SYNTH_ASSERT_ERR(pCtx->notes.buf.pNotes, SYNTH_MEM_ERR);
        /* Clear only the new part of the buffer */
        memset(&(pCtx->notes.buf.pNotes[pCtx->notes.used]), 0x0,
                (1 + pCtx->notes.len) * sizeof(synthNote));
        /* Actually increase the buffer length */
        pCtx->notes.len += 1 + pCtx->notes.len;
    }
    (*ppNote) = &(pCtx->notes.buf.pNotes[pCtx->notes.used]);
    pCtx->notes.used++;

    /* Initialize the note's default parameters */
    synthNote_setPan(*ppNote, 50);
    synthNote_setOctave(*ppNote, 4);
    synthNote_setWave(*ppNote, W_SQUARE);
    synthNote_setNote(*ppNote, N_A);
    synthNote_setDuration(*ppNote, pCtx, 60, 4);
    synthNote_setKeyoff(*ppNote, 0, 75, 0);
    synthNote_setVolume(*ppNote, 0);

    (*ppNote)->numIterations = 0;
    (*ppNote)->jumpPosition = 0;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Retrieve a new note pointer, already initialized as a loop
 * 
 * @param  [out]ppNote   The new note
 * @param  [ in]pCtx     The synthesizer context
 * @param  [ in]repeat   How many times the loop should repeat
 * @param  [ in]position Note to which the song should jump back, on loop
 * @return               SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthNote_initLoop(synthNote **ppNote, synthCtx *pCtx, int repeat,
        int position) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(ppNote, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Retrieve a new note 'object' */
    rv = synthNote_init(ppNote, pCtx);
    SYNTH_ASSERT(rv == SYNTH_OK);

    /* Set the note's parameters */
    (*ppNote)->note = N_LOOP;
    (*ppNote)->len = repeat;
    (*ppNote)->jumpPosition = position;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Set an attribute, clamping it to the desired range
 * 
 * @param [ in]function The function name
 * @param [ in]type     The type of the attribute being set
 * @param [ in]attr     The name of the attribute
 * @param [ in]min      The minimum valid value
 * @param [ in]max      The maximum valid value
 */
#define SYNTHNOTE_CLAMPEDSETTER(function, type, attr, min, max) \
  synth_err function(synthNote *pNote, type val) { \
    synth_err rv; \
  \
    /* Sanitize the arguments */ \
    SYNTH_ASSERT_ERR(pNote, SYNTH_BAD_PARAM_ERR); \
  \
    /* Clamp the value to the valid range */ \
    if (val < min) { \
        val = min; \
    } \
    else if (val > max) { \
        val = max; \
    } \
  \
    /* Set the value */ \
    pNote->attr = val; \
  \
    rv = SYNTH_OK; \
__err: \
    return rv; \
  }

/**
 * Set the note panning
 * 
 * Defines which channel, if any, should be louder; If the value is outside the
 * expected [0, 100] range, it's automatically clamped
 * 
 * @param [ in]pNote The note
 * @param [ in]pan   Panning level; 0 means completelly to the left and 100
 *                   means completelly to the right
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
SYNTHNOTE_CLAMPEDSETTER(synthNote_setPan, char, pan, 0, 100);

/**
 * Set the note octave
 * 
 * Define higher the pitch, the highed the numeric representation; The value is
 * clamped to the range [1, 8]
 * 
 * @param [ in]pNote  The note
 * @param [ in]octave The octave
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
SYNTHNOTE_CLAMPEDSETTER(synthNote_setOctave, char, octave, 1, 8);

/**
 * Set the note wave
 * 
 * If the wave isn't valid, it will be set to noise!
 * 
 * @param [ in]pNote The note
 * @param [ in]wave  The wave
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
SYNTHNOTE_CLAMPEDSETTER(synthNote_setWave, synth_wave, wave, W_SQUARE,
        SYNTH_MAX_WAVE - 1);

/**
 * Set the musical note
 * 
 * @param [ in]pNote The note
 * @param [ in]note  The musical note
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
SYNTHNOTE_CLAMPEDSETTER(synthNote_setNote, synth_note, note, N_CB, N_LOOP);

/**
 * Set the note duration
 * 
 * NOTE: The duration is stored in samples
 * 
 * @param [ in]pNote    The note
 * @param [ in]pCtx     The synthesizer context
 * @param [ in]bpm      The song's speed in beats-per-minute
 * @param [ in]duration Bitfield for the duration. Each bit represents a
 *                      fraction of the duration;
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_setDuration(synthNote *pNote, synthCtx *pCtx, int bpm,
        int duration) {
    double nSamples;
    int freq, len;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pNote, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(bpm > 0, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(duration > 0, SYNTH_BAD_PARAM_ERR);

    /* Store the synthesizer frequency */
    freq = pCtx->frequency;
    /* Caculate the duration (in samples) of a semibreve (i.e., "full") note:
     * 4 beats / N beats per min = 4 beats / (N / 60) beats per second
     * 4 * 60 / N = T s (duration of semibreve in seconds)
     * T s * F Hz = number of samples in a semibreve */
    nSamples = freq * 240.0 / (double)bpm;

    /* Accumulate the note's duration in samples */
    len = 0;
    while (duration > 0) {
        if ((duration & 1) == 1) {
            /* Accumulate this duration, remembering to transform from
             * samples-per-miliseconds back to hertz */
            len += (int)nSamples;
        }

        /* Remove a bit and halve the number of samples */
        duration >>= 1;
        nSamples *= 0.5;
    }

    /* Store the calculated duration */
    pNote->len = len;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Set the characteristics of the note's duration
 * 
 * NOTE: This parameter must be set after the duration
 * 
 * All values must be in the range [0, 100]. The attack is campled to the range
 * [0, keyoff] and the release is campled to the range [keyoff, 100]. Although
 * the parameter express the percentage of the note's duration, the value is
 * stored in samples.
 * 
 * @param  [ in]pNote  The note
 * @param  [ in]attack  The percentage of the note duration before it reaches
 *                      its full amplitude
 * @param  [ in]keyoff  The percentage of the note duration before it's released
 * @param  [ in]release The percentage of the note duration before it halts
 *                      completely
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_setKeyoff(synthNote *pNote, int attack, int keyoff,
        int release) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pNote, SYNTH_BAD_PARAM_ERR);

    /* Defines a macro to clamp a variable */
#define CLAMP(VAR, MIN, MAX) \
  if (VAR < MIN) { \
    VAR = MIN; \
  } \
  else if (VAR > MAX) { \
    VAR = MAX; \
  }

    /* Clamp the values to their valid ranges */
    CLAMP(keyoff, 0, 100);
    CLAMP(attack, 0, keyoff);
    CLAMP(release, keyoff, 100);

#undef CLAMP

    /* Calculate (and store) the keyoff in samples */
    pNote->attack = pNote->len * attack / 100;
    pNote->keyoff = pNote->len * keyoff / 100;
    pNote->release = pNote->len * release / 100;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Set the volume envelop
 * 
 * @param  [ in]pNote The note
 * @param  [ in]pVol  The volume
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_setVolume(synthNote *pNote, synthVolume *pVol) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pNote, SYNTH_BAD_PARAM_ERR);

    /* Store the volume */
    pNote->pVol = pVol;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Check if the note is a loop point
 * 
 * @param  [ in]pNote The note
 * @return            SYNTH_TRUE, SYNTH_FALSE
 */
synth_bool synthNote_isLoop(synthNote *pNote) {
    if (!pNote || pNote->note != N_LOOP) {
        return SYNTH_FALSE;
    }
    else {
        return SYNTH_TRUE;
    }
}

/**
 * Retrieve an attribute
 * 
 * @param [ in]function The function name
 * @param [ in]type     The type of the attribute being set
 * @param [ in]attr     The name of the attribute
 * @param [ in]loopOnly Whether this attribute is only valid for loops; If 0,
 *                      the attribute is only valid for notes (i.e., not loops)
 */
#define SYNTHNOTE_GETTER(function, type, attr, loopOnly) \
  synth_err function(type *pVal, synthNote *pNote) { \
    synth_err rv; \
  \
    /* Sanitize the arguments */ \
    SYNTH_ASSERT_ERR(pNote, SYNTH_BAD_PARAM_ERR); \
    SYNTH_ASSERT_ERR(pVal, SYNTH_BAD_PARAM_ERR); \
    /* Check that it's either a note or loop (as required by the attribute */ \
    SYNTH_ASSERT_ERR((!loopOnly && pNote->note != N_LOOP) || \
            (loopOnly && pNote->note == N_LOOP), SYNTH_BAD_PARAM_ERR); \
  \
    *pVal = pNote->attr; \
  \
    rv = SYNTH_OK; \
__err: \
    return rv; \
  }

/**
 * Retrieve the note duration, in samples
 * 
 * @param  [out]pVal  The duration
 * @param  [ in]pNote The note
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
SYNTHNOTE_GETTER(synthNote_getDuration, int, len, 0)

/**
 * Retrieve the panning of the note, where 0 means completely on the left
 * channel and 100 means completely on the right channel
 * 
 * @param  [out]pVal  The panning
 * @param  [ in]pNote The note
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
SYNTHNOTE_GETTER(synthNote_getPan, char, pan, 0)

/**
 * Retrieve the number of times this loop should repeat
 * 
 * @param  [out]pVal  The repeat count
 * @param  [ in]pNote The note
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
SYNTHNOTE_GETTER(synthNote_getRepeat, int, len, 1)

/**
 * Retrieve the position, in the track, to which it should jump on loop
 * 
 * @param  [out]pVal  The repeat position
 * @param  [ in]pNote The note
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
SYNTHNOTE_GETTER(synthNote_getJumpPosition, int, jumpPosition, 1)

/**
 * Render a note into a buffer
 * 
 * The buffer must have at least 'synthNote_getDuration' bytes time the number
 * of bytes required by the mode
 * 
 * @param  [ in]pBuf      Buffer that will be filled with the track
 * @param  [ in]pNote     The note
 * @param  [ in]pCtx      The synthesizer context
 * @param  [ in]mode      Desired mode for the wave
 * @param  [ in]synthFreq Synthesizer's frequency
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_render(char *pBuf, synthNote *pNote, synthCtx *pCtx,
        synthBufMode mode, int synthFreq) {
    int i, noteFreq, numBytes, spc;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pBuf, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pNote, SYNTH_BAD_PARAM_ERR);

    /* Calculate the number of bytes per samples */
    numBytes = 1;
    if (mode & SYNTH_16BITS) {
        numBytes = 2;
    }
    if (mode & SYNTH_2CHAN) {
        numBytes *= 2;
    }

    /* Clear the note */
    memset(pBuf, 0x0, pNote->len * numBytes);
    /* If it's a rest, simply return (since it was already cleared */
    if (pNote->note == N_REST) {
        rv = SYNTH_OK;
        goto __err;
    }

    /* Calculate the note frequency (or "cycle"). E.g., A4 = 440Hz */
    noteFreq = __synthNote_frequency[pNote->note] >> (9 - pNote->octave);
    /* Calculate how many 'samples-per-cycle' there are for the Note's note */
    spc = synthFreq / noteFreq;

    /* Synthesize the note audio */
    i = 0;
    while (i < pNote->release) {
        char pan;
        int amp, j;
        float clampAmp, perc, waveAmp;

        /* TODO Rewrite this loop without using floats */

        /* Calculate the percentage of the note into the current cycle */
        perc = ((float)(i % spc)) / spc;

        /* Retrieve the current amplitude */
        rv = synthVolume_getAmplitude(&amp, pNote->pVol, i / (float)pNote->len *
                1024);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

        /* Defines the value that encapsulates the note */
        if (i < pNote->attack) {
            /* Varies the value from 0.0f -> 1.0f */
            clampAmp = i / (float)pNote->attack;
        }
        else if (i > pNote->keyoff) {
            /* Varies the value from 1.0f -> 0.0f */
            clampAmp = 1.0f - (i - pNote->keyoff) /
                    (float)(pNote->release - pNote->keyoff);
        }
        else {
            clampAmp = 1.0f;
        }

        /* Retrieve the note panning (in case it uses 2 channels) */
        rv = synthNote_getPan(&pan, pNote);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

        /* Calculate the sample's actual index */
        j = i * numBytes;

        /* Retrieve the sample's amplitude, according to the note's wave form.
         * This amplitude is calculated in the range [-1.0f, 1.0f], so it can
         * correctly be downsampled for 8 and 16 bits amplitudes (as well as
         * signed and unsigned) */
        switch (pNote->wave) {
            case W_NOISE_SQUARE:
            case W_SQUARE: {
                /* 50% duty cycle */
                if (perc < 0.5f) {
                    waveAmp = 1.0f;
                }
                else {
                    if (mode & SYNTH_SIGNED) {
                        waveAmp = -1.0f;
                    }
                    else {
                        waveAmp = 0.0f;
                    }
                }
            } break;
            case W_NOISE_12_5:
            case W_PULSE_12_5: {
                /* 12.5% duty cycle */
                if (perc < 0.125f) {
                    waveAmp = 1.0f;
                }
                else {
                    if (mode & SYNTH_SIGNED) {
                        waveAmp = -1.0f;
                    }
                    else {
                        waveAmp = 0.0f;
                    }
                }
            } break;
            case W_NOISE_25:
            case W_PULSE_25: {
                /* 25% duty cycle */
                if (perc < 0.25f) {
                    waveAmp = 1.0f;
                }
                else {
                    if (mode & SYNTH_SIGNED) {
                        waveAmp = -1.0f;
                    }
                    else {
                        waveAmp = 0.0f;
                    }
                }
            } break;
            case W_NOISE_75:
            case W_PULSE_75: {
                /* 75% duty cycle */
                if (perc < 0.75f) {
                    waveAmp = 1.0f;
                }
                else {
                    if (mode & SYNTH_SIGNED) {
                        waveAmp = -1.0f;
                    }
                    else {
                        waveAmp = 0.0f;
                    }
                }
            } break;
            case W_NOISE_TRIANGLE:
            case W_TRIANGLE: {
                /* Convert the percentage into a triangular wave with its
                 * positive peak at 0.25% samples and its negative peak at 0.75%
                 * samples */
                if (mode & SYNTH_SIGNED) {
                    if (perc < 0.25f) {
                        waveAmp = 4.0f * perc;
                    }
                    else if (perc < 0.5f) {
                        waveAmp = 4.0f * (0.5f - perc);
                    }
                    else if (perc < 0.75f) {
                        waveAmp = -4.0f * (perc - 0.5f);
                    }
                    else {
                        waveAmp = -4.0f * (1.0f - perc);
                    }
                }
                else {
                    if (perc < 0.5f) {
                        waveAmp = 2.0f * perc;
                    }
                    else {
                        waveAmp = 2.0f * (1.0f - perc);
                    }
                }
                /* Make triangle waves a little louder */
                waveAmp *= 1.125;
            } break;
            case W_NOISE: {
                /* Simply set the amplitude to 1.0f, so it may be multiplied by
                 * the pseudo-random value later */
                waveAmp = 1.0f;
            } break;
            default: { waveAmp = 0.0f; }
        }

        if (pNote->wave >= W_NOISE && pNote->wave <= W_NOISE_TRIANGLE) {
            double noise;

            rv = synthPRNG_getGaussianNoise(&noise, &(pCtx->prngCtx));
            SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

            if (pNote->wave == W_NOISE) {
                /* For simple noises, simply export random values */
                waveAmp = (float)(noise * 2.0);
            }
            else if (pNote->wave == W_NOISE_TRIANGLE) {
                waveAmp = (float)(waveAmp * 0.75 + noise * waveAmp * 4.0 * 0.25);
            }
            else if (pNote->wave == W_NOISE_25) {
                if (waveAmp > 0.0f) {
                    waveAmp = (float)(noise * 6.0);
                }
                else {
                    waveAmp = (float)(noise * 1.5);
                }
            }
            else {
                /* If it's a simple rectangular wave, clamp the value to the
                 * desired range */
                if (waveAmp > 0.0f) {
                    waveAmp = (float)(noise * 4.0);
                }
                else {
                    waveAmp = (float)(noise * 0.25);
                }
            }
        }

        /* "Fix" the note amplitude */
        waveAmp *= clampAmp;

        /* Convert the amplitude to the desired format and store it at the
         * buffer */
        switch (mode) {
            case SYNTH_1CHAN_U8BITS:
            case SYNTH_1CHAN_8BITS: {
                /* Simply store the calculated value; The amplitude is
                 * lessened to only 8 bits */
                pBuf[j] = (amp >> 8) * waveAmp;
            } break;
            case SYNTH_1CHAN_U16BITS:
            case SYNTH_1CHAN_16BITS: {
                int amp16;

                /* Simply store the calculated value */
                amp16 = amp  * waveAmp;

                /* Simply store the calculated value; Storing the lower bits on
                 * byte 0 and the higher ones on bit 1 */
                pBuf[j] = amp16 & 0xff;
                pBuf[j + 1] = (amp16 >> 8) & 0xff;
            } break;
            case SYNTH_2CHAN_U8BITS:
            case SYNTH_2CHAN_8BITS: {
                char lAmp8, rAmp8;

                /* Calculate the amplitude on both channels, 0 means left only
                 * and 100 means right only */
                lAmp8 = (amp >> 8) * waveAmp * ((100 - pan) / 100.0f);
                rAmp8 = (amp >> 8) * waveAmp * (pan / 100.0f);

                pBuf[j] = lAmp8 & 0xff;
                pBuf[j + 1] = rAmp8 & 0xff;
            } break;
            case SYNTH_2CHAN_U16BITS:
            case SYNTH_2CHAN_16BITS: {
                int lAmp16, rAmp16;

                /* Calculate the amplitude on both channels, 0 means left only
                 * and 100 means right only */
                lAmp16 = amp  * waveAmp * ((100 - pan) / 100.0f);
                rAmp16 = amp  * waveAmp * (pan / 100.0f);

                /* Store the left channel on bytes 0 (low) and 1 (high) and the
                 * right one on 2 (low) and 3 (high) */
                pBuf[j] = lAmp16 & 0xff;
                pBuf[j + 1] = (lAmp16 >> 8) & 0xff;
                pBuf[j + 2] = rAmp16 & 0xff;
                pBuf[j + 3] = (rAmp16 >> 8) & 0xff;
            } break;
            default : { /* Avoids warnings */ }
        }

        /* Increase, since we are looping through the samples (and not through
         * the bytes) */
        i++;
#if 0

        char pan;
        int amp, j, perc, waveAmp;


        /* Calculate the percentage into the current cycle in the range
         * [0,1024) */
        perc = ((i % spc) << 10) / spc;

        /* Retrieve the current amplitude */
        rv = synthVolume_getAmplitude(&amp, pNote->pVol, perc);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

        /* Retrieve the sample's amplitude, according to the note's wave form.
         * This amplitude is calculated in the range [-0x10000, 0x10000], so it
         * can correctly be downsampled for 8 and 16 bits amplitudes (as well as
         * signed and unsigned) */
        switch (pNote->wave) {
            case W_SQUARE: {
                /* 50% duty cycle */
                if (perc > 512) {
                    waveAmp = 0x1000;
                }
                else {
                    waveAmp = -0x10000;
                }
            } break;
            case W_PULSE_12_5: {
                /* 12.5% duty cycle */
                if (perc > 128) {
                    waveAmp = 0x1000;
                }
                else {
                    waveAmp = -0x10000;
                }
            } break;
            case W_PULSE_25: {
                /* 25% duty cycle */
                if (perc > 256) {
                    waveAmp = 0x1000;
                }
                else {
                    waveAmp = -0x10000;
                }
            } break;
            case W_PULSE_75: {
                /* 75% duty cycle */
                if (perc > 768) {
                    waveAmp = 0x1000;
                }
                else {
                    waveAmp = -0x10000;
                }
            } break;
            case W_TRIANGLE: {
                /* Convert the percentage into a triangular wave with its
                 * positive peak at 256 samples and its negative peak at 768
                 * samples */
                if ((perc % 512) < 256) {
                    waveAmp = 0x10000 * (perc % 256) / 256.0f;
                }
                else {
                    waveAmp = 0x10000 * (256.0f - (perc % 256)) / 256.0f;
                }
                if (amp < 512) {
                    amp = (int)(amp * (perc / 512.0f)) & 0xff;
                }
                else {
                    amp = (int)(amp * ((1024.0f - perc) / 512.0f)) & 0xff;
                }
                /* TODO Conver the above to an integer calculation (something
                 * among these lines:
                 * 
                 * amp = (amp * perc * (1 - (perc >> 9)) + amp * (1024 - perc) *
                 *         (perc >> 9)) >> 9;
                 * 
                 * Note that it uses the fact that 'perc >> 9' comes out as 0 if
                 * the percentage is less than 512 and as 1, otherwise (since
                 * perc's range is [0, 1024))
                 * 
                 * The current problem with the above is that 'amp * perc' (and
                 * 'amp * (1024 - perc)') will have to be divided by 512, but it
                 * might come out as 0 for small amplitudes and at the start/end
                 * of the current cycle.
                 */
            } break;
            case W_NOISE: {
                /* TODO Implement a decent noise */
                SYNTH_ASSERT_ERR(0, SYNTH_FUNCTION_NOT_IMPLEMENTED);
            } break;
            default: { /* Avoids warnings */ }
        }

        /* Retrieve the note panning (in case it uses 2 channels) */
        rv = synthNote_getPan(&pan, pNote);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

        /* Calculate the sample's actual index */
        j = i * numBytes;

        /* Convert the amplitude to the desired format and store it at the
         * buffer */
        switch (mode) {
            case SYNTH_1CHAN_U8BITS: {
                /* Simply store the calculated value */
                pBuf[j] = amp;
            } break;
            case SYNTH_1CHAN_8BITS: {
                /* TODO */
                SYNTH_ASSERT_ERR(0, SYNTH_FUNCTION_NOT_IMPLEMENTED);
            } break;
            case SYNTH_1CHAN_U16BITS: {
                int amp16;

                /* Simply calculate the 16 bits amplitude by 'converting' the
                 * 8 bits one to the 16 bits range*/
                amp16 = amp << 8;

                /* Simply store the calculated value; Storing the lower bits on
                 * byte 0 and the higher ones on bit 1 */
                pBuf[j] = amp16 & 0xff;
                pBuf[j + 1] = (amp16 >> 8) & 0xff;
            } break;
            case SYNTH_1CHAN_16BITS: {
                /* TODO */
                SYNTH_ASSERT_ERR(0, SYNTH_FUNCTION_NOT_IMPLEMENTED);
            } break;
            case SYNTH_2CHAN_U8BITS: {
                /* Simply store the calculated value on both channels;
                 * Position 0 is the left channel and position 1 is the right
                 * one */
                pBuf[j] = ((amp * 100 - amp * pan) / 100) & 0xff;
                pBuf[j + 1] = ((amp * 100 - amp * (100 - pan)) / 100) & 0xff;
            } break;
            case SYNTH_2CHAN_8BITS: {
                /* TODO */
                SYNTH_ASSERT_ERR(0, SYNTH_FUNCTION_NOT_IMPLEMENTED);
            } break;
            case SYNTH_2CHAN_U16BITS: {
                int lAmp16, rAmp16;

                /* Calculate the panning between channels as having double the
                 * bits */
                lAmp16 = (((amp * 100 - amp * pan) << 8) / 100);
                rAmp16 = (((amp * 100 - amp * (100 - pan)) << 8) / 100);

                /* Store the left channel on bytes 0 (low) and 1 (high) and the
                 * right one on 2 (low) and 3 (high) */
                pBuf[j] = lAmp16 & 0xff;
                pBuf[j + 1] = (lAmp16 >> 8) & 0xff;
                pBuf[j + 2] = rAmp16 & 0xff;
                pBuf[j + 3] = (rAmp16 >> 8) & 0xff;
            } break;
            case SYNTH_2CHAN_16BITS: {
                /* TODO */
                SYNTH_ASSERT_ERR(0, SYNTH_FUNCTION_NOT_IMPLEMENTED);
            } break;
            default : { /* Avoids warnings */ }
        }

        /* Increase, since we are looping through the samples (and not through
         * the bytes) */
        i++;
#endif /* 0 */
    }
    /* The silence (after the key was released) was already cleared, so simply
     * return */

    rv = SYNTH_OK;
__err:
    return rv;
}


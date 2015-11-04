/**
 * 
 * @file src/synth_note.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>

#include <synth_internal/synth_note.h>
#include <synth_internal/synth_types.h>
#include <synth_internal/synth_volume.h>

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
    synthNote_setKeyoff(*ppNote, 75);
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
SYNTHNOTE_CLAMPEDSETTER(synthNote_setWave, synth_wave, wave, W_SQUARE, W_NOISE);

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
    int freq, len, nSamples, time;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pNote, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(bpm > 0, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(duration > 0, SYNTH_BAD_PARAM_ERR);

    /* Store the synthesizer frequency */
    freq = pCtx->frequency;
    /* Caculate the duration (in miliseconds) of a semibreve (i.e., "full")
     * note (i.e., 1000ms  * 60 * 4 beats = 60s * 4 beats  = 1 min * 4 beats*/
    time = 60 * 1000 * 4 / bpm;
    /* Calculate the duration of a semibreve in samples; Note that freq is in
     * hertz (i.e., samples-per-seconds), while time is in miliseconds; This is
     * done so the time loses less accuracy, when being divide by the BPM */
    nSamples = freq * time;

    /* Accumulate the note's duration in samples */
    len = 0;
    while (duration > 0) {
        if ((duration & 1) == 1) {
            /* Accumulate this duration, remembering to transform from
             * samples-per-miliseconds back to hertz */
            len += nSamples / 1000;
        }

        /* Remove a bit and halve the number of samples */
        duration >>= 1;
        nSamples >>= 1;
    }

    /* Store the calculated duration */
    pNote->len = len;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Set the note's keyoff time
 * 
 * NOTE: This parameter must be set after the duration
 * 
 * Calculate (and store) after how many samples this note should be released;
 * The value must be a number in the range [0, 100], represeting the percentage
 * of the note that it must keep playing
 * 
 * @param  [ in]pNote  The note
 * @param  [ in]keyoff The percentage of the note duration before it's released
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_setKeyoff(synthNote *pNote, int keyoff) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pNote, SYNTH_BAD_PARAM_ERR);

    /* Clamp the value to the valid range */
    if (keyoff < 0) {
        keyoff = 0;
    }
    else if (keyoff > 100) {
        keyoff = 100;
    }

    /* Calculate (and store) the keyoff in samples */
    pNote->keyoff = pNote->len * keyoff / 100;

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
 * @param  [ in]mode      Desired mode for the wave
 * @param  [ in]synthFreq Synthesizer's frequency
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthNote_render(char *pBuf, synthNote *pNote, synthBufMode mode,
        int synthFreq) {
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
    while (i < pNote->keyoff) {
        char pan;
        int amp, j;
        float perc, waveAmp;

        /* TODO Rewrite this loop without using floats */

        /* Calculate the percentage of the note into the current cycle */
        perc = ((float)(i % spc)) / spc;

        /* Retrieve the current amplitude */
        rv = synthVolume_getAmplitude(&amp, pNote->pVol, i / (float)pNote->len *
                1024);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

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
            case W_SQUARE: {
                /* 50% duty cycle */
                if (perc < 0.5f) {
                    waveAmp = 1.0f;
                }
                else {
                    waveAmp = -1.0f;
                }
            } break;
            case W_PULSE_12_5: {
                /* 12.5% duty cycle */
                if (perc < 0.125f) {
                    waveAmp = 1.0f;
                }
                else {
                    waveAmp = -1.0f;
                }
            } break;
            case W_PULSE_25: {
                /* 25% duty cycle */
                if (perc < 0.25f) {
                    waveAmp = 1.0f;
                }
                else {
                    waveAmp = -1.0f;
                }
            } break;
            case W_PULSE_75: {
                /* 75% duty cycle */
                if (perc < 0.75f) {
                    waveAmp = 1.0f;
                }
                else {
                    waveAmp = -1.0f;
                }
            } break;
            case W_TRIANGLE: {
                /* Convert the percentage into a triangular wave with its
                 * positive peak at 0.25% samples and its negative peak at 0.75%
                 * samples */
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
            } break;
            case W_NOISE: {
                /* TODO Implement a decent noise */
                SYNTH_ASSERT_ERR(0, SYNTH_FUNCTION_NOT_IMPLEMENTED);
            } break;
            default: { /* Avoids warnings */ }
        }

        /* If it's unsigned, convert it to the range [0.0f, 1.0f] */
        if (mode & SYNTH_UNSIGNED) {
            waveAmp = (waveAmp + 1.0f) * 0.5f;
        }

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

#if 0
#include <synth/synth_backend.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_cache.h>
#include <synth_internal/synth_note.h>
#include <synth_internal/synth_prng.h>
#include <synth_internal/synth_volume.h>

/**
 * Static array with note frequencies; to get lower octaves one, right-shift
 * by the number of octaves going down
 */
static int note_frequency[] = 
{
 0xffff, // 'Cb' 8
 4186, // C 8
 4435, // C#8
 4699, // D 8
 4978, // D#8
 5274, // E 8
 5588, // F 8
 5920, // F#8
 6272, // G 8
 6645, // G#8
 7040, // A 8
 7459, // A#8
 7902,  // B 8
 0xffff // 'B#8'
};

/**
 * Initialize everything to default values.
 * 
 * @param note The note
 */
void synth_note_init(synthNote *note) {
    synth_note_setPan(note, 50);
    synth_note_setOctave(note, 4);
    synth_note_setDuration(note, 60, 4);
    synth_note_setWave(note, W_SQUARE);
    synth_note_setNote(note, N_A);
    synth_note_setVolume(note, 0);
    synth_note_setKeyoff(note, 75);
    
    note->numIterations = 0;
    note->pos = 0;
}

/**
 * Clean up. Since nothing is instantiated, it isn't needed, for now...
 * 
 * @param note The note
 */
void synth_note_clean(synthNote *note) {
    synth_note_init(note);
}


/**
 * Set the note panning.
 * 
 * @param note The note
 * @param pan Panning level. 0 is completelly to the left and 100 is
 * completelly to the right. The value is corretly clamped.
 */
void synth_note_setPan(synthNote *note, char pan) {
    if (pan < 0)
        pan = 0;
    else if (pan > 100)
        pan = 100;
    note->pan = pan;
}

/**
 * Get the note panning.
 * 
 * @param note The note
 */
char synth_note_getPan(synthNote *note) {
    return note->pan;
}

/**
 * Set the note octave
 * 
 * @param note The note
 * @param octave The note octave, 1 being the lowest and 8 being the one of
 * highest pitch
 */
void synth_note_setOctave(synthNote *note, char octave) {
    if (octave < 1)
        octave = 1;
    else if (octave > 8)
        octave = 8;
    note->octave = octave-1;
}

/**
 * Get the note octave
 * 
 * @param note The note
 */
char synth_note_getOctave(synthNote *note) {
    return note->octave+1;
}

/**
 * Set the note duration. Since it uses the current backend frequency, should
 * only be called after it was setuped
 * 
 * @param note The note
 * @param bpm Beats per minute
 * @param duration Bitfield for the duration. Each bit represents its 1/2^n
 * duration; i.e., 1/8 = (1000)b; 1/16. = (110000)b. It's as straight forward
 * as it seems
 */
void synth_note_setDuration(synthNote *note, int bpm, int duration) {
    // Set duration, so caching is easier
    note->duration = duration;
    note->len = synth_note_getSampleSize(bpm, duration);
}

/**
 * Get the note duration.
 * 
 * @param note The note
 */
int synth_note_getDuration(synthNote *note) {
    return note->duration;
}

/**
 * Get the note length in samples
 * 
 * @param note The note
 */
int synth_note_getLen(synthNote *note) {
    return note->len;
}

/**
 * Set the kind of wave this note should play
 * 
 * @param note The note
 * @param wave The wave
 */
void synth_note_setWave(synthNote *note, synth_wave wave) {
    note->wave = wave;
}

/**
 * Get the kind of wave this note should play
 * 
 * @param note The note
 */
synth_wave synth_note_getWave(synthNote *note) {
    return note->wave;
}

/**
 * Set the musical note that should be played
 * 
 * @param note The note
 * @param N The musical note to be played
 */
void synth_note_setNote(synthNote *note, synth_note N) {
    note->note = N;
}

/**
 * Set the musical note that should be played
 * 
 * @param note The note
 * @return The current musical note
 */
synth_note synth_note_getNote(synthNote *note) {
    return note->note;
}

/**
 * Set how the volume behaves
 * 
 * @param note The note
 * @param vol The volume; must be caller released
 */
void synth_note_setVolume(synthNote *note, synthVolume *vol) {
    note->vol = vol;
}

/**
 * Get how the volume behaves
 * 
 * @param note The note
 */
synthVolume* synth_note_getVolume(synthNote *note) {
    return note->vol;
}

/**
 * Set the keyoff for the note; must be set after the duration!
 * 
 * @param note The note
 * @param keyoff Percentage (a integer between 0 and 100) of the duration that
 * should be played until the "note is released" (in a physical sense...
 * associate with how a keyboard note is pressed/released)
 */
void synth_note_setKeyoff(synthNote *note, int keyoff) {
    note->keyoff = note->len * keyoff / 100;
}

/**
 * Set the keyoff for the note; must be set after the duration!
 * 
 * @param note The note
 */
int synth_note_getKeyoff(synthNote *note) {
    return note->keyoff * 100 / note->len;
}

/**
 * Set how many times the 'looper' should count until continuing
 * 
 * @param note The note
 * @param num How many times it should loop
 */
void synth_note_setRepeatTimes(synthNote *note, int num) {
    note->len = num-1;
}

/**
 * To which position the looper should jump, when it still must repeat.
 * 
 * @param note The note
 * @param pos Position into the track that the repeatition starts
 */
void synth_note_setJumpPosition(synthNote *note, int pos) {
    note->pos = pos;
}

/**
 * Get the jump position and increase the jump counter
 * 
 * @param note The note
 * @return The position where the track should jump to
 */
int synth_note_jumpToPosition(synthNote *note) {
    note->numIterations++;
    
    return note->pos;
}

/**
 * Check if the note is a loop and if there are any jumps left.
 *
 * @param note The note
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_note_doLoop(synthNote *note) {
    if (note->note != N_LOOP || note->numIterations >= note->len)
        return SYNTH_FALSE;
    return SYNTH_TRUE;
}

/**
 * Check whether a note finished playing or not
 *
 * @param note The note
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_note_didFinish(synthNote *note) {
    if (note->pos >= note->len || note->note == N_LOOP)
        return SYNTH_TRUE;
    return SYNTH_FALSE;
}

synth_bool synth_note_didFinishHacky(synthNote *note, int _pos) {
    if (_pos >= note->len || note->note == N_LOOP)
        return SYNTH_TRUE;
    return SYNTH_FALSE;
}

/**
 * Reset the note so it can be played again at a later time
 * 
 * @param note The note
 */
void synth_note_reset(synthNote *note) {
    if (note->note == N_LOOP)
        note->numIterations = 0;
    else
        note->pos = 0;
}

/**
 * Synthesize part of this note to a buffer
 * 
 * @param note Note to be synthesized
 * @param samples How many samples there still are in this "slice"
 * @param left Left output buffer
 * @param right Right output buffer
 * @return How many samples from the next note should be synthesized (leftover)
 */
int synth_note_synthesize(synthNote *note, int samples, uint16_t *left,
    uint16_t *right) {
    // Synthesize from a single function!
    return synth_note_synthesizeHacky(note, samples, left, right, &(note->pos));
}

/**
 * Get how many samples a note would have
 * 
 * @param bpm Beats per minute
 * @param duration Bitfield for the duration. Each bit represents its 1/2^n
 * duration; i.e., 1/8 = (1000)b; 1/16. = (110000)b. It's as straight forward
 * @return The length
 */
int synth_note_getSampleSize(int bpm, int duration) {
    int freq, time, nSamples, len;
    
    len = 0;
    
    // Get the global frequency
    freq = synth_cache_getFrequency();
    // Calculate how long (in miliseconds) a semibreve note should last
    time = 1000 * 4 * 60 / bpm;
    // Calculate how many samples a semibreve should last
    nSamples = freq * time;
    
    // Accumulate the duration of the note, in samples
    while (duration > 0) {
        // If the note has this note value
        if ((duration & 1) == 1)
            // Accumulate the number of samples for thie note value
            len += nSamples / 1000;
        
        // Get the next note value (semibreve->minim->etc)
        duration >>= 1;
        nSamples >>= 1;
    }
    
    return len;
}

int synth_note_synthesizeHacky(synthNote *note, int samples, uint16_t *left,
    uint16_t *right, int *_pos) {
    int rem, pos;
    
    // Set the remainder for a case it returns before actually modifing it
    rem = samples;
    // Simply pass through, if it's a "loop note"
    SYNTH_ASSERT(note->note != N_LOOP);
    
    pos = *_pos;
    
    // Actually buffer the note
    if (note->note == N_REST) {
        pos += rem;
        if (pos > note->len) {
            rem = pos - note->len;
            pos = note->len;
        }
        else
            rem = 0;
    }
    else {
        int freq, spc, i, div;
        
        // See note bellow about samples per cycle
        freq = synth_cache_getFrequency();
        div = note_frequency[note->note] >> (8 - note->octave);
        
        if (div != 0)
            spc = freq / div;
        else
            spc = 1023;
        
        i = 0;
        while (i < rem && pos + i  < note->len) {
            int perc;
            char amp;
            
            // Get the position in the cycle as a 'percentage' in [0,1024)
            perc = (pos + i) % spc;
            perc = (perc << 10) / spc;
            
            // Get the amplitude in [0, 255] for the wave at that point
            switch (note->wave) {
                case W_SQUARE:
                    amp = (perc < 512)?synth_vol_getVolume(note->vol, perc):0;
                    break;
                case W_PULSE_12_5:
                    amp = (perc < 128)?synth_vol_getVolume(note->vol, perc):0;
                    break;
                case W_PULSE_25:
                    amp = (perc < 256)?synth_vol_getVolume(note->vol, perc):0;
                    break;
                case W_PULSE_75:
                    amp = (perc < 768)?synth_vol_getVolume(note->vol, perc):0;
                    break;
                case W_TRIANGLE: {
                    int abs;
                    
                    abs = (perc << 1) - 1024;
                    abs = (abs >= 0)?abs:-abs;
                    abs = 1024 - abs;
                    
                    amp = (synth_vol_getVolume(note->vol, perc) * abs * 2) >> 10;
                } break;
                case W_NOISE: {
                    perc >>= 2;
                    if (perc < 512) {
                        char v = 2*(char)synth_prng_getRand();
                        amp = synth_vol_getVolume(note->vol, perc) * v / 0xff;
                    }
                    else
                        amp = 0;
                } break;
                // TODO modularize
                default:
                    amp = 0;
            }
            
            // Pan the sound and buffer it
            if (pos + i < note->keyoff) {
                left[i] += 0x7fff * (100 - note->pan) * amp / 25600;
                right[i] += 0x7fff * note->pan * amp / 25600;
            }
            
            i++;
        }
        
        rem -= i;
        pos += i;
    }
    
    *_pos = pos;
__err:
    return rem;
}

#endif /* 0 */


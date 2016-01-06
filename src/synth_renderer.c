/**
 * @file src/synth_renderer.c
 */
#include <c_synth/synth_assert.h>
#include <c_synth/synth_errors.h>

#include <c_synth_internal/synth_audio.h>
#include <c_synth_internal/synth_note.h>
#include <c_synth_internal/synth_renderer.h>
#include <c_synth_internal/synth_types.h>

/**
 * Initialize the renderer for a given audio
 *
 * @param  [ in]pCtx      The renderer context
 * @param  [ in]pAudio    The audio to be rendered
 * @param  [ in]frequency The synth frequency in samples per second
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthRenderer_init(synthRendererCtx *pCtx, synthAudio *pAudio,
        int frequency) {
    synth_err rv;
    int bpm, timeSign;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pAudio, SYNTH_BAD_PARAM_ERR);

    /* Get the audio BPM */
    rv = synthAudio_getBpm(&bpm, pAudio);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    /* Get the time signature */
    rv = synthAudio_getTimeSignature(&timeSign, pAudio);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* TODO Accept various time signatures */

    /* Calculate the duration (in samples) of a complete compass (considering a
     * 4/4 time signature):
     *
     * 4 beats / N beats per min = 4 beats / (N / 60) beats per second
     * 4 * 60 / N = T s (duration of semibreve in seconds)
     * T s * F Hz = number of samples in a semibreve */
    pCtx->samplesPerCompass = frequency * 240 / bpm;
    /* Store the time signature */
    pCtx->timeSignature = timeSign;

    /* Make sure to reset the current length */
    rv = synthRenderer_resetPosition(pCtx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Returns the position back to the start but don't modify anything related to
 * audio
 *
 * @param  [ in]pCtx The renderer context
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthRenderer_resetPosition(synthRendererCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Reset the current length */
    pCtx->curCompassLength = 0;
    pCtx->curCompassPosition = 0;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Return a note's length in samples, considering its position within the
 * compass. This function also update the renderer internal state (i.e., the
 * position within the compass)
 *
 * @param  [out]pLen  The note's length in samples
 * @param  [ in]pCtx  The renderer context
 * @param  [ in]pNote The note
 */
synth_err synthRenderer_getNoteLengthAndUpdate(int *pLen,
        synthRendererCtx *pCtx, synthNote *pNote) {
    int bit, duration;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pLen, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pNote, SYNTH_BAD_PARAM_ERR);

    /* Retrieve the note's duration in binary fixed point notation */
    rv = synthNote_getDuration(&duration, pNote);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* TODO Accept various time signatures */

    /* Update the current position within the compass */
    pCtx->curCompassPosition += duration;
    /* This should be guaranteed else where, but... */
    SYNTH_ASSERT_ERR(pCtx->curCompassPosition <= pCtx->timeSignature,
            SYNTH_COMPASS_OVERFLOW);

    if (pCtx->curCompassPosition == pCtx->timeSignature) {
        /* If this is the last beat on a compass, use all remaining samples */
        *pLen = pCtx->samplesPerCompass - pCtx->curCompassLength;

        /* Reset the current compass */
        rv = synthRenderer_resetPosition(pCtx);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    }
    else {
        int len;

        /* Otherwise, calculate the beat's length */
        bit = 6;
        len = pCtx->samplesPerCompass;
        *pLen = 0;
        while (bit >= 0) {
            if (duration & (1 << bit)) {
                *pLen += len;
            }

            bit--;
            len >>= 1;
        }

        /* Update the position within the compass, in samples */
        pCtx->curCompassLength += *pLen;
    }

    rv = SYNTH_OK;
__err:
    return rv;
}


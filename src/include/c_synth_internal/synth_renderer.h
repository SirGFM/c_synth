/**
 * @file src/include/c_synth_internal/synth_renderer.h
 */
#ifndef __SYNTH_INTERNAL_RENDERER_H__
#define __SYNTH_INTERNAL_RENDERER_H__

/**
 * Initialize the renderer for a given audio
 *
 * @param  [ in]pCtx      The renderer context
 * @param  [ in]pAudio    The audio to be rendered
 * @param  [ in]frequency The synth frequency in samples per second
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthRenderer_init(synthRendererCtx *pCtx, synthAudio *pAudio,
        int frequency);

/**
 * Returns the position back to the start but don't modify anything related to
 * audio
 *
 * @param  [ in]pCtx The renderer context
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthRenderer_resetPosition(synthRendererCtx *pCtx);

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
        synthRendererCtx *pCtx, synthNote *pNote);

#endif /* __SYNTH_INTERNAL_RENDERER_H__ */


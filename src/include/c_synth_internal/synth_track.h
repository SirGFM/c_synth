/**
 * A sequence of notes
 * 
 * @file src/include/synth_internal/synth_track.h
 */
#ifndef __SYNTH_TRACK_H__
#define __SYNTH_TRACK_H__

#include <synth/synth.h>
#include <synth/synth_errors.h>

#include <synth_internal/synth_types.h>

/**
 * Retrieve a new track (alloc it as possible and necessary)
 * 
 * @param  [out]ppTrack The new track
 * @param  [ in]pCtx    The synthesizer context
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthTrack_init(synthTrack **ppTrack, synthCtx *pCtx);

/**
 * Retrieve the number of samples in a track
 * 
 * @param  [out]pLen   The length of the track in samples
 * @param  [ in]pTrack The track
 * @param  [ in]pCtx   The synthesizer context
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthTrack_getLength(int *pLen, synthTrack *pTrack, synthCtx *pCtx);

/**
 * Retrieve the number of samples until a track's loop point
 * 
 * @param  [out]pLen   The length of the track's intro
 * @param  [ in]pTrack The track
 * @param  [ in]pCtx   The synthesizer context
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthTrack_getIntroLength(int *pLen, synthTrack *pTrack,
        synthCtx *pCtx);

/**
 * Retrieve whether a track is loopable or not
 * 
 * @param  [ in]pTrack The track
 * @return             SYNTH_TRUE, SYNTH_FALSE
 */
synth_bool synthTrack_isLoopable(synthTrack *pTrack);

/**
 * Render a track into a buffer
 * 
 * The buffer must be prepared by the caller, and it must have
 * 'synth_getTrackLength' bytes times the number of bytes per samples
 * 
 * @param  [ in]pBuf   Buffer that will be filled with the track
 * @param  [ in]pTrack The track
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]mode   Desired mode for the wave
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthTrack_render(char *pBuf, synthTrack *pTrack, synthCtx *pCtx,
        synthBufMode mode);

#endif /* __SYNTH_TRACK_H__ */


/**
 * @file src/include/synth_internal/synth_audio.h
 */
#ifndef __SYNTH_INTERNAL_AUDIO_H_
#define __SYNTH_INTERNAL_AUDIO_H_

#include <synth/synth.h>
#include <synth/synth_errors.h>

#include <synth_internal/synth_types.h>

/**
 * Initialize a new audio, so a song can be compiled into it
 * 
 * @param  [out]pAudio Object that will be filled with the compiled song
 * @param  [ in]pCtx   The synthesizer context
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthAudio_init(synthAudio **ppAudio, synthCtx *pCtx);

/**
 * Compile a MML audio file into a object
 * 
 * @param  [ in]pAudio    Object that will be filled with the compiled song
 * @param  [ in]pCtx      The synthesizer context
 * @param  [ in]pFilename File with the song's MML
 */
synth_err synthAudio_compileFile(synthAudio *pAudio, synthCtx *pCtx,
        char *pFilename);

/**
 * Compile a MML audio string into a object
 * 
 * @param  [ in]pAudio  Object that will be filled with the compiled song
 * @param  [ in]pCtx    The synthesizer context
 * @param  [ in]pString The MML song
 * @param  [ in]len     The MML song's length
 */
synth_err synthAudio_compileString(synthAudio *pAudio, synthCtx *pCtx,
        char *pString, int len);

/**
 * Return the number of tracks in a song
 * 
 * @param  [out]pNum   The number of tracks
 * @param  [ in]pAudio The audio
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthAudio_getTrackCount(int *pNum, synthAudio *pAudio);

/**
 * Retrieve the number of samples in a track
 * 
 * @param  [out]pLen   The length of the track in samples
 * @param  [ in]pAudio The audio
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]track  Track index
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_INVALID_INDEX
 */
synth_err synthAudio_getTrackLength(int *pLen, synthAudio *pAudio,
        synthCtx *pCtx, int track);

/**
 * Retrieve the number of samples until a track's loop point
 * 
 * @param  [out]pLen   The length of the track's intro
 * @param  [ in]pAudio The audio
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]track  The track
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthAudio_getTrackIntroLength(int *pLen, synthAudio *pAudio,
        synthCtx *pCtx, int track);

/**
 * Retrieve whether a track is loopable or not
 * 
 * @param  [ in]pAudio The audio
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]track  The track
 * @return             SYNTH_TRUE, SYNTH_FALSE
 */
synth_bool synthAudio_isTrackLoopable(synthAudio *pAudio, synthCtx *pCtx,
        int track);

/**
 * Render a track into a buffer
 * 
 * The buffer must be prepared by the caller, and it must have
 * 'synth_getTrackLength' bytes times the number of bytes per samples
 * 
 * @param  [ in]pBuf   Buffer that will be filled with the track
 * @param  [ in]pAudio The audio
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]pTrack The track
 * @param  [ in]mode   Desired mode for the wave
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthAudio_renderTrack(char *pBuf, synthAudio *pAudio, synthCtx *pCtx,
        int track, synthBufMode mode);

#if 0

#include <synth/synth_audio.h>
#include <synth_internal/synth_track.h>


/**
 * Alloc a new audio structure and initialize it
 * 
 * @param audio Reference to the audio pointer
 * @return Error code
 */
synth_err synth_audio_allocAudio(synthAudio **audio);
/**
 * Adds a new track to the audio
 * Though the track will be released automatically, its notes and volume won't
 * 
 * @param audio The audio
 * @param track The track
 * @return Error code
 */
synth_err synth_audio_addTrack(synthAudio *audio, synthTrack *track);

/**
 * Synthesize part of this audio to a buffer
 * 
 * @param audio Audio to be synthesized
 * @param samples How many samples there still are in this "slice"
 * @param left Left output buffer
 * @param right Right output buffer
 */
void synth_audio_synthesize(synthAudio *aud, int samples, uint16_t *left,
    uint16_t *right);

/**
 * Reset an audio
 * 
 * @param The audio
 */
void synth_audio_reset(synthAudio *aud);

#endif /* 0 */

#endif


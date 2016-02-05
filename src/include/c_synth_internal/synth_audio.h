/**
 * @file src/include/c_synth_internal/synth_audio.h
 */
#ifndef __SYNTH_INTERNAL_AUDIO_H__
#define __SYNTH_INTERNAL_AUDIO_H__

#include <c_synth/synth.h>
#include <c_synth/synth_errors.h>

#include <c_synth_internal/synth_types.h>

/**
 * Initialize a new audio, so a song can be compiled into it
 * 
 * @param  [out]pAudio Object that will be filled with the compiled song
 * @param  [ in]pCtx   The synthesizer context
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthAudio_init(synthAudio **ppAudio, synthCtx *pCtx);

#if defined(USE_SDL2)
/**
 * Compile a MML audio SDL_RWops into an object
 * 
 * @param  [ in]pAudio Object that will be filled with the compiled song
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]pFile  File with the song's MML
 */
synth_err synthAudio_compileSDL_RWops(synthAudio *pAudio, synthCtx *pCtx,
        void *pFile);
#endif

/**
 * Compile a MML audio file into an object
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
 * Return the audio BPM
 * 
 * @param  [out]pBpm   The BPM
 * @param  [ in]pAudio The audio
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthAudio_getBpm(int *pBpm, synthAudio *pAudio);

/**
 * Return the audio time signature
 * 
 * @param  [out]pTime  The time signature
 * @param  [ in]pAudio The audio
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthAudio_getTimeSignature(int *pTime, synthAudio *pAudio);

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

#endif /* __SYNTH_INTERNAL_AUDIO_H__ */


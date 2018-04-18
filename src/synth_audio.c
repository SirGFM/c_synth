/**
 * @file src/synth_audio.c
 */
#include <c_synth/synth.h>
#include <c_synth/synth_assert.h>
#include <c_synth/synth_errors.h>

#include <c_synth_internal/synth_audio.h>
#include <c_synth_internal/synth_lexer.h>
#include <c_synth_internal/synth_parser.h>
#include <c_synth_internal/synth_renderer.h>
#include <c_synth_internal/synth_types.h>
#include <c_synth_internal/synth_track.h>

#include <stdlib.h>
#include <string.h>

/**
 * Initialize a new audio, so a song can be compiled into it
 * 
 * @param  [out]pAudio Object that will be filled with the compiled song
 * @param  [ in]pCtx   The synthesizer context
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthAudio_init(synthAudio **ppAudio, synthCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(ppAudio, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    /* Make sure there's enough space for another song */
    SYNTH_ASSERT_ERR(pCtx->songs.max == 0 || pCtx->songs.used < pCtx->songs.max,
            SYNTH_MEM_ERR);

    /* Retrieve the audio to be used */
    if (pCtx->songs.used >= pCtx->songs.len) {
        /* 'Double' the current buffer; Note that this will never be called if
         * the context was pre-alloc'ed, since 'max' will be set; The '+1' is
         * for the first audio, in which len will be 0 */
        pCtx->songs.buf.pAudios = (synthAudio*)realloc(pCtx->songs.buf.pAudios,
                (1 + pCtx->songs.len * 2) * sizeof(synthAudio));
        SYNTH_ASSERT_ERR(pCtx->songs.buf.pAudios, SYNTH_MEM_ERR);
        /* Clear only the new part of the buffer */
        memset(&(pCtx->songs.buf.pAudios[pCtx->songs.used]), 0x0,
                (1 + pCtx->songs.len) * sizeof(synthAudio));
        /* Actually increase the buffer length */
        pCtx->songs.len += 1 + pCtx->songs.len;
    }

    /* Set the default BPM */
    pCtx->songs.buf.pAudios[pCtx->songs.used].bpm = 60;
    /* Set the time signature to a whole note ('brevissima'); This should work
     * for any simple time signature (1/4, 2/4, 4/4 etc) */
    pCtx->songs.buf.pAudios[pCtx->songs.used].timeSignature = 1 << 6;

    *ppAudio = &(pCtx->songs.buf.pAudios[pCtx->songs.used]);
    pCtx->songs.used++;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Compile a MML audio SDL_RWops into an object
 * 
 * @param  [ in]pAudio Object that will be filled with the compiled song
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]pFile  File with the song's MML
 */
synth_err synthAudio_compileSDL_RWops(synthAudio *pAudio, synthCtx *pCtx,
        void *pFile) {
#if defined(USE_SDL2)
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pAudio, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pFile, SYNTH_BAD_PARAM_ERR);

    /* Clear the audio */
    memset(pAudio, 0x0, sizeof(synthAudio));
    pAudio->useNewEnvelope = SYNTH_FALSE;

    /* Init parser */
    rv = synthLexer_initFromSDL_RWops(&(pCtx->lexCtx), pFile);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    rv = synthParser_init(&(pCtx->parserCtx), pCtx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Parse the audio */
    rv = synthParser_getAudio(&(pCtx->parserCtx), pCtx, pAudio);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    /* Clear the lexer, so any open file is closed */
    synthLexer_clear(&(pCtx->lexCtx));

    return rv;
#else
    return SYNTH_FUNCTION_NOT_IMPLEMENTED;
#endif
}

/**
 * Compile a MML audio file into an object
 * 
 * @param  [ in]pAudio    Object that will be filled with the compiled song
 * @param  [ in]pCtx      The synthesizer context
 * @param  [ in]pFilename File with the song's MML
 */
synth_err synthAudio_compileFile(synthAudio *pAudio, synthCtx *pCtx,
        char *pFilename) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pAudio, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pFilename, SYNTH_BAD_PARAM_ERR);
    /* The file is checked for existance before, so no need to do it again */

    /* Clear the audio */
    memset(pAudio, 0x0, sizeof(synthAudio));
    pAudio->useNewEnvelope = SYNTH_FALSE;

    /* Init parser */
    rv = synthLexer_initFromFile(&(pCtx->lexCtx), pFilename);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    rv = synthParser_init(&(pCtx->parserCtx), pCtx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Parse the audio */
    rv = synthParser_getAudio(&(pCtx->parserCtx), pCtx, pAudio);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    /* Clear the lexer, so any open file is closed */
    synthLexer_clear(&(pCtx->lexCtx));

    return rv;
}

/**
 * Compile a MML audio string into a object
 * 
 * @param  [ in]pAudio  Object that will be filled with the compiled song
 * @param  [ in]pCtx    The synthesizer context
 * @param  [ in]pString The MML song
 * @param  [ in]len     The MML song's length
 */
synth_err synthAudio_compileString(synthAudio *pAudio, synthCtx *pCtx,
        char *pString, int len) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pAudio, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pString, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(len > 0, SYNTH_BAD_PARAM_ERR);

    /* Clear the audio */
    memset(pAudio, 0x0, sizeof(synthAudio));
    pAudio->useNewEnvelope = SYNTH_FALSE;

    /* Init parser */
    rv = synthLexer_initFromString(&(pCtx->lexCtx), pString, len);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    rv = synthParser_init(&(pCtx->parserCtx), pCtx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Parse the audio */
    rv = synthParser_getAudio(&(pCtx->parserCtx), pCtx, pAudio);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:

    return rv;
}

/**
 * Return the audio BPM
 * 
 * @param  [out]pBpm   The BPM
 * @param  [ in]pAudio The audio
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthAudio_getBpm(int *pBpm, synthAudio *pAudio) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pBpm, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pAudio, SYNTH_BAD_PARAM_ERR);

    *pBpm = pAudio->bpm;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Return the audio time signature
 * 
 * @param  [out]pTime  The time signature
 * @param  [ in]pAudio The audio
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthAudio_getTimeSignature(int *pTime, synthAudio *pAudio) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pTime, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pAudio, SYNTH_BAD_PARAM_ERR);

    *pTime = pAudio->timeSignature;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Return the number of tracks in a song
 * 
 * @param  [out]pNum   The number of tracks
 * @param  [ in]pAudio The audio
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthAudio_getTrackCount(int *pNum, synthAudio *pAudio) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pNum, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pAudio, SYNTH_BAD_PARAM_ERR);

    *pNum = pAudio->num;

    rv = SYNTH_OK;
__err:
    return rv;
}

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
        synthCtx *pCtx, int track) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pLen, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pAudio, SYNTH_BAD_PARAM_ERR);
    /* Check that the track is valid */
    SYNTH_ASSERT_ERR(track < pAudio->num, SYNTH_INVALID_INDEX);

    /* Make sure the renderer is at a compass start */
    rv = synthRenderer_resetPosition(&(pCtx->renderCtx));
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = synthTrack_getLength(pLen,
            &(pCtx->tracks.buf.pTracks[pAudio->tracksIndex + track]), pCtx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    return rv;
}

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
        synthCtx *pCtx, int track) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pLen, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pAudio, SYNTH_BAD_PARAM_ERR);
    /* Check that the track is valid */
    SYNTH_ASSERT_ERR(track < pAudio->num, SYNTH_INVALID_INDEX);

    /* Make sure the renderer is at a compass start */
    rv = synthRenderer_resetPosition(&(pCtx->renderCtx));
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = synthTrack_getIntroLength(pLen,
            &(pCtx->tracks.buf.pTracks[pAudio->tracksIndex + track]), pCtx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Retrieve whether a track is loopable or not
 * 
 * @param  [ in]pAudio The audio
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]track  The track
 * @return             SYNTH_TRUE, SYNTH_FALSE
 */
synth_bool synthAudio_isTrackLoopable(synthAudio *pAudio, synthCtx *pCtx,
        int track) {
    if (pAudio && track < pAudio->num) {
        return synthTrack_isLoopable(
                &(pCtx->tracks.buf.pTracks[pAudio->tracksIndex + track]));
    }
    return SYNTH_FALSE;
}

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
        int track, synthBufMode mode) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pBuf, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pAudio, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    /* Check that the track is valid */
    SYNTH_ASSERT_ERR(track < pAudio->num, SYNTH_INVALID_INDEX);

    /* Make sure the renderer is at a compass start */
    rv = synthRenderer_resetPosition(&(pCtx->renderCtx));
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = synthTrack_render(pBuf,
            &(pCtx->tracks.buf.pTracks[pAudio->tracksIndex + track]), pCtx,
            mode);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    return rv;
}


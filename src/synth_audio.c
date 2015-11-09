/**
 * @file src/synth_audio.c
 */
#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>

#include <synth_internal/synth_audio.h>
#include <synth_internal/synth_lexer.h>
#include <synth_internal/synth_parser.h>
#include <synth_internal/synth_types.h>
#include <synth_internal/synth_track.h>

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

    *ppAudio = &(pCtx->songs.buf.pAudios[pCtx->songs.used]);
    pCtx->songs.used++;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Compile a MML audio file into a object
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

    rv = synthTrack_getLength(pLen, &(pCtx->tracks.buf.pTracks[track]), pCtx);
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

    rv = synthTrack_getIntroLength(pLen, &(pCtx->tracks.buf.pTracks[track]),
            pCtx);
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
        return synthTrack_isLoopable(&(pCtx->tracks.buf.pTracks[track]));
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

    rv = synthTrack_render(pBuf, &(pCtx->tracks.buf.pTracks[track]), pCtx,
            mode);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    return rv;
}

#if 0

// TODO feature: per-track volume

/**
 * Parse a mml file into an audio
 * 
 * @param ctx Variable that will store the audio
 * @param filename MML's filename
 * @return The error code
 */
synth_err synth_audio_loadf(synthAudio **audio, char *filename) {
    synth_err rv;
    synthParserCtx *ctx = 0;
    
    // Sanitize input
    SYNTH_ASSERT_ERR(audio, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(filename, SYNTH_BAD_PARAM_ERR);
    
    // Init parsing
    rv = synth_parser_initParsef(&ctx, filename);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Parse the stream
    rv = synth_parser_audio(ctx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Get the generated audio
    *audio = synth_parser_getAudio(ctx);
    
    rv = SYNTH_OK;
__err:
    // Clean up memory
    synth_parser_clean(&ctx);
    
    return rv;
}

/**
 * Parse a mml string into an audio
 * 
 * @param ctx Variable that will store the audio
 * @param mml String with the mml audio (it needn't be null-terminated)
 * @param len String's length
 * @return The error code
 */
synth_err synth_audio_loads(synthAudio **audio, char *mml, int len) {
    synth_err rv;
    synthParserCtx *ctx = 0;
    
    // Sanitize input
    SYNTH_ASSERT_ERR(audio, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(mml, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(len > 0, SYNTH_BAD_PARAM_ERR);
    
    // Init parsing
    rv = synth_parser_initParses(&ctx, mml, len);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Parse the stream
    rv = synth_parser_audio(ctx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Get the generated audio
    *audio = synth_parser_getAudio(ctx);
    
    rv = SYNTH_OK;
__err:
    // Clean up memory
    synth_parser_clean(&ctx);
    
    return rv;
}

/**
 * Free up an audio structure
 * 
 * @param audio Reference to the audio pointer
 */
void synth_audio_free(synthAudio **audio) {
    int i;
    synthAudio *tmp;
    
    // Check if there's anything to be freed
    SYNTH_ASSERT(audio);
    SYNTH_ASSERT(*audio);
    
    tmp = *audio;
    
    if (tmp->tracks) {
        i = 0;
        // Release every track
        while (i < tmp->len) {
            if (tmp->tracks[i]) {
                synth_track_clean(tmp->tracks[i]);
                free(tmp->tracks[i]);
            }
            i++;
        }
        
        // Release the tracks array
        free(tmp->tracks);
    }
    
    // Release the audio structure
    free(tmp);
    *audio = 0;
    
__err:
    return;
}

/**
 * Reset and play an audio. The audio shouldn't be played, otherwise the
 * synthesizer will have unexpected behaviour
 * 
 * @param audio The audio
 * @return The error code
 */
synth_err synth_audio_playAudio(synthAudio *audio) {
    return synth_list_addAudio(audio);
}

/**
 * Play a single track audio. Different from playAudio, this can be called with
 * an already playing audio
 * 
 * @param audio The audio
 * @return The error code
 */
synth_err synth_audio_playSFX(synthAudio *audio) {
    return synth_list_addSfx(audio);
}

/**
 * Reset and play an audio as a bgm
 * 
 * @param audio The audio
 */
void synth_audio_playBGM(synthAudio *audio) {
    synth_list_setBgm(audio);
}

/**
 * Stop the currently playing bgm
 */
void synth_audio_stopBgm() {
    synth_list_stopBgm();
}

/**
 * Alloc a new audio structure and initialize it
 * 
 * @param audio Reference to the audio pointer
 * @return Error code
 */
synth_err synth_audio_allocAudio(synthAudio **audio) {
    synth_err rv;
    synthAudio *tmp;
    
    // Alloc the audio structure
    tmp = (synthAudio*)malloc(sizeof(synthAudio));
    SYNTH_ASSERT_ERR(tmp, SYNTH_MEM_ERR);
    
    // Alloc the tracks array
    tmp->tracks = (synthTrack**)malloc(sizeof(synthTrack*));
    SYNTH_ASSERT_ERR(tmp->tracks, SYNTH_MEM_ERR);
    
    tmp->tracksLen = 1;
    tmp->len = 0;
    
    // Set the return variables
    *audio = tmp;
    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK) {
        synth_audio_free(&tmp);
    }
    return rv;
}

/**
 * Adds a new track to the audio
 * Though the track will be released automatically, its notes and volume won't
 * 
 * @param audio The audio
 * @param track The track
 * @return Error code
 */
synth_err synth_audio_addTrack(synthAudio *audio, synthTrack *track) {
    synth_err rv;
    
    // Check if the buffer should be expanded
    if (audio->len >= audio->tracksLen) {
        int size;
        
        // Alloc double its current size
        size = audio->tracksLen*2;
        audio->tracks = realloc(audio->tracks, sizeof(synthTrack*)*size);
        SYNTH_ASSERT_ERR(audio->tracks, SYNTH_MEM_ERR);
        
        audio->tracksLen = size;
    }
    
    // 'Append' the track
    audio->tracks[audio->len] = track;
    audio->len++;
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Synthesize part of this audio to a buffer
 * 
 * @param audio Audio to be synthesized
 * @param samples How many samples there still are in this "slice"
 * @param left Left output buffer
 * @param right Right output buffer
 */
void synth_audio_synthesize(synthAudio *aud, int samples, uint16_t *left,
    uint16_t *right) {
    int i;
    
    i = 0;
    while (i < aud->len) {
        synthTrack *track;
        
        track = aud->tracks[i];
        synth_track_synthesize(track, samples, left, right);
        
        i++;
    }
}

/**
 * Reset an audio
 * 
 * @param The audio
 */
void synth_audio_reset(synthAudio *aud) {
    int i;
    
    i = 0;
    while (i < aud->len) {
        synth_track_reset(aud->tracks[i]);
        i++;
    }
}

/**
 * Whether the audio finished playing
 * 
 * @param audio The audio
 * @return Error code
 */
synth_bool synth_audio_didFinish(synthAudio *audio) {
    synth_bool rv;
    int i;
    
    rv = SYNTH_FALSE;
    
    // Check that every track finished playing
    i = 0;
    while (i < audio->len) {
        SYNTH_ASSERT(synth_track_didFinish(audio->tracks[i]) == SYNTH_TRUE);
        i++;
    }
    
    rv = SYNTH_TRUE;
__err:
    return rv;
}

#endif /* 0 */


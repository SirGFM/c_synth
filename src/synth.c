/**
 * @file src/synth.c
 */
#include <c_synth/synth.h>
#include <c_synth/synth_assert.h>
#include <c_synth/synth_errors.h>

#include <c_synth_internal/synth_audio.h>
#include <c_synth_internal/synth_lexer.h>
#include <c_synth_internal/synth_parser.h>
#include <c_synth_internal/synth_prng.h>
#include <c_synth_internal/synth_types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * Retrieve the total size for a context
 * 
 * This allows an application to alloc it however it wants; In memory constraint
 * environments, it might be desired not to use dynamic memory, so this function
 * call can determined how much memory would be required for a context with some
 * restrictions
 * 
 * @param  [out]pSize      The size of the context struct in bytes
 * @param  [ in]maxSongs   How many songs can be compiled at the same time
 * @param  [ in]maxTracks  How many tracks can be used through all songs
 * @param  [ in]maxNotes   How many notes can be used through all tracks
 * @param  [ in]maxVolumes How many volumes can be used through all tracks
 * @return                 SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synth_getStaticContextSize(int *pSize, int maxSongs, int maxTracks,
        int maxNotes, int maxVolumes) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pSize, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(maxNotes > 0, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(maxSongs > 0, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(maxTracks > 0, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(maxVolumes > 0, SYNTH_BAD_PARAM_ERR);

    /* Retrieve the struct size */
    *pSize = (int)sizeof(synthCtx);
    /* Increase it for each object used (in a list) */
    *pSize += (int)(sizeof(synthAudio) * maxSongs);
    *pSize += (int)(sizeof(synthTrack) * maxTracks);
    *pSize += (int)(sizeof(synthNote) * maxNotes);
    *pSize += (int)(sizeof(synthVolume) * maxVolumes);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Check how many bytes the context is currently using
 * 
 * @param  [out]pSize The size of the context struct in bytes
 * @param  [ in]pCtx  The synthesizer context
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synth_getContextSize(int *pSize, synthCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pSize, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Retrieve the struct size */
    *pSize = (int)sizeof(synthCtx);
    /* Increase it for each object used (in a list) */
    *pSize += (int)(sizeof(synthAudio) * pCtx->songs.len);
    *pSize += (int)(sizeof(synthTrack) * pCtx->tracks.len);
    *pSize += (int)(sizeof(synthNote) * pCtx->notes.len);
    *pSize += (int)(sizeof(synthVolume) * pCtx->volumes.len);

    /* TODO Ensure no object is missing!! */

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Initialize the synthesizer context from a previously alloc'ed memory from the
 * user
 * 
 * This function call initializes a context that enforces restrictions over the
 * use of memory by the synthesizer; It won't alloc no extra memory, so it's
 * highly advised that the synthesizer is first tested using its dynamic
 * version, so the required memory to whatever is desired is calculated, before
 * trying to use this mode;
 * 
 * @param  [out]ppCtx      The new synthesizer context
 * @param  [ in]pMem       'synth_getContextSize' bytes or NULL, if the library
 *                         should alloc the structure however it wants
 * @param  [ in]freq       Synthesizer frequency, in samples per seconds
 * @param  [ in]maxSongs   How many songs can be compiled at the same time
 * @param  [ in]maxTracks  How many tracks can be used through all songs
 * @param  [ in]maxNotes   How many notes can be used through all tracks
 * @param  [ in]maxVolumes How many volumes can be used through all tracks
 * @return                 SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synth_initStatic(synthCtx **ppCtx, void *pMem, int freq, int maxSongs,
        int maxTracks, int maxNotes, int maxVolumes) {
    /* TODO Implement the function 'synth_initStatic' */
    return SYNTH_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Alloc and initialize the synthesizer
 * 
 * @param  [out]ppCtx    The new synthesizer context
 * @param  [ in]freq     Synthesizer frequency, in samples per seconds
 * @return               SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synth_init(synthCtx **ppCtx, int freq) {
    synthCtx *pCtx;
    synth_err rv;

    /* Initialize this with NULL so it can be cleaned on error */
    pCtx = 0;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(ppCtx, SYNTH_BAD_PARAM_ERR);

    /* Alloc and initialize the context */
    pCtx = (synthCtx*)malloc(sizeof(synthCtx));
    SYNTH_ASSERT_ERR(pCtx, SYNTH_MEM_ERR);
    memset(pCtx, 0x0, sizeof(synthCtx));

    /* Set it as being dynamically alloc'ed */
    pCtx->autoAlloced = 1;
    /* Set the synthesizer frequency */
    pCtx->frequency = freq;
    /* Initialize the prng */
    rv = synthPRNG_init(&(pCtx->prngCtx), (unsigned int)time(0));
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    /* TODO Initialize anything else? */

    /* Set the return */
    *ppCtx = pCtx;
    rv = SYNTH_OK;
    /* Make sure the context isn't cleared */
    pCtx = 0;
__err:
    if (pCtx) {
        /* Free the context */
        synth_free(&pCtx);
    }

    return rv;
}

/**
 * Release any of the submodules in use and then release any other memory
 * alloc'ed by the library
 * 
 * @param  [ in]ppCtx The synthesizer context that will be dealloc'ed
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synth_free(synthCtx **ppCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(ppCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(*ppCtx, SYNTH_BAD_PARAM_ERR);

    /* This must be done either way, since any open file must be manually
     * closed */
    synthLexer_clear(&((*ppCtx)->lexCtx));

    /* Check that it was dynamic alloc'ed */
    if (!((*ppCtx)->autoAlloced)) {
        *ppCtx = 0;
        rv = SYNTH_OK;
        goto __err;
    }

    /* Dealloc the struct itself */
    if ((*ppCtx)->songs.buf.pAudios) {
        free((*ppCtx)->songs.buf.pAudios);
    }
    if ((*ppCtx)->tracks.buf.pTracks) {
        free((*ppCtx)->tracks.buf.pTracks);
    }
    if ((*ppCtx)->notes.buf.pNotes) {
        free((*ppCtx)->notes.buf.pNotes);
    }
    if ((*ppCtx)->volumes.buf.pVolumes) {
        free((*ppCtx)->volumes.buf.pVolumes);
    }
    (*ppCtx)->songs.buf.pAudios = 0;
    (*ppCtx)->tracks.buf.pTracks = 0;
    (*ppCtx)->notes.buf.pNotes = 0;
    (*ppCtx)->volumes.buf.pVolumes = 0;

    /* Finally, dealloc the struct itself */
    free(*ppCtx);
    *ppCtx = 0;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Parse a file into a compiled song
 * 
 * The compiled song can later be used to playback the audio, get its samples
 * (i.e., buffer the whole song) or to export it to WAVE or OGG
 * 
 * @param  [out]pHandle   Handle of the loaded song
 * @param  [ in]pCtx      The synthesizer context
 * @param  [ in]pFilename File with the song's MML
 * @param                 SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR,
 *                        SYNTH_OPEN_FILE_ERR
 */
synth_err synth_compileSongFromFile(int *pHandle, synthCtx *pCtx,
        char *pFilename) {
    synthAudio *pAudio;
    synth_err rv;

    /* TODO Store the previous buffer sizes so we can clean it on error */

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pHandle, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pFilename, SYNTH_BAD_PARAM_ERR);
    /* TODO Check that the filename is valid? (i.e., actually \0-terminated?) */

    /* Check that the file exists */
    do {
        FILE *pFp;

        pFp = fopen(pFilename, "rt");
        SYNTH_ASSERT_ERR(pFp, SYNTH_OPEN_FILE_ERR);
        fclose(pFp);
    } while (0);

    /* Retrieve the new audio */
    rv = synthAudio_init(&pAudio, pCtx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    /* Compile the song */
    rv = synthAudio_compileFile(pAudio, pCtx, pFilename);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Return the newly compiled song */
    *pHandle = pCtx->songs.used - 1;
    /* 'Push' the audio into the buffer */
    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK) {
        /* TODO Clear the newly used objects */
    }

    return rv;
}

/**
 * Parse a string into a compiled song
 * 
 * The compiled song can later be used to playback the audio, get its samples
 * (i.e., buffer the whole song) or to export it to WAVE or OGG
 * 
 * @param  [out]pHandle Handle of the loaded song
 * @param  [ in]pCtx    The synthesizer context
 * @param  [ in]pString Song's MML
 * @param  [ in]length  The string's length
 * @param               SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR, ...
 */
synth_err synth_compileSongFromString(int *pHandle, synthCtx *pCtx,
        char *pString, int length) {
    synthAudio *pAudio;
    synth_err rv;

    /* TODO Store the previous buffer sizes so we can clean it on error */

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pHandle, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pString, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(length, SYNTH_BAD_PARAM_ERR);
    /* TODO Check that the filename is valid? (i.e., actually \0-terminated?) */

    /* Retrieve the new audio */
    rv = synthAudio_init(&pAudio, pCtx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    /* Compile the song */
    rv = synthAudio_compileString(pAudio, pCtx, pString, length);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Return the newly compiled song */
    *pHandle = pCtx->songs.used - 1;
    /* 'Push' the audio into the buffer */
    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK) {
        /* TODO Clear the newly used objects */
    }

    return rv;
}

/**
 * Return a string representing the compiler error raised
 * 
 * This string is statically allocated and mustn't be freed by user
 * 
 * @param  [out]ppError The error string
 * @param  [ in]pCtx    The synthesizer context
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_NO_ERRORS
 */
synth_err synth_getCompilerErrorString(char **ppError, synthCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(ppError, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    rv = synthParser_getErrorString(ppError, &(pCtx->parserCtx), pCtx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Return the number of tracks in a song
 * 
 * @param  [out]pNum   The number of tracks
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]handle Handle of the audio
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_INVALID_INDEX
 */
synth_err synth_getAudioTrackCount(int *pNum, synthCtx *pCtx, int handle) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pNum, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    /* Check that the handle is valid */
    SYNTH_ASSERT_ERR(handle < pCtx->songs.used, SYNTH_INVALID_INDEX);

    rv = synthAudio_getTrackCount(pNum, &(pCtx->songs.buf.pAudios[handle]));
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Retrieve the number of samples in a track
 * 
 * @param  [out]pLen   The length of the track in samples
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]handle Handle of the audio
 * @param  [ in]track  Track index
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_INVALID_INDEX
 */
synth_err synth_getTrackLength(int *pLen, synthCtx *pCtx, int handle,
        int track) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pLen, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    /* Check that the handle is valid */
    SYNTH_ASSERT_ERR(handle < pCtx->songs.used, SYNTH_INVALID_INDEX);

    rv = synthAudio_getTrackLength(pLen, &(pCtx->songs.buf.pAudios[handle]),
            pCtx, track);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Retrieve the number of samples until a track's loop point
 * 
 * @param  [out]pLen   The length of the track's intro
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]handle Handle of the audio
 * @param  [ in]track  The track
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synth_getTrackIntroLength(int *pLen, synthCtx *pCtx, int handle,
        int track) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pLen, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    /* Check that the handle is valid */
    SYNTH_ASSERT_ERR(handle < pCtx->songs.used, SYNTH_INVALID_INDEX);

    rv = synthAudio_getTrackIntroLength(pLen,
            &(pCtx->songs.buf.pAudios[handle]), pCtx, track);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Check whether a track is loopable
 * 
 * @param  [out]pVal   1 if it does loop, 0 otherwise
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]handle Handle of the audio
 * @param  [ in]pTrack The track
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_INVALID_INDEX
 */
synth_err synth_isTrackLoopable(int *pVal, synthCtx *pCtx, int handle,
        int track) {
    synth_bool brv;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pVal, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    /* Check that the handle is valid */
    SYNTH_ASSERT_ERR(handle < pCtx->songs.used, SYNTH_INVALID_INDEX);

    brv = synthAudio_isTrackLoopable(&(pCtx->songs.buf.pAudios[handle]), pCtx,
            track);
    *pVal = (brv == SYNTH_TRUE);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Render a track into a buffer
 * 
 * The buffer must be prepared by the caller, and it must have
 * 'synth_getTrackLength' bytes times the number of bytes per samples
 * 
 * @param  [ in]pBuf   Buffer that will be filled with the track
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]handle Handle of the audio
 * @param  [ in]pTrack The track
 * @param  [ in]mode   Desired mode for the wave
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synth_renderTrack(char *pBuf, synthCtx *pCtx, int handle, int track,
        synthBufMode mode) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pBuf, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR((mode & SYNTH_VALID_MODE_MASK) != 0, SYNTH_BAD_PARAM_ERR);
    /* Check that the handle is valid */
    SYNTH_ASSERT_ERR(handle < pCtx->songs.used, SYNTH_INVALID_INDEX);

    rv = synthAudio_renderTrack(pBuf, &(pCtx->songs.buf.pAudios[handle]),
            pCtx, track, mode);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Check whether a song can loop nicely in a single iteration
 * 
 * @param  [out]pLen   The length of the track's intro
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]handle Handle of the audio
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_INVALID_INDEX,
 *                     SYNTH_COMPLEX_LOOPPOINT, SYNTH_NOT_LOOPABLE
 */
synth_err synth_canSongLoop(synthCtx *pCtx, int handle) {
    int i, maxLen, maxLoopPoint, numTracks;
    synthAudio *pAudio;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(handle >= 0, SYNTH_BAD_PARAM_ERR);
    /* Check if the song is valid */
    SYNTH_ASSERT_ERR(handle < pCtx->songs.used, SYNTH_INVALID_INDEX);

    /* Retrieve the audio */
    pAudio = &(pCtx->songs.buf.pAudios[handle]);

    /* Count how many tracks there are */
    rv = synthAudio_getTrackCount(&numTracks, pAudio);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Check if at least one of the tracks are loopable */
    i = 0;
    while (i < numTracks) {
        if (synthAudio_isTrackLoopable(pAudio, pCtx, i) == SYNTH_TRUE) {
            break;
        }

        i++;
    }
    SYNTH_ASSERT_ERR(i < numTracks, SYNTH_NOT_LOOPABLE);

    /* Search the longest length and loop point */
    i = 0;
    maxLen = 0;
    maxLoopPoint = 0;
    while (i < numTracks) {
        int len, loopPoint, track;

        track = i;
        i++;

        /* Move on to the next track if it isn't loop-able */
        if (synthAudio_isTrackLoopable(pAudio, pCtx, track) == SYNTH_FALSE) {
            continue;
        }

        /* Retrieve the current track length and loop point */
        rv = synthAudio_getTrackLength(&len, pAudio, pCtx, track);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
        rv = synthAudio_getTrackIntroLength(&loopPoint, pAudio, pCtx, track);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

        /* Check if this new one is greater than the previous */
        if (len > maxLen) {
            maxLen = len;
        }
        if (loopPoint > maxLoopPoint) {
            maxLoopPoint = loopPoint;
        }
    }

    /* Iterate, again, over every track and check if their limits overlaps
     * nicelly */
    i = 0;
    while (i < numTracks) {
        int len, loopPoint, track;

        track = i;
        i++;

        /* Move on to the next track if it isn't loop-able */
        if (synthAudio_isTrackLoopable(pAudio, pCtx, track) == SYNTH_FALSE) {
            continue;
        }

        /* Retrieve the current track length and loop point */
        rv = synthAudio_getTrackLength(&len, pAudio, pCtx, track);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
        rv = synthAudio_getTrackIntroLength(&loopPoint, pAudio, pCtx, track);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

        /* Check that this track's lengths are compatible with the song's
         * lengths */
        SYNTH_ASSERT_ERR((maxLoopPoint + maxLen - loopPoint) % len == 0,
                SYNTH_COMPLEX_LOOPPOINT);
    }

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Retrieve the length, in samples, of the longest track in a song
 * 
 * The song is checked for a single iteration loop. If that's impossible, the
 * function will exit with an error
 * 
 * @param  [out]pLen   The length of the track's intro
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]handle Handle of the audio
 * @param  [ in]pTrack The track
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_INVALID_INDEX,
 *                     SYNTH_COMPLEX_LOOPPOINT
 */
synth_err synth_getSongLength(int *pLen, synthCtx *pCtx, int handle) {
    int i, maxLen, numTracks;
    synthAudio *pAudio;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pLen, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(handle >= 0, SYNTH_BAD_PARAM_ERR);
    /* Check if the song is valid */
    SYNTH_ASSERT_ERR(handle < pCtx->songs.used, SYNTH_INVALID_INDEX);
    rv = SYNTH_OK;

    /* Check that either the song doesn't loop or that it's loopable */
    rv = synth_canSongLoop(pCtx, handle);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK || rv == SYNTH_NOT_LOOPABLE, rv);

    /* Retrieve the audio */
    pAudio = &(pCtx->songs.buf.pAudios[handle]);

    /* Count how many tracks there are */
    rv = synthAudio_getTrackCount(&numTracks, pAudio);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Retrieve the longets length */
    i = 0;
    maxLen = 0;
    while (i < numTracks) {
        int len;

        rv = synthAudio_getTrackLength(&len, pAudio, pCtx, i);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

        if (len > maxLen) {
            maxLen = len;
        }

        i++;
    }

    *pLen = maxLen;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Retrieve the number of samples until a song's loop point
 * 
 * This functions expect all tracks to loop at the same point, so it will fail
 * if this isn't possible in a single iteration of the longest track
 * 
 * @param  [out]pLen   The length of the track's intro
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]handle Handle of the audio
 * @param  [ in]pTrack The track
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_INVALID_INDEX,
 *                     SYNTH_COMPLEX_LOOPPOINT, SYNTH_NOT_LOOPABLE
 */
synth_err synth_getSongIntroLength(int *pLen, synthCtx *pCtx, int handle) {
    int i, maxLoopPoint, numTracks;
    synthAudio *pAudio;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pLen, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(handle >= 0, SYNTH_BAD_PARAM_ERR);
    /* Check if the song is valid */
    SYNTH_ASSERT_ERR(handle < pCtx->songs.used, SYNTH_INVALID_INDEX);
    rv = SYNTH_OK;

    /* Check that either the song doesn't loop or that it's loopable */
    rv = synth_canSongLoop(pCtx, handle);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Retrieve the audio */
    pAudio = &(pCtx->songs.buf.pAudios[handle]);

    /* Count how many tracks there are */
    rv = synthAudio_getTrackCount(&numTracks, pAudio);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Retrieve the longets length */
    i = 0;
    maxLoopPoint = 0;
    while (i < numTracks) {
        int loopPoint, track;

        track = i;
        i++;

        /* Move on to the next track if it isn't loop-able */
        if (synthAudio_isTrackLoopable(pAudio, pCtx, track) == SYNTH_FALSE) {
            continue;
        }

        /* Retrieve the current track length and loop point */
        rv = synthAudio_getTrackIntroLength(&loopPoint, pAudio, pCtx, track);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

        if (loopPoint > maxLoopPoint) {
            maxLoopPoint = loopPoint;
        }
    }

    *pLen = maxLoopPoint;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Accumulate a temporary buffer into another buffer
 * 
 * If, at any point, a sample overflow its bit-per-sample, it will drop the
 * least significant bits
 * 
 * @param  [ in]pBuf Buffer that will be joined by the other track
 * @param  [ in]pTmp Temporary buffer with the last track
 * @param  [ in]mode Desired mode for the song
 * @oaram  [ in]len  The number of samples to be accumulated
 * @return           Whether any overflow happened
 */
static synth_bool synth_accumulateSongTrack(char *pBuf, char *pTmp,
        synthBufMode mode, int len) {
    synth_bool rv;
    int i, j;

    rv = SYNTH_FALSE;

    i = 0;
    j = 0;
    while (i < len) {
        switch (mode) {
            case SYNTH_1CHAN_U8BITS: {
                unsigned int dst, src;

                src = pTmp[j] & 0xff;
                dst = pBuf[j] & 0xff;

                dst += src;
                while (dst > 0xff) {
                    dst >>= 1;
                    rv = SYNTH_TRUE;
                }

                pBuf[j] = dst;

                j++;
            } break;
            case SYNTH_1CHAN_8BITS: {
                int dst, src;

                src = pTmp[j] & 0xff;
                dst = pBuf[j] & 0xff;

                if (src & 0x80) {
                    src |= 0xffffff00;
                }
                if (dst & 0x80) {
                    dst |= 0xffffff00;
                }

                dst += src;
                while (dst > 0xff) {
                    dst >>= 1;
                    rv = SYNTH_TRUE;
                }

                pBuf[j] = dst;

                j++;
            } break;
            case SYNTH_1CHAN_U16BITS: {
                unsigned int dst, src;

                src = (pTmp[j] & 0xff) | ((pTmp[j + 1] << 8) & 0xff00);
                dst = (pBuf[j] & 0xff) | ((pBuf[j + 1] << 8) & 0xff00);

                dst += src;
                while (dst > 0xffff) {
                    dst >>= 1;
                    rv = SYNTH_TRUE;
                }

                pBuf[j] = dst & 0xff;
                pBuf[j + 1] = (dst >> 8) & 0xff;

                j += 2;
            } break;
            case SYNTH_1CHAN_16BITS: {
                int dst, src;

                src = (pTmp[j] & 0xff) | ((pTmp[j + 1] << 8) & 0xff00);
                dst = (pBuf[j] & 0xff) | ((pBuf[j + 1] << 8) & 0xff00);

                if (src & 0x8000) {
                    src |= 0xffff0000;
                }
                if (dst & 0x8000) {
                    dst |= 0xffff0000;
                }

                dst += src;
                while (dst > 0xffff) {
                    dst >>= 1;
                    rv = SYNTH_TRUE;
                }

                pBuf[j] = dst & 0xff;
                pBuf[j + 1] = (dst >> 8) & 0xff;

                j += 2;
            } break;
            case SYNTH_2CHAN_U8BITS: {
                unsigned int l_dst, l_src, r_dst, r_src;

                l_src = pTmp[j] & 0xff;
                r_src = pTmp[j + 1] & 0xff;
                l_dst = pBuf[j] & 0xff;
                r_dst = pBuf[j + 1] & 0xff;

                l_dst += l_src;
                r_dst += r_src;
                while (l_dst > 0xff) {
                    l_dst >>= 1;
                    rv = SYNTH_TRUE;
                }
                while (r_dst > 0xff) {
                    r_dst >>= 1;
                    rv = SYNTH_TRUE;
                }

                pBuf[j] = l_dst;
                pBuf[j + 1] = r_dst;

                j += 2;
            } break;
            case SYNTH_2CHAN_8BITS: {
                int l_dst, l_src, r_dst, r_src;

                l_src = pTmp[j] & 0xff;
                r_src = pTmp[j + 1] & 0xff;
                l_dst = pBuf[j] & 0xff;
                r_dst = pBuf[j + 1] & 0xff;

                if (l_src & 0x80) {
                    l_src |= 0xffffff00;
                }
                if (r_src & 0x80) {
                    r_src |= 0xffffff00;
                }
                if (l_dst & 0x80) {
                    l_dst |= 0xffffff00;
                }
                if (r_dst & 0x80) {
                    r_dst |= 0xffffff00;
                }

                l_dst += l_src;
                r_dst += r_src;
                while (l_dst > 0xff) {
                    l_dst >>= 1;
                    rv = SYNTH_TRUE;
                }
                while (r_dst > 0xff) {
                    r_dst >>= 1;
                    rv = SYNTH_TRUE;
                }

                pBuf[j] = l_dst;
                pBuf[j + 1] = r_dst;

                j += 2;
            } break;
            case SYNTH_2CHAN_U16BITS: {
                unsigned int l_dst, l_src, r_dst, r_src;

                l_src = (pTmp[j] & 0xff) | ((pTmp[j + 1] << 8) & 0xff00);
                r_src = (pTmp[j + 2] & 0xff) | ((pTmp[j + 3] << 8) & 0xff00);
                l_dst = (pBuf[j] & 0xff) | ((pBuf[j + 1] << 8) & 0xff00);
                r_dst = (pBuf[j + 2] & 0xff) | ((pBuf[j + 3] << 8) & 0xff00);

                l_dst += l_src;
                r_dst += r_src;
                while (l_dst > 0xffff) {
                    l_dst >>= 1;
                    rv = SYNTH_TRUE;
                }
                while (r_dst > 0xffff) {
                    r_dst >>= 1;
                    rv = SYNTH_TRUE;
                }

                pBuf[j] = l_dst & 0xff;
                pBuf[j + 1] = (l_dst >> 8) & 0xff;
                pBuf[j + 2] = r_dst & 0xff;
                pBuf[j + 3] = (r_dst >> 8) & 0xff;

                j += 4;
            } break;
            case SYNTH_2CHAN_16BITS: {
                int l_dst, l_src, r_dst, r_src;

                l_src = (pTmp[j] & 0xff) | ((pTmp[j + 1] << 8) & 0xff00);
                r_src = (pTmp[j + 2] & 0xff) | ((pTmp[j + 3] << 8) & 0xff00);
                l_dst = (pBuf[j] & 0xff) | ((pBuf[j + 1] << 8) & 0xff00);
                r_dst = (pBuf[j + 2] & 0xff) | ((pBuf[j + 3] << 8) & 0xff00);

                if (l_src & 0x8000) {
                    l_src |= 0xffff0000;
                }
                if (r_src & 0x8000) {
                    r_src |= 0xffff0000;
                }
                if (l_dst & 0x8000) {
                    l_dst |= 0xffff0000;
                }
                if (r_dst & 0x8000) {
                    r_dst |= 0xffff0000;
                }

                l_dst += l_src;
                r_dst += r_src;
                while (l_dst > 0xffff) {
                    l_dst >>= 1;
                    rv = SYNTH_TRUE;
                }
                while (r_dst > 0xffff) {
                    r_dst >>= 1;
                    rv = SYNTH_TRUE;
                }

                pBuf[j] = l_dst & 0xff;
                pBuf[j + 1] = (l_dst >> 8) & 0xff;
                pBuf[j + 2] = r_dst & 0xff;
                pBuf[j + 3] = (r_dst >> 8) & 0xff;

                j += 4;
            } break;
            default : { /* Avoids warnings */ }
        }

        i++;
    }

    return rv;
}
/* TODO */

/**
 * Render all of a song's tracks and accumulate 'em in a single buffer
 * 
 * The buffer must be prepared by the caller, and it must have
 * 'synth_getSongLength' bytes times the number of bytes per samples
 * 
 * A temporary buffer is necessary in order to render each track; If the same
 * buffer were to be used, the previously rendered data would be lost (when
 * accumulating the tracks on the destination buffer), so this situation is
 * checked and is actually an error
 * 
 * @param  [ in]pBuf   Buffer that will be filled with the song
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]handle Handle of the audio
 * @param  [ in]mode   Desired mode for the song
 * @param  [ in]pTmp   Temporary buffer that will be filled with each track
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_INVALID_INDEX,
 *                     SYNTH_COMPLEX_LOOPPOINT
 */
synth_err synth_renderSong(char *pBuf, synthCtx *pCtx, int handle,
        synthBufMode mode, char *pTmp) {
    int i, numBytes, numTracks, maxLen;
    synthAudio *pAudio;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pBuf, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(handle >= 0, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR((mode & SYNTH_VALID_MODE_MASK) != 0, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pTmp, SYNTH_BAD_PARAM_ERR);
    /* Check that the handle is valid */
    SYNTH_ASSERT_ERR(handle < pCtx->songs.used, SYNTH_INVALID_INDEX);

    /* Check that the song either doesn't loop or can loop nicely */
    rv = synth_canSongLoop(pCtx, handle);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK || rv == SYNTH_NOT_LOOPABLE, rv);

    /* Retrieve the audio */
    pAudio = &(pCtx->songs.buf.pAudios[handle]);

    /* Calculate the number of bytes per samples */
    numBytes = 1;
    if (mode & SYNTH_16BITS) {
        numBytes = 2;
    }
    if (mode & SYNTH_2CHAN) {
        numBytes *= 2;
    }

    rv = synth_getSongLength(&maxLen, pCtx, handle);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Clear the output buffer so every track can be accumulated into it */
    memset(pBuf, 0x0, maxLen * numBytes);

    /* Count how many tracks there are */
    rv = synthAudio_getTrackCount(&numTracks, pAudio);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Render each track and accumulated it into the output */
    i = 0;
    while (i < numTracks) {
        int didOverflow, len;

        didOverflow = 0;

        /* Render the track into the temporary buffer */
        rv = synthAudio_renderTrack(pTmp, pAudio, pCtx, i, mode);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

        /* Accumulate the track into the buffers start */
        rv = synthAudio_getTrackLength(&len, pAudio, pCtx, i);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
        if (synth_accumulateSongTrack(pBuf, pTmp, mode, len) == SYNTH_TRUE) {
            didOverflow = 1;
        }

        /* Check whether the track loops */
        if (synthAudio_isTrackLoopable(pAudio, pCtx, i) == SYNTH_TRUE) {
            char *pDst, *pSrc;
            int loopPoint, tmpLen;

            /* Retrieve the current track length and loop point */
            rv = synthAudio_getTrackIntroLength(&loopPoint, pAudio, pCtx, i);
            SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

            /* Advance the buffer the number of bytes that were accumulated */
            pDst = pBuf + len * numBytes;

            /* Update the source to place it at the start of the loop */
            pSrc = pTmp + loopPoint * numBytes;

            /* Loop until the new track accumulated over the complete track */
            tmpLen = maxLen - len;
            len -= loopPoint;
            while (tmpLen > 0) {
                if (synth_accumulateSongTrack(pDst, pSrc, mode, len) ==
                        SYNTH_TRUE) {
                    didOverflow = 1;
                }

                pDst += len * numBytes;
                tmpLen -= len;
            }
        }

        /* If the track did overflow at any point, halve all of it */
        if (didOverflow) {
            int i;

            i = 0;
            while (i < maxLen * numBytes) {

                /* Halve the lower byte */
                pBuf[i] >>= 1;
                /* Check if an underflow may happen */
                if (mode & SYNTH_16BITS) {
                    /* Set the highest bit, if an underflow will happen */
                    pBuf[i] |= (pBuf[i + 1] & 1) << 8;
                    /* Halve the higher byte */
                    pBuf[i + 1] >>= 1;

                    /* If the number was signed and negative, set its bit */
                    if ((mode & SYNTH_SIGNED) && (pBuf[i + 1] & 0x40)) {
                        pBuf[i + 1] |= 0x80;
                    }
                    i += 2;
                }
                else {
                    /* If the number was signed and negative, set its bit */
                    if ((mode & SYNTH_SIGNED) && (pBuf[i] & 0x40)) {
                        pBuf[i] |= 0x80;
                    }
                    i++;
                }
            }
        }

        i++;
    }

    rv = SYNTH_OK;
__err:
    return rv;
}


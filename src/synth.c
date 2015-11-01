/**
 * @file src/synth.c
 */
#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>

#include <synth_internal/synth_audio.h>
#include <synth_internal/synth_lexer.h>
#include <synth_internal/synth_parser.h>
#include <synth_internal/synth_types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 0
#include <synth/synth_backend.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_audiolist.h>
#include <synth_internal/synth_buffer.h>
#include <synth_internal/synth_cache.h>
#include <synth_internal/synth_thread.h>
#endif /* 0 */

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
    (*ppCtx)->songs.buf.pAudios = 0;
    (*ppCtx)->tracks.buf.pTracks = 0;
    (*ppCtx)->notes.buf.pNotes = 0;

    /* TODO Clean everything else */

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
 * @param  [ in]pTrack The track
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

#if 0
static synth_bool synth_inited = SYNTH_FALSE;

/**
 * Initialize the synthesizer, including buffering thread and other
 * buffers
 * 
 * @param freq At which frequency (samples per minute) should synthesizer work
 * @param doBuf Whether the buffering thread should run or not
 * @param size How many samples should be buffered per channel. Must be at least
 *             synth_bkend_getSamplesPerChannel()
 * @param doBkend Whether should start the compiled backend
 * @return Error code
 */
synth_err synth_init(int freq, synth_bool doBuf, int size, synth_bool doBkend) {
    synth_err rv;
    
    SYNTH_ASSERT_ERR(synth_inited == SYNTH_FALSE, SYNTH_ALREADY_INITIALIZED);
    SYNTH_ASSERT_ERR(size >= synth_bkend_getSamplesPerChannel(),
        SYNTH_BAD_PARAM_ERR);
    
    rv = synth_cache_init(freq);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    if (doBuf == SYNTH_TRUE) {
        rv = synth_buf_init(size);
        SYNTH_ASSERT(rv == SYNTH_OK);
        
        rv = synth_list_init();
        SYNTH_ASSERT(rv == SYNTH_OK);
        
        rv = synth_thread_init();
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    
    if (doBkend == SYNTH_TRUE) {
        rv = synth_bkend_setup();
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    
    synth_inited = SYNTH_TRUE;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Clean up any allocated memory and the buffering thread
 */
synth_err synth_clean() {
    synth_err rv;
    
    SYNTH_ASSERT_ERR(synth_inited == SYNTH_TRUE, SYNTH_NOT_INITIALIZED);
    
    synth_bkend_pause();
    synth_bkend_clean();
    synth_thread_clean();
    synth_buf_clean();
    synth_list_clean();
    synth_cache_clean();
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Get the frequency being used
 * 
 * @return The frequency
 */
int synth_getFrequency() {
    return synth_cache_getFrequency();
}

/**
 * Get how many samples should be buffered
 * 
 * @return The number of samples
 */
int synth_getSamples() {
    return synth_buf_getSize();
}

/**
 * Signal the buffering thread to buffer more samples
 */
void synth_requestBuffering() {
    synth_thread_wakeupThread();
}

/**
 * Try to lock buffer, allowing it to be read. SYNTH_COULDNT_LOCK means that the
 * buffer is already in use.
 * 
 * @return Error code
 */
synth_err synth_lockBuffer() {
    return synth_thread_tryLockBuffer();
}

/**
 * Unlock the buffer
 */
void synth_unlockBuffer() {
    return synth_thread_unlockBuffer();
}

/**
 * NOT thread-safe.
 * Read some samples from the buffer. If it returns
 * SYNTH_BUFFER_NOT_ENOUGH_SAMPLES then more samples were requested than there
 * are currently buffered.
 * 
 * @param left Returns the left channel buffer
 * @param right Returns the right channel buffer
 * @param samples How many samples should be read
 * @return Error code
 */
synth_err synth_getBuffer(uint16_t **left, uint16_t **right, int samples) {
    return synth_buf_getBuffer(left, right, samples);
}
#endif /* 0 */


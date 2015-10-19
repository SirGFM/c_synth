/**
 * @file src/synth.c
 */
#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_backend.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_audiolist.h>
#include <synth_internal/synth_buffer.h>
#include <synth_internal/synth_cache.h>
#include <synth_internal/synth_thread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Union with all possible buffers (i.e., list of items) */
union unSynthBuffer {
    synthAudio *pAudios;
    synthTrack *pTracks;
    synthNote *pNotes;
};
typedef union unSynthBuffer synthBuffer;

/** A generic list of a buffer */
struct stSynthList {
    /** How many itens may this list may hold, at most */
    int max;
    /** How big is the list; Useful if there's no limit */
    int len;
    /** How many itens are currently in use */
    int used;
    /* TODO Add a map of used items? */
    /** The actual list of itens */
    synthBuffer buf;
};
typedef stSynthList synthList;

/** synthCtx struct */
struct stSynthCtx {
    /**
     * Whether the context was alloc'ed by the library (and, thus, must be
     * freed) or if it was alloc'ed by the user
     */
    int autoAlloced;
    /** Synthesizer frequency in samples per second */
    int frequency;
    /** List of songs */
    synthList songs;
    /** List of tracks */
    synthList tracks;
    /** List of notes */
    synthList notes;
};

/**
 * Retrieve the total size for a context
 * 
 * This allows an application to alloc it however it wants; In memory constraint
 * environments, it might be desired not to use dynamic memory, so this function
 * call can determined how much memory would be required for a context with some
 * restrictions
 * 
 * @param  [out]pSize     The size of the context struct in bytes
 * @param  [ in]maxSongs  How many songs can be compiled at the same time
 * @param  [ in]maxTracks How many tracks can be used through all songs
 * @param  [ in]maxNotes  How many notes can be used through all tracks
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synth_getStaticContextSize(int *pSize, int maxSongs, int maxTracks,
        int maxNotes) {
    int noteSize, songSize, trackSize;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pSize, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(maxNotes > 0, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(maxSongs > 0, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(maxTracks > 0, SYNTH_BAD_PARAM_ERR);

    /* TODO Make sure these functions exists */
    rv = synth_getNoteSize(&noteSize);
    SYNTH_ASSERT_ERR(rv, rv);
    rv = synth_getSongSize(&songSize);
    SYNTH_ASSERT_ERR(rv, rv);
    rv = synth_getTrackSize(&trackSize);
    SYNTH_ASSERT_ERR(rv, rv);

    /* Retrieve the struct size */
    *pSize = (int)sizeof(synthCtx);
    /* Increase it for each object used (in a list) */
    *pSize += (int)(songSize * maxSongs);
    *pSize += (int)(trackSize * maxTracks);
    *pSize += (int)(noteSize * maxNotes);

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
    int noteSize, songSize, trackSize;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pSize, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* TODO Make sure these functions exists */
    rv = synth_getNoteSize(&noteSize);
    SYNTH_ASSERT_ERR(rv, rv);
    rv = synth_getSongSize(&songSize);
    SYNTH_ASSERT_ERR(rv, rv);
    rv = synth_getTrackSize(&trackSize);
    SYNTH_ASSERT_ERR(rv, rv);

    /* Retrieve the struct size */
    *pSize = (int)sizeof(synthCtx);
    /* Increase it for each object used (in a list) */
    *pSize += (int)(songSize * pCtx->songs.len);
    *pSize += (int)(trackSize * pCtx->tracks.len);
    *pSize += (int)(noteSize * pCtx->notes.len);

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
 * @param  [out]ppCtx     The new synthesizer context
 * @param  [ in]pMem      'synth_getContextSize' bytes or NULL, if the library
 *                        should alloc the structure however it wants
 * @param  [ in]freq      Synthesizer frequency, in samples per seconds
 * @param  [ in]maxSongs  How many songs can be compiled at the same time
 * @param  [ in]maxTracks How many tracks can be used through all songs
 * @param  [ in]maxNotes  How many notes can be used through all tracks
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synth_initStatic(synthCtx **ppCtx, void *pMem, int freq, int maxSongs,
        int maxTracks, int maxNotes) {
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
    synthCtx *pCtx;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(ppCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(*ppCtx, SYNTH_BAD_PARAM_ERR);

    /* Check that it was dynamic alloc'ed */
    if (!((*ppCtx)->autoAlloced)) {
        *ppCtx = 0;
        rv = SYNTH_ERR_OK;
        goto __ret;
    }

    /* Dealloc the struct itself */
    if (pCtx->songs.buf.pAudios) {
        free(pCtx->songs.buf.pAudios);
    }
    if (pCtx->tracks.buf.pTracks) {
        free(pCtx->tracks.buf.pTracks);
    }
    if (pCtx->notes.buf.pNotes) {
        free(pCtx->notes.buf.pNotes);
    }
    pCtx->songs.buf.pAudios = 0;
    pCtx->tracks.buf.pTracks = 0;
    pCtx->notes.buf.pNotes = 0;

    /* TODO Clean everything else */

    /* Finally, dealloc the struct itself */
    free(*ppCtx);
    *ppCtx = 0;

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


/**
 * @file include/synth/synth.h
 */
#ifndef __SYNTH_STRUCT__
#define __SYNTH_STRUCT__

/** 'Export' the synthCtx struct */
typedef struct stSynthCtx synthCtx;

#endif /* __SYNTH_STRUCT__ */

#ifndef __SYNTH_H__
#define __SYNTH_H__

#include <synth/synth_errors.h>
#include <synth/synth_types.h>

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
        int maxNotes);

/**
 * Check how many bytes the context is currently using
 * 
 * @param  [out]pSize The size of the context struct in bytes
 * @param  [ in]pCtx  The synthesizer context
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synth_getContextSize(int *pSize, synthCtx *pCtx);

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
        int maxTracks, int maxNotes);

/**
 * Alloc and initialize the synthesizer
 * 
 * @param  [out]ppCtx    The new synthesizer context
 * @param  [ in]freq     Synthesizer frequency, in samples per seconds
 * @return               SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synth_init(synthCtx **ppCtx, int freq);

/**
 * Release any of the submodules in use and then release any other memory
 * alloc'ed by the library
 * 
 * @param  [ in]ppCtx The synthesizer context that will be dealloc'ed
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synth_free(synthCtx **ppCtx);

/**
 * Load a already compiled song into the context (so it can be played/rendered)
 * 
 * This function exists mostly to remind me that I want to implement it sometime
 * in the future; It could be useful in memory-constrained environment, so the
 * user would compile and load it (using whichever memory he has access to) and
 * then he would load it to the context... Maybe it makes no sense at all, but I
 * want to keep this pointer here
 * 
 * @param  [out]pHandle Handle of the loaded song
 * @param  [ in]pCtx    The synthesizer context
 * @param  [ in]pSong   The song to be loaded
 * @return              SYNTH_FUNCTION_NOT_IMPLEMENTED
 */
synth_err synth_loadCompiledSong(int *pHandle, synthCtx *pCtx, void *pSong);

#if 0
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
synth_err synth_init(int freq, synth_bool doBuf, int size, synth_bool doBkend);

/**
 * Clean up any allocated memory and the buffering thread
 */
synth_err synth_clean();

/**
 * Get the frequency being used
 * 
 * @return The frequency
 */
int synth_getFrequency();

/**
 * Get how many samples should be buffered
 * 
 * @return The number of samples
 */
int synth_getSamples();

/**
 * Signal the buffering thread to buffer more samples
 */
void synth_requestBuffering();

/**
 * Try to lock buffer, allowing it to be read. SYNTH_COULDNT_LOCK means that the
 * buffer is already in use.
 * 
 * @return Error code
 */
synth_err synth_lockBuffer();

/**
 * Unlock the buffer
 */
void synth_unlockBuffer();

/**
 * Read some samples from the buffer. If it returns
 * SYNTH_BUFFER_NOT_ENOUGH_SAMPLES then more samples were requested than there
 * are currently buffered.
 * 
 * @param left Returns the left channel buffer
 * @param right Returns the right channel buffer
 * @param samples How many samples should be read
 * @return Error code
 */
synth_err synth_getBuffer(uint16_t **left, uint16_t **right, int samples);
#endif /* 0 */

#endif /* __SYNTH_H__ */


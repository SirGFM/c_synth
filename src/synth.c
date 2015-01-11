/**
 * @file src/synth.c
 */
#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_audiolist.h>
#include <synth_internal/synth_buffer.h>
#include <synth_internal/synth_cache.h>
#include <synth_internal/synth_thread.h>

static synth_bool synth_inited = SYNTH_FALSE;

/**
 * Initialize the synthesizer, including buffering thread and other
 * buffers
 * 
 * @param freq At which frequency (samples per minute) should synthesizer work
 * @param doBuffer Whether the buffering thread should run or not
 * @param size How many samples should be buffered
 * @return Error code
 */
synth_err synth_init(int freq, synth_bool doBuffer, int size) {
    synth_err rv;
    
    SYNTH_ASSERT_ERR(synth_inited == SYNTH_FALSE, SYNTH_ALREADY_INITIALIZED);
    
    if (doBuffer) {
        rv = synth_buf_init(size);
        SYNTH_ASSERT(rv == SYNTH_OK);
        
        rv = synth_list_init();
        SYNTH_ASSERT(rv == SYNTH_OK);
        
        rv = synth_thread_init();
        SYNTH_ASSERT(rv == SYNTH_OK);
    }
    
    rv = synth_cache_init(freq);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
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


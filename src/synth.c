/**
 * @file src/synth.c
 */
#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_cache.h>
#include <synth_internal/synth_thread.h>

static synth_bool synth_inited = SYNTH_FALSE;

/**
 * Initialize the synthesizer, including buffering thread and other
 * buffers
 * 
 * @param size How many samples should be buffered
 * @param freq At which frequency (samples per minute) should synthesizer work
 * @return Error code
 */
synth_err synth_init(int size, int freq) {
    synth_err rv;
    
    SYNTH_ASSERT_ERR(synth_inited == SYNTH_FALSE, SYNTH_ALREADY_INITIALIZED);
    
    rv = synth_cache_init(freq);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    rv = synth_thread_init();
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
    
    synth_cache_clean();
    
    rv = SYNTH_OK;
__err:
    return rv;
}


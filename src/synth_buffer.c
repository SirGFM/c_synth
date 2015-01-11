/**
 * @file src/synth_buffer.c
 */
#include <synth/synth_audio.h>
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_audio.h>
#include <synth_internal/synth_audiolist.h>
#include <synth_internal/synth_buffer.h>

#include <stdlib.h>
#include <string.h>

/**
 * Whether this module has been initialized
 */
synth_bool synth_buf_inited = SYNTH_FALSE;
/**
 * Size of the buffers, in samples
 */
int buflen;
/**
 * How many samples have been written into the buffer
 */
int len;
/**
 * How many samples have been read from the buffer
 */
int pos;
/**
 * Left buffer
 */
uint16_t *synth_buf_left;
/**
 * Right buffer
 */
uint16_t *synth_buf_right;

/**
 * Initialize the buffers
 * 
 * @param size How many samples each buffer should have
 * @return Error code
 */
synth_err synth_buf_init(int size) {
    synth_err rv;
    
    synth_buf_right = 0;
    
    SYNTH_ASSERT_ERR(synth_buf_inited == SYNTH_FALSE,
        SYNTH_BUFFER_ALREADY_INITIALIZED);
    
    synth_buf_left = (uint16_t*)malloc(sizeof(uint16_t)*size);
    SYNTH_ASSERT_ERR(synth_buf_left, SYNTH_MEM_ERR);
    
    synth_buf_right = (uint16_t*)malloc(sizeof(uint16_t)*size);
    SYNTH_ASSERT_ERR(synth_buf_right, SYNTH_MEM_ERR);
    
    pos = 0;
    len = 0;
    buflen = size;
    
    synth_buf_inited = SYNTH_TRUE;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Clean up memory
 */
void synth_buf_clean() {
    
    SYNTH_ASSERT(synth_buf_inited == SYNTH_TRUE);
    
    if (synth_buf_left)
        free(synth_buf_left);
    if (synth_buf_right)
        free(synth_buf_right);
    
    synth_buf_inited = SYNTH_FALSE;
__err:
    return;
}

/**
 * Get how many samples the buffer should have
 * 
 * @return The buffer size in samples
 */
int synth_buf_getSize() {
    return buflen;
}

/**
 * Whether should buffer more or not
 */
synth_bool synth_buf_doBuffer() {
    if (pos >= len)
        return SYNTH_FALSE;
    return SYNTH_TRUE;
}

/**
 * Update the buffer (synthesize stuff)
 */
void synth_buf_update() {
    int size;
    
    // If already read all the buffer
    if (pos == len) {
        size = buflen;
    }
    else {
        // Otherwise push remaining bytes to the start and set the size
        // accordingly
        memmove
            (
            synth_buf_left,
            synth_buf_left + pos,
            sizeof(uint16_t)*(len - pos)
            );
        memmove
            (
            synth_buf_right,
            synth_buf_right + pos,
            sizeof(uint16_t)*(len - pos)
            );
        size = buflen - pos;
        pos = len - pos;
    }
    
    // Then, synthesize the audio
    synth_list_synthesize(size, synth_buf_left+pos, synth_buf_right+pos);
    
    pos = 0;
    len = buflen;
}

/**
 * Read some samples from the buffer and updates its position. If it returns
 * SYNTH_BUFFER_NOT_ENOUGH_SAMPLES then more samples were requested than there
 * are currently buffered.
 * 
 * @param left Returns the left channel buffer
 * @param right Returns the right channel buffer
 * @param samples How many samples should be read
 * @return Error code
 */
synth_err synth_buf_getBuffer(uint16_t **left, uint16_t **right, int samples) {
    synth_err rv;
    
    // Make sure that there are (at least) as many samples as requested
    SYNTH_ASSERT_ERR(len - pos <= samples, SYNTH_BUFFER_NOT_ENOUGH_SAMPLES);
    
    // Set each returned buffer
    *left = synth_buf_left + pos;
    *right = synth_buf_right + pos;
    
    // Update the current position on the buffer
    pos += samples;
    
    rv = SYNTH_OK;
__err:
    return rv;
}


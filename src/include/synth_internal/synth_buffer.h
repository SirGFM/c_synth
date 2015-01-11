/**
 * @file src/include/synth_internal/synth_buffer.h
 */
#ifndef __SYNTH_BUFFER_H_
#define __SYNTH_BUFFER_H_

#include <synth/synth_errors.h>
#include <synth/synth_types.h>

/**
 * Left buffer
 */
extern uint16_t *synth_buf_left;
/**
 * Right buffer
 */
extern uint16_t *synth_buf_right;

/**
 * Initialize the buffers
 * 
 * @param size How many samples each buffer should have
 * @return Error code
 */
synth_err synth_buf_init(int size);

/**
 * Clean up memory
 */
void synth_buf_clean();

/**
 * Whether should buffer more or not
 */
synth_bool synth_buf_doBuffer();

/**
 * Update the buffer (synthesize stuff)
 */
void synth_buf_update();

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
synth_err synth_buf_getBuffer(uint16_t **left, uint16_t **right, int samples);

#endif


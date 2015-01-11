/**
 * @file include/synth/synth.h
 */
#ifndef __SYNTH_H_
#define __SYNTH_H_

#include <synth/synth_errors.h>
#include <synth/synth_types.h>

/**
 * Initialize the synthesizer, including buffering thread and other
 * buffers
 * 
 * @param freq At which frequency (samples per minute) should synthesizer work
 * @param doBuffer Whether the buffering thread should run or not
 * @param size How many samples should be buffered
 * @return Error code
 */
synth_err synth_init(int freq, synth_bool doBuffer, int size);

/**
 * Clean up any allocated memory and the buffering thread
 */
synth_err synth_clean();


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

#endif


/**
 * @file include/synth/synth.h
 */
#ifndef __SYNTH_H_
#define __SYNTH_H_

#include <synth/synth_errors.h>

/**
 * Initialize the synthesizer, including buffering thread and other
 * buffers
 * 
 * @param size How many samples should be buffered
 * @param freq At which frequency (samples per minute) should synthesizer work
 * @return Error code
 */
synth_err synth_init(int size, int freq);

/**
 * Clean up any allocated memory and the buffering thread
 */
synth_err synth_clean();

#endif


/**
 * @file include/synth_backend.h
 * 
 * Backend's generic interface
 */
#ifndef __SYNTH_BACKEND_H_
#define __SYNTH_BACKEND_H_

#include <synth/synth_errors.h>
#include <synth/synth_types.h>

/**
 * Setup the backend
 * 
 * @param freq Frequency in Hz (usual: 44100)
 * @param chan Number of channels (usual: 2)
 */
synth_err synth_bkend_setup(int freq, int chan);

/**
 * Cleanup the backend
 */
void synth_bkend_clean();

/**
 * Pauses the backend (if it runs on its own thread)
 */
void synth_bkend_pause();

/**
 * Unpauses the backend (if it runs on its own thread)
 */
void synth_bkend_unpause();

/**
 * Get the set frequency
 * 
 * @return Returns the frequency
 */
int synth_bkend_getFrequency();

/**
 * Fill up the buffer that shall be outputed.
 *
 * @param data Data to be fed the buffer
 * @param len How many bytes there are in data
 */
void synth_bkend_fillBuffer(void *data, int len);

/**
 * Set the requested parameter. Since setup only accepts two parameters, this
 * can be used to set additional data before hand.
 * 
 * @param data Pointer to where the parameter should be written to
 * @param param Enumeration representing which parameter to be read
 */
void synth_bkend_setParam(void *data, synth_param param);

/**
 * Get the requested parameter.
 * 
 * @param data Pointer to where the parameter should be written to
 * @param param Enumeration representing which parameter to be read
 */
void synth_bkend_getParam(void *data, synth_param param);

#endif


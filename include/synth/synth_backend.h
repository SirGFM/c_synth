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
 */
synth_err synth_bkend_setup();

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


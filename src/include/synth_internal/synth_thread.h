/**
 * @file src/include/synth_internal/synth_thread.h
 */
#ifndef __SYNTH_THREAD_H_
#define __SYNTH_THREAD_H_

#include <synth/synth_errors.h>

/**
 * Initialize the thread for buffering audio
 * 
 * @return Error code
 */
synth_err synth_thread_init();

/**
 * Signal the threads for stop and destroy mutex/conditional var
 */
void synth_thread_clean();

/**
 * Lock audio mutex
 */
void synth_thread_lockAudio();

/**
 * Unlock audio mutex
 */
void synth_thread_unlockAudio();

/**
 * Try to lock the buffer mutex
 * 
 * @return the Error code
 */
synth_err synth_thread_tryLockBuffer();

/**
 * Lock buffer mutex
 */
void synth_thread_lockBuffer();

/**
 * Unlock audio mutex
 */
void synth_thread_unlockBuffer();

/**
 * Signal the buffering thread to wakeup
 */
void synth_thread_wakeupThread();

#endif


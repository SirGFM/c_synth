/**
 * @file src/synth_thread.c
 * 
 * Handles thread for buffering audio
 */
#include <pthread.h>
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_audiolist.h>
#include <synth_internal/synth_buffer.h>
#include <synth_internal/synth_thread.h>

// So much for a simple NULL...
#include <stdio.h>

/**
 * Flag for whether the thread was started
 */
synth_bool synth_thread_inited = SYNTH_FALSE;
/**
 * Keeps the thread running
 */
synth_bool thread_running = SYNTH_FALSE;
/**
 * Buffering thread's indentifier
 */
pthread_t thread;
/**
 * Condition variable used wake up buffering thread
 */
pthread_cond_t signal;
/**
 * Mutex associated with the condition variable
 */
pthread_mutex_t sigmux;
/**
 * Mutex to lock access to the buffer
 */
pthread_mutex_t bufmux;
/**
 * Mutex to lock access to the audio list
 */
pthread_mutex_t audmux;

/**
 * Main function run by the thread
 */
void* synth_thread_main(void *arg);

/**
 * Initialize the thread for buffering audio
 * 
 * @return Error code
 */
synth_err synth_thread_init() {
    synth_err rv;
    int irv;
    
    //  Musn't initialize twice
    SYNTH_ASSERT_ERR(synth_thread_inited == SYNTH_FALSE,
        SYNTH_THREAD_ALREADY_INITIALIZED);
    
    // Initialize every mutex and condition variable
    irv = pthread_cond_init(&signal, NULL);
    SYNTH_ASSERT_ERR(irv == 0, SYNTH_THREAD_INIT_FAILED);
    
    irv = pthread_mutex_init(&sigmux, NULL);
    SYNTH_ASSERT_ERR(irv == 0, SYNTH_THREAD_INIT_FAILED);
    
    irv = pthread_mutex_init(&bufmux, NULL);
    SYNTH_ASSERT_ERR(irv == 0, SYNTH_THREAD_INIT_FAILED);
    
    irv = pthread_mutex_init(&audmux, NULL);
    SYNTH_ASSERT_ERR(irv == 0, SYNTH_THREAD_INIT_FAILED);
    
    // Actually initialize the  thread
    thread_running = SYNTH_TRUE;
    irv = pthread_create(&thread, NULL, synth_thread_main, NULL);
    SYNTH_ASSERT_ERR(irv == 0, SYNTH_THREAD_INIT_FAILED);
    
    synth_thread_inited = SYNTH_TRUE;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Signal the threads for stop and destroy mutex/conditional var
 */
void synth_thread_clean() {
    SYNTH_ASSERT(synth_thread_inited == SYNTH_TRUE);
    
    // Force the thread to be stopped
    thread_running = SYNTH_FALSE;
    pthread_cond_signal(&signal);
    
    // Releases the mutexes and condition variables
    pthread_mutex_destroy(&audmux);
    pthread_mutex_destroy(&bufmux);
    pthread_cond_destroy(&signal);
    
__err:
    return;
}

/**
 * Main function run by the thread
 */
void* synth_thread_main(void *arg) {
    // Just started this new thread
    
    while (thread_running == SYNTH_TRUE) {
        while (synth_buf_doBuffer() == SYNTH_FALSE &&
            thread_running == SYNTH_TRUE)
            pthread_cond_wait(&signal, &sigmux);
        if (thread_running == SYNTH_FALSE)
            break;
        
        // Update the list with added audio
        synth_list_merge();
        
        // Update the buffer
        synth_buf_update();
        
        pthread_mutex_unlock(&sigmux);   
    }
    
    // Unlock the mutex, destroy it and exit the thread
    pthread_mutex_unlock(&sigmux);
    pthread_mutex_destroy(&sigmux);
    
    pthread_exit(NULL);
}

/**
 * Lock audio mutex
 */
void synth_thread_lockAudio() {
    pthread_mutex_lock(&audmux);
}

/**
 * Unlock audio mutex
 */
void synth_thread_unlockAudio() {
    pthread_mutex_unlock(&audmux);
}

/**
 * Try to lock the buffer mutex
 * 
 * @return the Error code
 */
synth_err synth_thread_tryLockBuffer() {
    int irv;
    
    irv = pthread_mutex_trylock(&bufmux);
    
    if (irv != 0)
        return SYNTH_COULDNT_LOCK;
    return SYNTH_OK;
}

/**
 * Lock buffer mutex
 */
void synth_thread_lockBuffer() {
    pthread_mutex_lock(&bufmux);
}

/**
 * Unlock audio mutex
 */
void synth_thread_unlockBuffer() {
    pthread_mutex_unlock(&bufmux);
}

/**
 * Signal the buffering thread to wakeup
 */
void synth_thread_wakeupThread() {
    pthread_cond_signal(&signal);
}


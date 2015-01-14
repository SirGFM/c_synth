/**
 * @file src/synth_lpulse_backend.c
 */
#include <pthread.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#include <synth/synth.h>
#include <synth/synth_backend.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_thread.h>

#define LPULSE_LEN 1024

// TODO check that everything works as expected!!

/**
 * Whether the audio stream loop is active or not
 */
synth_bool audio_is_playing;
/**
 * Handle to the streaming thread
 */
pthread_t audio_thread;
/**
 * Condition variable on which the thread waits
 */
pthread_cond_t audsignal;
/**
 * Mutex associated with the condition variable
 */
pthread_mutex_t audsigmux;

/**
 * Streaming thread
 */
void *audio_thread_main(void *arg);

/**
 * Setup the backend
 */
synth_err synth_bkend_setup() {
    synth_err rv;
    
    // Initialize every mutex and condition variable
    irv = pthread_cond_init(&audsignal, NULL);
    SYNTH_ASSERT_ERR(irv == 0, SYNTH_THREAD_INIT_FAILED);
    
    irv = pthread_mutex_init(&audsigmux, NULL);
    SYNTH_ASSERT_ERR(irv == 0, SYNTH_THREAD_INIT_FAILED);
    
    // Actually initialize the  thread
    thread_running = SYNTH_TRUE;
    irv = pthread_create(&audio_thread, NULL, audio_thread_main, NULL);
    SYNTH_ASSERT_ERR(irv == 0, SYNTH_THREAD_INIT_FAILED);
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Cleanup the backend
 */
void synth_bkend_clean() {
    thread_running = SYNTH_FALSE;
    synth_bkend_unpause();
    
    // TODO [...]
}

/**
 * Pauses the backend (if it runs on its own thread)
 */
void synth_bkend_pause() {
    audio_is_playing = SYNTH_FALSE;
    
    // TODO libpulse cork?
}

/**
 * Unpauses the backend (if it runs on its own thread)
 */
void synth_bkend_unpause() {
    audio_is_playing = SYNTH_TRUE;
    pthread_cond_signal(&signal);
    
    // TODO libpulse uncork?
}

/**
 * Get how many samples should be buffered before playing
 * 
 * @return 
 */
int synth_bkend_getSamplesPerChannel() {
    return LPULSE_LEN / 2;
}

/**
 * Set the requested parameter. Since setup only accepts two parameters, this
 * can be used to set additional data before hand.
 * 
 * @param data Pointer to where the parameter should be written to
 * @param param Enumeration representing which parameter to be read
 */
void synth_bkend_setParam(void *data, synth_param param) {
    
}

/**
 * Get the requested parameter.
 * 
 * @param data Pointer to where the parameter should be written to
 * @param param Enumeration representing which parameter to be read
 */
void synth_bkend_getParam(void *data, synth_param param) {
    
}

void *audio_thread_main(void *arg) {
    pa_simple *s;
    pa_samples_spec ss;
    uint16_t data[LPULSE_LEN], *left, *right;
    
    // Set the playback specs
    ss.format = PA_SAMPLES_S16LE;
    ss.rate = synth_getFrequency();
    ss.channels = 2;
    
    // Create the playback stream
    s = pa_simples_new
            (
            0, // server
            "c_synth", // app name
            PA_STREAM_PLAYBACK, // stream direction
            0, // device (?)
            "game audio", // stream name
            &ss, // samples specs
            0, // channel map
            0, // buffering attributes
            0 // error
            );
    SYNTH_ASSERT(s);
    
    pthread_mutex_lock(&audsigmux);   
    // Keep streaming audio
    while (audio_is_running == SYNTH_TRUE) {
        int irv, err, i;
        
        pthread_mutex_unlock(&audsigmux);   
        // Pause the song using a conditional variable
        while (audio_is_playing == SYNTH_FALSE &&
               audio_is_running == SYNTH_TRUE) {
            pthread_cond_wait(&audsignal, &audsigmux);
            // Wait for signal
        }
        if (audio_is_running == SYNTH_FALSE)
            break;
        
        rv = synth_lockBuffer();
        if (rv == SYNTH_COULDNT_LOCK) {
            synth_requestBuffering();
            continue;
        }
        
        // Try to get enough samples to fill the buffer
        rv = synth_getBuffer(&left, &right, LPULSE_LEN / 2);
        if (rv == SYNTH_BUFFER_NOT_ENOUGH_SAMPLES) {
            // If there aren't enough samples, unlock and request more
            synth_unlockBuffer();
            synth_requestBuffering();
            continue;
        }
        
        // Copy (and merge) both channels
        i = 0;
        while (i < LPULSE_LEN / 2) {
            data[i * 2    ] = left[i];
            data[i * 2 + 1] = right[i];
            i++;
        }
        
        // Since we are done with the buffering thread, release it
        synth_unlockBuffer();
        synth_requestBuffering();
        
        // Output the current buffer
        irv = pa_simple_write(s, data, LPULSE_LEN, &err);
        SYNTH_ASSERT(irv >= 0);
        
        // Wait until everything is played
        irv = pa_simple_drain(s, &err);
        SYNTH_ASSERT(irv >= 0);
    }
    
__err:
    pthread_mutex_unlock(&audsigmux);
    pthread_mutex_destroy(&audsigmux);
    return;
}


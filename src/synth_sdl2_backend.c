/**
 * @file src/synth_sdl2_backend.c
 * 
 * A implementation of the backend, using SDL2
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_backend.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>

static int setuped = 0;
static SDL_AudioSpec spec;
static SDL_AudioDeviceID dev = 0;

static void synth_sdl2_bkend_callback(void *arg, Uint8 *stream, int len);

/**
 * Setup the backend
 */
synth_err synth_bkend_setup() {
    synth_err rv;
    int irv;
    SDL_AudioSpec desired;
    
    // Check that the backend hasn't been initialized yet
    SYNTH_ASSERT_ERR(setuped == 0, SYNTH_ALREADY_STARTED);
    
    // Initialize SDL's audio subsystem
    irv = SDL_InitSubSystem(SDL_INIT_AUDIO);
    SYNTH_ASSERT_ERR(irv == 0, SYNTH_INTERNAL_ERR);
    
    // Setup the desired specs
    desired.freq = synth_getFrequency();
    desired.channels = 2;
    desired.samples = 1024; //synth_getSamples() * 2;
    
    desired.callback = synth_sdl2_bkend_callback;
    desired.userdata = 0;
    desired.format = AUDIO_S16LSB;
    
    dev = SDL_OpenAudioDevice(NULL, 0, &desired, &spec, 0);
    SYNTH_ASSERT_ERR(dev > 0, SYNTH_INTERNAL_ERR);
    
    setuped = 1;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Cleanup the backend
 */
void synth_bkend_clean() {
    if (dev > 0) {
        SDL_PauseAudioDevice(dev, 1);
        SDL_CloseAudioDevice(dev);
        dev = 0;
    }
    if (setuped) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        setuped = 0;
    }
}

/**
 * Pauses the backend (if it runs on its own thread
 */
void synth_bkend_pause() {
    SDL_PauseAudioDevice(dev, 1);
}

/**
 * Unpauses the backend (if it runs on its own thread)
 */
void synth_bkend_unpause() {
    SDL_PauseAudioDevice(dev, 0);
}

/**
 * Get how many samples should be buffered before playing
 * 
 * @return 
 */
int synth_bkend_getSamplesPerChannel() {
    return 512;
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

/**
 * Specific to SDL2. Is called every once in a while (when buffer is empty) to
 * get more samples played.
 *
 * @param arg Not used
 * @param stream System buffer that will be played
 * @param len How many bytes there are in that buffer
 */
static void synth_sdl2_bkend_callback(void *arg, Uint8 *stream, int len) {
    synth_err rv;
    int i;
    Uint16 *samples;
    uint16_t *left, *right;
    
    // Access each sample as a 16-bits word
    samples = (Uint16*)stream;
    // fix the  len to this new access method
    len >>= 1;
    
    i = 0;
    // Clear the audio buffer
    while (i < len)
        samples[i++] = 0;
    
    // Try to lock the buffer, exiting if failed
    rv = synth_lockBuffer();
    if (rv == SYNTH_COULDNT_LOCK) {
        synth_requestBuffering();
        return;
    }
    
    // Get the number of samples per channel
    len >>= 1;
    
    // Try to get enough samples to fill the buffer
    rv = synth_getBuffer(&left, &right, len);
    if (rv == SYNTH_BUFFER_NOT_ENOUGH_SAMPLES) {
        // If there aren't enough samples, unlock and request more
        synth_unlockBuffer();
        synth_requestBuffering();
        return;
    }
    
    // Fill the buffer with the samples
    i = 0;
    while (i < len) {
        // left
        samples[i * 2  ] = left[i];
        // right
        samples[i * 2+1] = right[i];
        i++;
    }
    
    // Unlock and request more buffer, while playing the audio
    synth_unlockBuffer();
    synth_requestBuffering();
}


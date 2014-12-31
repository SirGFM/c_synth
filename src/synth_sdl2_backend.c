/**
 * @file src/synth_sdl2_backend.c
 * 
 * A implementation of the backend, using SDL2
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#include <synth/synth_assert.h>
#include <synth/synth_backend.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>

#include <stdlib.h>
#include <string.h>

static int setuped = 0;
static int samples = 1024;
static Uint16 *synth_buffer = 0;
static int sb_len = 0;
static int sb_pos = 0;

static SDL_AudioSpec spec;
static SDL_AudioDeviceID dev = 0;

static void synth_sdl2_bkend_callback(void *arg, Uint8 *stream, int len);

/**
 * Setup the backend
 * 
 * @param freq Frequency in Hz (usual: 44100)
 * @param chan Number of channels (usual: 2)
 */
synth_err synth_bkend_setup(int freq, int chan) {
    synth_err rv;
    int irv;
    SDL_AudioSpec desired;
    
    SYNTH_ASSERT_ERR(setuped == 0, SYNTH_ALREADY_STARTED);
    
    irv = SDL_InitSubSystem(SDL_INIT_AUDIO);
    SYNTH_ASSERT_ERR(irv == 0, SYNTH_INTERNAL_ERR);
    
    desired.freq = freq;
    desired.channels = chan;
    desired.samples = samples;
    
    desired.callback = synth_sdl2_bkend_callback;
    desired.userdata = 0;
    desired.format = AUDIO_S16LSB;
    
    dev = SDL_OpenAudioDevice(NULL, 0, &desired, &spec, 0);
    SYNTH_ASSERT_ERR(dev > 0, SYNTH_INTERNAL_ERR);
    
    //SDL_PauseAudioDevice(dev, 0);
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
    if (synth_buffer != 0) {
        free(synth_buffer);
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
 * Get the set frequency
 * 
 * @return Returns the frequency
 */
int synth_bkend_getFrequency() {
    return spec.freq;
}

/**
 * Fill up the buffer that shall be outputed.
 *
 * @param data Data to be fed the buffer
 * @param len How many bytes there are in data
 */
void synth_bkend_fillBuffer(void *data, int len) {
    // TODO use a nice ciclic buffer
    if (len > sb_len + sb_pos) {
        synth_buffer = realloc(synth_buffer, len + sb_pos);
        sb_len = sb_pos + len;
    }
    memcpy(synth_buffer + sb_pos, data, len);
    synth_bkend_unpause();
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
    int i;
    Uint16 *samples;
    
    // Access each sample as a 16-bits word
    samples = (Uint16*)stream;
    // fix the  len to this new access method
    len >>= 1;
    
    i = 0;
    // Clear the audio buffer
    while (i < len)
        samples[i++] = 0;
    
    i = 0;
    while (i < len / 2 && sb_pos < sb_len) {
        // left
        samples[i*2+0] = synth_buffer[sb_pos++];
        // right
        samples[i*2+1] = synth_buffer[sb_pos++];
        i++;
    }
    
    if (sb_pos >= sb_len)
        synth_bkend_pause();
}


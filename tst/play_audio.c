/**
 * @file tst/play_audio.c
 */

#ifdef USE_SDL
#  include <SDL2/SDL.h>
#endif

#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_audio.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>

#include <stdio.h>
#include <unistd.h>

static char mml[] = "t90 l16 o5 e e8 e r c e r g4 > g4 <";
#define TIME 3
#define FREQ 44100
#define SAMPLES 2048

int main(int argc, char *argv[]) {
    int rv, time;
    synth_err srv;
    synthAudio *aud;
    
    aud = NULL;
    
    // Initialize everything
#ifdef USE_SDL
    rv = SDL_Init(0);
    SYNTH_ASSERT(rv >= 0);
#endif
    
    srv = synth_init(FREQ, SYNTH_TRUE, SAMPLES, SYNTH_TRUE);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    
    // Create the audio
    if (argc > 1) {
        printf("Reading mml from file: %s\n", argv[1]);
        
        // Initialize the contex for parsing
        srv = synth_audio_loadf(&aud, argv[1]);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    }
    else {
        printf("mml: %s\n", mml);
        
        // Initialize the contex for parsing
        srv = synth_audio_loads(&aud, mml, sizeof(mml));
        SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    }
    
    // Get for how long it should play
    if (argc > 2) {
        char *t;
        int n;
        
        t = argv[2];
        n = 0;
        while (*t) {
            n = n * 10 + *t - '0';
            t++;
        }
        
        time = n;
    }
    else
        time = TIME;
    
    // Play the audio and wait while it's playing
    synth_audio_playAudio(aud);
    sleep(time);
    
    rv = 0;
__err:
    synth_clean();
#ifdef USE_SDL
    SDL_Quit();
#endif
    synth_audio_free(&aud);
    
    return rv;
}


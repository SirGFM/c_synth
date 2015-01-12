/**
 * @file tst/cmd_parse.c
 */
#include <signal.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_audio.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>

#define FREQ 44100
#define SAMPLES 2048

int doRun = 1;

void sighnd(int sig) {
    doRun = 0;
}

int main(int argc, char *argv[]) {
    int rv;
    synth_err srv;
    synthAudio *aud;
    struct sigaction act;
    
    act.sa_sigaction = sighnd;
    act.sa_flags = SA_SIGINFO;
    
    sigaction(SIGINT, &act, NULL);
    
    aud = 0;
    
    // Initialize everything
    rv = SDL_Init(0);
    SYNTH_ASSERT(rv >= 0);
    
    srv = synth_init(FREQ, SYNTH_TRUE, SAMPLES, SYNTH_TRUE);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    
    while (doRun) {
        // TODO read cmd and create an audio
    }
    
    rv = 0;
__err:
    synth_clean();
    SDL_Quit();
    synth_audio_free(&aud);
    
    return rv;
}


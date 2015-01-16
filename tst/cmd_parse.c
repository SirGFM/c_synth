/**
 * @file tst/cmd_parse.c
 */
#include <signal.h>

#ifdef USE_SDL
#include <SDL2/SDL.h>
#endif

#include <stdio.h>

#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_audio.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>

#include <unistd.h>

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
#ifdef USE_SDL
    rv = SDL_Init(0);
    SYNTH_ASSERT(rv >= 0);
#endif
    
    srv = synth_init(FREQ, SYNTH_TRUE, SAMPLES, SYNTH_TRUE);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    
    while (doRun) {
        char mml[1024], *ret;
        int i;
        
        ret = fgets(mml, 1024, stdout);
        
        SYNTH_ASSERT_ERR(mml == ret, SYNTH_MEM_ERR);
        
        i = 0;
        while (i < 1024) {
            if (mml[i] == '\n' || mml[i] == '\0')
                break;
            i++;
        }
        
        mml[i] = '\0';
        
        if (aud)
            synth_audio_free(&aud);
        
        srv = synth_audio_loads(&aud, mml, i);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
        
        synth_audio_playAudio(aud);
    }
    
    rv = 0;
__err:
    synth_clean();
#ifdef USE_SDL
    SDL_Quit();
#endif
    synth_audio_free(&aud);
    
    return rv;
}


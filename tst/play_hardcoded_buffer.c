/**
 *
 */
#include <SDL2/SDL.h>

#include <synth/synth_assert.h>
#include <synth/synth_backend.h>
#include <synth/synth_errors.h>

#include <stdio.h>
#include <stdlib.h>

#define FREQ 44100
#define SAMPLESIZE 1024*2
#define BUFFERLEN SAMPLESIZE*FREQ

void bufferLa(Uint16 *buffer, int len);

int main(int argc, char *argv[]) {
    int rv;
    synth_err srv;
    Uint16 *data;
    
    data = (Uint16*)malloc(sizeof(Uint16)*BUFFERLEN);
    SYNTH_ASSERT_ERR(data, 1);
    
    bufferLa(data, BUFFERLEN);
    
    rv = SDL_Init(0);
    SYNTH_ASSERT(rv >= 0);
    
    srv = synth_bkend_setup(FREQ, 2);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    
    synth_bkend_fillBuffer(data, BUFFERLEN);
    
    SDL_Delay(1100);
    
__err:
    synth_bkend_clean();
    SDL_Quit();
    return rv;
}

void bufferLa(Uint16 *buffer, int len) {
    int i;
    int spc; // samples per cycle
    
    spc = FREQ / 440;
    len >>= 1;
    
    i = 0;
    while (i < len) {
        Uint16 amp;
        
        if (i % spc < spc / 2)
            amp = 0x7fff;
        else
            amp = 0;
        
        buffer[i*2+0] = amp;
        buffer[i*2+1] = amp;
        
        i++;
    }
}


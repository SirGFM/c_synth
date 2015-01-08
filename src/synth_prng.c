/**
 * @file src/synth_prng.c
 * 
 * Simple pseudo random number generator, used by the noise wave
 */
#include <synth_internal/synth_prng.h>

unsigned int seed = 0;

void synth_prng_setSeed(int val) {
    seed = val;
}

unsigned int synth_prng_getRand() {
    long int tmp = seed;
    
    tmp *= 0x19660d;
    tmp += 0x3c6ef35f;
    
    seed = tmp;
    return seed;
}


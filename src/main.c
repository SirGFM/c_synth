#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_events.h>

#include <stdio.h>

#define ASSERT(rv) do { if (!(rv)) goto __ret; } while(0)
#define LOG(str, ...) do { printf(str"\n", ##__VA_ARGS__); } while (0)

SDL_AudioSpec spec;
SDL_AudioDeviceID dev = 0;
int audio_started = 0;
int num = 0;
int points = 0;

int init_audio();
void close_audio();
void callback(void *arg, Uint8 *stream, int len);

int main(int argc, char *argv[]) {
    int rv = 0;
    
    LOG("begin");
    
    rv = SDL_Init(0);
    ASSERT(rv >= 0);
    
    rv = init_audio();
    ASSERT(rv == 0);
    
    SDL_Delay(1000);
    
__ret:
    close_audio();
    SDL_Quit();
    LOG("end (rv=%i)", rv);
    LOG("samples=%i", points);
    LOG("freq=%i", spec.freq);
    return rv;
}

int init_audio() {
    int rv = 0;
    
    SDL_AudioSpec wanted;
        
    rv = SDL_InitSubSystem(SDL_INIT_AUDIO);
    ASSERT(rv == 0);
    audio_started = 1;
    
    wanted.freq = 44100;
    wanted.format = AUDIO_S16LSB;
    wanted.channels = 2;
    wanted.samples = 1024;
    wanted.callback = callback;
    wanted.userdata = 0;
    
    dev = SDL_OpenAudioDevice(NULL, 0, &wanted, &spec, 0);
    rv = (dev > 0)?0:1;
    ASSERT(rv == 0);
    
	SDL_PauseAudioDevice(dev, 0);
__ret:
    return rv;
}

void close_audio() {
	SDL_PauseAudioDevice(dev, 1);
    if (dev > 0) {
        SDL_CloseAudioDevice(dev);
        dev = 0;
    }
    if (audio_started) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        audio_started = 0;
    }
}

#define MAX 0x7fff
#define MIN 0x8001

void callback(void *arg, Uint8 *stream, int len) {
    int i, l;
    num++;
    Uint16 *samples;
    
    Uint32 *left;
    Uint32 *right;
    
    // Access each sample as a 16-bits word
    samples = (Uint16*)stream;
    // fix the  len to this new access method
    len >>= 1;
    // Requires(?) unaligned access, but gives access to each buffer separatelly
    left = (Uint32*)(stream+2);
    right= (Uint32*)(stream);
    
    // Clear the audio buffer
    i = 0;
    while (i < len) {
        points++;
        samples[i++] = 0;
    }
    
    i = 0;
    l = len >> 3;
    while (i < l) {
        right[i] |= MAX;
        i++;
    }
    
    i = 0;
    l = len >> 3;
    while (i < l) {
        left[i] |= MAX;
        i++;
    }
    /*
    // samples[odd] = left speaker
    // samples[even] = right speaker
    
    i = 0;
    l = len / 8;
    while (i < l) {
        samples[i] = 0;
        samples[i+1] = MAX;
        samples[i+2] = 0;
        samples[i+3] = 0;
        i+=4;
    }
    */
}


/**
 * @file tst/play_hc_mario.c
 */
#include <SDL2/SDL.h>

#include <synth/synth_assert.h>
#include <synth/synth_backend.h>
#include <synth/synth_errors.h>

#include <synth_internal/synth_note.h>
#include <synth_internal/synth_track.h>
#include <synth_internal/synth_volume.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIME 3
#define FREQ 44100
#define BUFFERLEN FREQ*TIME

int main(int argc, char *argv[]) {
    int rv, i;
    synth_err srv;
    uint16_t *left, *right, *data;
    synthTrack track;
    synthNote noteC5_16;
    synthNote noteE5_16;
    synthNote noteE5_8;
    synthNote noteG5_4;
    synthNote noteG4_4;
    synthNote noteREST_16;
    synthVolume vol;
    
    left = NULL;
    right = NULL;
    data = NULL;
    
    rv = SDL_Init(0);
    SYNTH_ASSERT(rv >= 0);
    
    srv = synth_bkend_setup(FREQ, 2);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    
    synth_bkend_pause();
    
    srv = synth_track_init(&track);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    
    synth_vol_init(&vol);
    
    #define setNOD(NOTE, OCTAVE, DURATION) \
        synth_note_init(&note ## NOTE ## OCTAVE ## _ ## DURATION); \
        synth_note_setWave(&note ## NOTE ## OCTAVE ## _ ## DURATION, W_PULSE_25); \
        synth_note_setNote(&note ## NOTE ## OCTAVE ## _ ## DURATION, N_ ## NOTE); \
        synth_note_setOctave(&note ## NOTE ## OCTAVE ## _ ## DURATION, OCTAVE); \
        synth_note_setDuration(&note ## NOTE ## OCTAVE ## _ ## DURATION, 90, DURATION); \
        synth_note_setVolume(&note ## NOTE ## OCTAVE ## _ ## DURATION, &vol)
    #define setND(NOTE, DURATION) \
        synth_note_init(&note ## NOTE ## _ ## DURATION); \
        synth_note_setWave(&note ## NOTE ## _ ## DURATION, W_PULSE_25); \
        synth_note_setNote(&note ## NOTE ## _ ## DURATION, N_ ## NOTE); \
        synth_note_setDuration(&note ## NOTE ## _ ## DURATION, 90, DURATION); \
        synth_note_setVolume(&note ## NOTE ## _ ## DURATION, &vol)
    #define addNote(NOTE) \
      srv = synth_track_addNote(&track, &note ## NOTE); \
      SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv)
    
    setNOD(C, 5, 16);
    setNOD(E, 5, 16);
    setNOD(E, 5, 8);
    setNOD(G, 5, 4);
    setNOD(G, 4, 4);
    setND(REST, 16);
    
    addNote(E5_16);
    addNote(E5_8);
    addNote(E5_16);
    addNote(REST_16);
    addNote(C5_16);
    addNote(E5_16);
    addNote(REST_16);
    addNote(G5_4);
    addNote(G4_4);
    
    left = (uint16_t*)malloc(sizeof(uint16_t)*BUFFERLEN);
    SYNTH_ASSERT_ERR(left != NULL, SYNTH_MEM_ERR);
    
    right = (uint16_t*)malloc(sizeof(uint16_t)*BUFFERLEN);
    SYNTH_ASSERT_ERR(left != NULL, SYNTH_MEM_ERR);
    
    memset(left, 0x0, BUFFERLEN*sizeof(uint16_t));
    memset(right, 0x0, BUFFERLEN*sizeof(uint16_t));
    
    synth_track_synthesize(&track, BUFFERLEN, left, right);
    
    data = (uint16_t*)malloc(sizeof(uint16_t)*BUFFERLEN*2);
    SYNTH_ASSERT_ERR(data != NULL, SYNTH_MEM_ERR);
    
    memset(data, 0x0, BUFFERLEN*sizeof(uint16_t)*2);
    
    i = 0;
    while (i < BUFFERLEN) {
        data[i*2+0] = left[i];
        data[i*2+1] = right[i];
        i++;
    }
    
    synth_bkend_fillBuffer(data, sizeof(uint16_t)*BUFFERLEN*2);
    
    SDL_Delay((TIME)*1000);
    
    rv = 0;
__err:
    if (left)
        free(left);
    if (right)
        free(right);
    if (data)
        free(data);
    
    synth_track_clean(&track);
    synth_bkend_clean();
    SDL_Quit();
    
    return rv;
}



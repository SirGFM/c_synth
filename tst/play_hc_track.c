/**
 * @file tst/play_hc_track.c
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

#define TIME 48
#define FREQ 44100
#define BUFFERLEN FREQ*TIME

int main(int argc, char *argv[]) {
    int rv, i;
    synth_err srv;
    uint16_t *left, *right;
    char *data;
    synthTrack track;
    synthNote noteC;
    synthNote noteD;
    synthNote noteD2;
    synthNote noteE;
    synthNote noteE2;
    synthNote noteF;
    synthNote noteF2;
    synthNote noteG;
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
    
    #define setNote(NOTE) \
      synth_note_init(&note ## NOTE); \
      synth_note_setNote(&note ## NOTE, N_ ## NOTE); \
      synth_note_setVolume(&note ## NOTE, &vol)
    #define setNoteDuration(NOTE, DUR) \
      synth_note_init(&note ## NOTE ## DUR); \
      synth_note_setDuration(&note ## NOTE ## DUR, 60, DUR); \
      synth_note_setNote(&note ## NOTE ## DUR, N_ ## NOTE); \
      synth_note_setVolume(&note ## NOTE ## DUR, &vol)
    #define addNote(NOTE) \
      srv = synth_track_addNote(&track, &note ## NOTE); \
      SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv)
    
    setNote(C);
    setNote(D);
    setNoteDuration(D, 2);
    setNote(E);
    setNoteDuration(E, 2);
    setNote(F);
    setNoteDuration(F, 2);
    setNote(G);
    
    addNote(C);
    addNote(D);
    addNote(E);
    addNote(F2);
    addNote(F);
    addNote(F2);
    
    addNote(C);
    addNote(D);
    addNote(C);
    addNote(D2);
    addNote(D);
    addNote(D2);
    
    addNote(C);
    addNote(G);
    addNote(F);
    addNote(E2);
    addNote(E);
    addNote(E2);
    
    addNote(C);
    addNote(D);
    addNote(E);
    addNote(F2);
    addNote(F);
    addNote(F2);
    
    left = (uint16_t*)malloc(sizeof(uint16_t)*BUFFERLEN);
    SYNTH_ASSERT_ERR(left != NULL, SYNTH_MEM_ERR);
    
    right = (uint16_t*)malloc(sizeof(uint16_t)*BUFFERLEN);
    SYNTH_ASSERT_ERR(left != NULL, SYNTH_MEM_ERR);
    
    memset(left, 0x0, BUFFERLEN*sizeof(uint16_t));
    memset(right, 0x0, BUFFERLEN*sizeof(uint16_t));
    
    synth_track_synthesize(&track, BUFFERLEN, left, right);
    
    data = (char*)malloc(sizeof(uint16_t)*BUFFERLEN*2);
    
    i = 0;
    while (i < BUFFERLEN) {
        data[i*2+0] = left[i];
        data[i*2+1] = right[i];
        i++;
    }
    
    synth_bkend_fillBuffer(data, sizeof(uint16_t)*BUFFERLEN*2);
    
    SDL_Delay((TIME+1)*1000);
    
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


/**
 * @file src/synth_audio.c
 */

#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth_internal/synth_audio.h>
#include <synth_internal/synth_parser.c>
#include <synth_internal/synth_track.h>

#include <stdlib.h>
#include <string.h>

synth_err synth_audio_loadf(synthAudio **audio, char *filename) {
    return SYNTH_FUNCTION_NOT_IMPLEMENTED;
}

synth_err synth_audio_loads(synthAudio **audio, char *mml, int len) {
    return SYNTH_FUNCTION_NOT_IMPLEMENTED;
}

void synth_audio_free(synthAudio **audio) {
    int i;
    synthAudio *tmp;
    
    SYNTH_ASSERT(audio);
    SYNTH_ASSERT(*audio);
    
    tmp = *audio;
    
    if (tmp->tracks) {
        i = 0;
        while (i < tmp->len) {
            synth_track_clean(tmp->tracks[i]);
            free(tmp->tracks[i]);
            i++;
        }
        
        free(tmp->tracks);
    }
    
    free(tmp);
    *audio = 0;
    
__err:
    return;
}

void synth_audio_play(synthAudio *audio) {
    
}

synth_err synth_audio_allocAudio(synthAudio **audio) {
    synth_err rv;
    synthAudio *tmp;
    
    tmp = (synthAudio*)malloc(sizeof(synthAudio));
    SYNTH_ASSERT_ERR(tmp, SYNTH_MEM_ERR);
    
    tmp->tracks = (synthTrack**)malloc(sizeof(synthTrack*));
    SYNTH_ASSERT_ERR(tmp->tracks, SYNTH_MEM_ERR);
    
    tmp->tracksLen = 1;
    tmp->len = 0;
    
    *audio = tmp;
    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK) {
        synth_audio_free(&tmp);
    }
    return rv;
}

synth_err synth_audio_addTrack(synthAudio *audio, synthTrack *track) {
    synth_err rv;
    
    if (audio->len >= audio->tracksLen) {
        int size;
        
        size = audio->tracksLen*2;
        audio->tracks = realloc(audio->tracks, sizeof(synthTrack*)*size);
        SYNTH_ASSERT_ERR(audio->tracks, SYNTH_MEM_ERR);
        
        audio->tracksLen = size;
    }
    
    audio->tracks[audio->len] = track;
    audio->len++;
    
    rv = SYNTH_OK;
__err:
    return rv;
}


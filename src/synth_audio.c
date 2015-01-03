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

/**
 * Free up an audio structure
 * 
 * @param audio Reference to the audio pointer
 */
void synth_audio_free(synthAudio **audio) {
    int i;
    synthAudio *tmp;
    
    // Check if there's anything to be freed
    SYNTH_ASSERT(audio);
    SYNTH_ASSERT(*audio);
    
    tmp = *audio;
    
    if (tmp->tracks) {
        i = 0;
        // Release every track
        while (i < tmp->len) {
            synth_track_clean(tmp->tracks[i]);
            free(tmp->tracks[i]);
            i++;
        }
        
        // Release the tracks array
        free(tmp->tracks);
    }
    
    // Release the audio structure
    free(tmp);
    *audio = 0;
    
__err:
    return;
}

void synth_audio_play(synthAudio *audio) {
    
}

/**
 * Alloc a new audio structure and initialize it
 * 
 * @param audio Reference to the audio pointer
 * @return Error code
 */
synth_err synth_audio_allocAudio(synthAudio **audio) {
    synth_err rv;
    synthAudio *tmp;
    
    // Alloc the audio structure
    tmp = (synthAudio*)malloc(sizeof(synthAudio));
    SYNTH_ASSERT_ERR(tmp, SYNTH_MEM_ERR);
    
    // Alloc the tracks array
    tmp->tracks = (synthTrack**)malloc(sizeof(synthTrack*));
    SYNTH_ASSERT_ERR(tmp->tracks, SYNTH_MEM_ERR);
    
    tmp->tracksLen = 1;
    tmp->len = 0;
    
    // Set the return variables
    *audio = tmp;
    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK) {
        synth_audio_free(&tmp);
    }
    return rv;
}

/**
 * Adds a new track to the audio
 * Though the track will be released automatically, its notes and volume won't
 * 
 * @param audio The audio
 * @param track The track
 * @return Error code
 */
synth_err synth_audio_addTrack(synthAudio *audio, synthTrack *track) {
    synth_err rv;
    
    // Check if the buffer should be expanded
    if (audio->len >= audio->tracksLen) {
        int size;
        
        // Alloc double its current size
        size = audio->tracksLen*2;
        audio->tracks = realloc(audio->tracks, sizeof(synthTrack*)*size);
        SYNTH_ASSERT_ERR(audio->tracks, SYNTH_MEM_ERR);
        
        audio->tracksLen = size;
    }
    
    // 'Append' the track
    audio->tracks[audio->len] = track;
    audio->len++;
    
    rv = SYNTH_OK;
__err:
    return rv;
}


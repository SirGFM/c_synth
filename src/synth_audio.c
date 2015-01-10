/**
 * @file src/synth_audio.c
 */

#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth_internal/synth_audio.h>
#include <synth_internal/synth_parser.h>
#include <synth_internal/synth_track.h>

#include <stdlib.h>
#include <string.h>

// TODO feature: per-track volume

/**
 * Parse a mml file into an audio
 * 
 * @param ctx Variable that will store the audio
 * @param filename MML's filename
 * @return The error code
 */
synth_err synth_audio_loadf(synthAudio **audio, char *filename) {
    synth_err rv;
    synthParserCtx *ctx = 0;
    
    // Sanitize input
    SYNTH_ASSERT_ERR(audio, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(filename, SYNTH_BAD_PARAM_ERR);
    
    // Init parsing
    rv = synth_parser_initParsef(&ctx, filename);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Parse the stream
    rv = synth_parser_audio(ctx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Get the generated audio
    *audio = synth_parser_getAudio(ctx);
    
    rv = SYNTH_OK;
__err:
    // Clean up memory
    synth_parser_clean(&ctx);
    
    return rv;
}

/**
 * Parse a mml string into an audio
 * 
 * @param ctx Variable that will store the audio
 * @param mml String with the mml audio (it needn't be null-terminated)
 * @param len String's length
 * @return The error code
 */
synth_err synth_audio_loads(synthAudio **audio, char *mml, int len) {
    synth_err rv;
    synthParserCtx *ctx = 0;
    
    // Sanitize input
    SYNTH_ASSERT_ERR(audio, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(mml, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(len > 0, SYNTH_BAD_PARAM_ERR);
    
    // Init parsing
    rv = synth_parser_initParses(&ctx, mml, len);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Parse the stream
    rv = synth_parser_audio(ctx);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    // Get the generated audio
    *audio = synth_parser_getAudio(ctx);
    
    rv = SYNTH_OK;
__err:
    // Clean up memory
    synth_parser_clean(&ctx);
    
    return rv;
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
            if (tmp->tracks[i]) {
                synth_track_clean(tmp->tracks[i]);
                free(tmp->tracks[i]);
            }
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

/**
 * Reset and play an audio
 */
void synth_audio_playAudio(synthAudio *audio) {
    
}

/**
 * Play a single track audio
 * If called consecutive times on the same source, they'll stack
 */
synth_err synth_audio_playSFX(synthAudio *audio) {
    return SYNTH_FUNCTION_NOT_IMPLEMENTED;
}

/**
 * Stop (and reset) the previous bgm and play the new one
 */
void synth_audio_playBGM(synthAudio *audio) {
    
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

/**
 * Synthesize part of this audio to a buffer
 * 
 * @param audio Audio to be synthesized
 * @param samples How many samples there still are in this "slice"
 * @param left Left output buffer
 * @param right Right output buffer
 */
void synth_audio_synthesize(synthAudio *aud, int samples, uint16_t *left,
    uint16_t *right) {
    int i;
    
    i = 0;
    while (i < aud->len) {
        synthTrack *track;
        
        track = aud->tracks[i];
        synth_track_synthesize(track, samples, left, right);
        
        i++;
    }
}

/**
 * Reset an audio
 * 
 * @param The audio
 */
void synth_audio_reset(synthAudio *aud) {
    int i;
    
    i = 0;
    while (i < aud->len) {
        synth_track_reset(aud->tracks[i]);
        i++;
    }
}

/**
 * Whether the audio finished playing
 * 
 * @param audio The audio
 * @return Error code
 */
synth_bool synth_audio_didFinish(synthAudio *audio) {
    synth_bool rv;
    int i;
    
    rv = SYNTH_FALSE;
    
    // Check that every track finished playing
    i = 0;
    while (i < audio->len) {
        SYNTH_ASSERT(synth_track_didFinish(audio->tracks[i]) == SYNTH_TRUE);
        i++;
    }
    
    rv = SYNTH_TRUE;
__err:
    return rv;
}


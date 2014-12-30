/**
 * @file src/synth_track.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_note.h>
#include <synth_internal/synth_track.h>

#include <stdlib.h>

/**
 * Initialize the track
 * 
 * @param track The track
 * @return Error code (SYNTH_OK on success)
 */
synth_err synth_track_init(synthTrack *track) {
    synth_err rv;
    
    // Initialize the track as non-looping
    synth_track_setLoopPoint(track, -1);
    track->len = 0;
    track->pos = 0;
    
    // Initialize the notes array with a single note
    track->notes = (synthNote**)malloc(sizeof(synthNote*));
    SYNTH_ASSERT_ERR(track->notes, SYNTH_MEM_ERR);
    track->buflen = 1;
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Clean up the track
 * 
 * @param track The track
 */
void synth_track_clean(synthTrack *track) {
    synth_track_setLoopPoint(track, -1);
    track->len = 0;
    // If necessary, clear up the notes array
    // Note that notes must be cleared by the caller
    if (track->notes) {
        free(track->notes);
        track->notes = 0;
    }
    track->buflen = 0;
}

/**
 * Adds a single note to the track and expand it, if necessary
 * 
 * @param track Track where the note will be added
 * @param note Note to be added to the track
 */
synth_err synth_track_addNote(synthTrack *track, synthNote *note) {
    synth_err rv;
    
    // Check if there's still space for the new note
    if (track->len >= track->buflen) {
        track->buflen *= 2;
        track->notes = (synthNote**)realloc(track->notes,
            track->buflen*sizeof(synthNote*));
        SYNTH_ASSERT_ERR(track->notes, SYNTH_MEM_ERR);
    }
    
    // Append the note at the end
    track->notes[track->len] = note;
    track->len++;
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Set the track loop point
 * 
 * @param track Track where the note will be added
 * @param loopPoint At which note the track should loop (0 indexed)
 */
void synth_track_setLoopPoint(synthTrack *track, int loopPoint) {
    track->loopPoint = loopPoint;
}

/**
 * Get the current track length
 * 
 * @param track Track where the note will be added
 * @return Current track length in notes
 */
int synth_track_getLength(synthTrack *track) {
    return track->len;
}

/**
 * Return whether the track finished playing
 * This will never return true if the track can loop.
 * 
 * @param track Track where the note will be added
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_track_didFinish(synthTrack *track) {
    if (track->loopPoint == -1 && track->pos >= track->len)
        return SYNTH_TRUE;
    return SYNTH_FALSE;
}

/**
 * Synthesize part of this track to a buffer
 * 
 * @param track Track to be synthesized
 * @param samples How many samples there still are in this "slice"
 * @param left Left output buffer
 * @param right Right output buffer
 */
void synth_track_synthesize(synthTrack *track, int samples, uint16_t *left,
    uint16_t *right) {
    // remainder
    int rem;
    
    rem = samples;
    while (1) {
        int prev;
        
        prev = rem;
        // Synthesize samples for the current note
        rem = synth_note_synthesize(track->notes[track->pos], rem, left,
            right);
        
        if (synth_note_didFinish(track->notes[track->pos]) == SYNTH_TRUE) {
            // Check if the current note is a loop point and if there's any jump
            if (synth_note_doLoop(track->notes[track->pos]) == SYNTH_TRUE)
                // Go to the loop point and increase the counter
                track->pos =
                    synth_note_jumpToPosition(track->notes[track->pos]);
            // Otherwise, go to the next note
            else {
                synth_note_reset(track->notes[track->pos]);
                track->pos++;
                
                // Check if we should loop or stop the track
                if (track->pos >= track->len) {
                    if (track->loopPoint != -1)
                        track->pos = track->loopPoint;
                    else
                        break;
                }
            }
        }
        // Stop if there are no samples left
        if (rem == 0)
            break;
        
        // Update buffer position
        left += prev - rem;
        right += prev - rem;
    }
}


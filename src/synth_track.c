/**
 * A sequence of notes
 * 
 * @file src/synth_track.c
 */
#include <synth/synth.h>
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>

#include <synth_internal/synth_note.h>
#include <synth_internal/synth_track.h>
#include <synth_internal/synth_types.h>

#include <stdlib.h>
#include <string.h>

/**
 * Retrieve a new track (alloc it as possible and necessary)
 * 
 * @param  [out]ppTrack The new track
 * @param  [ in]pCtx    The synthesizer context
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthTrack_init(synthTrack **ppTrack, synthCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(ppTrack, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Make sure there's enough space for another track */
    SYNTH_ASSERT_ERR(pCtx->tracks.max == 0 ||
            pCtx->tracks.used < pCtx->tracks.max, SYNTH_MEM_ERR);

    /* Expand the array as necessary */
    if (pCtx->tracks.used >= pCtx->tracks.len) {
        /* 'Double' the current buffer; Note that this will never be called if
         * the context was pre-alloc'ed, since 'max' will be set; The '+1' is
         * for the first audio, in which len will be 0 */
        pCtx->tracks.buf.pTracks = (synthTrack*)realloc(
                pCtx->tracks.buf.pTracks, (1 + pCtx->tracks.len * 2) *
                sizeof(synthTrack));
        SYNTH_ASSERT_ERR(pCtx->tracks.buf.pTracks, SYNTH_MEM_ERR);
        /* Clear only the new part of the buffer */
        memset(&(pCtx->tracks.buf.pTracks[pCtx->tracks.used]), 0x0,
                (1 + pCtx->tracks.len) * sizeof(synthTrack));
        /* Actually increase the buffer length */
        pCtx->tracks.len += 1 + pCtx->tracks.len;
    }

    /* Retrieve the next track */
    *ppTrack = &(pCtx->tracks.buf.pTracks[pCtx->tracks.used]);
    pCtx->tracks.used++;

    /* Initialize the track as not being looped and without any notes */
    (*ppTrack)->loopPoint = -1;
    (*ppTrack)->notesIndex = pCtx->notes.used;

    rv = SYNTH_OK;
__err:
    return rv;
}

/* !!!WARNING!!! This functions calls 'synthTrack_countSample', which in turn
 * MAY call this function back. Since the song is parsed from a file or from a
 * string it's guaranteed to end and there should be no infinite loops.
 * 
 * However, beware that "bad things may happen"... */
static synth_err synthTrack_getLoopLength(int *pLen, synthTrack *pTrack,
        synthCtx *pCtx, int pos);

/**
 * Loop through some notes and accumulate their samples
 * 
 * This was mostly done to avoid repeating the loop count, thus all
 * verifications are done in previous calls
 * 
 * Also, don't forget that the loop is done in inverse order, from the last note
 * to the first. So, initialPos must always be greater than finalPosition
 * 
 * @param  [out]pLen         The length of the track in samples
 * @param  [ in]pTrack       The track
 * @param  [ in]pCtx         The synthesizer context
 * @param  [ in]initalPos    Initial position (inclusive)
 * @param  [ in]finalPos     Final position (inclusive)
 * @return                   SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
static synth_err synthTrack_countSample(int *pLen, synthTrack *pTrack,
        synthCtx *pCtx, int initialPos, int finalPosition) {
    int i, len;
    synthNote *pNote;
    synth_err rv;

    len = 0;

    /* Simply loop though all notes */
    i = initialPos;
    while (i >= finalPosition) {
        int tmp;

        /* Retrieve the current note */
        pNote = &(pCtx->notes.buf.pNotes[pTrack->notesIndex + i]);

        /* Check if note is a loop */
        if (synthNote_isLoop(pNote) == SYNTH_TRUE) {
            int pos;

            /* Retrieve the length of the loop, in samples (already taking into
             * account the number of repetitions) */
            rv = synthTrack_getLoopLength(&tmp, pTrack, pCtx, i);
            SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

            /* Get the loop destination */
            rv = synthNote_getJumpPosition(&pos, pNote);
            SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
            /* Update the curent note accordingly */
            i = pos;

            len += tmp;
        }
        else {
            /* Accumulate the note duration */
            rv = synthNote_getDuration(&tmp, pNote);
            SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

            len += tmp;
        }

        i--;
    }

    *pLen = len;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Count how many samples there are in a loop
 * 
 * Since loops are recursive, this function may also be called recursively
 * 
 * @param  [out]pLen   The length of the track in samples
 * @param  [ in]pTrack The track
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]pos    Position of the loop note
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
static synth_err synthTrack_getLoopLength(int *pLen, synthTrack *pTrack,
        synthCtx *pCtx, int pos) {
    int jumpPosition, repeatCount;
    synthNote *pNote;
    synth_err rv;

    /* Retrieve the current note */
    pNote = &(pCtx->notes.buf.pNotes[pTrack->notesIndex + pos]);

    /* Retrieve the loop attributes */
    rv = synthNote_getRepeat(&repeatCount, pNote);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    rv = synthNote_getJumpPosition(&jumpPosition, pNote);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Loop through all notes and calculate the total length */
    rv = synthTrack_countSample(pLen, pTrack, pCtx, pos - 1, jumpPosition);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    *pLen *= repeatCount;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Retrieve the number of samples in a track
 * 
 * @param  [out]pLen   The length of the track in samples
 * @param  [ in]pTrack The track
 * @param  [ in]pCtx   The synthesizer context
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthTrack_getLength(int *pLen, synthTrack *pTrack, synthCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pLen, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pTrack, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Count from the last note so we can recursivelly calculate all loops lengths */
    rv = synthTrack_countSample(pLen, pTrack, pCtx, pTrack->num - 1, 0);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Retrieve the number of samples until a track's loop point
 * 
 * @param  [out]pLen   The length of the track's intro
 * @param  [ in]pTrack The track
 * @param  [ in]pCtx   The synthesizer context
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthTrack_getIntroLength(int *pLen, synthTrack *pTrack,
        synthCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pLen, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pTrack, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Count how many samples there are from the loop point to the song start */
    if (pTrack->loopPoint != -1) {
        rv = synthTrack_countSample(pLen, pTrack, pCtx, pTrack->loopPoint - 1,
                0);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    }
    else {
        *pLen = 0;
    }

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Renders a sequence of notes
 * 
 * Note that the track is rendered in inverse order, from the last to the first
 * note. Therefore, 'i' must be greater than 'dst' (though it isn't checked!!)
 * 
 * @param  [out]pBytes How many bytes were rendered (so the buffer's position
 *                     can be updated accordingly)
 * @param  [ in]pBuf   End of the buffer to be filled with the rendered sequence
 * @param  [ in]pTrack The track
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]mode   Current rendering mode
 * @param  [ in]i      Current position into the sequence of notes
 * @param  [ in]dst    Last note to be rendered
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR, ...
 */
static synth_err synthTrack_renderSequence(int *pBytes, char *pBuf,
        synthTrack *pTrack, synthCtx *pCtx, synthBufMode mode, int i, int dst) {
    int bytes;
    synth_err rv;

    bytes = 0;

    while (i >= dst) {
        synthNote *pNote;

        /* Retrieve the current note */
        pNote = &(pCtx->notes.buf.pNotes[pTrack->notesIndex + i]);

        /* Check if it's a loop or a common note */
        if (synthNote_isLoop(pNote) == SYNTH_TRUE) {
            int count, jumpPosition, repeatCount, tmpBytes;

            /* Get the loop parameters */
            rv = synthNote_getRepeat(&repeatCount, pNote);
            SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
            rv = synthNote_getJumpPosition(&jumpPosition, pNote);
            SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

            /* Render the loop and any sub-loops */
            rv = synthTrack_renderSequence(&tmpBytes, pBuf, pTrack, pCtx, mode,
                    i - 1, jumpPosition);
            SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

            /* Move the buffer back as many bytes as were rendered */
            pBuf -= tmpBytes;

            /* Copy the sequence back into the buffer as many times as
             * necessary */
            count = 1;
            while (count < repeatCount) {
                memcpy(pBuf - tmpBytes, pBuf, tmpBytes);
                count++;
                /* Move the buffer back before the sequence */
                pBuf -= tmpBytes;
            }

            /* Update the number of bytes rendered */
            bytes += tmpBytes * repeatCount;

            /* Place the buffer as if it just rendered the last note on the
             * loop (it will be decreased afterward */
            i = jumpPosition;
        }
        else {
            int duration;

            /* Get the note's duration in samples */
            rv = synthNote_getDuration(&duration, pNote);
            SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

            /* Convert the number of samples into bytes */
            if (mode & SYNTH_16BITS) {
                duration *= 2;
            }
            if (mode & SYNTH_2CHAN) {
                duration *= 2;
            }

            /* Place the buffer at the start of the note */
            pBuf -= duration;

            /* Render the current note */
            rv = synthNote_render(pBuf, pNote, mode, pCtx->frequency);
            SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

            /* Update the amount of bytes rendered */
            bytes += duration;
        }

        i--;
    }

    *pBytes = bytes;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Render a full track into a buffer
 * 
 * The buffer must be prepared by the caller, and it must have
 * 'synth_getTrackLength' bytes times the number of bytes per samples
 * 
 * @param  [ in]pBuf   Buffer that will be filled with the track
 * @param  [ in]pTrack The track
 * @param  [ in]pCtx   The synthesizer context
 * @param  [ in]mode   Desired mode for the wave
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthTrack_render(char *pBuf, synthTrack *pTrack, synthCtx *pCtx,
        synthBufMode mode) {
    int len, tmp;
    synth_err rv;

    /* Retrieve the track's duration in samples */
    rv = synthTrack_getLength(&len, pTrack, pCtx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    /* Convert the number of samples into bytes */
    if (mode & SYNTH_16BITS) {
        len *= 2;
    }
    if (mode & SYNTH_2CHAN) {
        len *= 2;
    }

    /* Place the buffer at its expected end */
    pBuf += len;

    /* Loop through all notes and render 'em */
    rv = synthTrack_renderSequence(&tmp, pBuf, pTrack, pCtx, mode,
            pTrack->num - 1, 0);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);

    rv = SYNTH_OK;
__err:
    return rv;
}

#if 0

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
    track->notepos = 0;
    
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
    
    SYNTH_ASSERT(synth_track_didFinish(track) == SYNTH_FALSE);
    
    rem = samples;
    while (1) {
        int prev;
        
        prev = rem;
        // Synthesize samples for the current note
        rem = synth_note_synthesizeHacky(track->notes[track->pos], rem, left,
            right, &(track->notepos));
        
        if (synth_note_didFinishHacky(track->notes[track->pos], track->notepos) == SYNTH_TRUE) {
            track->notepos = 0;
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
    
__err:
    return;
}

/**
 * Reset a track
 * 
 * @param track The track
 */
void synth_track_reset(synthTrack *track) {
    int i;
    
    // Reset every note on this track
    i = 0;
    while (i < track->len) {
        synth_note_reset(track->notes[i]);
        i++;
    }
    track->notepos = 0;
    track->pos = 0;
}

#endif


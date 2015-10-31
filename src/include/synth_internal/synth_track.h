/**
 * A sequence of notes
 * 
 * @file src/include/synth_internal/synth_track.h
 */
#ifndef __SYNTH_TRACK_H__
#define __SYNTH_TRACK_H__

#include <synth/synth_errors.h>

#include <synth_internal/synth_types.h>

/**
 * Retrieve a new track (alloc it as possible and necessary)
 * 
 * @param  [out]ppTrack The new track
 * @param  [ in]pCtx    The synthesizer context
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthTrack_init(synthTrack **ppTrack, synthCtx *pCtx);

#if 0

#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_note.h>

/**
 * Initialize the track
 * 
 * @param track The track
 * @return Error code (SYNTH_OK on success)
 */
synth_err synth_track_init(synthTrack *track);

/**
 * Clean up the track
 * 
 * @param track The track
 */
void synth_track_clean(synthTrack *track);

/**
 * Adds a single note to the track and expand it, if necessary
 * 
 * @param track Track where the note will be added
 * @param note Note to be added to the track
 */
synth_err synth_track_addNote(synthTrack *track, synthNote *note);

/**
 * Set the track loop point
 * 
 * @param track Track where the note will be added
 * @param loopPoint At which note the track should loop (0 indexed)
 */
void synth_track_setLoopPoint(synthTrack *track, int loopPoint);

/**
 * Get the current track length
 * 
 * @param track Track where the note will be added
 * @return Current track length in notes
 */
int synth_track_getLength(synthTrack *track);

/**
 * Return whether the track finished playing
 * This will never return true if the track can loop.
 * 
 * @param track Track where the note will be added
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
synth_bool synth_track_didFinish(synthTrack *track);

/**
 * Synthesize part of this track to a buffer
 * 
 * @param track Track to be synthesized
 * @param samples How many samples there still are in this "slice"
 * @param left Left output buffer
 * @param right Right output buffer
 */
void synth_track_synthesize(synthTrack *track, int samples, uint16_t *left,
    uint16_t *right);

/**
 * Reset a track
 * 
 * @param track The track
 */
void synth_track_reset(synthTrack *track);

#endif /* 0 */

#endif /* __SYNTH_TRACK_H__ */


/**
 * @file src/include/synth_internal/synth_audiolist.h
 * 
 * Linked-list for keeping track of which audio should be played
 */
#ifndef __SYNTH_AUDIOLIST_H_
#define __SYNTH_AUDIOLIST_H_

#include <synth/synth_audio.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>

/**
 * Initialize necessary stuff
 */
synth_err synth_list_init();

/**
 * Clean up memory used by linked lists
 */
void synth_list_clean();

/**
 * Thread-safe. Set up a bgm change
 * 
 * @param aud The new bgm
 * @return Error code
 */
void synth_list_setBgm(synthAudio *aud);
/**
 * Thread-safe. Stop the currently playing bgm
 */
void synth_list_stopBgm();
/**
 * Thread-safe. Set up an audio addition.
 * The audio will be reset, so be sure it's not playing already
 * 
 * @param aud The audio to be added
 * @return Error code
 */
synth_err synth_list_addAudio(synthAudio *aud);
/**
 * Thread-safe. Set up a sfx addition.
 * Different from an audio, this keep track of the current position on the
 * audio, but must be a single track.
 * 
 * @param aud The sfx to be added
 * @return Error code
 */
synth_err synth_list_addSfx(synthAudio *aud);

/**
 * Thread-safe. Merge the the back list to the playing one. Also, update the playing
 * bgm
 */
void synth_list_merge();

/**
 * Synthesize every 
 * 
 * @param samples How many samples will be synthesized
 * @param left The left buffer
 * @param right The right buffer
 */
void synth_list_synthesize(int samples, uint16_t *left, uint16_t *right);

#endif


/**
 * @file src/include/synth_internal/synth_cache.h
 */
#ifndef __SYNTH_CACHE_H_
#define __SYNTH_CACHE_H_

typedef struct stSynthVolume synthVolume;
typedef struct stSynthNote synthNote;

/**
 * Initialize every global cache
 * 
 * @return Error code
 */
synth_err synth_cache_init(int freq);

/**
 * Clean the global caches
 */
void synth_cache_clean();

/**
 * Get the frequency
 * 
 * @return The frequency
 */
int synth_cache_getFrequency();

/**
 * Search for a given note in the cache and alloc it, if not found
 * 
 * @param pNote Reference where the note will be returned
 * @param n The musical note
 * @param d The note duration
 * @param bpm The audio BPM
 * @param o The note's octave
 * @param p The note pan
 * @param q The note keyoff
 * @param v The note volume
 * @param w The note wave
 * @return The error code
 */
synth_err synth_cache_getNote(synthNote **pNote, synth_note n, int d, int bpm,
    int o, int p, int q, synthVolume *v, synth_wave w);

/**
 * Add a new note to the buffer, expanding it if necessary
 * 
 * @param note The note
 * @return Error code
 */
synth_err synth_cache_addNote(synthNote *note);

/**
 * Search for a given volume in the cache and alloc it, if not found
 * 
 * @param pVol Reference where the volume will be returned
 * @param ini Initial amplitude
 * @param fin Final amplitude
 * @return The error code
 */
synth_err synth_cache_getVolume(synthVolume **pVol, char ini, char fin);

/**
 * Add a new volume to the buffer, expanding it if necessary
 * 
 * @param vol The volume
 * @return Error code
 */
synth_err synth_cache_addVolume(synthVolume *vol);

/**
 * Add a new loop to the cache
 * 
 * @param pLoop Reference where the note will be returned
 * @param loopPos Where should loop to
 * @param count How many times it should loop
 * @return The error code
 */
synth_err synth_cache_getLoop(synthNote **pLoop, int loopPos, int count);

/**
 * Add a new loop note to the buffer, expanding it if necessary
 * 
 * @param loop The loop note
 * @return Error code
 */
synth_err synth_cache_addLoop(synthNote *loop);

#endif


/**
 * @file src/synth_cache.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_cache.h>
#include <synth_internal/synth_note.h>
#include <synth_internal/synth_volume.h>

#include <stdlib.h>

/**
 * Frequency of the synthesizer (in samples per minute)
 */
static int gl_freq;

/**
 * Buffer for storing notes (but not loop notes!)
 */
static int nlen;
static int nbuflen;
static synthNote **notes;

/**
 * Buffer for storing volumes
 */
static int vlen;
static int vbuflen;
static synthVolume **volumes;

/**
 * Ugly buffer for storing loops (totally un-optmized memory wise)
 */
static int llen;
static int lbuflen;
static synthNote **loops;

/**
 * Initialize every global cache
 * 
 * @return Error code
 */
synth_err synth_cache_init(int freq) {
    synth_err rv;
    
    // Set global frequency
    gl_freq = freq;
    
    // In case notes allocation fails, this must be initialized
    volumes = 0;
    loops = 0;
    
    // Alloc the notes array and its size
    notes = (synthNote**)malloc(sizeof(synthNote*));
    SYNTH_ASSERT_ERR(notes, SYNTH_MEM_ERR);
    
    nlen = 0;
    nbuflen = 1;
    
    // Alloc the volumes array and its size
    volumes = (synthVolume**)malloc(sizeof(synthVolume*));
    SYNTH_ASSERT_ERR(notes, SYNTH_MEM_ERR);
    
    vlen = 0;
    vbuflen = 1;
    
    // Alloc the loops array and its size
    loops = (synthNote**)malloc(sizeof(synthNote*));
    SYNTH_ASSERT_ERR(notes, SYNTH_MEM_ERR);
    
    llen = 0;
    lbuflen = 1;
    
    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK)
        synth_cache_clean();
    
    return rv;
}

/**
 * Clean the global caches
 */
void synth_cache_clean() {
    int i;
    
    // Clean up the notes array, and any of its objects
    if (notes) {
        i = 0;
        while (i < nlen) {
            synth_note_clean(notes[i]);
            i++;
        }
        free(notes);
        
        nlen = 0;
        nbuflen = 0;
        notes = 0;
    }
    
    // Clean up the volumes array, and any of its objects
    if (volumes) {
        i = 0;
        while (i < vlen) {
            synth_vol_clean(volumes[i]);
            i++;
        }
        free(volumes);
        
        vlen = 0;
        vbuflen = 0;
        volumes = 0;
    }
    
    // Clean up the notes array, and any of its objects
    if (loops) {
        i = 0;
        while (i < llen) {
            synth_note_clean(loops[i]);
            i++;
        }
        free(loops);
        
        llen = 0;
        lbuflen = 0;
        loops = 0;
    }
   
}

/**
 * Get the frequency
 * 
 * @return The frequency
 */
int synth_cache_getFrequency() {
    return gl_freq;
}

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
    int o, int p, int q, synthVolume *v, synth_wave w) {
    synth_err rv;
    int i, l;
    
    // Get the note lenth in samples
    l = synth_note_getSampleSize(bpm, d);
    
    i = 0;
    // Search for the note in the cache
    while (i < nlen) {
        synthNote *note;
        
        note = notes[i];
        // Check if the note is the one requested
        if (synth_note_getNote(note) == n && 
            synth_note_getOctave(note) == o &&
            synth_note_getLen(note) == l &&
            synth_note_getVolume(note) == v &&
            synth_note_getPan(note) == p &&
            synth_note_getKeyoff(note) == q &&
            synth_note_getWave(note) == w) {
            // Since we found the note, stop
            *pNote = note;
            break;
        }
        
        i++;
    }
    
    // If we didn't find the note, add a new one with those specs
    if (i == nlen) {
        synthNote *note;
        
        note = (synthNote*)malloc(sizeof(synthNote));
        SYNTH_ASSERT_ERR(note, SYNTH_MEM_ERR);
        
        synth_note_init(note);
        synth_note_setPan(note, p);
        synth_note_setOctave(note, o);
        synth_note_setDuration(note, bpm, d);
        synth_note_setWave(note, w);
        synth_note_setNote(note, n);
        synth_note_setVolume(note, v);
        synth_note_setKeyoff(note, q);
        
        rv = synth_cache_addNote(note);
        SYNTH_ASSERT(rv == SYNTH_OK);
        
        *pNote = note;
    }
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Add a new note to the buffer, expanding it if necessary
 * 
 * @param note The note
 * @return Error code
 */
synth_err synth_cache_addNote(synthNote *note) {
    synth_err rv;
    
    // Extend the buffer, if necessary
    if (nlen >= nbuflen) {
        notes = (synthNote**)realloc(notes, sizeof(synthNote*)*nbuflen*2);
        SYNTH_ASSERT_ERR(notes, SYNTH_MEM_ERR);
        
        nbuflen *= 2;
    }
    
    // Append the note
    notes[nlen] = note;
    nlen++;
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Search for a given volume in the cache and alloc it, if not found
 * 
 * @param pVol Reference where the volume will be returned
 * @param ini Initial amplitude
 * @param fin Final amplitude
 * @return The error code
 */
synth_err synth_cache_getVolume(synthVolume **pVol, char ini, char fin) {
    synth_err rv;
    int i;
    
    
    i = 0;
    // Search for the note in the cache
    while (i < vlen) {
        synthVolume *vol;
        
        vol = volumes[i];
        // Check if the note is the one requested
        if (synth_vol_getVolumeIni(vol) == ini &&
            synth_vol_getVolumeFin(vol) == fin) {
            // Since we found the note, stop
            *pVol = vol;
            break;
        }
        
        i++;
    }
    
    // If we didn't find the note, add a new one with those specs
    if (i == vlen) {
        synthVolume *vol;
        
        vol = (synthVolume*)malloc(sizeof(synthVolume));
        SYNTH_ASSERT_ERR(vol, SYNTH_MEM_ERR);
        
        synth_vol_init(vol);
        synth_vol_setLinear(vol, ini, fin);
        
        rv = synth_cache_addVolume(vol);
        SYNTH_ASSERT(rv == SYNTH_OK);
        
        *pVol = vol;
    }
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Add a new volume to the buffer, expanding it if necessary
 * 
 * @param vol The volume
 * @return Error code
 */
synth_err synth_cache_addVolume(synthVolume *vol) {
    synth_err rv;
    
    // Extend the buffer, if necessary
    if (vlen >= vbuflen) {
        volumes = (synthVolume**)realloc(volumes,
            sizeof(synthVolume*)*vbuflen*2);
        SYNTH_ASSERT_ERR(volumes, SYNTH_MEM_ERR);
        
        vbuflen *= 2;
    }
    
    // Append the note
    volumes[vlen] = vol;
    vlen++;
    
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Add a new loop to the cache
 * 
 * @param pLoop Reference where the note will be returned
 * @param loopPos Where should loop to
 * @param count How many times it should loop
 * @return The error code
 */
synth_err synth_cache_getLoop(synthNote **pLoop, int loopPos, int count) {
    synthNote *loop;
    synth_err rv;
    
    loop = (synthNote*)malloc(sizeof(synthNote));
    SYNTH_ASSERT_ERR(loop, SYNTH_MEM_ERR);
    
    synth_note_init(loop);
    synth_note_setNote(loop, N_LOOP);
    synth_note_setJumpPosition(loop, loopPos);
    synth_note_setRepeatTimes(loop, count);
    
    rv = synth_cache_addLoop(loop);
    SYNTH_ASSERT(rv == SYNTH_OK);
    
    *pLoop = loop;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Add a new loop note to the buffer, expanding it if necessary
 * 
 * @param loop The loop note
 * @return Error code
 */
synth_err synth_cache_addLoop(synthNote *loop) {
    synth_err rv;
    
    // Extend the buffer, if necessary
    if (llen >= lbuflen) {
        loops = (synthNote**)realloc(loops, sizeof(synthNote*)*lbuflen*2);
        SYNTH_ASSERT_ERR(loops, SYNTH_MEM_ERR);
        
        lbuflen *= 2;
    }
    
    // Append the note
    loops[llen] = loop;
    llen++;
    
    rv = SYNTH_OK;
__err:
    return rv;
}

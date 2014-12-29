/**
 * @file src/include/synth_internal/synth_volume.h
 * 
 * A simple volume envelop. Will, someday, allow for more complexes functions.
 */
#ifndef __SYNTH_VOLUME_H_
#define __SYNTH_VOLUME_H_

struct stSynthVolume {
    /**
     * Initial volume
     */
    char ini;
    /**
     * Final volume
     */
    char fin;
};
typedef struct stSynthVolume synthVolume;

/**
 * Initialize the volume with default values
 * 
 * @param vol The volume
 */
void synth_vol_init(synthVolume *vol);

/**
 * Clean up the volume (unnecessary, for now)
 * 
 * @param vol The volume
 */
void synth_vol_clean(synthVolume *vol);

/**
 * Set the volume as a simple linear function
 * 
 * @param vol The volume
 * @param ini The initial volume
 * @param fin The final volume
 */
void synth_vol_setLinear(synthVolume *vol, int ini, int fin);

/**
 * Gets the volume at a given percentage of the note
 * 
 * @param vol A synthVolume pointer
 * @param perc A number between 0 and 1024 representing how much of the note
 * has already been played
 * @return Volume at the given percentage
 */
char synth_vol_getVolume(synthVolume *vol, int perc);

#endif


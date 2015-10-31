/**
 * A 'volume' represents a function on the time domain, which controls each the
 * volume for each sample on a note; For simplicity (on the lib's side),
 * whenever a new volume is created it can never be deleted; However, if two
 * notes shares the same function, they will point to the same 'volume object'
 * 
 * Volumes are stored in a simple array, since they are only searched and
 * created on compilation time; So, even if lots of volumes are used and they
 * slow down the compilation, there will be no side effects while rendering the
 * song
 * 
 * @file src/include/synth_internal/synth_volume.h
 */
#ifndef __SYNTH_VOLUME_H__
#define __SYNTH_VOLUME_H__

#include <synth/synth_errors.h>

#include <synth_internal/synth_types.h>

/**
 * Retrieve a constant volume
 * 
 * If the required volume isn't found, it will be instantiated and returned
 * 
 * @param  [out]ppVol The volume
 * @param  [ in]pCtx  The synthesizer context
 * @param  [ in]amp   The requested amplitude (in the range [0, 255])
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthVolume_getConst(synthVolume **ppVol, synthCtx *pCtx, char amp);

/**
 * Retrieve a linear volume
 * 
 * If the required volume isn't found, it will be instantiated and returned
 * 
 * @param  [out]ppVol The volume
 * @param  [ in]pCtx  The synthesizer context
 * @param  [ in]ini   The initial amplitude (in the range [0, 255])
 * @param  [ in]fin   The final amplitude (in the range [0, 255])
 * @return            SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthVolume_getLinear(synthVolume **ppVol, synthCtx *pCtx, char ini,
        char fin);

#if 0

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
 * @param amp The volume's amplitude
 */
void synth_vol_setConst(synthVolume *vol, char amp);

/**
 * Set the volume as a simple linear function
 * 
 * @param vol The volume
 * @param ini The initial volume
 * @param fin The final volume
 */
void synth_vol_setLinear(synthVolume *vol, char ini, char fin);

/**
 * Gets the volume at a given percentage of the note
 * 
 * @param vol A synthVolume pointer
 * @param perc A number between 0 and 1024 representing how much of the note
 * has already been played
 * @return Volume at the given percentage
 */
char synth_vol_getVolume(synthVolume *vol, int perc);

/**
 * Get the volume's initial amplitude
 * 
 * @param vol A synthVolume pointer
 * @return Volume's initial amplitude
 */
char synth_vol_getVolumeIni(synthVolume *vol);

/**
 * Get the volume's final amplitude
 * 
 * @param vol A synthVolume pointer
 * @return Volume's final amplitude
 */
char synth_vol_getVolumeFin(synthVolume *vol);

#endif /* 0 */

#endif /* __SYNTH_VOLUME_H__ */


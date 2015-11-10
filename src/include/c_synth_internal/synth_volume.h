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

#include <c_synth/synth_errors.h>

#include <c_synth_internal/synth_types.h>

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
synth_err synthVolume_getConst(synthVolume **ppVol, synthCtx *pCtx, int amp);

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
synth_err synthVolume_getLinear(synthVolume **ppVol, synthCtx *pCtx, int ini,
        int fin);

/**
 * Retrieve the volume at a given percentage of a note
 * 
 * @param  [out]pAmp The note's amplitude
 * @param  [ in]pVol The volume
 * @param  [ in]perc Percentage into the note (in the range [0, 1024))
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthVolume_getAmplitude(int *pAmp, synthVolume *pVol, int perc);

#endif /* __SYNTH_VOLUME_H__ */


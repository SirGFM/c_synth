/**
 * @file src/synth_volume.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>

#include <synth_internal/synth_volume.h>
#include <synth_internal/synth_types.h>

/**
 * Set the volume as a simple linear function
 * 
 * @param [ in]pCtx The volume
 * @param [ in]amp  The volume's amplitude
 * @return          SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthVolume_setConst(synthVolume *pCtx, char amp) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    pCtx->ini = 0x7f;
    pCtx->fin = 0x7f;
    
    rv = SYNTH_OK;
__err:
    return rv;
}

#if 0
/**
 * Initialize the volume with default values
 * 
 * @param vol The volume
 */
void synth_vol_init(synthVolume *vol) {
    vol->ini = 0x7f;
    vol->fin = 0x7f;
}

/**
 * Clean up the volume (unnecessary, for now)
 * 
 * @param vol The volume
 */
void synth_vol_clean(synthVolume *vol) {
    synth_vol_init(vol);
}

void synth_vol_setConst(synthVolume *vol, char amp) {
    vol->ini = amp;
    vol->fin = amp;
}

/**
 * Set the volume as a simple linear function
 * 
 * @param vol The volume
 * @param ini The initial volume
 * @param fin The final volume
 */
void synth_vol_setLinear(synthVolume *vol, char ini, char fin) {
    vol->ini = ini;
    vol->fin = fin;
}

/**
 * Gets the volume at a given percentage of the note
 * 
 * @param vol A synthVolume pointer
 * @param perc A number between 0 and 1024 representing how much of the note
 * has already been played
 * @return Volume at the given percentage
 */
char synth_vol_getVolume(synthVolume *vol, int perc) {
    int val;
    
    val = (((vol->fin - vol->ini) * perc) >> 10) + vol->ini;
    
    return val & 0xff;
}

/**
 * Get the volume's initial amplitude
 * 
 * @param vol A synthVolume pointer
 * @return Volume's initial amplitude
 */
char synth_vol_getVolumeIni(synthVolume *vol) {
    return vol->ini;
}

/**
 * Get the volume's final amplitude
 * 
 * @param vol A synthVolume pointer
 * @return Volume's final amplitude
 */
char synth_vol_getVolumeFin(synthVolume *vol) {
    return vol->fin;
}

#endif /* 0 */


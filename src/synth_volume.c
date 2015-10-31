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
 * @file src/synth_volume.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>

#include <synth_internal/synth_volume.h>
#include <synth_internal/synth_types.h>

#include <stdlib.h>
#include <string.h>

/**
 * Alloc and initialize a new volume
 * 
 * @param [out]ppVol The new volume
 * @param [ in]pCtx  The synthesizer context
 * @return           SYNTH_OK, SYNTH_MEM_ERR
 */
static synth_err synthVolume_init(synthVolume **ppVol, synthCtx *pCtx) {
    synth_err rv;

    /* Make sure there's enough space for another volume */
    SYNTH_ASSERT_ERR(pCtx->volumes.max == 0 ||
            pCtx->volumes.used < pCtx->volumes.max, SYNTH_MEM_ERR);

    /* Check if the buffer must be expanded and do so */
    if (pCtx->volumes.used >= pCtx->volumes.len) {
        /* 'Double' the current buffer; Note that this will never be called if
         * the context was pre-alloc'ed, since 'max' will be set; The '+1' is
         * for the first volume, in which len will be 0 */
        pCtx->volumes.buf.pVolumes = (synthVolume*)realloc(
                pCtx->volumes.buf.pVolumes, (1 + pCtx->volumes.len * 2) *
                sizeof(synthVolume));
        SYNTH_ASSERT_ERR(pCtx->volumes.buf.pVolumes, SYNTH_MEM_ERR);
        /* Clear only the new part of the buffer */
        memset(&(pCtx->volumes.buf.pVolumes[pCtx->volumes.used]), 0x0,
                (1 + pCtx->volumes.len) * sizeof(synthVolume));
        /* Actually increase the buffer length */
        pCtx->volumes.len += 1 + pCtx->volumes.len;
    }

    /* Retrieve the volume to be used */
    *ppVol = &(pCtx->volumes.buf.pVolumes[pCtx->volumes.used]);
    pCtx->volumes.used++;
    rv = SYNTH_OK;
__err:
    return rv;
}

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
synth_err synthVolume_getConst(synthVolume **ppVol, synthCtx *pCtx, char amp) {
    int i;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(ppVol, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Clean the return, so we now if anything was found */
    *ppVol = 0;

    /* Search for the requested volume through the existing ones */
    i = 0;
    while (i < pCtx->volumes.used) {
        if ((pCtx->volumes.buf.pVolumes[i].ini ==
                pCtx->volumes.buf.pVolumes[i].fin) &&
                (pCtx->volumes.buf.pVolumes[i].ini == amp)) {
            /* If a volume matched, simply return it */
            *ppVol = &(pCtx->volumes.buf.pVolumes[i]);
            break;
        }
        i++;
    }

    /* If the volume wasn't found, create a new one */
    if (*ppVol != 0) {
        rv = synthVolume_init(ppVol, pCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Set both values to the same, since this is a constant volume */
        (*ppVol)->ini = amp;
        (*ppVol)->fin = amp;
    }

    rv = SYNTH_OK;
__err:
    return rv;
}

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
        char fin) {
    int i;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(ppVol, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Clean the return, so we now if anything was found */
    *ppVol = 0;

    /* Search for the requested volume through the existing ones */
    i = 0;
    while (i < pCtx->volumes.used) {
        if (pCtx->volumes.buf.pVolumes[i].ini == ini &&
                pCtx->volumes.buf.pVolumes[i].fin == fin) {
            /* If a volume matched, simply return it */
            *ppVol = &(pCtx->volumes.buf.pVolumes[i]);
            break;
        }
        i++;
    }

    /* If the volume wasn't found, create a new one */
    if (*ppVol != 0) {
        rv = synthVolume_init(ppVol, pCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Set both values to the same, since this is a constant volume */
        (*ppVol)->ini = ini;
        (*ppVol)->fin = fin;
    }

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


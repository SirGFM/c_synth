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
#include <c_synth/synth_assert.h>
#include <c_synth/synth_errors.h>

#include <c_synth_internal/synth_volume.h>
#include <c_synth_internal/synth_types.h>

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
 * @param  [out]pVol The index of the volume
 * @param  [ in]pCtx The synthesizer context
 * @param  [ in]amp  The requested amplitude (in the range [0, 128])
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthVolume_getConst(int *pVol, synthCtx *pCtx, int amp) {
    int i;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pVol, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Clamp the note to the valid range */
    if (amp < 0) {
        amp = 0;
    }
    else if (amp > 128) {
        amp = 128;
    }

    /* Increase the amplitude to a 16 bits value */
    amp <<= 8;

    /* Clean the return, so we now if anything was found */
    *pVol = 0;

    /* Search for the requested volume through the existing ones */
    i = 0;
    while (i < pCtx->volumes.used) {
        if ((pCtx->volumes.buf.pVolumes[i].ini ==
                pCtx->volumes.buf.pVolumes[i].fin) &&
                (pCtx->volumes.buf.pVolumes[i].ini == amp)) {
            /* If a volume matched, simply return it */
            *pVol = i;
            break;
        }
        i++;
    }

    /* If the volume wasn't found, create a new one */
    if (*pVol == 0) {
        synthVolume *pVolume;

        rv = synthVolume_init(&pVolume, pCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Set both values to the same, since this is a constant volume */
        pVolume->ini = amp;
        pVolume->fin = amp;

        /* Retrieve the volume's index */
        *pVol = pCtx->volumes.used - 1;
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
 * @param  [out]pVol The index of the volume
 * @param  [ in]pCtx The synthesizer context
 * @param  [ in]ini  The initial amplitude (in the range [0, 255])
 * @param  [ in]fin  The final amplitude (in the range [0, 255])
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_MEM_ERR
 */
synth_err synthVolume_getLinear(int *pVol, synthCtx *pCtx, int ini, int fin) {
    int i;
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pVol, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Clamp the note to the valid range */
    if (ini < 0) {
        ini = 0;
    }
    else if (ini > 128) {
        ini = 128;
    }
    if (fin < 0) {
        fin = 0;
    }
    else if (fin > 128) {
        fin = 128;
    }

    /* Clean the return, so we now if anything was found */
    *pVol = 0;

    /* Increase the amplitude to a 16 bits value */
    ini <<= 8;
    fin <<= 8;

    /* Search for the requested volume through the existing ones */
    i = 0;
    while (i < pCtx->volumes.used) {
        if (pCtx->volumes.buf.pVolumes[i].ini == ini &&
                pCtx->volumes.buf.pVolumes[i].fin == fin) {
            /* If a volume matched, simply return it */
            *pVol = i;
            break;
        }
        i++;
    }

    /* If the volume wasn't found, create a new one */
    if (*pVol == 0) {
        synthVolume *pVolume;

        rv = synthVolume_init(&pVolume, pCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Set both values to the same, since this is a constant volume */
        pVolume->ini = ini;
        pVolume->fin = fin;

        /* Retrieve the volume's index */
        *pVol = pCtx->volumes.used - 1;
    }

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Retrieve the volume at a given percentage of a note
 * 
 * @param  [out]pAmp The note's amplitude
 * @param  [ in]pVol The volume
 * @param  [ in]perc Percentage into the note (in the range [0, 1024))
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthVolume_getAmplitude(int *pAmp, synthVolume *pVol, int perc) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pAmp, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pVol, SYNTH_BAD_PARAM_ERR);

    /* Calculate the current amplitude */
    *pAmp = (((pVol->ini * (1024 - perc)) + (pVol->fin * perc)) >> 10) & 0xffff;
    /* If the previous didn't work out (because of integer division), use the
     * following */
    /* *pAmp = (pVol->ini + (pVol->fin - pVol->ini) * (perc / 1024.0f)) &
     *         0xff; */

    rv = SYNTH_OK;
__err:
    return rv;
}


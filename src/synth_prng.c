/**
 * Pseudo-random number generator. There are also other PRNG-based
 * functionalities, like a gaussian white noise generator.
 * 
 * @file src/include/synth_internal/synth_prng.h
 */
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>

#include <synth_internal/synth_types.h>
#include <synth_internal/synth_prng.h>

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define TAU 2.0 * 3.1415926535897

/**
 * Advance the internal context to the next pseudo-random number
 * 
 * @param  [ in]pCtx The context
 */
static void synthPRNG_iterate(synthPRNGCtx *pCtx) {
    pCtx->seed = pCtx->a * pCtx->seed + pCtx->c;
}

/**
 * (Re)Initialize the Pseudo-random number generator with the desired seed
 * 
 * @param  [ in]pCtx The context
 * @param  [out]seed The desired seed
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthPRNG_init(synthPRNGCtx *pCtx, unsigned int seed) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Set the parameters */
    pCtx->a = 0x0019660d;
    pCtx->c = 0x3c6ef35f;
    pCtx->seed = seed;

    /* Set the noise type to Box-Muller */
    pCtx->type = NW_BOXMULLER;

    /* Advance the internal state because... why not? */
    synthPRNG_iterate(pCtx);

    pCtx->isInit = 1;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Retrieve a (pseudo) random value in the range [0, 0xFFFFFFFF]
 * 
 * @param  [out]pVal The generated value
 * @param  [ in]pCtx The context
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthPRNG_getUint(unsigned int *pVal, synthPRNGCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pVal, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* If it isn't initialized, initialize with the time (a bad seed, but
     * still...) */
    if (!pCtx->isInit) {
        rv = synthPRNG_init(pCtx, (unsigned int)time(0));
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    }

    /* Retrieve the random number */
    *pVal = pCtx->seed;

    /* Update the internal state */
    synthPRNG_iterate(pCtx);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Retrieve a (pseudo) random value in the range [0.0, 1.0]
 * 
 * @param  [out]pVal The generated value
 * @param  [ in]pCtx The context
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthPRNG_getDouble(double *pVal, synthPRNGCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pVal, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* If it isn't initialized, initialize with the time (a bad seed, but
     * still...) */
    if (!pCtx->isInit) {
        rv = synthPRNG_init(pCtx, (unsigned int)time(0));
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    }

    /* Retrieve the random number */
    *pVal = (double)pCtx->seed / (double)(0xffffffff);

    /* Update the internal state */
    synthPRNG_iterate(pCtx);

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Generate points for a gaussian white noise
 * 
 * @param  [out]pVal The generated noise, in range [-1.0, 1.0]
 * @param  [ in]pCtx The contx
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthPRNG_getGaussianNoise(double *pVal, synthPRNGCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pVal, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    if (pCtx->type == NW_BOXMULLER) {
        struct stBoxMullerParams  *pParams;

        pParams = &(pCtx->noiseParams.boxMuller);

        if (pParams->didGenerate) {
            *pVal = pParams->z1;
            pParams->didGenerate = 0;
        }
        else {
            double u1, u2;

            do {
                synthPRNG_getDouble(&u1, pCtx);
                synthPRNG_getDouble(&u2, pCtx);
            } while (u1 <= DBL_MIN);

            pParams->z0 = sqrt(-2.0 * log(u1)) * cos(TAU * u2);
            pParams->z1 = sqrt(-2.0 * log(u1)) * sin(TAU * u2);

            *pVal = pParams->z0;
            pParams->didGenerate = 1;
        }

        /* Convert it to the desired range */
        *pVal = (*pVal) / 6.7;
    }
    else {
        SYNTH_ASSERT_ERR(0, SYNTH_FUNCTION_NOT_IMPLEMENTED);
    }

    rv = SYNTH_OK;
__err:
    return rv;
}


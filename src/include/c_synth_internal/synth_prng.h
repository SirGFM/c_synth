/**
 * Pseudo-random number generator. There are also other PRNG-based
 * functionalities, like a gaussian white noise generator.
 * 
 * @file src/include/synth_internal/synth_prng.h
 */
#ifndef __SYNTH_PRNG_H__
#define __SYNTH_PRNG_H__

#include <synth/synth_errors.h>
#include <synth_internal/synth_types.h>

/**
 * (Re)Initialize the Pseudo-random number generator with the desired seed
 * 
 * @param  [ in]pCtx The context
 * @param  [ in]seed The desired seed
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthPRNG_init(synthPRNGCtx *pCtx, unsigned int seed);

/**
 * Retrieve a (pseudo) random value in the range [0, 0xFFFFFFFF]
 * 
 * @param  [out]pVal The generated value
 * @param  [ in]pCtx The context
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthPRNG_getUint(unsigned int *pVal, synthPRNGCtx *pCtx);

/**
 * Retrieve a (pseudo) random value in the range [0.0, 1.0]
 * 
 * @param  [out]pVal The generated value
 * @param  [ in]pCtx The context
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthPRNG_getDouble(double *pVal, synthPRNGCtx *pCtx);

/**
 * Generate points for a gaussian white noise
 * 
 * @param  [out]pVal The generated noise, in range [-1.0, 1.0]
 * @param  [ in]pCtx The contx
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthPRNG_getGaussianNoise(double *pVal, synthPRNGCtx *pCtx);

#endif /* __SYNTH_PRNG_H__ */


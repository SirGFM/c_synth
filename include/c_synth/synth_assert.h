/**
 * @file include/synth/synth_assert.h
 * 
 * Macros for asserting stuff, without killing the process.
 * TODO LOG whenever an assert fails!
 */
#ifndef __SYNTH_ASSERT_H_
#define __SYNTH_ASSERT_H_

/**
 * Assert that the expression is true and jump to a __err label on failure.
 * 
 * You must set that label, and the error handling code, whenever you use this!
 */
#define SYNTH_ASSERT(stmt) \
  do { \
    if (!(stmt)) { \
      goto __err; \
    } \
  } while(0)

/**
 * Assert that the expression is true and jump to a __err label on failure.
 * Also, 'rv' is set to the desired error code 'err'.
 * 
 * You must set that label -  and the error handling code - and define an 'err'
 *variable whenever you use this!
 */
#define SYNTH_ASSERT_ERR(stmt, err) \
  do { \
    if (!(stmt)) { \
      rv = err; \
      goto __err; \
    } \
  } while(0)

#endif


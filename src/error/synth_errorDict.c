/**
 * @project   c_synth
 * @license   zlib license
 * @file      src/synth_errorDict.c
 *
 * @summary   Error code dictionary
 *
 * Implements 'synth_getErrorString'.
 */
#include <c_synth/synth_error.h>

static const char* _synth_errorDict[SYNTH_MAX_ERR] = {
      [SYNTH_OK] = "No error detected"
    , [SYNTH_FUNCTION_NOT_IMPLEMENTED] = "Function not implemented"
    , [SYNTH_PARSER_ERROR] = "Parsing error"
    , [SYNTH_DUPLICATED_STRING] = "Trying to add duplicated string"
    , [SYNTH_VALUE_RANGE] = "Value outside the allowed range"
};

/**
 * Returns a human readable string for a given error.
 *
 * @param  [ in]rv The error code
 * @return         Error description
 */
const char* synth_getErrorString(synth_err rv) {
    if (rv >= SYNTH_MAX_ERR) {
        return "Unknown error";
    }
    return _synth_errorDict[rv];
}


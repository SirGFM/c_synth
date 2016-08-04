
#include <c_synth_internal/synth_types.h>

/**
 * Initialize a non-constant instrument
 *
 * @param  [out]pInstrument The instrument
 */
inline void synth_setDefaultInstrument(synth_instrument *pInstrument) {
    pInstrument->envelop.start = 7;
    pInstrument->envelop.end = 12;
    pInstrument->wave = WF_50_PW;
    pInstrument->pan = 3;
    pInstrument->attack = 1;
    pInstrument->keyoff = 5;
    pInstrument->release = 6;
}


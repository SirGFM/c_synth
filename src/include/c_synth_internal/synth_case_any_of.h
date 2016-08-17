/**
 * @project   c_synth
 * @license   zlib license
 * @file      src/include/c_synth_internal/synth_case_any_of.h
 *
 * @summary   Include a macro for simplify writing switch statements.
 *
 * @macro     CASE_ANY_OF Creates a 'case' statement for each argument.
 */
#ifndef __SYNTH_CASE_ANY_OF_H__
#define __SYNTH_CASE_ANY_OF_H__

/* The PP_NARG macro returns the number of arguments that have been
 * passed to it.
 * Source:
 *   https://groups.google.com/forum/#!topic/comp.std.c/d-6Mj5Lko_s
 */
#define PP_NARG(...) \
    PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
    PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
        _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
        _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
        _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
        _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
        _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
        _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
        _61,_62,_63,N,...) N
#define PP_RSEQ_N() \
    63,62,61,60,                   \
    59,58,57,56,55,54,53,52,51,50, \
    49,48,47,46,45,44,43,42,41,40, \
    39,38,37,36,35,34,33,32,31,30, \
    29,28,27,26,25,24,23,22,21,20, \
    19,18,17,16,15,14,13,12,11,10, \
    9,8,7,6,5,4,3,2,1,0


/**
 * Expansion for variadic list of constants.
 *
 * The number "X" in CASE_X acts as the level of recursion into the
 * macro CASE_ANY_OF_N, which is decrease by 1 after each "call".
 * Essentially, this iterates through all arguments.
 */
#define CASE_63(_x, ...) case _x: CASE_62(__VA_ARGS__)
#define CASE_62(_x, ...) case _x: CASE_61(__VA_ARGS__)
#define CASE_61(_x, ...) case _x: CASE_60(__VA_ARGS__)
#define CASE_60(_x, ...) case _x: CASE_59(__VA_ARGS__)
#define CASE_59(_x, ...) case _x: CASE_58(__VA_ARGS__)
#define CASE_58(_x, ...) case _x: CASE_57(__VA_ARGS__)
#define CASE_57(_x, ...) case _x: CASE_56(__VA_ARGS__)
#define CASE_56(_x, ...) case _x: CASE_55(__VA_ARGS__)
#define CASE_55(_x, ...) case _x: CASE_54(__VA_ARGS__)
#define CASE_54(_x, ...) case _x: CASE_53(__VA_ARGS__)
#define CASE_53(_x, ...) case _x: CASE_52(__VA_ARGS__)
#define CASE_52(_x, ...) case _x: CASE_51(__VA_ARGS__)
#define CASE_51(_x, ...) case _x: CASE_50(__VA_ARGS__)
#define CASE_50(_x, ...) case _x: CASE_49(__VA_ARGS__)
#define CASE_49(_x, ...) case _x: CASE_48(__VA_ARGS__)
#define CASE_48(_x, ...) case _x: CASE_47(__VA_ARGS__)
#define CASE_47(_x, ...) case _x: CASE_46(__VA_ARGS__)
#define CASE_46(_x, ...) case _x: CASE_45(__VA_ARGS__)
#define CASE_45(_x, ...) case _x: CASE_44(__VA_ARGS__)
#define CASE_44(_x, ...) case _x: CASE_43(__VA_ARGS__)
#define CASE_43(_x, ...) case _x: CASE_42(__VA_ARGS__)
#define CASE_42(_x, ...) case _x: CASE_41(__VA_ARGS__)
#define CASE_41(_x, ...) case _x: CASE_40(__VA_ARGS__)
#define CASE_40(_x, ...) case _x: CASE_39(__VA_ARGS__)
#define CASE_39(_x, ...) case _x: CASE_38(__VA_ARGS__)
#define CASE_38(_x, ...) case _x: CASE_37(__VA_ARGS__)
#define CASE_37(_x, ...) case _x: CASE_36(__VA_ARGS__)
#define CASE_36(_x, ...) case _x: CASE_35(__VA_ARGS__)
#define CASE_35(_x, ...) case _x: CASE_34(__VA_ARGS__)
#define CASE_34(_x, ...) case _x: CASE_33(__VA_ARGS__)
#define CASE_33(_x, ...) case _x: CASE_32(__VA_ARGS__)
#define CASE_32(_x, ...) case _x: CASE_31(__VA_ARGS__)
#define CASE_31(_x, ...) case _x: CASE_30(__VA_ARGS__)
#define CASE_30(_x, ...) case _x: CASE_29(__VA_ARGS__)
#define CASE_29(_x, ...) case _x: CASE_28(__VA_ARGS__)
#define CASE_28(_x, ...) case _x: CASE_27(__VA_ARGS__)
#define CASE_27(_x, ...) case _x: CASE_26(__VA_ARGS__)
#define CASE_26(_x, ...) case _x: CASE_25(__VA_ARGS__)
#define CASE_25(_x, ...) case _x: CASE_24(__VA_ARGS__)
#define CASE_24(_x, ...) case _x: CASE_23(__VA_ARGS__)
#define CASE_23(_x, ...) case _x: CASE_22(__VA_ARGS__)
#define CASE_22(_x, ...) case _x: CASE_21(__VA_ARGS__)
#define CASE_21(_x, ...) case _x: CASE_20(__VA_ARGS__)
#define CASE_20(_x, ...) case _x: CASE_19(__VA_ARGS__)
#define CASE_19(_x, ...) case _x: CASE_18(__VA_ARGS__)
#define CASE_18(_x, ...) case _x: CASE_17(__VA_ARGS__)
#define CASE_17(_x, ...) case _x: CASE_16(__VA_ARGS__)
#define CASE_16(_x, ...) case _x: CASE_15(__VA_ARGS__)
#define CASE_15(_x, ...) case _x: CASE_14(__VA_ARGS__)
#define CASE_14(_x, ...) case _x: CASE_13(__VA_ARGS__)
#define CASE_13(_x, ...) case _x: CASE_12(__VA_ARGS__)
#define CASE_12(_x, ...) case _x: CASE_11(__VA_ARGS__)
#define CASE_11(_x, ...) case _x: CASE_10(__VA_ARGS__)
#define CASE_10(_x, ...) case _x: CASE_9(__VA_ARGS__)
#define CASE_9( _x, ...) case _x: CASE_8(__VA_ARGS__)
#define CASE_8( _x, ...) case _x: CASE_7(__VA_ARGS__)
#define CASE_7( _x, ...) case _x: CASE_6(__VA_ARGS__)
#define CASE_6( _x, ...) case _x: CASE_5(__VA_ARGS__)
#define CASE_5( _x, ...) case _x: CASE_4(__VA_ARGS__)
#define CASE_4( _x, ...) case _x: CASE_3(__VA_ARGS__)
#define CASE_3( _x, ...) case _x: CASE_2(__VA_ARGS__)
#define CASE_2( _x, ...) case _x: CASE_1(__VA_ARGS__)
#define CASE_1( _x     ) case _x:

/**
 * Setup as many 'case' statements as necessary.
 *
 * @param  [ in]_n  Number of arguments
 * @param  [ in]... Comma separated list of  constants that should
 *                  trigger the case
 */
#define CASE_ANY_OF_N(_n, ...) CASE_ ## _n(__VA_ARGS__)

/**
 * Helper macro to 'CASE_ANY'.
 *
 * Expands the number of arguments and create one 'case' statement for
 * each constant on the variadic list of arguments.
 *
 * @param  [ in]_nx Must be PP_NARG(__VA_ARGS__). It gets expanded
 *                  before "calling" 'CASE_ANY_OF_N'
 * @param  [ in]... Comma separated list of  constants that should
 *                  trigger the case
 */
#define CASE_ANY_OF_(_nx,...) \
  CASE_ANY_OF_N(_nx, __VA_ARGS__)

/**
 * Create 'case' statements for a variable number of constants
 *
 * NOTE: In order to 'PP_NARG' to work, this macro requires an
 * indirection through another macro (so 'PP_NARG' gets expanded).
 *
 * @param  [ in]... Comma separated list of  constants that should
 *                  trigger the case
 */
#define CASE_ANY_OF(...) \
  CASE_ANY_OF_(PP_NARG(__VA_ARGS__), __VA_ARGS__)


#endif /* __SYNTH_CASE_ANY_OF_H__ */


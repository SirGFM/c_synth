/**
 * @file src/synth_lexer.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>

#include <synth_internal/synth_lexer.h>
#include <synth_internal/synth_types.h>

#include <stdio.h>
#include <string.h>

static char *__synthLexer_tokenString[TK_MAX + 1] = {
    "mml",
    "set bpm",
    "set duration",
    "set octave",
    "set relative octave",
    "set loop point",
    "set end of track",
    "set volume",
    "set keyoff",
    "set pan",
    "set loop start",
    "set loop end",
    "set wave",
    "increase volume",
    "decrease volume",
    "note",
    "duration",
    "number",
    "comma",
    "done",
    "unknown token"
};

/**
 * Initialize the lexer, reading tokens from a file
 * 
 * If the lexer has already been initialized, it will be reset and
 * re-initialized with this new source
 * 
 * @param  [ in]pCtx      The lexer context, to be initialized
 * @param  [ in]pFilename The file
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR, SYNTH_OPEN_FILE_ERR
 */
synth_err synthLexer_initFromFile(synthLexCtx *pCtx, char *pFilename) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pFilename, SYNTH_BAD_PARAM_ERR);

    /* Clean the lexer */
    rv = synthLexer_clear(pCtx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    /* Store its source (i.e., a file) */
    pCtx->source.file = fopen(pFilename, "rt");
    SYNTH_ASSERT_ERR(pCtx->source.file, SYNTH_OPEN_FILE_ERR);

    pCtx->isFile = SYNTH_TRUE;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Initialize the lexer, reading tokens from a string
 * 
 * If the lexer has already been initialized, it will be reset and
 * re-initialized with this new source
 * 
 * @param  [ in]pCtx    The lexer context, to be initialized
 * @param  [ in]pString The string
 * @param  [ in]len     The string's length
 * @return              SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_initFromString(synthLexCtx *pCtx, char *pString, int len) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pString, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(len > 0, SYNTH_BAD_PARAM_ERR);
    /* Check that the string is NULL-terminated */
    SYNTH_ASSERT_ERR(pString[len] == '\0', SYNTH_BAD_PARAM_ERR);

    /* Clean the lexer */
    rv = synthLexer_clear(pCtx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, rv);
    /* Store its source (i.e., a string) */
    pCtx->source.str.pStr = pString;
    pCtx->source.str.len = len;
    pCtx->source.str.pos = 0;

    pCtx->isFile = SYNTH_FALSE;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Clear a lexer and all of its resources
 * 
 * This functions only needs really to be called when using a file as input
 * source, since, otherwise, everything is kept in RAM;
 * 
 * @param  [ in]pCtx      The lexer context, to be initialized
 * @return                SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_clear(synthLexCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Close the file, if it's open */
    if (pCtx->isFile == SYNTH_TRUE && pCtx->source.file) {
        fclose(pCtx->source.file);
    }
    /* If it's a string, it'll be cleaned on the memset */

    /* Clear everything (except for everything related to an error ) */
    pCtx->isFile = 0;
    pCtx->ivalue = 0;
    pCtx->lastToken = 0;
    memset(&(pCtx->source), 0x0, sizeof(synthSource));

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Get the current line number
 * 
 * @param [out]pVal The current line
 * @param [ in]pCtx The contex
 * return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_getCurrentLine(int *pVal, synthLexCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pVal, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    *pVal = pCtx->line;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Get the current position inside the line
 * 
 * @param [out]pVal The the current position
 * @param [ in]pCtx The contex
 * return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_getCurrentLinePosition(int *pVal, synthLexCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pVal, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    *pVal = pCtx->linePos;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Get the last chracter read (that probably triggered an error)
 * 
 * @param [out]pVal The last character
 * @param [ in]pCtx The contex
 * return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_getLastCharacter(char *pVal, synthLexCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pVal, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    *pVal = pCtx->lastChar;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Get the token read on the previous getToken call
 * 
 * @param  [out]pToken The last read token
 * @param  [ in]pCtx   The context
 * @return             SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_lookupToken(synth_token *pToken, synthLexCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pToken, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    *pToken = pCtx->lastToken;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Get the last read integer value
 * 
 * @param  [out]pVal The read integer
 * @param  [ in]pCtx The context
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_getValuei(int *pVal, synthLexCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pVal, SYNTH_BAD_PARAM_ERR);
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    *pVal = pCtx->ivalue;

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Returns a printable string for a given token
 * 
 * That that even if the token is invalid, the function returns OK; In that
 * case, though, the returned string says "unknown token"
 * 
 * @param  [out]ppStr The null-terminated, static string
 * @param  [ in]token The token
 * @return           SYNTH_OK, SYNTH_BAD_PARAM_ERR
 */
synth_err synthLexer_printToken(char **ppStr, synth_token token) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(ppStr, SYNTH_BAD_PARAM_ERR);

    /* Check if the token is valid and return the proper string */
    if (token >= 0 && token < TK_MAX) {
        *ppStr = __synthLexer_tokenString[token];
    }
    else {
        *ppStr = __synthLexer_tokenString[TK_MAX];
    }

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Read the character on the current position.
 * 
 * Since this function is static, and it's has already been sanitized on the
 * previous 'public' call, there's no need to do it again here
 * 
 * @param  [out]pChar The character that was read
 * @param  [ in]pCtx  The context
 * @return            SYNTH_OK, SYNTH_EOF, SYNTH_EOS
 */
static synth_err synthLexer_getRawChar(char *pChar, synthLexCtx *pCtx) {
    synth_err rv;
    int tmp;

    if (pCtx->isFile == SYNTH_TRUE) {
        /* Read a character from the file */
        tmp = fgetc(pCtx->source.file);
        SYNTH_ASSERT_ERR(tmp != EOF, SYNTH_EOF);
    }
    else {
        /* Get the current character and increase the position */
        SYNTH_ASSERT_ERR(pCtx->source.str.pos < pCtx->source.str.len,
                SYNTH_EOS);
        tmp = pCtx->source.str.pStr[pCtx->source.str.pos];
        pCtx->source.str.pos++;
    }

    /* Return the read character */
    *pChar = (char)tmp;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Read the character on the current position. Commentaries, whitespace and
 * similars are already ignored
 * 
 * Since this function is static, and it's has already been sanitized on the
 * previous 'public' call, there's no need to do it again here
 * 
 * @param  [out]pChar The character that was read
 * @param  [ in]pCtx  The context
 * @return            SYNTH_OK, SYNTH_EOF, SYNTH_EOS
 */
static synth_err synthLexer_getChar(char *pChar, synthLexCtx *pCtx) {
    synth_err rv;
    char tmp;
    int isComment;

    isComment = 0;
    /* Read as many characters until the next valid one is found */
    while (1) {
        rv = synthLexer_getRawChar(&tmp, pCtx);
        SYNTH_ASSERT(rv == SYNTH_OK);

        /* Store the character that has been just read */
        pCtx->lastChar = tmp;
        /* Update the current position on the source */
        if (tmp != '\n' && tmp != '\r') {
            pCtx->linePos++;
        }
        else if (tmp == '\n') {
            pCtx->linePos = 0;
            pCtx->line++;
        }

        if (isComment <= 1) {
            /* Add 1 to each consecutive '/', so it will be 2 on when a comment
             * starts */
            if (tmp == '/') {
                isComment++;
            }
            else {
                isComment = 0;

                /* If it's a valid printable non-whitespace character, stop */
                if (tmp >= '!' && tmp <= '}') {
                    break;
                }
            }
        }
        else if (isComment == 2 && tmp == '\n') {
            /* If it's a comment, stop as soon as a new-line is found */
            isComment = 0;
        }
    }

    /* Return the read character */
    *pChar = (char)tmp;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * 'Return' the last character to the 'stream'
 * 
 * Note that the character isn't actually returned, the position is simply
 * rewinded to the previous one
 * 
 * Since this function is static, and it's has already been sanitized on the
 * previous 'public' call, there's no need to do it again here
 * 
 * @param [ in]pCtx The context
 * @param [ in]c    The character that was previously read
 * @return          SYNTH_OK, SYNTH_INTERNAL_ERR
 */
static synth_err synthLexer_ungetChar(synthLexCtx *pCtx, char c) {
    synth_err rv;

    if (pCtx->isFile == SYNTH_TRUE) {
        int tmp;

        /* Return the character to the file */
        tmp = ungetc((int)c, pCtx->source.file);
        SYNTH_ASSERT_ERR(tmp != EOF, SYNTH_INTERNAL_ERR);
    }
    else {
        /* Or simply decrement the position */
        SYNTH_ASSERT_ERR(pCtx->source.str.pos > 0, SYNTH_INTERNAL_ERR);
        pCtx->source.str.pos--;
    }
    /* Update the current position in the file */
    pCtx->linePos--;
    /* TODO Check if the new position also returned to the previous line */

    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Check if the current character is a single char token and set its value
 * 
 * Instead of returning one of the error codes, this function simply return
 * 'true' or 'false'
 * 
 * @param  [ in]func     The name of the function
 * @param  [ in]expected The expected character
 * @param  [ in]token    The gotten token, on success
 * @return               SYNTH_TRUE, SYNTH_FALSE
 */
#define SYNTHLEXER_ISTOKEN(func, expected, token) \
  static synth_bool func(synthLexCtx *pCtx) { \
    synth_bool rv; \
    synth_err srv; \
    char c; \
\
    /* Read the current character */ \
    srv = synthLexer_getChar(&c, pCtx); \
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE); \
\
    /* Check if it's what was expected */ \
    if (c != expected) { \
        synthLexer_ungetChar(pCtx, c); \
        rv = SYNTH_FALSE; \
    } \
    else { \
        rv = SYNTH_TRUE; \
        pCtx->lastToken = token; \
    } \
\
__err: \
    return rv; \
  }

SYNTHLEXER_ISTOKEN(synthLexer_isSetBPM,       't', T_SET_BPM)
SYNTHLEXER_ISTOKEN(synthLexer_isSetDuration,  'l', T_SET_DURATION)
SYNTHLEXER_ISTOKEN(synthLexer_isSetOctave,    'o', T_SET_OCTAVE)
SYNTHLEXER_ISTOKEN(synthLexer_isSetLoopPoint, '$', T_SET_LOOPPOINT)
SYNTHLEXER_ISTOKEN(synthLexer_isEndOfTrack,   ';', T_END_OF_TRACK)
SYNTHLEXER_ISTOKEN(synthLexer_isSetVolume,    'v', T_SET_VOLUME)
SYNTHLEXER_ISTOKEN(synthLexer_isOpenBracket,  '(', T_OPEN_BRACKET)
SYNTHLEXER_ISTOKEN(synthLexer_isCloseBracket, ')', T_CLOSE_BRACKET)
SYNTHLEXER_ISTOKEN(synthLexer_isSetKeyoff,    'q', T_SET_KEYOFF)
SYNTHLEXER_ISTOKEN(synthLexer_isSetPan,       'p', T_SET_PAN)
SYNTHLEXER_ISTOKEN(synthLexer_isSetLoopStart, '[', T_SET_LOOP_START)
SYNTHLEXER_ISTOKEN(synthLexer_isSetLoopEnd,   ']', T_SET_LOOP_END)
SYNTHLEXER_ISTOKEN(synthLexer_isSetWave,      'w', T_SET_WAVE)
SYNTHLEXER_ISTOKEN(synthLexer_isSetComma,     ',', T_COMMA)

/**
 * Check if the current stream is a valid MML (i.e., if it starts with "MML")
 * 
 * The stream isn't checked to be at its start
 * 
 * @param  [ in]pCtx The contex
 * @return           SYNTH_TRUE, SYNTH_FALSE
 */
static synth_bool synthLexer_isMML(synthLexCtx *pCtx) {
    synth_bool rv;
    synth_err srv;
    char c;
    char target[] = "MML";
    int i;

    i = 0;
    while (i < sizeof(target) - 1) {
        /* Get the current character, without ignoring blank and whatnot */
        srv = synthLexer_getChar(&c, pCtx);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);

        /* Check that it's of the expected pattern */
        SYNTH_ASSERT_ERR(c == target[i], SYNTH_FALSE);

        /* Go to the next char */
        i++;
    }

    pCtx->lastToken = T_MML;
    rv = SYNTH_TRUE;
__err:
    if (rv != SYNTH_TRUE) {
        /* On error, return the last char (that didn't belong to the pattern */
        synthLexer_ungetChar(pCtx, c);

        /* Then, return everything that belonged partially to the pattern */
        while (i > 0) {
            i--;
            synthLexer_ungetChar(pCtx, target[i]);
        }
    }

    return rv;
}

/**
 * Check if the current octave should be increased or decreased, through the
 * character '<' and '>', repectively
 * 
 * @param  [ in]pCtx The contex
 * @return           SYNTH_TRUE, SYNTH_FALSE
 */
static synth_bool synthLexer_isSetRelOctave(synthLexCtx *pCtx) {
    synth_bool rv;
    synth_err srv;
    char c;

    /* Get the current character */
    srv = synthLexer_getChar(&c, pCtx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);

    if (c == '<') {
        /* Check if the octave should be increased */
        pCtx->ivalue = 1;
        rv = SYNTH_TRUE;
        pCtx->lastToken = T_SET_REL_OCTAVE;
    }
    else if (c == '>') {
        /* Check if the octave should be decreased */
        pCtx->ivalue = -1;
        rv = SYNTH_TRUE;
        pCtx->lastToken = T_SET_REL_OCTAVE;
    }
    else {
        /* Return the character to the source */
        synthLexer_ungetChar(pCtx, c);
        rv = SYNTH_FALSE;
    }

__err:
    return rv;
}

/**
 * Check if there's a note on the current position on the source and read it
 * 
 * @param  [ in]pCtx The contex
 * @return           SYNTH_TRUE, SYNTH_FALSE
 */
static synth_bool synthLexer_isNote(synthLexCtx *pCtx) {
    synth_bool rv;
    synth_err srv;
    char note;
    char mod;

    /* Set these to 0 so we know which has to be returned to the stream, on
     * error */
    note = 0;
    mod = 0;

    /* Get the current character */
    srv = synthLexer_getChar(&note, pCtx);
    /* It's OK to reach the end of the stream, if the note is the last char */
    SYNTH_ASSERT_ERR(srv == SYNTH_OK || srv == SYNTH_EOF || srv == SYNTH_EOS
        , SYNTH_FALSE);

    /* Check if it's a valid note (and which) */
    switch (note) {
        case 'c': pCtx->ivalue = N_C; break;
        case 'd': pCtx->ivalue = N_D; break;
        case 'e': pCtx->ivalue = N_E; break;
        case 'f': pCtx->ivalue = N_F; break;
        case 'g': pCtx->ivalue = N_G; break;
        case 'a': pCtx->ivalue = N_A; break;
        case 'b': pCtx->ivalue = N_B; break;
        case 'r': pCtx->ivalue = N_REST; break;
        default:
            /* Something that isn't a note was read */
            SYNTH_ASSERT_ERR(0, SYNTH_FALSE);
    }
    
    /* Try to read a modifier ('+' or '-') */
    srv = synthLexer_getChar(&mod, pCtx);
    /* Again, it's OK to reach the end of the stream, if the note is the last
     * char, no matter wheter there's a modifier or not */
    SYNTH_ASSERT_ERR(srv == SYNTH_OK || srv == SYNTH_EOF || srv == SYNTH_EOS
        , SYNTH_FALSE);

    /* Check if the read character is a modifier, and that note isn't a rest */
    if (mod == '+') {
        SYNTH_ASSERT_ERR(pCtx->ivalue != N_REST, SYNTH_FALSE);
        pCtx->ivalue++;
    }
    else if (mod == '-') {
        SYNTH_ASSERT_ERR(pCtx->ivalue != N_REST, SYNTH_FALSE);
        pCtx->ivalue--;
    }
    else if (mod != 0) {
        /* A note is valid even without the modifier, so simply return the char,
         * but if it's 0, the the end of the stream was reached, and there's
         * nothing to 'unread' */
        synthLexer_ungetChar(pCtx, mod);
    }
    
    pCtx->lastToken = T_NOTE;
    rv = SYNTH_TRUE;
__err:
    /* Make sure to return any read character to the stream, if it wasn't a
     * note */
    if (rv == SYNTH_FALSE) {
        if (mod != 0) {
            synthLexer_ungetChar(pCtx, mod);
        }
        if (note != 0) {
            synthLexer_ungetChar(pCtx, note);
        }
    }

    return rv;
}

/**
 * Check if the current token is a "dot duration", i.e., a fraction of the
 * current note's duration
 * 
 * @param  [ in]pCtx The contex
 * @return           SYNTH_TRUE, SYNTH_FALSE
 */
static synth_bool synthLexer_isDotDuration(synthLexCtx *pCtx) {
    synth_bool rv;
    synth_err srv;
    char c;

    /* Clean the current value so we can return as many characters as were
     * read, on error */
    pCtx->ivalue = 0;

    /* Get the current character */
    srv = synthLexer_getChar(&c, pCtx);
    /* It's OK to reach the end of the stream, if the '.' is the last char */
    SYNTH_ASSERT_ERR(srv == SYNTH_OK || srv == SYNTH_EOF || srv == SYNTH_EOS
        , SYNTH_FALSE);
    SYNTH_ASSERT_ERR(c == '.', SYNTH_FALSE);

    /* Read as many '.' as there are on the stream, adding half the duration
     * each time */
    while (1) {
        /* Stop at the first non '.' */
        if (c != '.') {
            break;
        }

        /* Add a new byte to the value, to represent another "half time" */
        pCtx->ivalue = (pCtx->ivalue << 1) | 1;

        /* Read the next character before looping */
        srv = synthLexer_getChar(&c, pCtx);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK || srv == SYNTH_EOF || srv == SYNTH_EOS
            , SYNTH_FALSE);

        /* Also stop if the source finished */
        if (srv == SYNTH_EOF || srv == SYNTH_EOS) {
            break;
        }
    }
    
    pCtx->lastToken = T_DURATION;
    rv = SYNTH_TRUE;
    /* Return that last non-dot char */
    synthLexer_ungetChar(pCtx, c);
__err:
    /* If the funtion failed, return every invalid char */
    if (rv != SYNTH_TRUE) {
        /* Return the last invalid char read */
        synthLexer_ungetChar(pCtx, c);
        /* Now, return any '.' that was read */
        while (pCtx->ivalue > 0) {
            c = '.';
            pCtx->ivalue >>= 1;

            synthLexer_ungetChar(pCtx, c);
        }
    }

    return rv;
}

/**
 * Check if there's a number on the stream and read it
 * 
 * @param  [ in]pCtx The contex
 * @return           SYNTH_TRUE, SYNTH_FALSE
 */
static synth_bool synthLexer_isNumber(synthLexCtx *pCtx) {
    synth_bool rv;
    synth_err srv;
    char c;

    /* Clean the current value so we can return as many characters as were
     * read, on error */
    pCtx->ivalue = 0;
    
    /* Get the current character */
    srv = synthLexer_getChar(&c, pCtx);
    /* It's OK to reach the end of the stream, if a digit is the last char */
    SYNTH_ASSERT_ERR(srv == SYNTH_OK || srv == SYNTH_EOF || srv == SYNTH_EOS
        , SYNTH_FALSE);
    SYNTH_ASSERT_ERR(c >= '0' && c <= '9', SYNTH_FALSE);
    
    /* Get the integer part */
    while (1) {
        /* Stop on the first non-digit found */
        if (c < '0' || c > '9') {
            break;
        }

        /* Accumulate the last gotten char */
        pCtx->ivalue = pCtx->ivalue * 10 + (int)(c - '0');

        /* Read the next one */
        srv = synthLexer_getChar(&c, pCtx);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK || srv == SYNTH_EOF || srv == SYNTH_EOS
            , SYNTH_FALSE);

        /* Also stop if the source finished */
        if (srv == SYNTH_EOF || srv == SYNTH_EOS) {
            break;
        }
    }

    pCtx->lastToken = T_NUMBER;
    rv = SYNTH_TRUE;
    /* Return that last non-digit char */
    synthLexer_ungetChar(pCtx, c);
__err:
    /* If the funtion failed, return every invalid char */
    if (rv != SYNTH_TRUE) {
        /* Return the last invalid char read */
        synthLexer_ungetChar(pCtx, c);
        /* Now, return any digit that was read */
        while (pCtx->ivalue > 0) {
            c = (pCtx->ivalue % 10) + '0';
            pCtx->ivalue /= 10;
            
            synthLexer_ungetChar(pCtx, c);
        }
    }

    return rv;
}

/**
 * Check if there are any more characters in the 'stream' or not
 * 
 * @param  [ in]pCtx The contex
 * @return           SYNTH_TRUE, SYNTH_FALSE
 */
static synth_bool synthLexer_didFinish(synthLexCtx *pCtx) {
    if (pCtx->isFile == SYNTH_TRUE) {
        /* If the stream is a file, first check for the EOF flag */
        if (feof(pCtx->source.file) != 0) {
            pCtx->lastToken = T_DONE;
            return SYNTH_TRUE;
        }
        else {
            char c;

            /* Otherwise, try to read a character to make sure there's nothing
             * else on the stream */
            if (synthLexer_getChar(&c, pCtx) == SYNTH_EOF) {
                pCtx->lastToken = T_DONE;
                return SYNTH_TRUE;
            }
            else {
                /* On error, return the character to the stream */
                synthLexer_ungetChar(pCtx, c);
                return SYNTH_FALSE;
            }
        }
    }
    else {
        /* If the stream is a string, simply check it's length and, then, if its
         * NULL terminator was found */
        if (pCtx->source.str.pos >= pCtx->source.str.len ||
                (pCtx->source.str.pos == pCtx->source.str.len - 1 &&
                pCtx->source.str.pStr[pCtx->source.str.pos] == '\0')) {
            pCtx->lastToken = T_DONE;
            return SYNTH_TRUE;
        }
        else {
            return SYNTH_FALSE;
        }
    }
}

/**
 * Get the next token on the context and its value (if any)
 * 
 * @param  [ in]pCtx The context
 * return            SYNTH_OK, SYNTH_INVALID_TOKEN
 */
synth_err synthLexer_getToken(synthLexCtx *pCtx) {
    synth_err rv;

    /* Sanitize the arguments */
    SYNTH_ASSERT_ERR(pCtx, SYNTH_BAD_PARAM_ERR);

    /* Check if a valid token, and which, was found */
    if (synthLexer_isMML(pCtx) == SYNTH_TRUE ||
            synthLexer_isSetBPM(pCtx) == SYNTH_TRUE ||
            synthLexer_isSetDuration(pCtx) == SYNTH_TRUE ||
            synthLexer_isSetOctave(pCtx) == SYNTH_TRUE ||
            synthLexer_isSetRelOctave(pCtx) == SYNTH_TRUE ||
            synthLexer_isSetLoopPoint(pCtx) == SYNTH_TRUE ||
            synthLexer_isEndOfTrack(pCtx) == SYNTH_TRUE ||
            synthLexer_isSetVolume(pCtx) == SYNTH_TRUE ||
            synthLexer_isOpenBracket(pCtx) == SYNTH_TRUE ||
            synthLexer_isCloseBracket(pCtx) == SYNTH_TRUE ||
            synthLexer_isSetKeyoff(pCtx) == SYNTH_TRUE ||
            synthLexer_isSetPan(pCtx) == SYNTH_TRUE ||
            synthLexer_isSetLoopStart(pCtx) == SYNTH_TRUE ||
            synthLexer_isSetLoopEnd(pCtx) == SYNTH_TRUE ||
            synthLexer_isSetWave(pCtx) == SYNTH_TRUE ||
            synthLexer_isNote(pCtx) == SYNTH_TRUE ||
            synthLexer_isDotDuration(pCtx) == SYNTH_TRUE ||
            synthLexer_isNumber(pCtx) == SYNTH_TRUE ||
            synthLexer_isSetComma(pCtx) == SYNTH_TRUE ||
            synthLexer_didFinish(pCtx) == SYNTH_TRUE) {
        rv = SYNTH_OK;
    }
    else {
        rv = SYNTH_INVALID_TOKEN;
    }

__err:
    return rv;
}


#if 0
#include <stdlib.h>

/**
 * Setup a context for tokenizing a mml file
 * 
 * @param ctx Variable that will hold the allocated context. Must be freed
 * afterward, by calling synth_lex_freeCtx
 * @param mml String with the mml audio (it needn't be null-terminated)
 * @param len String's length
 * @return Error code
 */
synth_err synth_lex_tokenizes(synthLexCtx **ctx, char *mml, int len) {
    synth_err rv;
    synthLexCtx *tmp;
    
    // Alloc the contex
    tmp = (synthLexCtx*)malloc(sizeof(synthLexCtx));
    SYNTH_ASSERT_ERR(tmp, SYNTH_MEM_ERR);
    
    // Assign info about the string
    tmp->isFile = SYNTH_FALSE;
    tmp->str.str = mml;
    tmp->str.len = len;
    tmp->str.pos = 0;
    
    // Set return stuff
    *ctx = tmp;
    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK && tmp)
        free(tmp);
    
    return rv;
}

/**
 * Get the token read on the previous getToken call
 * 
 * @param ctx The context
 * @return The token that was read
 */
synth_token synth_lex_lookupToken(synthLexCtx *ctx) {
    return ctx->lastToken;
}

/**
 * Get the next token on the context and its value (if any)
 * 
 * @param ctx The context
 * @return Error code
 */
synth_err synth_lex_getToken(synthLexCtx *ctx) {
    synth_err rv;
    
    rv = SYNTH_OK;
    
    if (synth_lex_isSetBPM(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_SET_BPM;
    else if (synth_lex_isSetDuration(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_SET_DURATION;
    else if (synth_lex_isSetOctave(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_SET_OCTAVE;
    else if (synth_lex_isSetRelOctave(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_SET_REL_OCTAVE;
    else if (synth_lex_isSetLoopPoint(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_SET_LOOPPOINT;
    else if (synth_lex_isEndOfTrack(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_END_OF_TRACK;
    else if (synth_lex_isSetVolume(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_SET_VOLUME;
    else if (synth_lex_isOpenBracket(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_OPEN_BRACKET;
    else if (synth_lex_isCloseBracket(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_CLOSE_BRACKET;
    else if (synth_lex_isSetKeyoff(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_SET_KEYOFF;
    else if (synth_lex_isSetPan(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_SET_PAN;
    else if (synth_lex_isSetLoopStart(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_SET_LOOP_START;
    else if (synth_lex_isSetLoopEnd(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_SET_LOOP_END;
    else if (synth_lex_isSetWave(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_SET_WAVE;
    else if (synth_lex_isNote(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_NOTE;
    else if (synth_lex_isDotDuration(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_DURATION;
    else if (synth_lex_isNumber(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_NUMBER;
    else if (synth_lex_didFinish(ctx) == SYNTH_TRUE)
        ctx->lastToken = T_DONE;
    else
        rv = SYNTH_INVALID_TOKEN;
    
    return rv;
}

/**
 * Get the last read integer value
 * 
 * @param ctx The context
 * @return The value
 */
int synth_lex_getValuei(synthLexCtx *ctx) {
    return ctx->ivalue;
}

/**
 * Read the character on the current position.
 * 
 * @param c The character that was read
 * @param ctx The context
 * @return Error code
 */
static synth_err synth_lex_getRawChar(char *c, synthLexCtx *ctx) {
    synth_err rv;
    int tmp;
    
    if (ctx->isFile == SYNTH_TRUE) {
        // Read a character from the file
        tmp = fgetc(ctx->file);
        SYNTH_ASSERT_ERR(tmp != EOF, SYNTH_EOF);
    }
    else {
        // Get the current character and increase the position
        SYNTH_ASSERT_ERR(ctx->str.pos < ctx->str.len, SYNTH_EOS);
        tmp = ctx->str.str[ctx->str.pos];
        ctx->str.pos++;
    }
    
    // Return the read character
    *c = (char)tmp;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Read the character on the current position. Commentaries, whitespace and
 * similars are already ignored
 * 
 * @param c The character that was read
 * @param ctx The context
 * @return Error code
 */
synth_err synth_lex_getChar(char *c, synthLexCtx *ctx) {
    synth_err rv;
    char tmp;
    int isCommentary;
    
    isCommentary = 0;
    while (1) {
        // Read a character from the 'stream'
        rv = synth_lex_getRawChar(&tmp, ctx);
        SYNTH_ASSERT(rv == SYNTH_OK);
        
        // Store context info
        ctx->lastChar = tmp;
        if (tmp != '\n' && tmp != '\r')
            ctx->linePos++;
        else if (tmp == '\n') {
            ctx->linePos = 0;
            ctx->line++;
        }
        
        if (isCommentary <= 1) {
            if (tmp == '/')
                isCommentary++;
            else {
                isCommentary = 0;
                // Stop if it's a valid printable non-whitespace character
                if (tmp >= '!' && tmp <= '}')
                    break;
            }
        }
        // If it's a commentary, ignore until a newline
        else if (isCommentary == 2 && tmp == '\n')
            isCommentary = 0;
    }
    
    *c = tmp;
    rv = SYNTH_OK;
__err:
    return rv;
}

/**
 * Return the character to the 'stream'
 * 
 * @param ctx The context
 * @param c The character that was previously read
 */
void synth_lex_ungetChar(synthLexCtx *ctx, char c) {
    if (ctx->isFile == SYNTH_TRUE) {
        // Return the character to the file
        ungetc((int)c, ctx->file);
    }
    else {
        // Or simply decrement the position
        SYNTH_ASSERT(ctx->str.pos > 0);
        ctx->str.pos--;
    }
    ctx->linePos--;
__err:
    return;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isSetBPM(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c != 't') {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
    else
        rv = SYNTH_TRUE;
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isSetDuration(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c != 'l') {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
    else
        rv = SYNTH_TRUE;
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isSetOctave(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c != 'o') {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
    else
        rv = SYNTH_TRUE;
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isSetRelOctave(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c == '<') {
        ctx->ivalue = 1;
        rv = SYNTH_TRUE;
    }
    else if (c == '>') {
        ctx->ivalue = -1;
        rv = SYNTH_TRUE;
    }
    else {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isSetLoopPoint(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c != '$') {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
    else
        rv = SYNTH_TRUE;
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isEndOfTrack(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c != ';') {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
    else
        rv = SYNTH_TRUE;
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isSetVolume(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c != 'v') {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
    else
        rv = SYNTH_TRUE;
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isOpenBracket(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c == '(')
        rv = SYNTH_TRUE;
    else {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isCloseBracket(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c == ')')
        rv = SYNTH_TRUE;
    else {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isSetKeyoff(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c != 'q') {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
    else
        rv = SYNTH_TRUE;
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isSetPan(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c != 'p') {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
    else
        rv = SYNTH_TRUE;
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isSetLoopStart(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c != '[') {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
    else
        rv = SYNTH_TRUE;
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isSetLoopEnd(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK || srv == SYNTH_EOF || srv == SYNTH_EOS
        , SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c != ']') {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
    else
        rv = SYNTH_TRUE;
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isSetWave(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c != '%' && c != '@') {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
    else
        rv = SYNTH_TRUE;
__err:
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isNote(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char note = 0;
    char mod = 0;
    
    // Get the current character
    srv = synth_lex_getChar(&note, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK || srv == SYNTH_EOF || srv == SYNTH_EOS
        , SYNTH_FALSE);
    
    switch (note) {
        case 'c': ctx->ivalue = N_C; break;
        case 'd': ctx->ivalue = N_D; break;
        case 'e': ctx->ivalue = N_E; break;
        case 'f': ctx->ivalue = N_F; break;
        case 'g': ctx->ivalue = N_G; break;
        case 'a': ctx->ivalue = N_A; break;
        case 'b': ctx->ivalue = N_B; break;
        case 'r': ctx->ivalue = N_REST; break;
        default:
            // Something not a note was read
            SYNTH_ASSERT_ERR(0, SYNTH_FALSE);
    }
    
    // Try to read a modifier ('+' or '-')
    srv = synth_lex_getChar(&mod, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK || srv == SYNTH_EOF || srv == SYNTH_EOS
        , SYNTH_FALSE);
    
    if (mod == '+') {
        SYNTH_ASSERT_ERR(ctx->ivalue != N_REST, SYNTH_FALSE);
        ctx->ivalue++;
    }
    else if (mod == '-') {
        SYNTH_ASSERT_ERR(ctx->ivalue != N_REST, SYNTH_FALSE);
        ctx->ivalue--;
    }
    else
        // A note is valid even without the modifier, so simply return the char
        synth_lex_ungetChar(ctx, mod);
    
    rv = SYNTH_TRUE;
__err:
    if (rv == SYNTH_FALSE) {
        if (mod != 0)
            synth_lex_ungetChar(ctx, mod);
        if (note != 0)
            synth_lex_ungetChar(ctx, note);
    }
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isDotDuration(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    ctx->ivalue = 0;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK || srv == SYNTH_EOF || srv == SYNTH_EOS
        , SYNTH_FALSE);
    SYNTH_ASSERT_ERR(c == '.', SYNTH_FALSE);
    
    while (1) {
        if (c != '.')
            break;
        
        ctx->ivalue = (ctx->ivalue << 1) | 1;
        
        srv = synth_lex_getChar(&c, ctx);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK || srv == SYNTH_EOF || srv == SYNTH_EOS
            , SYNTH_FALSE);
    }
    
    rv = SYNTH_TRUE;
    // Return that last non-dot char
    synth_lex_ungetChar(ctx, c);
__err:
    if (rv != SYNTH_TRUE) {
        // Return the last invalid char read
        synth_lex_ungetChar(ctx, c);
        // Now, return any digit that was read
        while (ctx->ivalue > 0) {
            c = '.';
            ctx->ivalue >>= 1;
            
            synth_lex_ungetChar(ctx, c);
        }
    }
    return rv;
}

/**
 * Check if the context is at this token
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_isNumber(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    ctx->ivalue = 0;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK || srv == SYNTH_EOF || srv == SYNTH_EOS
        , SYNTH_FALSE);
    SYNTH_ASSERT_ERR(c >= '0' && c <= '9', SYNTH_FALSE);
    
    // Get the integer part
    while (1) {
        if (c < '0' || c > '9')
            break;
        
        // Accumulate the last gotten char
        ctx->ivalue = ctx->ivalue * 10 + (int)(c - '0');
        
        // Read the next one
        srv = synth_lex_getChar(&c, ctx);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK || srv == SYNTH_EOF || srv == SYNTH_EOS
            , SYNTH_FALSE);
    }
    
    rv = SYNTH_TRUE;
    // Return that last non-digit char
    synth_lex_ungetChar(ctx, c);
__err:
    if (rv != SYNTH_TRUE) {
        // Return the last invalid char read
        synth_lex_ungetChar(ctx, c);
        // Now, return any digit that was read
        while (ctx->ivalue > 0) {
            c = (ctx->ivalue % 10) + '0';
            ctx->ivalue /= 10;
            
            synth_lex_ungetChar(ctx, c);
        }
    }
    return rv;
}

/**
 * Check if there are any more characters in the 'stream' or not
 * 
 * @param ctx The contex
 * @return Either SYNTH_TRUE or SYNTH_FALSE
 */
static synth_bool synth_lex_didFinish(synthLexCtx *ctx) {
    if (ctx->isFile == SYNTH_TRUE) {
        if (feof(ctx->file)  != 0)
            return SYNTH_TRUE;
        else {
            char c;
            
            if (synth_lex_getChar(&c, ctx) == SYNTH_EOF)
                return SYNTH_TRUE;
            else {
                synth_lex_ungetChar(ctx, c);
                return SYNTH_FALSE;
            }
        }
    }
    else {
        if (ctx->str.pos >= ctx->str.len
            || (ctx->str.pos == ctx->str.len - 1
            && ctx->str.str[ctx->str.pos] == '\0'))
            return SYNTH_TRUE;
        else
            return SYNTH_FALSE;
    }
}

/**
 * Returns a printable string for a given token
 * 
 * @param tk The token
 * @return A null-terminated string representing the token
 */
char *synth_lex_printToken(synth_token tk) {
    switch (tk) {
        case T_SET_BPM: return "set bpm"; break;
        case T_SET_DURATION: return "set duration"; break;
        case T_SET_OCTAVE: return "set octave"; break;
        case T_SET_REL_OCTAVE: return "set relative octave"; break;
        case T_SET_LOOPPOINT: return "set loop point"; break;
        case T_END_OF_TRACK: return "set end of track"; break;
        case T_SET_VOLUME: return "set volume"; break;
        case T_SET_KEYOFF: return "set keyoff"; break;
        case T_SET_PAN: return "set pan"; break;
        case T_SET_LOOP_START: return "set loop start"; break;
        case T_SET_LOOP_END: return "set loop end"; break;
        case T_SET_WAVE: return "set wave"; break;
        case T_NOTE: return "note"; break;
        case T_DURATION: return "duration"; break;
        case T_NUMBER: return "number"; break;
        case T_DONE: return "done"; break;
        default: return "unkown token";
    }
}

/**
 * Get the current line number
 * 
 * @param ctx The contex
 */
int synth_lex_getCurrentLine(synthLexCtx *ctx) {
    return ctx->line;
}

/**
 * Get the current position inside the line
 * 
 * @param ctx The contex
 */
int synth_lex_getCurrentLinePosition(synthLexCtx *ctx) {
    return ctx->linePos;
}

/**
 * Get the last chracter read (that probably triggered an error)
 */
char synth_lex_getLastCharacter(synthLexCtx *ctx) {
    return ctx->lastChar;
}

#endif /* 0 */


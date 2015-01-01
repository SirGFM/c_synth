/**
 * @file src/synth_lexer.c
 */

#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth/synth_types.h>
#include <synth_internal/synth_lexer.h>

#include <stdio.h>
#include <stdlib.h>

struct stString {
    /**
     * Total length of the string
     */
    int len;
    /**
     * Current position on the string
     */
    int pos;
    /**
     * The string
     */
    char *str;
};

struct stSynthLexCtx {
    /**
     * Last read character
     */
    char lastChar;
    /**
     * Whether it's currently processing a file or a string
     */
    synth_bool isFile;
    /**
     * Current line on the stream
     */
    int line;
    /**
     * Position inside the current line
     */
    int linePos;
    /**
     * Integer value gotten when reading a token
     */
    int ivalue;
    /**
     * MML's source; either a file descriptor or a string
     */
    union {
        FILE *file;
        struct stString str;
    };
};

/**
 * Setup a context for tokenizing a mml file
 * 
 * @param ctx Variable that will hold the allocated context. Must be freed
 * afterward, by calling synth_lex_freeCtx
 * @param filename MML's filename
 * @return Error code
 */
synth_err synth_lex_tokenizef(synthLexCtx **ctx, char *filename) {
    synth_err rv;
    synthLexCtx *tmp;
    
    // Alloc the contex
    tmp = (synthLexCtx*)malloc(sizeof(synthLexCtx));
    SYNTH_ASSERT_ERR(tmp, SYNTH_MEM_ERR);
    
    // Assign info about the file
    tmp->isFile = SYNTH_TRUE;
    tmp->file = fopen(filename, "rt");
    SYNTH_ASSERT_ERR(tmp->file, SYNTH_OPEN_FILE_ERR);
    
    tmp->lastChar = '\0';
    tmp->line = 0;
    tmp->linePos = 0;
    
    // Set return stuff
    *ctx = tmp;
    rv = SYNTH_OK;
__err:
    if (rv != SYNTH_OK) {
        if (tmp->file)
            fclose(tmp->file);
        if (tmp)
            free(tmp);
    }
    
    return rv;
}

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
 * Clean up memory allocated during setup (on tokenize)
 * 
 * @param ctx The contex to be freed
 */
void synth_lex_freeCtx(synthLexCtx **ctx) {
    // Assert that the context exists
    SYNTH_ASSERT(ctx);
    SYNTH_ASSERT(*ctx);
    
    // Close the file, if it's open
    if ((*ctx)->isFile == SYNTH_TRUE) {
        if ((*ctx)->file != NULL)
            fclose((*ctx)->file);
    }
    
    // Dealloc the context
    free(*ctx);
    *ctx = NULL;
__err:
    return;
}

/**
 * Get the next token on the context and its value (if any)
 * 
 * @param tk The token that was read
 * @param ctx The context
 * @return Error code
 */
synth_err synth_lex_getToken(synth_token *tk, synthLexCtx *ctx) {
    synth_err rv;
    
    rv = SYNTH_OK;
    
    if (synth_lex_isSetBPM(ctx) == SYNTH_TRUE)
        *tk = T_SET_BPM;
    else if (synth_lex_isSetDuration(ctx) == SYNTH_TRUE)
        *tk = T_SET_DURATION;
    else if (synth_lex_isSetOctave(ctx) == SYNTH_TRUE)
        *tk = T_SET_OCTAVE;
    else if (synth_lex_isSetRelOctave(ctx) == SYNTH_TRUE)
        *tk = T_SET_REL_OCTAVE;
    else if (synth_lex_isSetLoopPoint(ctx) == SYNTH_TRUE)
        *tk = T_SET_LOOPPOINT;
    else if (synth_lex_isEndOfTrack(ctx) == SYNTH_TRUE)
        *tk = T_END_OF_TRACK;
    else if (synth_lex_isSetVolume(ctx) == SYNTH_TRUE)
        *tk = T_SET_VOLUME;
    else if (synth_lex_isSetRelVolume(ctx) == SYNTH_TRUE)
        *tk = T_SET_REL_VOLUME;
    else if (synth_lex_isSetKeyoff(ctx) == SYNTH_TRUE)
        *tk = T_SET_KEYOFF;
    else if (synth_lex_isSetPan(ctx) == SYNTH_TRUE)
        *tk = T_SET_PAN;
    else if (synth_lex_isSetLoopStart(ctx) == SYNTH_TRUE)
        *tk = T_SET_LOOP_START;
    else if (synth_lex_isSetLoopEnd(ctx) == SYNTH_TRUE)
        *tk = T_SET_LOOP_END;
    else if (synth_lex_isSetWave(ctx) == SYNTH_TRUE)
        *tk = T_SET_WAVE;
    else if (synth_lex_isNote(ctx) == SYNTH_TRUE)
        *tk = T_NOTE;
    else if (synth_lex_isDotDuration(ctx) == SYNTH_TRUE)
        *tk = T_DURATION;
    else if (synth_lex_isNumber(ctx) == SYNTH_TRUE)
        *tk = T_NUMBER;
    else if (synth_lex_didFinish(ctx) == SYNTH_TRUE)
        *tk = T_DONE;
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
int synth_lex_getCurValuei(synthLexCtx *ctx) {
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
synth_bool synth_lex_isSetBPM(synthLexCtx *ctx) {
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
synth_bool synth_lex_isSetDuration(synthLexCtx *ctx) {
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
synth_bool synth_lex_isSetOctave(synthLexCtx *ctx) {
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
synth_bool synth_lex_isSetRelOctave(synthLexCtx *ctx) {
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
synth_bool synth_lex_isSetLoopPoint(synthLexCtx *ctx) {
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
synth_bool synth_lex_isEndOfTrack(synthLexCtx *ctx) {
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
synth_bool synth_lex_isSetVolume(synthLexCtx *ctx) {
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
synth_bool synth_lex_isSetRelVolume(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c == '(') {
        ctx->ivalue = 1;
        rv = SYNTH_TRUE;
    }
    else if (c == ')') {
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
synth_bool synth_lex_isSetKeyoff(synthLexCtx *ctx) {
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
synth_bool synth_lex_isSetPan(synthLexCtx *ctx) {
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
synth_bool synth_lex_isSetLoopStart(synthLexCtx *ctx) {
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
synth_bool synth_lex_isSetLoopEnd(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
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
synth_bool synth_lex_isSetWave(synthLexCtx *ctx) {
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
synth_bool synth_lex_isNote(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char note = 0;
    char mod = 0;
    
    // Get the current character
    srv = synth_lex_getChar(&note, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
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
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    
    if (mod == '+')
        ctx->ivalue++;
    else if (mod == '-')
        ctx->ivalue--;
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
synth_bool synth_lex_isDotDuration(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    ctx->ivalue = 0;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    SYNTH_ASSERT_ERR(c == '.', SYNTH_FALSE);
    
    while (1) {
        if (c != '.')
            break;
        
        ctx->ivalue = (ctx->ivalue << 1) | 1;
        
        srv = synth_lex_getChar(&c, ctx);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
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
synth_bool synth_lex_isNumber(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    ctx->ivalue = 0;
    
    // Get the current character
    srv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
    SYNTH_ASSERT_ERR(c >= '0' && c <= '9', SYNTH_FALSE);
    
    // Get the integer part
    while (1) {
        if (c < '0' || c > '9')
            break;
        
        // Accumulate the last gotten char
        ctx->ivalue = ctx->ivalue * 10 + (int)(c - '0');
        
        // Read the next one
        srv = synth_lex_getChar(&c, ctx);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK, SYNTH_FALSE);
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
synth_bool synth_lex_didFinish(synthLexCtx *ctx) {
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
        case T_SET_REL_VOLUME: return "set relative volume"; break;
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


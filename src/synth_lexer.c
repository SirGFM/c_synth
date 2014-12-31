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
     * Whether it's currently processing a file or a string
     */
    synth_bool isFile;
    /**
     * MML's source; either a file descriptor or a string
     */
    union {
        FILE *file;
        struct stString str;
    };
    /**
     * Integer value gotten when reading a token
     */
    int ivalue;
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
synth_err synth_lex_getToken(synthToken *tk, synthLexCtx *ctx) {
    synth_err rv;
    
    // TODO implement
    SYNTH_ASSERT_ERR(0, SYNTH_FUNCTION_NOT_IMPLEMENTED);
    
    if (0)
        {}
    else
        rv = SYNTH_INVALID_TOKEN;
    
__err:
    return rv;
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
        SYNTH_ASSERT_ERR(tmp != EOF, SYNTH_EOF_ERR);
    }
    else {
        // Get the current character and increase the position
        SYNTH_ASSERT_ERR(ctx->str.pos < ctx->str.len, SYNTH_EOS_ERR);
        tmp = ctx->str.str[ctx->str.pos];
        ctx->str.pos++;
    }
    
    // Return the read character
    *c = (char)tmp;
    rv = SYNTH_OK;
_err:
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
        
        if (isCommentary <= 1) {
            if (tmp == '/')
                isCommentary++;
            else {
                isComentary = 0;
                // Stop if it's a valid printable non-whitespace character
                if (tmp >= '!' && tmp <= '}')
                    break;
            }
        }
        // If it's a commentary, ignore until a newline
        else if (isComentary == 2 && tmp == '\n')
            isCommentary = 0;
    }
    
    *c = tmp;
    rv = SYNTH_OK;
_err:
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
__err:
    return;
}

synth_bool synth_lex_isSetBPM(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
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

synth_bool synth_lex_isSetDuration(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
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

synth_bool synth_lex_isSetOctave(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
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

synth_bool synth_lex_isSetRelOctave(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
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

synth_bool synth_lex_isSetLoopPoint(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
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

synth_bool synth_lex_isEndOfTrack(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
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

synth_bool synth_lex_isSetVolume(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
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

synth_bool synth_lex_isSetRelVolume(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
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

synth_bool synth_lex_isSetKeyoff(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
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

synth_bool synth_lex_isSetPan(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
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

synth_bool synth_lex_isSetLoopStart(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
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

synth_bool synth_lex_isSetLoopEnd(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
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

synth_bool synth_lex_isSetWave(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
    // Check if it's what was expected
    if (c != '%') {
        synth_lex_ungetChar(ctx, c);
        rv = SYNTH_FALSE;
    }
    else
        rv = SYNTH_TRUE;
__err:
    return rv;
}

synth_bool synth_lex_isNote(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char note = 0;
    char mod = 0;
    
    // Get the current character
    rv = synth_lex_getChar(&note, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
    switch (note) {
        case 'c': ctx->ivalue = N_C; break;
        case 'd': ctx->ivalue = N_D; break;
        case 'e': ctx->ivalue = N_E; break;
        case 'f': ctx->ivalue = N_F; break;
        case 'g': ctx->ivalue = N_G; break;
        case 'a': ctx->ivalue = N_A; break;
        case 'b': ctx->ivalue = N_B; break;
        default:
            // Something not a note was read SYNTH_ASSERT_ERR(0, SYNTH_FALSE);
    }
    
    // Try to read a modifier ('+' or '-')
    rv = synth_lex_getChar(&mod, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    
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

synth_bool synth_lex_isDotDuration(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    ctx->ivalue = 1;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    SYNTH_ASSERT_ERR(c == '.', SYNTH_FALSE);
    
    while (1) {
        if (c != '.')
            break;
        
        ctx->ivalue = (ctx->ivalue << 1) | ctx->ivalue;
        
        rv = synth_lex_getChar(&c, ctx);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    }
    
    rv = SYNTH_TRUE;
    // Return that last non-digit char
    synth_lex_ungetChar(ctx, c);
__err:
    return rv;
}

synth_bool synth_lex_isNumber(synthLexCtx *ctx) {
    synth_bool rv;
    synth_err srv;
    char c;
    
    ctx->ivalue = 0;
    
    // Get the current character
    rv = synth_lex_getChar(&c, ctx);
    SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    SYNTH_ASSERT_ERR(c >= '0' && c <= '9', SYNTH_FALSE);
    
    // Get the integer part
    while (1) {
        if (c < '0' || c > '9')
            break;
        
        // Accumulate the last gotten char
        ctx->ivalue = ctx->ivalue * 10 + (int)(c - '0');
        
        // Read the next one
        rv = synth_lex_getChar(&c, ctx);
        SYNTH_ASSERT_ERR(rv == SYNTH_OK, SYNTH_FALSE);
    }
    
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


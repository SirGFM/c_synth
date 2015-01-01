/**
 * @file tst/parse_string.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth_internal/synth_lexer.h>

#include <stdio.h>

static char mml[] = "t90 l16 o5 e e8 e r c e r g4 > g4 <";
int len = sizeof(mml);

int main(int argc, char *argv[]) {
    int rv;
    synth_err srv;
    synthLexCtx *ctx = 0;
    synth_token tk;
    
    if (argc > 1) {
        printf("Reading mml from file: %s\n", argv[1]);
        
        // Initialize the contex for parsing
        srv = synth_lex_tokenizef(&ctx, argv[1]);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    }
    else {
        printf("mml: %s\n", mml);
        
        // Initialize the contex for parsing
        srv = synth_lex_tokenizes(&ctx, mml, len);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    }
    
    while (1) {
        srv = synth_lex_getToken(&tk, ctx);
        SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
        
        printf("  %s\n", synth_lex_printToken(tk));
        if (tk == T_DONE)
            break;
    }
    
    rv = 0;
__err:
    if (ctx) {
        if (rv != 0) {
            printf("[ERROR] - line: %i\n", synth_lex_getCurrentLine(ctx));
            printf("        -  pos: %i\n", synth_lex_getCurrentLinePosition(ctx));
            printf("        - char: %c\n", synth_lex_getLastCharacter(ctx));
        }
        synth_lex_freeCtx(&ctx);
    }
    
    return rv;
}


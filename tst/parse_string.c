/**
 * @file tst/parse_string.c
 */
#include <synth/synth_assert.h>
#include <synth/synth_errors.h>
#include <synth_internal/synth_lexer.h>

static char mml[] = "t90 l16 o5 e e8 e r c e r g4 > g4 <";
int len = sizeof(mml);

int main(int argc, char *argv[]) {
    int rv;
    synth_err srv;
    synthLexCtx *ctx = 0;
    
    // Initialize the contex for parsing
    srv = synth_lex_tokenizes(&ctx, mml, len);
    SYNTH_ASSERT_ERR(srv == SYNTH_OK, srv);
    
    rv = 0;
__err:
    if (ctx)
        synth_lex_freeCtx(&ctx);
    
    return rv;
}


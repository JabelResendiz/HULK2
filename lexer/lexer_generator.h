#ifndef LEXER_GENERATOR_H
#define LEXER_GENERATOR_H

#include "token.h"
#include "dfa.h"

typedef struct
{
    DFA *dfa;
    char *input;
    int pos;
    int line;
    int column;
} Lexer;

// Funciones del lexer
Lexer *lexer_create(void);
void lexer_destroy(Lexer *lexer);
void lexer_init(Lexer *lexer);
Token *lexer_next_token(Lexer *lexer);
Token *lexer_peek_token(Lexer *lexer);
void lexer_set_input(Lexer *lexer, const char *input);

// Función auxiliar para prioridad de tokens
int get_token_priority(TokenType type);

#endif
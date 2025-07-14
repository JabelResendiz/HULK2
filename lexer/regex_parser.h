#ifndef REGEX_PARSER_H
#define REGEX_PARSER_H

#include "nfa.h"

typedef struct
{
    char *input;
    int pos;
    int length;
} RegexParser;

// Funciones del parser
NFA *regex_parse(const char *pattern, int is_literal);
NFA *parse_union(RegexParser *parser);
NFA *parse_concat(RegexParser *parser);
NFA *parse_closure(RegexParser *parser);
NFA *parse_atom(RegexParser *parser);

// Funciones auxiliares
char peek(RegexParser *parser);
char get(RegexParser *parser);
bool eat(RegexParser *parser, char c);

#endif
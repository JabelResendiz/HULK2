#ifndef PATTERNS_H
#define PATTERNS_H

#include "token.h"

typedef struct
{
    TokenType type;
    char *pattern;
    int is_literal; // 1 si es literal, 0 si es regex
} TokenPattern;

// Patrones basados en lexer.l
static TokenPattern patterns[] = {
    // Palabras clave (literales) - deben ir antes que los identificadores
    {TOKEN_FUNCTION, "function", 1},
    {TOKEN_LET, "let", 1},
    {TOKEN_IN, "in", 1},
    {TOKEN_IF, "if", 1},
    {TOKEN_ELIF, "elif", 1},
    {TOKEN_ELSE, "else", 1},
    {TOKEN_WHILE, "while", 1},
    {TOKEN_AS, "as", 1},
    {TOKEN_IS, "is", 1},
    {TOKEN_TYPE, "type", 1},
    {TOKEN_INHERITS, "inherits", 1},
    {TOKEN_NEW, "new", 1},
    {TOKEN_BASE, "base", 1},
    {TOKEN_FOR, "for", 1},
    {TOKEN_RANGE, "range", 1},
    {TOKEN_PI, "PI", 1},
    {TOKEN_E, "E", 1},

    // Identificadores (regex) - después de las palabras clave
    {TOKEN_ID, "[a-zA-ZñÑ][a-zA-ZñÑ0-9_]*", 0},

    // Literales
    {TOKEN_STRING, "\"([^\"\\\\]|\\\\.)*\"", 0},
    {TOKEN_NUMBER, "[0-9]+(\\.[0-9]+)?", 0},
    {TOKEN_BOOLEAN, "true|false", 0},

    // Operadores de asignación (literales)
    {TOKEN_PLUSEQUAL, "+=", 1},
    {TOKEN_MINUSEQUAL, "-=", 1},
    {TOKEN_TIMESEQUAL, "*=", 1},
    {TOKEN_DIVEQUAL, "/=", 1},
    {TOKEN_MODEQUAL, "%=", 1},
    {TOKEN_POWEQUAL, "^=", 1},
    {TOKEN_ANDEQUAL, "&=", 1},
    {TOKEN_OREQUAL, "|=", 1},
    {TOKEN_CONCATEQUAL, "@=", 1},
    {TOKEN_DEQUALS, ":=", 1},
    {TOKEN_EQUALSEQUALS, "==", 1},
    {TOKEN_NEQUALS, "!=", 1},
    {TOKEN_EGREATER, ">=", 1},
    {TOKEN_ELESS, "<=", 1},
    {TOKEN_DCONCAT, "@@", 1},
    {TOKEN_ARROW, "=>", 1},

    // Operadores simples (literales)
    {TOKEN_PLUS, "+", 1},
    {TOKEN_MINUS, "-", 1},
    {TOKEN_TIMES, "*", 1},
    {TOKEN_DIVIDE, "/", 1},
    {TOKEN_MOD, "%", 1},
    {TOKEN_POWER, "^", 1},
    {TOKEN_AND, "&", 1},
    {TOKEN_OR, "|", 1},
    {TOKEN_NOT, "!", 1},
    {TOKEN_GREATER, ">", 1},
    {TOKEN_LESS, "<", 1},
    {TOKEN_EQUALS, "=", 1},
    {TOKEN_CONCAT, "@", 1},
    {TOKEN_QUESTION, "?", 1},

    // Delimitadores (literales)
    {TOKEN_LPAREN, "(", 1},
    {TOKEN_RPAREN, ")", 1},
    {TOKEN_LBRACKET, "{", 1},
    {TOKEN_RBRACKET, "}", 1},
    {TOKEN_SEMICOLON, ";", 1},
    {TOKEN_COMMA, ",", 1},
    {TOKEN_DOT, ".", 1},
    {TOKEN_COLON, ":", 1}};

#define NUM_PATTERNS (sizeof(patterns) / sizeof(patterns[0]))

#endif
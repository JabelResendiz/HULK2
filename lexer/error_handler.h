#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tipos de errores del lexer
typedef enum {
    ERROR_UNKNOWN_CHARACTER,
    ERROR_UNTERMINATED_STRING,
    ERROR_UNTERMINATED_COMMENT,
    ERROR_INVALID_NUMBER,
    ERROR_INVALID_ESCAPE_SEQUENCE,
    ERROR_UNEXPECTED_EOF
} LexerErrorType;

// Estructura para errores del lexer
typedef struct {
    LexerErrorType type;
    int line;
    int column;
    char *message;
    char *lexeme;
} LexerError;

// Estructura para errores del parser
typedef enum {
    ERROR_UNEXPECTED_TOKEN,
    ERROR_MISSING_TOKEN,
    ERROR_INVALID_PRODUCTION,
    ERROR_UNEXPECTED_EOF_PARSER,
    ERROR_SYNTAX_ERROR
} ParserErrorType;

typedef struct {
    ParserErrorType type;
    int line;
    int column;
    char *message;
    char *expected;
    char *found;
} ParserError;

// Funciones para crear errores del lexer
LexerError *create_lexer_error(LexerErrorType type, int line, int column, const char *message, const char *lexeme);
void destroy_lexer_error(LexerError *error);
void print_lexer_error(LexerError *error, const char *source_code);

// Funciones para crear errores del parser
ParserError *create_parser_error(ParserErrorType type, int line, int column, const char *message, const char *expected, const char *found);
void destroy_parser_error(ParserError *error);
void print_parser_error(ParserError *error, const char *source_code);

// Funciones auxiliares
const char *get_lexer_error_type_name(LexerErrorType type);
const char *get_parser_error_type_name(ParserErrorType type);
void print_error_context(const char *source_code, int line, int column, int context_lines);

#endif // ERROR_HANDLER_H 
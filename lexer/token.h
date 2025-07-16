#ifndef TOKEN_H
#define TOKEN_H

typedef enum
{
    // Literales
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_BOOLEAN,
    TOKEN_ID,

    // Operadores aritméticos
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_TIMES,
    TOKEN_DIVIDE,
    TOKEN_MOD,
    TOKEN_POWER,

    // Operadores de asignación
    TOKEN_EQUALS,
    TOKEN_PLUSEQUAL,
    TOKEN_MINUSEQUAL,
    TOKEN_TIMESEQUAL,
    TOKEN_DIVEQUAL,
    TOKEN_MODEQUAL,
    TOKEN_POWEQUAL,
    TOKEN_ANDEQUAL,
    TOKEN_OREQUAL,
    TOKEN_CONCATEQUAL,
    TOKEN_DEQUALS,

    // Operadores de comparación
    TOKEN_EQUALSEQUALS,
    TOKEN_NEQUALS,
    TOKEN_GREATER,
    TOKEN_LESS,
    TOKEN_EGREATER,
    TOKEN_ELESS,

    // Operadores lógicos
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    // Operadores especiales
    TOKEN_CONCAT,
    TOKEN_DCONCAT,
    TOKEN_ARROW,
    TOKEN_QUESTION,

    // Delimitadores
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_COLON,

    // Palabras clave
    TOKEN_FUNCTION,
    TOKEN_LET,
    TOKEN_IN,
    TOKEN_IF,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_AS,
    TOKEN_IS,
    TOKEN_TYPE,
    TOKEN_INHERITS,
    TOKEN_NEW,
    TOKEN_BASE,
    TOKEN_FOR,
    TOKEN_RANGE,
    TOKEN_PI,
    TOKEN_E,

    // Especiales
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_COMMENT
} TokenType;

typedef struct
{
    TokenType type;
    char *lexeme;
    int line;
    int column;
    union
    {
        double number;
        char *string;
    } value;
} Token;

// Funciones de utilidad para tokens
Token *create_token(TokenType type, const char *lexeme, int line, int column);
void destroy_token(Token *token);
const char *get_token_name(TokenType type);

#endif
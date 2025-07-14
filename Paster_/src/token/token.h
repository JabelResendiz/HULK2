#ifndef TOKEN_H
#define TOKEN_H

// Enum de tipos de token basado en lexer.l
typedef enum {
    // EOF
    TOKEN_EOF = 0,

    // Literales
    NUMBER,
    STRING,
    TRUE,
    FALSE,

    // Identificadores
    IDENT,

    // Palabras clave
    IF,
    ELSE,
    ELIF,
    LET,
    IN,
    WHILE,
    FOR,
    TYPE,
    SELF,
    NEW,
    BASE,
    INHERITS,
    FUNCTION,
    IS,
    AS,
    ATTRIBUTE,
    METHOD,

    // Operadores
    PLUS,  // +
    MINUS, // -
    MULT,  // *
    DIV,   // /
    MOD,   // %
    POW,   // ** o ^

    // Operadores de comparación
    LE,           // <=
    GE,           // >=
    EQ,           // ==
    NEQ,          // !=
    LESS_THAN,    // <
    GREATER_THAN, // >

    // Operadores lógicos
    OR,  // ||
    AND, // &&
    NOT, // !

    // Asignación
    ASSIGN,          // =
    ASSIGN_DESTRUCT, // :=

    // Operadores de asignación compuesta
    PLUSEQUAL,    // +=
    MINUSEQUAL,   // -=
    TIMESEQUAL,   // *=
    DIVEQUAL,     // /=
    MODEQUAL,     // %=
    POWEQUAL,     // ^=
    CONCATEQUAL,  // @=
    ANDEQUAL,     // &=
    OREQUAL,      // |=

    // Símbolos especiales
    ARROW,     // =>
    CONCAT,    // @
    CONCAT_WS, // @@
    RANGE,     // ..
    DEQUALS,   // :=
    RETURN,    // return

    // Puntuación
    COMMA,     // ,
    SEMICOLON, // ;
    DOT,       // .
    COLON,     // :
    LPAREN,    // (
    RPAREN,    // )
    LBRACKET,  // {
    RBRACKET,  // }
    LBRACE,    // [
    RBRACE,    // ]
    PIPE,      // |
    QUESTION,  // ?

    // Tokens especiales
    NEWLINE,
    WHITESPACE,
    COMMENT,
    UNKNOWN,

    // Error
    T_ERROR
} TokenType;

// Estructura Token con información completa
typedef struct Token {
    TokenType type;
    char* lexeme;
    int line;
    int column;
    struct Token* next;
} Token;

// Funciones para manejo de tokens
Token* token_create(TokenType type, const char* lexeme, int line, int column);
void token_destroy(Token* token);
Token* token_create_empty(void);

// Función para obtener el nombre del token como string
const char* get_token_name(TokenType type);

// Función para verificar si un string es una palabra clave
TokenType get_keyword_type(const char* word);

// Función para liberar un token
void free_token(Token* token);

// Función para liberar una lista de tokens
void free_token_list(Token* list);

#endif // TOKEN_H
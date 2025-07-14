#include "token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Token* token_create(TokenType type, const char* lexeme, int line, int column) {
    Token* token = (Token*)malloc(sizeof(Token));
    if (!token) return NULL;

    token->type = type;
    token->line = line;
    token->column = column;

    if (lexeme) {
        token->lexeme = (char*)malloc(strlen(lexeme) + 1);
        if (!token->lexeme) {
            free(token);
            return NULL;
        }
        strcpy(token->lexeme, lexeme);
    } else {
        token->lexeme = NULL;
    }

    return token;
}

void token_destroy(Token* token) {
    if (token) {
        if (token->lexeme) {
            free(token->lexeme);
        }
        free(token);
    }
}

Token* token_create_empty(void) {
    return token_create(TOKEN_EOF, "", 0, 0);
}

const char* get_token_name(TokenType type) {
    printf("DEBUG: get_token_name called with type %d\n", type);
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case NUMBER: return "NUMBER";
        case STRING: return "STRING";
        case TRUE: return "TRUE";
        case FALSE: return "FALSE";
        case IDENT: return "IDENT";
        case IF: return "IF";
        case ELSE: return "ELSE";
        case ELIF: return "ELIF";
        case LET: return "LET";
        case IN: return "IN";
        case WHILE: return "WHILE";
        case FOR: return "FOR";
        case TYPE: return "TYPE";
        case SELF: return "SELF";
        case NEW: return "NEW";
        case BASE: return "BASE";
        case INHERITS: return "INHERITS";
        case FUNCTION: return "FUNCTION";
        case IS: return "IS";
        case AS: return "AS";
        case ATTRIBUTE: return "ATTRIBUTE";
        case METHOD: return "METHOD";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case MULT: return "MULT";
        case DIV: return "DIV";
        case MOD: return "MOD";
        case POW: return "POW";
        case LE: return "LE";
        case GE: return "GE";
        case EQ: return "EQ";
        case NEQ: return "NEQ";
        case LESS_THAN: return "LESS_THAN";
        case GREATER_THAN: return "GREATER_THAN";
        case OR: return "OR";
        case AND: return "AND";
        case NOT: return "NOT";
        case ASSIGN: return "ASSIGN";
        case ASSIGN_DESTRUCT: return "ASSIGN_DESTRUCT";
        case PLUSEQUAL: return "PLUSEQUAL";
        case MINUSEQUAL: return "MINUSEQUAL";
        case TIMESEQUAL: return "TIMESEQUAL";
        case DIVEQUAL: return "DIVEQUAL";
        case MODEQUAL: return "MODEQUAL";
        case POWEQUAL: return "POWEQUAL";
        case CONCATEQUAL: return "CONCATEQUAL";
        case ANDEQUAL: return "ANDEQUAL";
        case OREQUAL: return "OREQUAL";
        case ARROW: return "ARROW";
        case CONCAT: return "CONCAT";
        case CONCAT_WS: return "CONCAT_WS";
        case RANGE: return "RANGE";
        case DEQUALS: return "DEQUALS";
        case RETURN: return "RETURN";
        case COMMA: return "COMMA";
        case SEMICOLON: return "SEMICOLON";
        case DOT: return "DOT";
        case COLON: return "COLON";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
        case PIPE: return "PIPE";
        case QUESTION: return "QUESTION";
        case NEWLINE: return "NEWLINE";
        case WHITESPACE: return "WHITESPACE";
        case COMMENT: return "COMMENT";
        case UNKNOWN: return "UNKNOWN";
        case T_ERROR: return "ERROR";
        default:
            printf("DEBUG: Unknown token type %d, returning UNKNOWN\n", type);
            return "UNKNOWN";
    }
}

TokenType get_keyword_type(const char* word) {
    if (!word) return IDENT;

    if (strcmp(word, "if") == 0) return IF;
    if (strcmp(word, "else") == 0) return ELSE;
    if (strcmp(word, "elif") == 0) return ELIF;
    if (strcmp(word, "let") == 0) return LET;
    if (strcmp(word, "in") == 0) return IN;
    if (strcmp(word, "while") == 0) return WHILE;
    if (strcmp(word, "for") == 0) return FOR;
    if (strcmp(word, "type") == 0) return TYPE;
    if (strcmp(word, "self") == 0) return SELF;
    if (strcmp(word, "new") == 0) return NEW;
    if (strcmp(word, "base") == 0) return BASE;
    if (strcmp(word, "inherits") == 0) return INHERITS;
    if (strcmp(word, "function") == 0) return FUNCTION;
    if (strcmp(word, "is") == 0) return IS;
    if (strcmp(word, "as") == 0) return AS;
    if (strcmp(word, "return") == 0) return RETURN;
    if (strcmp(word, "true") == 0) return TRUE;
    if (strcmp(word, "false") == 0) return FALSE;

    return IDENT;
}

// Libera la memoria asignada para un único token.
void free_token(Token* token) {
    if (token) {
        // Libera las cadenas de caracteres si no son nulas
        free(token->lexeme);
        // Libera la estructura del token en sí
        free(token);
    }
}

// Libera una lista enlazada completa de tokens.
// Esta función ahora compilará y funcionará correctamente gracias
// a la corrección en token.h.
void free_token_list(Token* token) {
    while (token) {
        Token* next = token->next;
        free_token(token);
        token = next;
    }
}

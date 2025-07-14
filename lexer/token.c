#include "token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Token *create_token(TokenType type, const char *lexeme, int line, int column)
{
    Token *token = malloc(sizeof(Token));
    if (!token)
        return NULL;

    token->type = type;
    token->lexeme = strdup(lexeme);
    token->line = line;
    token->column = column;

    // Inicializar value.string a NULL por defecto
    token->value.string = NULL;

    // Convertir valor según el tipo
    if (type == TOKEN_NUMBER)
    {
        token->value.number = atof(lexeme);
    }
    else if (type == TOKEN_STRING)
    {
        // Remover comillas
        int len = strlen(lexeme);
        if (len >= 2)
        {
            char *str = malloc(len - 1);
            strncpy(str, lexeme + 1, len - 2);
            str[len - 2] = '\0';
            token->value.string = str;
        }
        else
        {
            token->value.string = strdup("");
        }
    }
    else if (type == TOKEN_ID)
    {
        // Para identificadores
        token->value.string = strdup(lexeme);
    }
    else if (type >= TOKEN_FUNCTION && type <= TOKEN_E)
    {
        // Para palabras clave
        token->value.string = strdup(lexeme);
    }
    // Para otros tipos (operadores, delimitadores, etc.), value.string queda en NULL

    return token;
}

void destroy_token(Token *token)
{
    if (!token)
        return;

    if (token->lexeme)
    {
        free(token->lexeme);
    }

    // Solo liberar value.string si corresponde
    if ((token->type == TOKEN_STRING || token->type == TOKEN_ID || (token->type >= TOKEN_FUNCTION && token->type <= TOKEN_E)) && token->value.string)
    {
        free(token->value.string);
        token->value.string = NULL;
    }

    free(token);
}

const char *get_token_name(TokenType type)
{
    switch (type)
    {
    case TOKEN_NUMBER:
        return "NUMBER";
    case TOKEN_STRING:
        return "STRING";
    case TOKEN_BOOLEAN:
        return "BOOLEAN";
    case TOKEN_ID:
        return "ID";

    case TOKEN_PLUS:
        return "PLUS";
    case TOKEN_MINUS:
        return "MINUS";
    case TOKEN_TIMES:
        return "TIMES";
    case TOKEN_DIVIDE:
        return "DIVIDE";
    case TOKEN_MOD:
        return "MOD";
    case TOKEN_POWER:
        return "POWER";

    case TOKEN_EQUALS:
        return "EQUALS";
    case TOKEN_PLUSEQUAL:
        return "PLUSEQUAL";
    case TOKEN_MINUSEQUAL:
        return "MINUSEQUAL";
    case TOKEN_TIMESEQUAL:
        return "TIMESEQUAL";
    case TOKEN_DIVEQUAL:
        return "DIVEQUAL";
    case TOKEN_MODEQUAL:
        return "MODEQUAL";
    case TOKEN_POWEQUAL:
        return "POWEQUAL";
    case TOKEN_ANDEQUAL:
        return "ANDEQUAL";
    case TOKEN_OREQUAL:
        return "OREQUAL";
    case TOKEN_CONCATEQUAL:
        return "CONCATEQUAL";
    case TOKEN_DEQUALS:
        return "DEQUALS";

    case TOKEN_EQUALSEQUALS:
        return "EQUALSEQUALS";
    case TOKEN_NEQUALS:
        return "NEQUALS";
    case TOKEN_GREATER:
        return "GREATER";
    case TOKEN_LESS:
        return "LESS";
    case TOKEN_EGREATER:
        return "EGREATER";
    case TOKEN_ELESS:
        return "ELESS";

    case TOKEN_AND:
        return "AND";
    case TOKEN_OR:
        return "OR";
    case TOKEN_NOT:
        return "NOT";

    case TOKEN_CONCAT:
        return "CONCAT";
    case TOKEN_DCONCAT:
        return "DCONCAT";
    case TOKEN_ARROW:
        return "ARROW";
    case TOKEN_QUESTION:
        return "QUESTION";

    case TOKEN_LPAREN:
        return "LPAREN";
    case TOKEN_RPAREN:
        return "RPAREN";
    case TOKEN_LBRACKET:
        return "LBRACKET";
    case TOKEN_RBRACKET:
        return "RBRACKET";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_COMMA:
        return "COMMA";
    case TOKEN_DOT:
        return "DOT";
    case TOKEN_COLON:
        return "COLON";

    case TOKEN_FUNCTION:
        return "FUNCTION";
    case TOKEN_LET:
        return "LET";
    case TOKEN_IN:
        return "IN";
    case TOKEN_IF:
        return "IF";
    case TOKEN_ELIF:
        return "ELIF";
    case TOKEN_ELSE:
        return "ELSE";
    case TOKEN_WHILE:
        return "WHILE";
    case TOKEN_AS:
        return "AS";
    case TOKEN_IS:
        return "IS";
    case TOKEN_TYPE:
        return "TYPE";
    case TOKEN_INHERITS:
        return "INHERITS";
    case TOKEN_NEW:
        return "NEW";
    case TOKEN_BASE:
        return "BASE";
    case TOKEN_FOR:
        return "FOR";
    case TOKEN_RANGE:
        return "RANGE";
    case TOKEN_PI:
        return "PI";
    case TOKEN_E:
        return "E";

    case TOKEN_EOF:
        return "EOF";
    case TOKEN_ERROR:
        return "ERROR";

    default:
        return "UNKNOWN";
    }
}
#include "src/token/token.h"
#include <stdio.h>

int main() {
    printf("TokenType enum values:\n");
    printf("TOKEN_EOF = %d\n", TOKEN_EOF);
    printf("NUMBER = %d\n", NUMBER);
    printf("STRING = %d\n", STRING);
    printf("TRUE = %d\n", TRUE);
    printf("FALSE = %d\n", FALSE);
    printf("IDENT = %d\n", IDENT);
    printf("IF = %d\n", IF);
    printf("ELSE = %d\n", ELSE);
    printf("LET = %d\n", LET);
    printf("WHILE = %d\n", WHILE);
    printf("FUNCTION = %d\n", FUNCTION);
    printf("PLUS = %d\n", PLUS);
    printf("MINUS = %d\n", MINUS);
    printf("MULT = %d\n", MULT);
    printf("DIV = %d\n", DIV);
    printf("MOD = %d\n", MOD);
    printf("POW = %d\n", POW);
    printf("LE = %d\n", LE);
    printf("GE = %d\n", GE);
    printf("EQ = %d\n", EQ);
    printf("NEQ = %d\n", NEQ);
    printf("LESS_THAN = %d\n", LESS_THAN);
    printf("GREATER_THAN = %d\n", GREATER_THAN);
    printf("OR = %d\n", OR);
    printf("AND = %d\n", AND);
    printf("ASSIGN = %d\n", ASSIGN);
    printf("ASSIGN_DESTRUCT = %d\n", ASSIGN_DESTRUCT);
    printf("ARROW = %d\n", ARROW);
    printf("CONCAT = %d\n", CONCAT);
    printf("CONCAT_WS = %d\n", CONCAT_WS);
    printf("COMMA = %d\n", COMMA);
    printf("SEMICOLON = %d\n", SEMICOLON);
    printf("DOT = %d\n", DOT);
    printf("COLON = %d\n", COLON);
    printf("LPAREN = %d\n", LPAREN);
    printf("RPAREN = %d\n", RPAREN);
    printf("LBRACE = %d\n", LBRACE);
    printf("RBRACE = %d\n", RBRACE);
    printf("UNKNOWN = %d\n", UNKNOWN);
    printf("T_ERROR = %d\n", T_ERROR);
    
    return 0;
} 
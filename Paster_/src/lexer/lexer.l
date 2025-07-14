%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "y.tab.h"
    #include "ast/ast.h"


    int yywrap(void) {return 1;}
    int line_num = 1;
    #define RED "\x1B[31m"
    #define RESET "x1B[0m"

%}

%%


\n              { line_num++; }
[ \t]           { ; }
";"             { return SEMICOLON; }
","             { return COMMA; }
"."             { return DOT; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULT; }
"/"             { return DIVIDE; }
"%"             { return MOD; }
"^"             { return POWER; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACKET; }
"}"             { return RBRACKET; }
">="            { return GE; }
"<="            { return LE; }
"=="            { return EQ; }
">"             { return GT; }
"<"             { return LT; }
"="             { return EQUALS; }
"=>"            { return ARROW;}
"if"            { return IF;}
"else"          { return ELSE;}
"let"           { return LET; }
"in"            { return IN; }
":="            { return DESTRUCTOR; }
":"             { return COLON; }    
"!="            { return NEQUALS;}
"!"             { return NOT;}
"&"             { return AND;}
"|"             { return OR;}
"@@"            { return DCONCAT;}
"@"             { return CONCAT;}
"PI"            { return PI;}
"E"             { return E;}
"function"      { return FUNCTION;}
"elif"          { return ELIF;}
"while"         { return WHILE;}
"return"        { return RETURN;}
"as"            { return AS; }
"is"            { return IS; }
"type"          { return TYPE; }
"inherits"      { return INHERITS; }
"new"           { return NEW; }
"base"          { return BASE; }

[0-9]+          { yylval.val = atof(yytext); return NUMBER; }
[0-9]+\.[0-9]+  { yylval.val = atof(yytext); return NUMBER; }

[a-zA-ZñÑ][a-zA-ZñÑ0-9_]*  { yylval.var = strdup(yytext); return ID; }
"//"[^\n]*?   { /* COMMENT */ }

\"([^"\\]|\\.)*\" {
    int len = strlen(yytext);
    yylval.var = strdup(yytext + 1);  // Skip first quote
    yylval.var[len - 2] = '\0';       // Replace last quote for '\0'
    return STRING;
}

.           { 
                fprintf(stderr, RED"!!LEXICAL ERROR: Invalid character '%s'. Line: %d \n"RESET, yytext, line_num);
                return T_ERROR;
            }

%%
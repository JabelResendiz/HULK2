#ifndef LL1_PARSER_H
#define LL1_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../token/token.h"
#include "../ast/ast_compat.h"
#include "../error/error.h"

// Estructura para el parser LL1
typedef struct {
    Token* tokens;
    int token_count;
    int current_token;
    int max_tokens;
    ErrorContext* error_context;
} LL1Parser;

// Funciones del parser
LL1Parser* ll1_parser_create(Token* tokens, int token_count);
LL1Parser* ll1_parser_create_with_errors(Token* tokens, int token_count, ErrorContext* error_context);
void ll1_parser_destroy(LL1Parser* parser);
Program* ll1_parser_parse(LL1Parser* parser);

// Funciones auxiliares
Token* ll1_parser_peek(LL1Parser* parser);
Token* ll1_parser_advance(LL1Parser* parser);
bool ll1_parser_match(LL1Parser* parser, TokenType token_type);
bool ll1_parser_is_at_end(LL1Parser* parser);

// Funciones de parsing para diferentes construcciones
Stmt* ll1_parse_statement(LL1Parser* parser);
Expr* ll1_parse_expression(LL1Parser* parser);
Expr* ll1_parse_equality(LL1Parser* parser);
Expr* ll1_parse_comparison(LL1Parser* parser);
Expr* ll1_parse_term(LL1Parser* parser);
Expr* ll1_parse_factor(LL1Parser* parser);
Expr* ll1_parse_unary(LL1Parser* parser);
Expr* ll1_parse_primary(LL1Parser* parser);
Expr* ll1_parse_call(LL1Parser* parser);
Expr* ll1_parse_get_attr(LL1Parser* parser);
Expr* ll1_parse_new(LL1Parser* parser);
Expr* ll1_parse_is(LL1Parser* parser);
Expr* ll1_parse_as(LL1Parser* parser);
Expr* ll1_parse_base_call(LL1Parser* parser);
Expr* ll1_parse_let(LL1Parser* parser);
Expr* ll1_parse_if(LL1Parser* parser);
Expr* ll1_parse_while(LL1Parser* parser);
Expr* ll1_parse_for(LL1Parser* parser);
Expr* ll1_parse_block(LL1Parser* parser);
Expr* ll1_parse_assignment(LL1Parser* parser);

// Funciones para declaraciones
FunctionDecl* ll1_parse_function_decl(LL1Parser* parser);
TypeDecl* ll1_parse_type_decl(LL1Parser* parser);
Expr* ll1_parse_let_in(LL1Parser* parser);
Expr** ll1_parse_let_definitions(LL1Parser* parser, int* out_count);
Expr* ll1_parse_simple_var_decl(LL1Parser* parser);
AttributeDecl* ll1_parse_attribute_decl(LL1Parser* parser);
MethodDecl* ll1_parse_method_decl(LL1Parser* parser);

// Nuevas funciones de parsing para statements
Stmt* ll1_parse_let_declaration(LL1Parser* parser);
Stmt* ll1_parse_if_statement(LL1Parser* parser);
Stmt* ll1_parse_while_statement(LL1Parser* parser);

// Funciones para parsing de tipos
TypeInfo* ll1_parse_type(LL1Parser* parser);
VarBinding* ll1_parse_var_binding(LL1Parser* parser);

// Prototipos de funciones de parsing de tipos
TypeValue* ll1_parse_type_annotation(LL1Parser* parser);
TypeValue* ll1_parse_generic_type(LL1Parser* parser);

// Funciones para manejo de errores sintácticos
void parser_error_unexpected_token(LL1Parser* parser, TokenType expected, TokenType found);
void parser_error_unexpected_character(LL1Parser* parser, const char* expected, const char* found);
void parser_error_missing_token(LL1Parser* parser, TokenType expected);
void parser_error_invalid_expression(LL1Parser* parser, const char* context);

#endif // LL1_PARSER_H
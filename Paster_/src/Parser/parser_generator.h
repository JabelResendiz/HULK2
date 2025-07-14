#ifndef PARSER_GENERATOR_H
#define PARSER_GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../token/token.h"
#include "../ast/ast.h"
#include "../error/error.h"

// Estructuras para representar la gramática
typedef enum {
    GRAMMAR_SYMBOL_TERMINAL,
    GRAMMAR_SYMBOL_NONTERMINAL,
    GRAMMAR_SYMBOL_ACTION
} GrammarSymbolType;

typedef struct GrammarSymbol {
    char* name;
    GrammarSymbolType type;
    int token_type;  // Para símbolos terminales
    struct GrammarSymbol* next;
} GrammarSymbol;

typedef struct Production {
    GrammarSymbol* left_side;
    GrammarSymbol** right_side;
    int right_side_count;
    char* action;  // Código de acción semántica
    struct Production* next;
} Production;

typedef struct Grammar {
    GrammarSymbol* symbols;
    Production* productions;
    GrammarSymbol* start_symbol;
    int symbol_count;
    int production_count;
} Grammar;

// Estructuras para las tablas de parsing
typedef struct ParsingTable {
    int** table;  // [nonterminal][terminal] -> production_index
    int nonterminal_count;
    int terminal_count;
    GrammarSymbol** nonterminals;
    GrammarSymbol** terminals;
} ParsingTable;

// Estructura para el parser generado
typedef struct GeneratedParser {
    ParsingTable* table;
    Grammar* grammar;
    Token* tokens;
    int token_count;
    int current_token;
    ErrorContext* error_context;
} GeneratedParser;

// Funciones del generador de parser
Grammar* grammar_load_from_file(const char* filename);
void grammar_destroy(Grammar* grammar);
void grammar_print(Grammar* grammar);

// Funciones para generar tablas de parsing
ParsingTable* generate_parsing_table(Grammar* grammar);
void parsing_table_destroy(ParsingTable* table);
void parsing_table_print(ParsingTable* table);

// Funciones para el parser generado
GeneratedParser* generated_parser_create(Token* tokens, int token_count, 
                                       ParsingTable* table, Grammar* grammar,
                                       ErrorContext* error_context);
void generated_parser_destroy(GeneratedParser* parser);
ASTNode* generated_parser_parse(GeneratedParser* parser);

// Funciones auxiliares
GrammarSymbol* grammar_symbol_create(const char* name, GrammarSymbolType type, int token_type);
void grammar_symbol_destroy(GrammarSymbol* symbol);
Production* production_create(GrammarSymbol* left_side, GrammarSymbol** right_side, 
                           int right_side_count, const char* action);
void production_destroy(Production* production);

// Funciones para calcular FIRST y FOLLOW
bool* calculate_first_set(Grammar* grammar, GrammarSymbol* symbol);
bool* calculate_follow_set(Grammar* grammar, GrammarSymbol* symbol);

// Funciones para generar código C del parser
void generate_parser_code(Grammar* grammar, ParsingTable* table, 
                         const char* output_filename);

#endif // PARSER_GENERATOR_H 
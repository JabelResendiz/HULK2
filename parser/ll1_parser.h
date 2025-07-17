#ifndef LL1_PARSER_H
#define LL1_PARSER_H

#include "ll1_structures.h"
#include "ll1_table.h"
#include "first_calculator.h"
#include "follow_calculator.h"
#include "../lexer/error_handler.h"

// ===== PARSER LL(1) =====

// Estado del parser
typedef struct
{
    ParserToken **tokens; // Array de tokens
    int token_count;      // Total de tokens
    int current_token;    // Índice del token actual
    Grammar *grammar;     // Gramática
    LL1Table *table;      // Tabla LL(1)
    CSTNode *root;        // Raíz del árbol CST
    int debug_mode;       // Modo debug para imprimir pasos
    ParserError *last_error; // Último error del parser
} LL1Parser;

// ===== FUNCIONES DEL PARSER =====

// Crear parser LL(1)
LL1Parser *create_ll1_parser(Grammar *grammar, LL1Table *table);

// Liberar memoria del parser
void free_ll1_parser(LL1Parser *parser);

// Agregar token al parser
void add_token(LL1Parser *parser, ParserToken *token);

// Crear token
ParserToken *create_parser_token(const char *type, const char *value, int line, int column);

// Liberar token
void free_token(ParserToken *token);

// Imprimir token
void print_token(ParserToken *token);

// Parsear usando algoritmo iterativo con stack (O(1) por producción)
CSTNode *parse_iterative(LL1Parser *parser);

// Función wrapper para parsear (llama a parse_iterative)
CSTNode *parse_ll1(LL1Parser *parser);

// ===== FUNCIONES AUXILIARES =====

// Obtener token actual
ParserToken *get_current_token(LL1Parser *parser);

// Avanzar al siguiente token
void advance_token(LL1Parser *parser);

// Imprimir árbol CST
void print_cst_tree(CSTNode *root, int depth);

// Imprimir información de debug
void print_debug_info(LL1Parser *parser, const char *message);

// Funciones de manejo de errores del parser
ParserError *parser_get_last_error(LL1Parser *parser);
void parser_clear_error(LL1Parser *parser);
void parser_print_error(LL1Parser *parser, const char *source_code);

#endif // LL1_PARSER_H
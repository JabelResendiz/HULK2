#ifndef GRAMMAR_PARSER_H
#define GRAMMAR_PARSER_H

#include "ll1_structures.h"

// ===== PARSER DE GRAMÁTICA =====

// Leer gramática desde archivo
Grammar *parse_grammar_file(const char *filename);

// Parsear una línea de producción (puede devolver múltiples producciones)
Production **parse_production_line(const char *line, int *count);

// Extraer símbolos de una cadena
char **split_symbols(const char *str, int *count);

// Extraer símbolos ignorando el operador de alternancia |
char **split_symbols_ignore_alternation(const char *str, int *count);

// Dividir una cadena por el operador de alternancia |
char **split_alternatives(const char *str, int *count);

// Limpiar cadena (eliminar espacios extra)
char *clean_string(const char *str);

// Verificar si una línea es válida para parsear
int is_valid_production_line(const char *line);

// Imprimir información de la gramática parseada
void print_grammar_info(Grammar *grammar);

#endif // GRAMMAR_PARSER_H
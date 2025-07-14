#ifndef LL1_GRAMMAR_H
#define LL1_GRAMMAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SYMBOLS 128
#define MAX_PRODUCTIONS 256
#define MAX_RHS 16
#define MAX_SETS 128
#define MAX_NAME 32
#define MAX_LINE 256

// Tipos de símbolo
typedef enum
{
    LL1_TERMINAL,
    LL1_NONTERMINAL,
    LL1_EPSILON
} LL1_SymbolType;

// Símbolo
typedef struct
{
    char name[MAX_NAME];
    LL1_SymbolType type;
} LL1_Symbol;

// Producción
typedef struct
{
    LL1_Symbol left;
    LL1_Symbol right[MAX_RHS];
    int right_len;
} LL1_Production;

// Gramática
typedef struct
{
    LL1_Symbol terminals[MAX_SYMBOLS];
    int num_terminals;
    LL1_Symbol nonterminals[MAX_SYMBOLS];
    int num_nonterminals;
    LL1_Production productions[MAX_PRODUCTIONS];
    int num_productions;
    LL1_Symbol start_symbol;
} LL1_Grammar;

// Funciones para cargar y analizar gramática
void ll1_init_grammar(LL1_Grammar *g);
bool ll1_load_grammar_from_file(LL1_Grammar *g, const char *filename);
void ll1_print_grammar(const LL1_Grammar *g);
int ll1_find_symbol(const LL1_Grammar *g, const char *name, LL1_SymbolType *type_out);

// Funciones auxiliares para parsing
void ll1_parse_terminals_line(LL1_Grammar *g, const char *line);
void ll1_parse_nonterminals_line(LL1_Grammar *g, const char *line);
void ll1_parse_productions_line(LL1_Grammar *g, const char *line);
char **ll1_split_string(const char *str, const char *delimiter, int *count);
void ll1_free_string_array(char **arr, int count);

#endif // LL1_GRAMMAR_H
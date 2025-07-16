#ifndef LL1_STRUCTURES_H
#define LL1_STRUCTURES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== ESTRUCTURAS BÁSICAS =====

// Una producción: A -> α
typedef struct
{
    char *lhs;     // Lado izquierdo (no terminal)
    char **rhs;    // Lado derecho (array de símbolos)
    int rhs_count; // Cantidad de símbolos en rhs
} Production;

// Gramática completa
typedef struct
{
    Production **productions; // Array de punteros a producciones
    int production_count;     // Cantidad de producciones
    char *start_symbol;       // Símbolo inicial
} Grammar;

// Conjunto de símbolos (para FIRST y FOLLOW)
typedef struct
{
    char **symbols; // Array de símbolos
    int count;      // Cantidad de símbolos
} SymbolSet;

// Tabla LL(1): [No Terminal][Terminal] -> Producción
typedef struct
{
    char ***table;       // table[no_terminal][terminal] = producción
    char **no_terminals; // Lista de no terminales
    char **terminals;    // Lista de terminales
    int no_terminal_count;
    int terminal_count;
} LL1Table;

// Nodo del árbol de derivación (CST)
typedef struct CSTNode
{
    char *symbol;              // Símbolo del nodo
    struct CSTNode **children; // Hijos del nodo
    int child_count;           // Cantidad de hijos
    int line;                  // Línea en el código fuente
    int column;                // Columna en el código fuente
} CSTNode;

// ===== FUNCIONES AUXILIARES =====

// Crear un conjunto de símbolos vacío
SymbolSet *create_symbol_set();

// Agregar símbolo a un conjunto (sin duplicados)
void add_symbol(SymbolSet *set, const char *symbol);

// Verificar si un símbolo está en un conjunto
int contains_symbol(SymbolSet *set, const char *symbol);

// Liberar memoria de un conjunto
void free_symbol_set(SymbolSet *set);

// Crear una producción
Production *create_production(const char *lhs);

// Agregar símbolo al lado derecho de una producción
void add_rhs_symbol(Production *prod, const char *symbol);

// Liberar memoria de una producción
void free_production(Production *prod);

// Crear gramática vacía
Grammar *create_grammar();

// Agregar producción a la gramática
void add_production(Grammar *grammar, Production *prod);

// Liberar memoria de la gramática
void free_grammar(Grammar *grammar);

// Crear tabla LL(1) vacía
LL1Table *create_ll1_table();

// Liberar memoria de la tabla LL(1)
void free_ll1_table(LL1Table *table);

// Crear nodo CST
CSTNode *create_cst_node(const char *symbol);

// Agregar hijo a un nodo CST
void add_cst_child(CSTNode *parent, CSTNode *child);

// Liberar memoria de un árbol CST
void free_cst_tree(CSTNode *root);

// ===== FUNCIONES DE UTILIDAD =====

// Verificar si un símbolo es terminal
int is_terminal(const char *symbol);

// Verificar si un símbolo es no terminal
int is_non_terminal(const char *symbol);

// Verificar si un símbolo es epsilon (ε)
int is_epsilon(const char *symbol);

// Imprimir conjunto de símbolos (para debugging)
void print_symbol_set(SymbolSet *set, const char *name);

// Imprimir producción (para debugging)
void print_production(Production *prod);

// Imprimir gramática (para debugging)
void print_grammar(Grammar *grammar);

#endif // LL1_STRUCTURES_H
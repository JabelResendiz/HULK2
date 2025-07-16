#ifndef FIRST_CALCULATOR_H
#define FIRST_CALCULATOR_H

#include "ll1_structures.h"

// ===== CALCULADOR DE FIRST =====

// Calcular conjuntos FIRST para todos los símbolos de la gramática
typedef struct
{
    SymbolSet **first_sets;
    char **symbols;
    int symbol_count;
} FirstResult;

FirstResult compute_first_sets(Grammar *grammar);

// Calcular FIRST de una cadena específica (array de símbolos)
SymbolSet *compute_first_of_string(char **symbols, int symbol_count, SymbolSet **first_sets, char **all_symbols, int total_symbols);

// Verificar si un símbolo puede derivar ε (epsilon)
int can_derive_epsilon(const char *symbol, Grammar *grammar, SymbolSet **first_sets, int total_symbols);

// Obtener el índice de un símbolo en el array de símbolos
int get_symbol_index(const char *symbol, char **symbols, int symbol_count);

// Imprimir todos los conjuntos FIRST
void print_first_sets(SymbolSet **first_sets, char **symbols, int symbol_count);

// Liberar memoria de los conjuntos FIRST y símbolos
void free_first_result(FirstResult result);

#endif // FIRST_CALCULATOR_H
#ifndef FOLLOW_CALCULATOR_H
#define FOLLOW_CALCULATOR_H

#include "ll1_structures.h"
#include "first_calculator.h"

// Estructura para el resultado del cálculo de FOLLOW
typedef struct
{
    SymbolSet **follow_sets; // Array de conjuntos FOLLOW
    char **symbols;          // Array de símbolos únicos
    int symbol_count;        // Número de símbolos
} FollowResult;

// Calcular conjuntos FOLLOW para una gramática
FollowResult *compute_follow_sets(Grammar *grammar, FirstResult *first_result);

// Imprimir todos los conjuntos FOLLOW
void print_follow_sets(FollowResult *result);

// Liberar memoria de FollowResult
void free_follow_result(FollowResult *result);

#endif // FOLLOW_CALCULATOR_H
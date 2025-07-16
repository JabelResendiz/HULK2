#ifndef LL1_TABLE_H
#define LL1_TABLE_H

#include "ll1_structures.h"
#include "first_calculator.h"
#include "follow_calculator.h"

// Construir tabla LL(1) completa usando la estructura existente
LL1Table *build_ll1_table(Grammar *grammar, FirstResult *first_result, FollowResult *follow_result);

// Buscar entrada en la tabla LL(1)
int find_ll1_entry(LL1Table *table, const char *non_terminal, const char *terminal);

// Imprimir tabla LL(1)
void print_ll1_table(LL1Table *table, Grammar *grammar);

#endif // LL1_TABLE_H
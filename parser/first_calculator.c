#include "first_calculator.h"
#include <string.h>

// ===== IMPLEMENTACIÓN DEL CALCULADOR DE FIRST =====

FirstResult compute_first_sets(Grammar *grammar)
{
    // Primero, recolectar todos los símbolos únicos
    SymbolSet *all_symbols = create_symbol_set();

    // Agregar todos los símbolos de las producciones
    for (int i = 0; i < grammar->production_count; i++)
    {
        Production *prod = grammar->productions[i];
        add_symbol(all_symbols, prod->lhs);
        for (int j = 0; j < prod->rhs_count; j++)
        {
            add_symbol(all_symbols, prod->rhs[j]);
        }
    }

    // Crear array de símbolos únicos
    char **symbols = malloc(all_symbols->count * sizeof(char *));
    for (int i = 0; i < all_symbols->count; i++)
    {
        symbols[i] = strdup(all_symbols->symbols[i]);
    }
    int symbol_count = all_symbols->count;

    // Crear array de conjuntos FIRST
    SymbolSet **first_sets = malloc(symbol_count * sizeof(SymbolSet *));
    for (int i = 0; i < symbol_count; i++)
    {
        first_sets[i] = create_symbol_set();
        if (is_terminal(symbols[i]))
        {
            add_symbol(first_sets[i], symbols[i]);
        }
    }

    // Algoritmo iterativo hasta convergencia
    int changed = 1;
    while (changed)
    {
        changed = 0;
        for (int i = 0; i < grammar->production_count; i++)
        {
            Production *prod = grammar->productions[i];
            int lhs_index = get_symbol_index(prod->lhs, symbols, symbol_count);
            if (prod->rhs_count == 0)
            {
                if (!contains_symbol(first_sets[lhs_index], "ε"))
                {
                    add_symbol(first_sets[lhs_index], "ε");
                    changed = 1;
                }
            }
            else
            {
                SymbolSet *rhs_first = compute_first_of_string(prod->rhs, prod->rhs_count, first_sets, symbols, symbol_count);
                for (int j = 0; j < rhs_first->count; j++)
                {
                    if (!contains_symbol(first_sets[lhs_index], rhs_first->symbols[j]))
                    {
                        add_symbol(first_sets[lhs_index], rhs_first->symbols[j]);
                        changed = 1;
                    }
                }
                free_symbol_set(rhs_first);
            }
        }
    }
    free_symbol_set(all_symbols);
    FirstResult result = {first_sets, symbols, symbol_count};
    return result;
}

SymbolSet *compute_first_of_string(char **symbols, int symbol_count, SymbolSet **first_sets, char **all_symbols, int total_symbols)
{
    SymbolSet *result = create_symbol_set();

    for (int i = 0; i < symbol_count; i++)
    {
        const char *X = symbols[i];

        // Si X es terminal, agregar X a FIRST
        if (is_terminal(X))
        {
            add_symbol(result, X);
            break; // Parar aquí, no continuar
        }

        // Si X es no terminal, agregar FIRST(X) - {ε} a FIRST
        int X_index = get_symbol_index(X, all_symbols, total_symbols);
        if (X_index != -1)
        {
            // Agregar todos los símbolos de FIRST(X) excepto ε
            for (int j = 0; j < first_sets[X_index]->count; j++)
            {
                const char *symbol = first_sets[X_index]->symbols[j];
                if (strcmp(symbol, "ε") != 0)
                {
                    add_symbol(result, symbol);
                }
            }

            // Si X no tiene ε, parar
            if (!contains_symbol(first_sets[X_index], "ε"))
            {
                break;
            }
        }
    }

    return result;
}

int can_derive_epsilon(const char *symbol, Grammar *grammar, SymbolSet **first_sets, int total_symbols)
{
    for (int i = 0; i < grammar->production_count; i++)
    {
        Production *prod = grammar->productions[i];
        if (strcmp(prod->lhs, symbol) == 0)
        {
            if (prod->rhs_count == 0)
            {
                return 1;
            }
            int all_can_derive_epsilon = 1;
            for (int j = 0; j < prod->rhs_count; j++)
            {
                if (!can_derive_epsilon(prod->rhs[j], grammar, first_sets, total_symbols))
                {
                    all_can_derive_epsilon = 0;
                    break;
                }
            }
            if (all_can_derive_epsilon)
            {
                return 1;
            }
        }
    }
    return 0;
}

int get_symbol_index(const char *symbol, char **symbols, int symbol_count)
{
    for (int i = 0; i < symbol_count; i++)
    {
        if (strcmp(symbols[i], symbol) == 0)
        {
            return i;
        }
    }
    return -1;
}

void print_first_sets(SymbolSet **first_sets, char **symbols, int symbol_count)
{
    printf("\n=== CONJUNTOS FIRST ===\n");
    for (int i = 0; i < symbol_count; i++)
    {
        printf("FIRST(%s) = {", symbols[i]);
        for (int j = 0; j < first_sets[i]->count; j++)
        {
            printf("%s", first_sets[i]->symbols[j]);
            if (j < first_sets[i]->count - 1)
            {
                printf(", ");
            }
        }
        printf("}\n");
    }
}

void free_first_result(FirstResult result)
{
    for (int i = 0; i < result.symbol_count; i++)
    {
        free_symbol_set(result.first_sets[i]);
        free(result.symbols[i]);
    }
    free(result.first_sets);
    free(result.symbols);
}
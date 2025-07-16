#include "follow_calculator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Función auxiliar para verificar si un símbolo puede derivar ε
static int can_derive_epsilon_follow(const char *symbol, FirstResult *first_result)
{
    int index = get_symbol_index(symbol, first_result->symbols, first_result->symbol_count);
    if (index != -1)
    {
        return contains_symbol(first_result->first_sets[index], "ε");
    }
    return 0;
}

// Calcular FIRST de una cadena de símbolos (para FOLLOW)
static SymbolSet *compute_first_of_string_for_follow(char **symbols, int symbol_count,
                                                     FirstResult *first_result)
{
    SymbolSet *result = create_symbol_set();
    int all_can_derive_epsilon = 1;

    for (int i = 0; i < symbol_count; i++)
    {
        const char *X = symbols[i];

        // Si X es terminal, agregar X a FIRST
        if (is_terminal(X))
        {
            add_symbol(result, X);
            all_can_derive_epsilon = 0;
            break;
        }

        // Si X es no terminal, agregar FIRST(X) - {ε} a FIRST
        int X_index = get_symbol_index(X, first_result->symbols, first_result->symbol_count);
        if (X_index != -1)
        {
            // Agregar todos los símbolos de FIRST(X) excepto ε
            for (int j = 0; j < first_result->first_sets[X_index]->count; j++)
            {
                const char *symbol = first_result->first_sets[X_index]->symbols[j];
                if (strcmp(symbol, "ε") != 0)
                {
                    add_symbol(result, symbol);
                }
            }

            // Si X no tiene ε, parar
            if (!contains_symbol(first_result->first_sets[X_index], "ε"))
            {
                all_can_derive_epsilon = 0;
                break;
            }
        }
    }

    // Si todos los símbolos pueden derivar ε, agregar ε al resultado
    if (all_can_derive_epsilon && symbol_count > 0)
    {
        add_symbol(result, "ε");
    }

    return result;
}

FollowResult *compute_follow_sets(Grammar *grammar, FirstResult *first_result)
{
    FollowResult *result = malloc(sizeof(FollowResult));
    result->symbols = first_result->symbols;
    result->symbol_count = first_result->symbol_count;

    // Inicializar conjuntos FOLLOW
    result->follow_sets = malloc(result->symbol_count * sizeof(SymbolSet *));
    for (int i = 0; i < result->symbol_count; i++)
    {
        result->follow_sets[i] = create_symbol_set();
    }

    // FOLLOW(start_symbol) contiene $
    int start_index = get_symbol_index(grammar->start_symbol, result->symbols, result->symbol_count);
    if (start_index != -1)
    {
        add_symbol(result->follow_sets[start_index], "$");
    }

    // Algoritmo de punto fijo para calcular FOLLOW
    int changed = 1;
    int iteration = 0;
    while (changed && iteration < 100) // Límite de seguridad
    {
        changed = 0;
        iteration++;

        for (int p = 0; p < grammar->production_count; p++)
        {
            Production *prod = grammar->productions[p];
            const char *A = prod->lhs;
            int A_index = get_symbol_index(A, result->symbols, result->symbol_count);
            if (A_index == -1)
                continue;

            for (int i = 0; i < prod->rhs_count; i++)
            {
                const char *B = prod->rhs[i];

                // Solo procesamos no terminales
                if (is_terminal(B))
                {
                    continue;
                }

                int B_index = get_symbol_index(B, result->symbols, result->symbol_count);
                if (B_index == -1)
                    continue;

                // Para A -> αBβ
                char **beta = &prod->rhs[i + 1];
                int beta_count = prod->rhs_count - i - 1;

                if (beta_count == 0)
                {
                    // B está al final: FOLLOW(A) ⊆ FOLLOW(B)
                    for (int j = 0; j < result->follow_sets[A_index]->count; j++)
                    {
                        const char *symbol = result->follow_sets[A_index]->symbols[j];
                        if (!contains_symbol(result->follow_sets[B_index], symbol))
                        {
                            add_symbol(result->follow_sets[B_index], symbol);
                            changed = 1;
                        }
                    }
                }
                else
                {
                    // Calcular FIRST(β)
                    SymbolSet *first_beta = compute_first_of_string_for_follow(beta, beta_count, first_result);

                    // FIRST(β) - {ε} ⊆ FOLLOW(B)
                    for (int j = 0; j < first_beta->count; j++)
                    {
                        const char *symbol = first_beta->symbols[j];
                        if (strcmp(symbol, "ε") != 0)
                        {
                            if (!contains_symbol(result->follow_sets[B_index], symbol))
                            {
                                add_symbol(result->follow_sets[B_index], symbol);
                                changed = 1;
                            }
                        }
                    }

                    // Si ε ∈ FIRST(β), entonces FOLLOW(A) ⊆ FOLLOW(B)
                    if (contains_symbol(first_beta, "ε"))
                    {
                        for (int j = 0; j < result->follow_sets[A_index]->count; j++)
                        {
                            const char *symbol = result->follow_sets[A_index]->symbols[j];
                            if (!contains_symbol(result->follow_sets[B_index], symbol))
                            {
                                add_symbol(result->follow_sets[B_index], symbol);
                                changed = 1;
                            }
                        }
                    }

                    free_symbol_set(first_beta);
                }
            }
        }
    }

    if (iteration >= 100)
    {
        printf("⚠️  Advertencia: El cálculo de FOLLOW alcanzó el límite de iteraciones\n");
    }

    return result;
}

void print_follow_sets(FollowResult *result)
{
    printf("=== CONJUNTOS FOLLOW ===\n");
    for (int i = 0; i < result->symbol_count; i++)
    {
        // Solo mostrar FOLLOW para no terminales
        if (!is_terminal(result->symbols[i]))
        {
            printf("FOLLOW(%s) = {", result->symbols[i]);
            for (int j = 0; j < result->follow_sets[i]->count; j++)
            {
                printf("%s", result->follow_sets[i]->symbols[j]);
                if (j < result->follow_sets[i]->count - 1)
                {
                    printf(", ");
                }
            }
            printf("}\n");
        }
    }
}

void free_follow_result(FollowResult *result)
{
    if (result)
    {
        for (int i = 0; i < result->symbol_count; i++)
        {
            free_symbol_set(result->follow_sets[i]);
        }
        free(result->follow_sets);
        free(result);
    }
}
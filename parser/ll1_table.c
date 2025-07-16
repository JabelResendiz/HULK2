#include "ll1_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Calcular FIRST de una cadena de símbolos (para tabla LL(1))
static SymbolSet *compute_first_for_table(char **symbols, int symbol_count, FirstResult *first_result)
{
    SymbolSet *result = create_symbol_set();

    for (int i = 0; i < symbol_count; i++)
    {
        const char *X = symbols[i];

        // Si X es terminal, agregar X a FIRST
        if (is_terminal(X))
        {
            add_symbol(result, X);
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

            // Si ε no está en FIRST(X), terminar
            if (!contains_symbol(first_result->first_sets[X_index], "ε"))
            {
                break;
            }
        }
    }

    // Si todos los símbolos pueden derivar ε, agregar ε
    int all_epsilon = 1;
    for (int i = 0; i < symbol_count; i++)
    {
        const char *X = symbols[i];
        if (is_terminal(X))
        {
            all_epsilon = 0;
            break;
        }
        int X_index = get_symbol_index(X, first_result->symbols, first_result->symbol_count);
        if (X_index == -1 || !contains_symbol(first_result->first_sets[X_index], "ε"))
        {
            all_epsilon = 0;
            break;
        }
    }

    if (all_epsilon)
    {
        add_symbol(result, "ε");
    }

    return result;
}

// Obtener índice de un no terminal en la tabla
static int get_non_terminal_index(LL1Table *table, const char *non_terminal)
{
    for (int i = 0; i < table->no_terminal_count; i++)
    {
        if (strcmp(table->no_terminals[i], non_terminal) == 0)
        {
            return i;
        }
    }
    return -1;
}

// Obtener índice de un terminal en la tabla
static int get_terminal_index(LL1Table *table, const char *terminal)
{
    for (int i = 0; i < table->terminal_count; i++)
    {
        if (strcmp(table->terminals[i], terminal) == 0)
        {
            return i;
        }
    }
    return -1;
}

// Construir tabla LL(1) completa
LL1Table *build_ll1_table(Grammar *grammar, FirstResult *first_result, FollowResult *follow_result)
{
    // Crear tabla usando la función existente
    LL1Table *table = create_ll1_table();

    // Recolectar todos los no terminales y terminales únicos
    SymbolSet *no_terminals = create_symbol_set();
    SymbolSet *terminals = create_symbol_set();

    // Agregar símbolo inicial
    add_symbol(no_terminals, grammar->start_symbol);
    printf("DEBUG: Agregado símbolo inicial: '%s'\n", grammar->start_symbol);

    // Recolectar no terminales y terminales de las producciones
    for (int p = 0; p < grammar->production_count; p++)
    {
        Production *prod = grammar->productions[p];

        // Agregar lado izquierdo (no terminal)
        add_symbol(no_terminals, prod->lhs);
        printf("DEBUG: Agregado no terminal: '%s'\n", prod->lhs);

        // Agregar símbolos del lado derecho
        for (int i = 0; i < prod->rhs_count; i++)
        {
            const char *symbol = prod->rhs[i];
            if (is_terminal(symbol))
            {
                add_symbol(terminals, symbol);
                printf("DEBUG: Agregado terminal: '%s'\n", symbol);
            }
            else
            {
                add_symbol(no_terminals, symbol);
                printf("DEBUG: Agregado no terminal: '%s'\n", symbol);
            }
        }
    }

    // Agregar $ (EOF) como terminal si no está presente
    int has_eof = 0;
    for (int i = 0; i < terminals->count; i++)
    {
        if (strcmp(terminals->symbols[i], "$") == 0)
        {
            has_eof = 1;
            break;
        }
    }

    if (!has_eof)
    {
        // Agregar $ al final de la lista de terminales
        add_symbol(terminals, "$");
    }

    // Configurar la tabla
    table->no_terminal_count = no_terminals->count;
    table->terminal_count = terminals->count;

    // Asignar arrays
    table->no_terminals = malloc(no_terminals->count * sizeof(char *));
    table->terminals = malloc(terminals->count * sizeof(char *));

    // Copiar no terminales
    for (int i = 0; i < no_terminals->count; i++)
    {
        table->no_terminals[i] = strdup(no_terminals->symbols[i]);
        printf("DEBUG: Copiado no terminal[%d]: '%s' -> '%s'\n", i, no_terminals->symbols[i], table->no_terminals[i]);
    }

    // Copiar terminales (incluyendo $ si se agregó)
    for (int i = 0; i < terminals->count; i++)
    {
        table->terminals[i] = strdup(terminals->symbols[i]);
        printf("DEBUG: Copiado terminal[%d]: '%s' -> '%s'\n", i, terminals->symbols[i], table->terminals[i]);
    }

    // Liberar conjuntos temporales
    printf("DEBUG: Antes de liberar conjuntos temporales:\n");
    for (int i = 0; i < table->no_terminal_count; i++)
    {
        printf("DEBUG: Tabla[%d] = '%s'\n", i, table->no_terminals[i]);
    }
    free_symbol_set(no_terminals);
    free_symbol_set(terminals);

    // Crear tabla 2D
    table->table = malloc(table->no_terminal_count * sizeof(char **));
    for (int i = 0; i < table->no_terminal_count; i++)
    {
        table->table[i] = malloc(table->terminal_count * sizeof(char *));
        for (int j = 0; j < table->terminal_count; j++)
        {
            table->table[i][j] = NULL; // Inicializar como vacío
        }
    }

    // PASO 1: Procesar todas las producciones NO-EPSILON primero
    for (int p = 0; p < grammar->production_count; p++)
    {
        Production *prod = grammar->productions[p];
        const char *A = prod->lhs;

        // Solo procesar producciones no-epsilon en este paso
        if (prod->rhs_count == 0)
        {
            continue; // Saltar producciones epsilon
        }

        // Calcular FIRST(α)
        SymbolSet *first_alpha = compute_first_for_table(prod->rhs, prod->rhs_count, first_result);

        // Para cada terminal a en FIRST(α) - {ε}
        for (int i = 0; i < first_alpha->count; i++)
        {
            const char *a = first_alpha->symbols[i];
            if (strcmp(a, "ε") != 0)
            {
                // Agregar M[A, a] = p
                int A_idx = get_non_terminal_index(table, A);
                int a_idx = get_terminal_index(table, a);
                if (A_idx != -1 && a_idx != -1)
                {
                    // Verificar conflictos
                    if (table->table[A_idx][a_idx] != NULL)
                    {
                        printf("Conflicto LL(1): M[%s, %s] ya tiene entrada\n", A, a);
                    }
                    else
                    {
                        // Convertir índice de producción a string
                        char prod_str[20];
                        sprintf(prod_str, "%d", p);
                        table->table[A_idx][a_idx] = strdup(prod_str);
                    }
                }
            }
        }

        free_symbol_set(first_alpha);
    }

    // PASO 2: Procesar producciones EPSILON solo donde no hay conflicto
    for (int p = 0; p < grammar->production_count; p++)
    {
        Production *prod = grammar->productions[p];
        const char *A = prod->lhs;

        // Calcular FIRST(α)
        SymbolSet *first_alpha = compute_first_for_table(prod->rhs, prod->rhs_count, first_result);

        // Si ε está en FIRST(α), para cada terminal b en FOLLOW(A)
        if (contains_symbol(first_alpha, "ε"))
        {
            int A_index = get_symbol_index(A, follow_result->symbols, follow_result->symbol_count);
            if (A_index != -1)
            {
                for (int i = 0; i < follow_result->follow_sets[A_index]->count; i++)
                {
                    const char *b = follow_result->follow_sets[A_index]->symbols[i];

                    // Agregar M[A, b] = p solo si no hay una producción no-epsilon
                    int A_idx = get_non_terminal_index(table, A);
                    int b_idx = get_terminal_index(table, b);
                    if (A_idx != -1 && b_idx != -1)
                    {
                        // Solo agregar epsilon si no hay una producción no-epsilon
                        if (table->table[A_idx][b_idx] == NULL)
                        {
                            char prod_str[20];
                            sprintf(prod_str, "%d", p);
                            table->table[A_idx][b_idx] = strdup(prod_str);
                        }
                        // Si ya existe una entrada, no sobrescribir (prioridad a no-epsilon)
                    }
                }
            }
        }

        free_symbol_set(first_alpha);
    }

    return table;
}

// Buscar entrada en la tabla LL(1)
int find_ll1_entry(LL1Table *table, const char *non_terminal, const char *terminal)
{
    int nt_idx = get_non_terminal_index(table, non_terminal);
    int t_idx = get_terminal_index(table, terminal);

    if (nt_idx != -1 && t_idx != -1 && table->table[nt_idx][t_idx] != NULL)
    {
        return atoi(table->table[nt_idx][t_idx]);
    }
    return -1; // No encontrado
}

// Imprimir tabla LL(1)
void print_ll1_table(LL1Table *table, Grammar *grammar)
{
    printf("=== TABLA LL(1) ===\n");
    printf("Total de entradas: %d\n\n", table->no_terminal_count * table->terminal_count);

    // Imprimir cada entrada de la tabla
    for (int i = 0; i < table->no_terminal_count; i++)
    {
        for (int j = 0; j < table->terminal_count; j++)
        {
            if (table->table[i][j] != NULL)
            {
                // Obtener el índice de la producción
                int prod_index = atoi(table->table[i][j]);
                if (prod_index >= 0 && prod_index < grammar->production_count)
                {
                    Production *prod = grammar->productions[prod_index];

                    // Construir la producción como string
                    char prod_str[200];
                    sprintf(prod_str, "%s -> ", prod->lhs);

                    // Si es una producción epsilon (rhs_count == 0), mostrar "ε"
                    if (prod->rhs_count == 0)
                    {
                        strcat(prod_str, "ε");
                    }
                    else
                    {
                        // Para producciones no-epsilon, mostrar todos los símbolos
                        for (int k = 0; k < prod->rhs_count; k++)
                        {
                            if (k > 0)
                                strcat(prod_str, " ");
                            strcat(prod_str, prod->rhs[k]);
                        }
                    }

                    printf("M[%s, %s] = %s\n",
                           table->no_terminals[i],
                           table->terminals[j],
                           prod_str);
                }
                else
                {
                    printf("M[%s, %s] = %s\n",
                           table->no_terminals[i],
                           table->terminals[j],
                           table->table[i][j]);
                }
            }
        }
    }
    printf("\n");
}
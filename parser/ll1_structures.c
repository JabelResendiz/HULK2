#include "ll1_structures.h"

// ===== IMPLEMENTACIÓN DE CONJUNTOS DE SÍMBOLOS =====

SymbolSet *create_symbol_set()
{
    SymbolSet *set = malloc(sizeof(SymbolSet));
    set->symbols = NULL;
    set->count = 0;
    return set;
}

void add_symbol(SymbolSet *set, const char *symbol)
{
    // Verificar si ya existe
    for (int i = 0; i < set->count; i++)
    {
        if (strcmp(set->symbols[i], symbol) == 0)
        {
            return; // Ya existe
        }
    }

    // Agregar nuevo símbolo
    set->count++;
    set->symbols = realloc(set->symbols, set->count * sizeof(char *));
    set->symbols[set->count - 1] = strdup(symbol);
}

int contains_symbol(SymbolSet *set, const char *symbol)
{
    for (int i = 0; i < set->count; i++)
    {
        if (strcmp(set->symbols[i], symbol) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void free_symbol_set(SymbolSet *set)
{
    if (!set)
        return;
    for (int i = 0; i < set->count; i++)
    {
        free(set->symbols[i]);
    }
    free(set->symbols);
    free(set);
}

// ===== IMPLEMENTACIÓN DE PRODUCCIONES =====

Production *create_production(const char *lhs)
{
    Production *prod = malloc(sizeof(Production));
    prod->lhs = strdup(lhs);
    prod->rhs = NULL;
    prod->rhs_count = 0;
    return prod;
}

void add_rhs_symbol(Production *prod, const char *symbol)
{
    prod->rhs_count++;
    prod->rhs = realloc(prod->rhs, prod->rhs_count * sizeof(char *));
    prod->rhs[prod->rhs_count - 1] = strdup(symbol);
}

void free_production(Production *prod)
{
    if (!prod)
        return;
    free(prod->lhs);
    for (int i = 0; i < prod->rhs_count; i++)
    {
        free(prod->rhs[i]);
    }
    free(prod->rhs);
    free(prod);
}

// ===== IMPLEMENTACIÓN DE GRAMÁTICA =====

Grammar *create_grammar()
{
    Grammar *grammar = malloc(sizeof(Grammar));
    grammar->productions = NULL;
    grammar->production_count = 0;
    grammar->start_symbol = NULL;
    return grammar;
}

void add_production(Grammar *grammar, Production *prod)
{
    grammar->production_count++;
    grammar->productions = realloc(grammar->productions,
                                   grammar->production_count * sizeof(Production *));
    grammar->productions[grammar->production_count - 1] = prod;
}

void free_grammar(Grammar *grammar)
{
    if (!grammar)
        return;
    for (int i = 0; i < grammar->production_count; i++)
    {
        free_production(grammar->productions[i]);
    }
    free(grammar->productions);
    free(grammar->start_symbol);
    free(grammar);
}

// ===== IMPLEMENTACIÓN DE TABLA LL(1) =====

LL1Table *create_ll1_table()
{
    LL1Table *table = malloc(sizeof(LL1Table));
    table->table = NULL;
    table->no_terminals = NULL;
    table->terminals = NULL;
    table->no_terminal_count = 0;
    table->terminal_count = 0;
    return table;
}

void free_ll1_table(LL1Table *table)
{
    if (!table)
        return;

    // Liberar tabla
    if (table->table)
    {
        for (int i = 0; i < table->no_terminal_count; i++)
        {
            for (int j = 0; j < table->terminal_count; j++)
            {
                if (table->table[i][j])
                {
                    free(table->table[i][j]);
                }
            }
            free(table->table[i]);
        }
        free(table->table);
    }

    // Liberar listas
    for (int i = 0; i < table->no_terminal_count; i++)
    {
        free(table->no_terminals[i]);
    }
    free(table->no_terminals);

    for (int i = 0; i < table->terminal_count; i++)
    {
        free(table->terminals[i]);
    }
    free(table->terminals);

    free(table);
}

// ===== IMPLEMENTACIÓN DE ÁRBOL CST =====

CSTNode *create_cst_node(const char *symbol)
{
    CSTNode *node = malloc(sizeof(CSTNode));
    node->symbol = strdup(symbol);
    node->children = NULL;
    node->child_count = 0;
    node->line = 0;
    node->column = 0;
    return node;
}

void add_cst_child(CSTNode *parent, CSTNode *child)
{
    parent->child_count++;
    parent->children = realloc(parent->children,
                               parent->child_count * sizeof(CSTNode *));
    parent->children[parent->child_count - 1] = child;
}

void free_cst_tree(CSTNode *root)
{
    if (!root)
        return;

    // Liberar hijos recursivamente
    for (int i = 0; i < root->child_count; i++)
    {
        free_cst_tree(root->children[i]);
    }

    free(root->children);
    free(root->symbol);
    free(root);
}

// ===== FUNCIONES DE UTILIDAD =====

int is_terminal(const char *symbol)
{
    // Un símbolo es terminal si:
    // 1. Está en mayúsculas (como ID, NUMBER, etc.)
    // 2. Es un símbolo especial (+, -, *, etc.)
    // 3. No es epsilon
    // 4. No es el operador de alternancia |
    if (is_epsilon(symbol))
        return 0;

    if (strcmp(symbol, "|") == 0)
        return 0; // | no es un terminal

    // Verificar si es un símbolo en mayúsculas (terminal)
    for (int i = 0; symbol[i]; i++)
    {
        if (symbol[i] >= 'a' && symbol[i] <= 'z')
        {
            return 0; // Tiene minúsculas, probablemente no terminal
        }
    }
    return 1;
}

int is_non_terminal(const char *symbol)
{
    // Un símbolo es no terminal si:
    // 1. No es terminal
    // 2. No es epsilon
    return !is_terminal(symbol) && !is_epsilon(symbol);
}

int is_epsilon(const char *symbol)
{
    return strcmp(symbol, "ε") == 0 || strcmp(symbol, "epsilon") == 0;
}

// ===== FUNCIONES DE DEBUGGING =====

void print_symbol_set(SymbolSet *set, const char *name)
{
    printf("%s = {", name);
    for (int i = 0; i < set->count; i++)
    {
        printf("%s", set->symbols[i]);
        if (i < set->count - 1)
            printf(", ");
    }
    printf("}\n");
}

void print_production(Production *prod)
{
    printf("%s -> ", prod->lhs);
    if (prod->rhs_count == 0)
    {
        printf("ε");
    }
    else
    {
        for (int i = 0; i < prod->rhs_count; i++)
        {
            printf("%s", prod->rhs[i]);
            if (i < prod->rhs_count - 1)
                printf(" ");
        }
    }
    printf("\n");
}

void print_grammar(Grammar *grammar)
{
    printf("Gramática:\n");
    printf("Símbolo inicial: %s\n", grammar->start_symbol ? grammar->start_symbol : "NO DEFINIDO");
    printf("Producciones:\n");
    for (int i = 0; i < grammar->production_count; i++)
    {
        printf("  %d: ", i + 1);
        print_production(grammar->productions[i]);
    }
}
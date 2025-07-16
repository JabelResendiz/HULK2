#include "grammar_parser.h"
#include <ctype.h>

// ===== IMPLEMENTACIÓN DEL PARSER DE GRAMÁTICA =====

Grammar *parse_grammar_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("❌ Error: No se pudo abrir el archivo %s\n", filename);
        return NULL;
    }

    Grammar *grammar = create_grammar();
    char line[1024];
    int line_number = 0;

    printf("📖 Leyendo gramática desde: %s\n", filename);

    while (fgets(line, sizeof(line), file))
    {
        line_number++;

        // Limpiar la línea
        char *clean_line = clean_string(line);
        if (!clean_line || strlen(clean_line) == 0)
        {
            free(clean_line);
            continue;
        }

        // Verificar si es una línea válida de producción
        if (is_valid_production_line(clean_line))
        {
            int prod_count;
            Production **prods = parse_production_line(clean_line, &prod_count);
            if (prods)
            {
                for (int i = 0; i < prod_count; i++)
                {
                    add_production(grammar, prods[i]);
                    printf("✅ Línea %d (prod %d): ", line_number, i + 1);
                    print_production(prods[i]);
                }
                free(prods); // Solo liberar el array, no las producciones
            }
            else
            {
                printf("❌ Línea %d: Error al parsear producción\n", line_number);
            }
        }

        free(clean_line);
    }

    fclose(file);

    // Establecer símbolo inicial (primera producción)
    if (grammar->production_count > 0)
    {
        grammar->start_symbol = strdup(grammar->productions[0]->lhs);
    }

    printf("✅ Gramática parseada: %d producciones\n", grammar->production_count);
    return grammar;
}

Production **parse_production_line(const char *line, int *count)
{
    // Buscar el separador "->"
    char *arrow = strstr(line, "->");
    if (!arrow)
    {
        *count = 0;
        return NULL;
    }

    // Extraer lado izquierdo
    int lhs_len = arrow - line;
    char *lhs = malloc(lhs_len + 1);
    strncpy(lhs, line, lhs_len);
    lhs[lhs_len] = '\0';
    lhs = clean_string(lhs);

    // Extraer lado derecho
    char *rhs_part = arrow + 2; // Saltar "->"
    rhs_part = clean_string(rhs_part);

    // Dividir por alternancias (|)
    int alt_count;
    char **alternatives = split_alternatives(rhs_part, &alt_count);

    // Crear array de producciones
    Production **productions = malloc(alt_count * sizeof(Production *));
    *count = alt_count;

    for (int i = 0; i < alt_count; i++)
    {
        // Crear producción para esta alternativa
        Production *prod = create_production(lhs);

        // Parsear la alternativa
        char *alt = alternatives[i];
        if (strcmp(alt, "ε") == 0 || strlen(alt) == 0)
        {
            // Producción epsilon - no agregar nada al rhs
        }
        else
        {
            // Dividir en símbolos
            int symbol_count;
            char **symbols = split_symbols(alt, &symbol_count);

            for (int j = 0; j < symbol_count; j++)
            {
                add_rhs_symbol(prod, symbols[j]);
                free(symbols[j]);
            }
            free(symbols);
        }

        productions[i] = prod;
        free(alternatives[i]);
    }

    free(alternatives);
    free(lhs);
    free(rhs_part);

    return productions;
}

char **split_symbols(const char *str, int *count)
{
    char **symbols = NULL;
    *count = 0;

    char *str_copy = strdup(str);
    char *token = strtok(str_copy, " \t\n");

    while (token)
    {
        char *clean_token = clean_string(token);
        if (clean_token && strlen(clean_token) > 0)
        {
            (*count)++;
            symbols = realloc(symbols, *count * sizeof(char *));
            symbols[*count - 1] = strdup(clean_token);
        }
        free(clean_token);
        token = strtok(NULL, " \t\n");
    }

    free(str_copy);
    return symbols;
}

// Nueva función que ignora el símbolo de alternancia |
char **split_symbols_ignore_alternation(const char *str, int *count)
{
    char **symbols = NULL;
    *count = 0;

    char *str_copy = strdup(str);
    char *token = strtok(str_copy, " \t\n");

    while (token)
    {
        char *clean_token = clean_string(token);
        if (clean_token && strlen(clean_token) > 0 && strcmp(clean_token, "|") != 0)
        {
            (*count)++;
            symbols = realloc(symbols, *count * sizeof(char *));
            symbols[*count - 1] = strdup(clean_token);
        }
        free(clean_token);
        token = strtok(NULL, " \t\n");
    }

    free(str_copy);
    return symbols;
}

// Dividir una cadena por el operador de alternancia |
char **split_alternatives(const char *str, int *count)
{
    char **alternatives = NULL;
    *count = 0;

    char *str_copy = strdup(str);
    char *token = strtok(str_copy, "|");

    while (token)
    {
        char *clean_token = clean_string(token);
        if (clean_token && strlen(clean_token) > 0)
        {
            (*count)++;
            alternatives = realloc(alternatives, *count * sizeof(char *));
            alternatives[*count - 1] = strdup(clean_token);
        }
        free(clean_token);
        token = strtok(NULL, "|");
    }

    free(str_copy);
    return alternatives;
}

char *clean_string(const char *str)
{
    if (!str)
        return NULL;

    // Eliminar espacios al inicio y final
    while (*str && isspace(*str))
        str++;

    int len = strlen(str);
    while (len > 0 && isspace(str[len - 1]))
        len--;

    char *result = malloc(len + 1);
    strncpy(result, str, len);
    result[len] = '\0';

    return result;
}

int is_valid_production_line(const char *line)
{
    if (!line || strlen(line) == 0)
        return 0;

    // Verificar que contenga "->"
    if (!strstr(line, "->"))
        return 0;

    // Verificar que no sea solo espacios
    char *clean = clean_string(line);
    int valid = (clean && strlen(clean) > 0);
    free(clean);

    return valid;
}

void print_grammar_info(Grammar *grammar)
{
    printf("\n=== INFORMACIÓN DE LA GRAMÁTICA ===\n");
    printf("Símbolo inicial: %s\n", grammar->start_symbol ? grammar->start_symbol : "NO DEFINIDO");
    printf("Total de producciones: %d\n", grammar->production_count);

    // Contar terminales y no terminales
    SymbolSet *terminals = create_symbol_set();
    SymbolSet *non_terminals = create_symbol_set();

    for (int i = 0; i < grammar->production_count; i++)
    {
        Production *prod = grammar->productions[i];

        // Agregar no terminal del lado izquierdo
        add_symbol(non_terminals, prod->lhs);

        // Agregar símbolos del lado derecho
        for (int j = 0; j < prod->rhs_count; j++)
        {
            if (is_terminal(prod->rhs[j]))
            {
                add_symbol(terminals, prod->rhs[j]);
            }
            else if (!is_epsilon(prod->rhs[j]))
            {
                add_symbol(non_terminals, prod->rhs[j]);
            }
        }
    }

    printf("No terminales: %d\n", non_terminals->count);
    printf("Terminales: %d\n", terminals->count);

    print_symbol_set(non_terminals, "No Terminales");
    print_symbol_set(terminals, "Terminales");

    free_symbol_set(terminals);
    free_symbol_set(non_terminals);
}
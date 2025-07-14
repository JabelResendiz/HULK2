#include "grammar.h"

void ll1_init_grammar(LL1_Grammar *g)
{
    g->num_terminals = 0;
    g->num_nonterminals = 0;
    g->num_productions = 0;
    memset(&g->start_symbol, 0, sizeof(LL1_Symbol));
}

int ll1_find_symbol(const LL1_Grammar *g, const char *name, LL1_SymbolType *type_out)
{
    for (int i = 0; i < g->num_terminals; ++i)
    {
        if (strcmp(g->terminals[i].name, name) == 0)
        {
            if (type_out)
                *type_out = LL1_TERMINAL;
            return i;
        }
    }
    for (int i = 0; i < g->num_nonterminals; ++i)
    {
        if (strcmp(g->nonterminals[i].name, name) == 0)
        {
            if (type_out)
                *type_out = LL1_NONTERMINAL;
            return i;
        }
    }
    if (strcmp(name, "ε") == 0)
    {
        if (type_out)
            *type_out = LL1_EPSILON;
        return -2;
    }
    return -1;
}

void ll1_print_grammar(const LL1_Grammar *g)
{
    printf("No terminales: ");
    for (int i = 0; i < g->num_nonterminals; ++i)
    {
        printf("%s ", g->nonterminals[i].name);
    }
    printf("\nTerminales: ");
    for (int i = 0; i < g->num_terminals; ++i)
    {
        printf("%s ", g->terminals[i].name);
    }
    printf("\nProducciones:\n");
    for (int i = 0; i < g->num_productions; ++i)
    {
        printf("  %s -> ", g->productions[i].left.name);
        for (int j = 0; j < g->productions[i].right_len; ++j)
        {
            printf("%s ", g->productions[i].right[j].name);
        }
        if (g->productions[i].right_len == 0)
            printf("ε");
        printf("\n");
    }
}

// Funciones auxiliares para parsing
char **ll1_split_string(const char *str, const char *delimiter, int *count)
{
    char *temp = strdup(str);
    char **result = malloc(MAX_SYMBOLS * sizeof(char *));
    *count = 0;

    char *token = strtok(temp, delimiter);
    while (token != NULL && *count < MAX_SYMBOLS)
    {
        result[*count] = strdup(token);
        (*count)++;
        token = strtok(NULL, delimiter);
    }

    free(temp);
    return result;
}

void ll1_free_string_array(char **arr, int count)
{
    for (int i = 0; i < count; i++)
    {
        free(arr[i]);
    }
    free(arr);
}

void ll1_parse_terminals_line(LL1_Grammar *g, const char *line)
{
    // Buscar después de "Terminals:"
    const char *start = strstr(line, "Terminals:");
    if (!start)
        return;
    start += 10; // Saltar "Terminals:"

    int count;
    char **tokens = ll1_split_string(start, ",", &count);

    for (int i = 0; i < count; i++)
    {
        // Limpiar espacios
        char *token = tokens[i];
        while (*token == ' ')
            token++;
        char *end = token + strlen(token) - 1;
        while (end > token && *end == ' ')
            end--;
        *(end + 1) = '\0';

        if (strlen(token) > 0)
        {
            // Agregar terminal directamente
            LL1_Symbol s;
            strncpy(s.name, token, MAX_NAME);
            s.type = LL1_TERMINAL;
            g->terminals[g->num_terminals] = s;
            g->num_terminals++;
        }
    }

    ll1_free_string_array(tokens, count);
}

void ll1_parse_nonterminals_line(LL1_Grammar *g, const char *line)
{
    // Buscar después de "NonTerminals:"
    const char *start = strstr(line, "NonTerminals:");
    if (!start)
        return;
    start += 13; // Saltar "NonTerminals:"

    int count;
    char **tokens = ll1_split_string(start, ",", &count);

    for (int i = 0; i < count; i++)
    {
        // Limpiar espacios
        char *token = tokens[i];
        while (*token == ' ')
            token++;
        char *end = token + strlen(token) - 1;
        while (end > token && *end == ' ')
            end--;
        *(end + 1) = '\0';

        if (strlen(token) > 0)
        {
            // Agregar no terminal directamente
            LL1_Symbol s;
            strncpy(s.name, token, MAX_NAME);
            s.type = LL1_NONTERMINAL;
            g->nonterminals[g->num_nonterminals] = s;
            g->num_nonterminals++;
        }
    }

    ll1_free_string_array(tokens, count);
}

void ll1_parse_productions_line(LL1_Grammar *g, const char *line)
{
    // Buscar "->"
    const char *arrow = strstr(line, "->");
    if (!arrow)
        return;

    // Extraer lado izquierdo
    char left[MAX_NAME];
    int left_len = arrow - line;
    strncpy(left, line, left_len);
    left[left_len] = '\0';

    // Limpiar espacios del lado izquierdo
    while (*left == ' ')
        memmove(left, left + 1, strlen(left));
    char *left_end = left + strlen(left) - 1;
    while (left_end > left && *left_end == ' ')
        left_end--;
    *(left_end + 1) = '\0';

    // Extraer lado derecho
    const char *right_start = arrow + 2;
    while (*right_start == ' ')
        right_start++;

    // Dividir por "|" para alternativas
    int alt_count;
    char **alternatives = ll1_split_string(right_start, "|", &alt_count);

    for (int i = 0; i < alt_count; i++)
    {
        char *alt = alternatives[i];
        // Limpiar espacios
        while (*alt == ' ')
            alt++;
        char *alt_end = alt + strlen(alt) - 1;
        while (alt_end > alt && *alt_end == ' ')
            alt_end--;
        *(alt_end + 1) = '\0';

        // Crear producción
        LL1_Production *p = &g->productions[g->num_productions];
        strncpy(p->left.name, left, MAX_NAME);
        p->left.type = LL1_NONTERMINAL;

        if (strcmp(alt, "ε") == 0)
        {
            // Producción vacía
            p->right_len = 0;
        }
        else
        {
            // Dividir por espacios
            int symbol_count;
            char **symbols = ll1_split_string(alt, " ", &symbol_count);
            p->right_len = symbol_count;

            for (int j = 0; j < symbol_count; j++)
            {
                LL1_SymbolType type;
                ll1_find_symbol(g, symbols[j], &type);
                strncpy(p->right[j].name, symbols[j], MAX_NAME);
                p->right[j].type = type;
            }

            ll1_free_string_array(symbols, symbol_count);
        }

        g->num_productions++;
    }

    ll1_free_string_array(alternatives, alt_count);
}

bool ll1_load_grammar_from_file(LL1_Grammar *g, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Error: No se pudo abrir el archivo %s\n", filename);
        return false;
    }

    ll1_init_grammar(g);
    char line[MAX_LINE];
    bool in_productions = false;

    while (fgets(line, sizeof(line), file))
    {
        // Remover salto de línea
        line[strcspn(line, "\n")] = 0;

        // Saltar líneas vacías
        if (strlen(line) == 0)
            continue;

        if (strstr(line, "Terminals:") == line)
        {
            ll1_parse_terminals_line(g, line);
        }
        else if (strstr(line, "NonTerminals:") == line)
        {
            ll1_parse_nonterminals_line(g, line);
        }
        else if (strstr(line, "Start:") == line)
        {
            // Extraer símbolo inicial
            const char *start = line + 6;
            while (*start == ' ')
                start++;
            strncpy(g->start_symbol.name, start, MAX_NAME);
            g->start_symbol.type = LL1_NONTERMINAL;
        }
        else if (strstr(line, "Productions:") == line)
        {
            in_productions = true;
        }
        else if (in_productions && strstr(line, "->") != NULL)
        {
            ll1_parse_productions_line(g, line);
        }
    }

    fclose(file);
    return true;
}
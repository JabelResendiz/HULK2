#include "ll1_parser.h"
#include "ll1_structures.h"
#include "../lexer/error_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== ESTRUCTURA DE STACK PARA PARSING ITERATIVO =====

typedef struct StackNode
{
    CSTNode *node;
    struct StackNode *next;
} StackNode;

typedef struct
{
    StackNode *top;
} ParseStack;

// Crear stack vacío
ParseStack *create_parse_stack()
{
    ParseStack *stack = malloc(sizeof(ParseStack));
    stack->top = NULL;
    return stack;
}

// Push un nodo al stack
void push_stack(ParseStack *stack, CSTNode *node)
{
    StackNode *new_node = malloc(sizeof(StackNode));
    new_node->node = node;
    new_node->next = stack->top;
    stack->top = new_node;
}

// Pop un nodo del stack
CSTNode *pop_stack(ParseStack *stack)
{
    if (!stack->top)
        return NULL;

    StackNode *top_node = stack->top;
    CSTNode *node = top_node->node;
    stack->top = top_node->next;
    free(top_node);
    return node;
}

// Verificar si el stack está vacío
int is_stack_empty(ParseStack *stack)
{
    return stack->top == NULL;
}

// Liberar stack
void free_parse_stack(ParseStack *stack)
{
    while (!is_stack_empty(stack))
    {
        pop_stack(stack);
    }
    free(stack);
}

// ===== FUNCIONES DEL PARSER =====

// Crear parser LL(1)
LL1Parser *create_ll1_parser(Grammar *grammar, LL1Table *table)
{
    LL1Parser *parser = malloc(sizeof(LL1Parser));
    parser->tokens = NULL;
    parser->token_count = 0;
    parser->current_token = 0;
    parser->grammar = grammar;
    parser->table = table;
    parser->root = NULL;
    parser->debug_mode = 1;
    parser->last_error = NULL;
    return parser;
}

// Liberar memoria del parser
void free_ll1_parser(LL1Parser *parser)
{
    if (!parser)
        return;

    // Liberar tokens
    for (int i = 0; i < parser->token_count; i++)
    {
        free_token(parser->tokens[i]);
    }
    free(parser->tokens);

    // Liberar árbol CST
    if (parser->root)
    {
        free_cst_tree(parser->root);
    }

    // Liberar último error
    if (parser->last_error)
    {
        destroy_parser_error(parser->last_error);
    }

    free(parser);
}

// Agregar token al parser
void add_token(LL1Parser *parser, ParserToken *token)
{
    parser->tokens = realloc(parser->tokens, (parser->token_count + 1) * sizeof(ParserToken *));
    parser->tokens[parser->token_count] = token;
    parser->token_count++;
}

// Crear token
ParserToken *create_parser_token(const char *type, const char *value, int line, int column)
{
    ParserToken *token = malloc(sizeof(ParserToken));
    token->type = strdup(type);
    token->value = strdup(value);
    token->line = line;
    token->column = column;
    return token;
}

// Liberar token
void free_token(ParserToken *token)
{
    if (!token)
        return;
    free(token->type);
    free(token->value);
    free(token);
}

// Imprimir token
void print_token(ParserToken *token)
{
    if (!token)
    {
        printf("Token: NULL\n");
        return;
    }
    printf("Token: %s = '%s' (línea %d, col %d)\n",
           token->type, token->value, token->line, token->column);
}

// Obtener token actual
ParserToken *get_current_token(LL1Parser *parser)
{
    if (parser->current_token >= parser->token_count)
        return NULL;
    return parser->tokens[parser->current_token];
}

// Avanzar al siguiente token
void advance_token(LL1Parser *parser)
{
    parser->current_token++;
}

// Imprimir información de debug
void print_debug_info(LL1Parser *parser, const char *message)
{
    if (!parser->debug_mode)
        return;

    ParserToken *current = get_current_token(parser);
    if (current)
    {
        printf("🔍 %s (Token: %s = '%s', Pos: %d/%d)\n",
               message, current->type, current->value,
               parser->current_token, parser->token_count);
    }
    else
    {
        printf("🔍 %s (EOF)\n", message);
    }
}

// ===== PARSER LL(1) ITERATIVO =====

CSTNode *parse_iterative(LL1Parser *parser)
{
    if (!parser || !parser->grammar || !parser->table)
    {
        if (parser && parser->last_error) {
            destroy_parser_error(parser->last_error);
        }
        parser->last_error = create_parser_error(
            ERROR_SYNTAX_ERROR,
            0, 0,
            "Parser, gramática o tabla no válidos",
            NULL, NULL
        );
        return NULL;
    }

    if (parser->token_count == 0)
    {
        if (parser && parser->last_error) {
            destroy_parser_error(parser->last_error);
        }
        parser->last_error = create_parser_error(
            ERROR_SYNTAX_ERROR,
            0, 0,
            "No hay tokens para procesar",
            NULL, NULL
        );
        return NULL;
    }

    print_debug_info(parser, "Iniciando parsing iterativo");

    // Crear nodo raíz
    CSTNode *root = create_cst_node(parser->grammar->start_symbol);
    parser->root = root;

    // Crear stack para parsing predictivo
    ParseStack *stack = create_parse_stack();

    // Inicializar stack con el símbolo inicial
    push_stack(stack, root);

    int token_index = 0;

    while (!is_stack_empty(stack))
    {
        CSTNode *current_node = pop_stack(stack);
        char *symbol = current_node->symbol;

        // Obtener token actual
        ParserToken *current_token = NULL;
        int created_eof_token = 0;
        if (token_index < parser->token_count)
        {
            current_token = parser->tokens[token_index];
        }
        else
        {
            // Crear token EOF si no hay más tokens
            current_token = create_parser_token("EOF", "", 0, 0);
            created_eof_token = 1;
        }

        char *current_terminal = current_token->type;
        if (strcmp(current_terminal, "EOF") == 0)
        {
            current_terminal = "$";
        }

        print_debug_info(parser, "Procesando símbolo");
        printf("  📊 Símbolo: %s, Terminal: %s\n", symbol, current_terminal);
        printf("  🔍 Token actual: %s = '%s' (línea %d, col %d)\n", 
               current_token->type, current_token->value, current_token->line, current_token->column);

        if (is_terminal(symbol))
        {
            // Es un terminal
            if (strcmp(symbol, "ε") == 0)
            {
                // Producción epsilon - no hacer nada
                printf("  ✅ Producción ε aplicada\n");
            }
            else if (strcmp(symbol, "$") == 0)
            {
                // Es el símbolo de fin de entrada - manejar de forma especial
                if (strcmp(current_terminal, "$") == 0)
                {
                    printf("  ✅ Match exitoso con EOF\n");
                    token_index++; // Avanzar al siguiente token
                }
                else
                {
                    if (parser->last_error) {
                        destroy_parser_error(parser->last_error);
                    }
                    parser->last_error = create_parser_error(
                        ERROR_UNEXPECTED_TOKEN,
                        current_token ? current_token->line : 0,
                        current_token ? current_token->column : 0,
                        "Se esperaba fin de archivo",
                        "$", current_terminal
                    );
                    free_parse_stack(stack);
                    return NULL;
                }
            }
            else if (strcmp(symbol, current_terminal) == 0)
            {
                // Match exitoso - asignar token y avanzar
                current_node->line = current_token->line;
                current_node->column = current_token->column;

                // Asignar el token al nodo CST
                set_cst_token(current_node, current_token);

                token_index++;
                printf("  ✅ Match exitoso: %s\n", symbol);
            }
            else
            {
                // Error de sintaxis
                if (parser->last_error) {
                    destroy_parser_error(parser->last_error);
                }
                parser->last_error = create_parser_error(
                    ERROR_UNEXPECTED_TOKEN,
                    current_token ? current_token->line : 0,
                    current_token ? current_token->column : 0,
                    "Token inesperado",
                    symbol, current_terminal
                );
                free_parse_stack(stack);
                return NULL;
            }
        }
        else
        {
            // Es un no terminal - consultar la tabla LL(1) O(1)
            // PERO si el terminal es $, no buscar en la tabla para símbolos que pueden derivar a ε
            if (strcmp(current_terminal, "$") == 0)
            {
                // Para el terminal $, solo permitir símbolos que pueden derivar a ε
                // o que tengan una entrada explícita en la tabla
                int symbol_index = get_symbol_index(symbol, parser->table->no_terminals, parser->table->no_terminal_count);
                int terminal_index = get_symbol_index(current_terminal, parser->table->terminals, parser->table->terminal_count);

                if (symbol_index == -1)
                {
                    if (parser->last_error) {
                        destroy_parser_error(parser->last_error);
                    }
                    parser->last_error = create_parser_error(
                        ERROR_INVALID_PRODUCTION,
                        current_token ? current_token->line : 0,
                        current_token ? current_token->column : 0,
                        "No terminal no encontrado en la tabla",
                        symbol, NULL
                    );
                    free_parse_stack(stack);
                    return NULL;
                }

                if (terminal_index == -1)
                {
                    if (parser->last_error) {
                        destroy_parser_error(parser->last_error);
                    }
                    parser->last_error = create_parser_error(
                        ERROR_INVALID_PRODUCTION,
                        current_token ? current_token->line : 0,
                        current_token ? current_token->column : 0,
                        "Terminal '$' no encontrado en la tabla",
                        "$", NULL
                    );
                    free_parse_stack(stack);
                    return NULL;
                }

                char *production_index_str = parser->table->table[symbol_index][terminal_index];
                if (!production_index_str)
                {
                    // Si no hay entrada en la tabla para $, verificar si el símbolo puede derivar a ε
                    // Buscar en los conjuntos FIRST si contiene ε
                    int first_index = -1;
                    for (int i = 0; i < parser->grammar->production_count; i++)
                    {
                        if (strcmp(parser->grammar->productions[i]->lhs, symbol) == 0 && 
                            parser->grammar->productions[i]->rhs_count == 0)
                        {
                            // Encontramos una producción epsilon para este símbolo
                            printf("  🔧 Aplicando producción epsilon para $: %s -> ε\n", symbol);
                            // Crear nodo epsilon
                            CSTNode *epsilon_node = create_cst_node("ε");
                            add_cst_child(current_node, epsilon_node);
                            // Avanzar el token $ después de aplicar epsilon
                            token_index++;
                            continue;
                        }
                    }
                    
                    // Si no se encontró producción epsilon, es un error
                    if (parser->last_error) {
                        destroy_parser_error(parser->last_error);
                    }
                    parser->last_error = create_parser_error(
                        ERROR_UNEXPECTED_TOKEN,
                        current_token ? current_token->line : 0,
                        current_token ? current_token->column : 0,
                        "Token inesperado al final del archivo",
                        symbol, current_terminal
                    );
                    free_parse_stack(stack);
                    return NULL;
                }
            }

            int symbol_index = get_symbol_index(symbol, parser->table->no_terminals, parser->table->no_terminal_count);
            int terminal_index = get_symbol_index(current_terminal, parser->table->terminals, parser->table->terminal_count);

            // DEBUG: Imprimir símbolos antes de buscar
            if (strcmp(symbol, "FunctionDef") == 0 || strcmp(symbol, "StmtList") == 0)
            {
                printf("DEBUG: Buscando %s en la tabla...\n", symbol);
                printf("DEBUG: Total no terminales en tabla: %d\n", parser->table->no_terminal_count);
                for (int i = 0; i < parser->table->no_terminal_count; i++)
                {
                    printf("DEBUG: Tabla[%d] = '%s'\n", i, parser->table->no_terminals[i]);
                }
                
                // Buscar la entrada específica en la tabla
                int symbol_idx = get_symbol_index(symbol, parser->table->no_terminals, parser->table->no_terminal_count);
                int terminal_idx = get_symbol_index(current_terminal, parser->table->terminals, parser->table->terminal_count);
                if (symbol_idx != -1 && terminal_idx != -1)
                {
                    printf("DEBUG: Buscando M[%s, %s] = tabla[%d][%d]\n", symbol, current_terminal, symbol_idx, terminal_idx);
                    if (parser->table->table[symbol_idx][terminal_idx])
                    {
                        printf("DEBUG: Encontrado: %s\n", parser->table->table[symbol_idx][terminal_idx]);
                    }
                    else
                    {
                        printf("DEBUG: NO ENCONTRADO (NULL)\n");
                    }
                }
            }

            if (symbol_index == -1)
            {
                if (parser->last_error) {
                    destroy_parser_error(parser->last_error);
                }
                parser->last_error = create_parser_error(
                    ERROR_INVALID_PRODUCTION,
                    current_token ? current_token->line : 0,
                    current_token ? current_token->column : 0,
                    "No terminal no encontrado en la tabla",
                    symbol, NULL
                );
                free_parse_stack(stack);
                return NULL;
            }

            if (terminal_index == -1)
            {
                if (parser->last_error) {
                    destroy_parser_error(parser->last_error);
                }
                parser->last_error = create_parser_error(
                    ERROR_INVALID_PRODUCTION,
                    current_token ? current_token->line : 0,
                    current_token ? current_token->column : 0,
                    "Terminal no encontrado en la tabla",
                    current_terminal, NULL
                );
                free_parse_stack(stack);
                return NULL;
            }

            char *production_index_str = parser->table->table[symbol_index][terminal_index];
            if (!production_index_str)
            {
                if (parser->last_error) {
                    destroy_parser_error(parser->last_error);
                }
                parser->last_error = create_parser_error(
                    ERROR_INVALID_PRODUCTION,
                    current_token ? current_token->line : 0,
                    current_token ? current_token->column : 0,
                    "No hay producción válida para la combinación de símbolos",
                    symbol, current_terminal
                );
                free_parse_stack(stack);
                return NULL;
            }

            int production_index = atoi(production_index_str);
            if (production_index < 0 || production_index >= parser->grammar->production_count)
            {
                if (parser->last_error) {
                    destroy_parser_error(parser->last_error);
                }
                parser->last_error = create_parser_error(
                    ERROR_INVALID_PRODUCTION,
                    current_token ? current_token->line : 0,
                    current_token ? current_token->column : 0,
                    "Índice de producción inválido",
                    NULL, NULL
                );
                free_parse_stack(stack);
                return NULL;
            }

            Production *prod = parser->grammar->productions[production_index];

            printf("  🔧 Aplicando producción: %s -> ", prod->lhs);
            if (prod->rhs_count == 0)
            {
                printf("ε\n");
            }
            else
            {
                for (int i = 0; i < prod->rhs_count; i++)
                {
                    printf("%s ", prod->rhs[i]);
                }
                printf("\n");
            }

            // Crear nodos hijos para cada símbolo de la producción
            if (prod->rhs_count == 0)
            {
                // Producción epsilon
                CSTNode *epsilon_node = create_cst_node("ε");
                add_cst_child(current_node, epsilon_node);
            }
            else
            {
                // Crear nodos hijos y agregarlos al stack en orden inverso
                CSTNode **child_nodes = malloc(prod->rhs_count * sizeof(CSTNode *));

                for (int i = 0; i < prod->rhs_count; i++)
                {
                    child_nodes[i] = create_cst_node(prod->rhs[i]);
                    add_cst_child(current_node, child_nodes[i]);
                }

                // Push los nodos hijos en orden inverso al stack
                for (int i = prod->rhs_count - 1; i >= 0; i--)
                {
                    push_stack(stack, child_nodes[i]);
                }

                free(child_nodes);
            }
        }

        // Liberar token EOF temporal si se creó
        if (created_eof_token)
        {
            free_token(current_token);
        }
    }

    // Verificar que se procesaron todos los tokens (solo puede quedar $ sin consumir)
    if (token_index < parser->token_count)
    {
        // Permitir que solo quede $ (EOF) sin consumir
        if (token_index == parser->token_count - 1 && strcmp(parser->tokens[token_index]->type, "$") == 0)
        {
            // OK: solo queda EOF
        }
        else
        {
            if (parser->last_error) {
                destroy_parser_error(parser->last_error);
            }
            parser->last_error = create_parser_error(
                ERROR_UNEXPECTED_TOKEN,
                parser->tokens[token_index] ? parser->tokens[token_index]->line : 0,
                parser->tokens[token_index] ? parser->tokens[token_index]->column : 0,
                "Tokens restantes sin procesar",
                NULL, parser->tokens[token_index] ? parser->tokens[token_index]->type : NULL
            );
            free_parse_stack(stack);
            return NULL;
        }
    }

    printf("✅ Parsing LL(1) completado exitosamente\n");
    free_parse_stack(stack);
    return root;
}

// Función wrapper para parsear
CSTNode *parse_ll1(LL1Parser *parser)
{
    return parse_iterative(parser);
}

// ===== FUNCIONES DE UTILIDAD =====

// Imprimir árbol CST
void print_cst_tree(CSTNode *root, int depth)
{
    if (!root)
        return;

    // Imprimir indentación
    for (int i = 0; i < depth; i++)
    {
        printf("  ");
    }

    // Imprimir nodo
    printf("├─ %s", root->symbol);
    if (root->line > 0)
    {
        printf(" (línea %d, col %d)", root->line, root->column);
    }
    printf("\n");

    // Imprimir hijos
    for (int i = 0; i < root->child_count; i++)
    {
        print_cst_tree(root->children[i], depth + 1);
    }
}

// Funciones de manejo de errores del parser
ParserError *parser_get_last_error(LL1Parser *parser)
{
    return parser ? parser->last_error : NULL;
}

void parser_clear_error(LL1Parser *parser)
{
    if (!parser) return;
    
    if (parser->last_error) {
        destroy_parser_error(parser->last_error);
        parser->last_error = NULL;
    }
}

void parser_print_error(LL1Parser *parser, const char *source_code)
{
    if (!parser || !parser->last_error) return;
    
    print_parser_error(parser->last_error, source_code);
}
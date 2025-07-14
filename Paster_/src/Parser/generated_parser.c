#include "parser_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estructura para la pila del parser
typedef struct ParseStack {
    GrammarSymbol* symbol;
    struct ParseStack* next;
} ParseStack;

typedef struct GeneratedParserState {
    GeneratedParser* parser;
    ParseStack* stack;
    ASTNode** ast_stack;
    int ast_stack_size;
    int ast_stack_capacity;
} GeneratedParserState;

// Funciones auxiliares para la pila
ParseStack* stack_push(ParseStack* stack, GrammarSymbol* symbol) {
    ParseStack* new_node = malloc(sizeof(ParseStack));
    if (!new_node) return stack;
    
    new_node->symbol = symbol;
    new_node->next = stack;
    return new_node;
}

ParseStack* stack_pop(ParseStack* stack) {
    if (!stack) return NULL;
    
    ParseStack* next = stack->next;
    free(stack);
    return next;
}

void stack_destroy(ParseStack* stack) {
    while (stack) {
        ParseStack* next = stack->next;
        free(stack);
        stack = next;
    }
}

// Funciones auxiliares para la pila AST
void ast_stack_push(GeneratedParserState* state, ASTNode* node) {
    if (state->ast_stack_size >= state->ast_stack_capacity) {
        state->ast_stack_capacity = state->ast_stack_capacity * 2 + 1;
        state->ast_stack = realloc(state->ast_stack, 
                                  sizeof(ASTNode*) * state->ast_stack_capacity);
    }
    state->ast_stack[state->ast_stack_size++] = node;
}

ASTNode* ast_stack_pop(GeneratedParserState* state) {
    if (state->ast_stack_size <= 0) return NULL;
    return state->ast_stack[--state->ast_stack_size];
}

// Función para encontrar un símbolo en la tabla
GrammarSymbol* find_symbol_by_name(Grammar* grammar, const char* name) {
    GrammarSymbol* symbol = grammar->symbols;
    while (symbol) {
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
        symbol = symbol->next;
    }
    return NULL;
}

// Función para encontrar un símbolo terminal por tipo de token
GrammarSymbol* find_terminal_by_token_type(ParsingTable* table, int token_type) {
    for (int i = 0; i < table->terminal_count; i++) {
        if (table->terminals[i]->token_type == token_type) {
            return table->terminals[i];
        }
    }
    return NULL;
}

// Función para encontrar un símbolo no terminal por nombre
GrammarSymbol* find_nonterminal_by_name(ParsingTable* table, const char* name) {
    for (int i = 0; i < table->nonterminal_count; i++) {
        if (strcmp(table->nonterminals[i]->name, name) == 0) {
            return table->nonterminals[i];
        }
    }
    return NULL;
}

// Función para obtener el índice de un símbolo en la tabla
int get_symbol_index(ParsingTable* table, Symbol* symbol) {
    if (symbol->type == SYMBOL_TERMINAL) {
        for (int i = 0; i < table->terminal_count; i++) {
            if (table->terminals[i] == symbol) {
                return i;
            }
        }
    } else {
        for (int i = 0; i < table->nonterminal_count; i++) {
            if (table->nonterminals[i] == symbol) {
                return i;
            }
        }
    }
    return -1;
}

// Función para crear nodos AST basados en el tipo de símbolo
ASTNode* create_ast_node_from_symbol(GrammarSymbol* symbol, Token* token) {
    if (symbol->type == GRAMMAR_SYMBOL_TERMINAL) {
        switch (symbol->token_type) {
            case NUMBER:
                return create_num_node(atof(token->lexeme));
            case STRING:
                return create_string_node(token->lexeme);
            case IDENT:
                return create_var_node(token->lexeme, NULL, 0);
            case TRUE:
                return create_boolean_node("true");
            case FALSE:
                return create_boolean_node("false");
            default:
                return NULL;
        }
    }
    return NULL;
}

// Función para aplicar una producción y crear nodos AST
void apply_production(GeneratedParserState* state, Production* production) {
    printf("DEBUG: Aplicando producción %s ::= ", production->left_side->name);
    for (int i = 0; i < production->right_side_count; i++) {
        printf("%s ", production->right_side[i]->name);
    }
    printf("\n");
    
    // Crear nodo AST basado en el tipo de producción
    ASTNode* ast_node = NULL;
    
    if (strcmp(production->left_side->name, "program") == 0) {
        // Crear nodo de programa
        ASTNode** statements = malloc(sizeof(ASTNode*) * state->ast_stack_size);
        int stmt_count = 0;
        
        // Recoger statements de la pila AST
        while (state->ast_stack_size > 0) {
            statements[stmt_count++] = ast_stack_pop(state);
        }
        
        ast_node = create_program_node(statements, stmt_count, AST_PROGRAM);
    } else if (strcmp(production->left_side->name, "expression") == 0) {
        // Para expresiones, tomar el último nodo de la pila
        if (state->ast_stack_size > 0) {
            ast_node = ast_stack_pop(state);
        }
    } else if (strcmp(production->left_side->name, "statement") == 0) {
        // Para statements, tomar el último nodo de la pila
        if (state->ast_stack_size > 0) {
            ast_node = ast_stack_pop(state);
        }
    }
    
    if (ast_node) {
        ast_stack_push(state, ast_node);
    }
}

// Función principal de parsing usando tabla LL(1)
ASTNode* generated_parser_parse_improved(GeneratedParser* parser) {
    if (!parser || !parser->tokens || parser->token_count == 0) {
        return NULL;
    }
    
    GeneratedParserState state;
    state.parser = parser;
    state.stack = NULL;
    state.ast_stack = NULL;
    state.ast_stack_size = 0;
    state.ast_stack_capacity = 10;
    
    // Inicializar pila AST
    state.ast_stack = malloc(sizeof(ASTNode*) * state.ast_stack_capacity);
    
    // Inicializar pila con símbolo inicial y EOF
    GrammarSymbol* start_symbol = parser->grammar->start_symbol;
    GrammarSymbol* eof_symbol = find_terminal_by_token_type(parser->table, EOF);
    
    if (!start_symbol || !eof_symbol) {
        fprintf(stderr, "Error: Símbolos iniciales no encontrados\n");
        return NULL;
    }
    
    state.stack = stack_push(state.stack, eof_symbol);
    state.stack = stack_push(state.stack, start_symbol);
    
    printf("DEBUG: Iniciando parsing con tabla generada\n");
    printf("DEBUG: Símbolo inicial: %s\n", start_symbol->name);
    
    while (state.stack && parser->current_token < parser->token_count) {
        Token* current_token = &parser->tokens[parser->current_token];
        ParseStack* top = state.stack;
        
        if (!top) {
            fprintf(stderr, "Error: Pila vacía\n");
            break;
        }
        
        printf("DEBUG: Token actual: %s, Top de pila: %s\n", 
               current_token->lexeme ? current_token->lexeme : get_token_name(current_token->type),
               top->symbol->name);
        
        if (top->symbol->type == GRAMMAR_SYMBOL_TERMINAL) {
            // Símbolo terminal - verificar coincidencia
            if (top->symbol->token_type == current_token->type) {
                printf("DEBUG: Coincidencia de terminal\n");
                
                // Crear nodo AST para el terminal
                ASTNode* terminal_node = create_ast_node_from_symbol(top->symbol, current_token);
                if (terminal_node) {
                    ast_stack_push(&state, terminal_node);
                }
                
                // Consumir token y símbolo
                state.stack = stack_pop(state.stack);
                parser->current_token++;
            } else {
                fprintf(stderr, "Error: Token inesperado. Esperado: %s, Encontrado: %s\n",
                       top->symbol->name, 
                       current_token->lexeme ? current_token->lexeme : get_token_name(current_token->type));
                
                if (parser->error_context) {
                    // Reportar error
                    parser_error_unexpected_token(parser, top->symbol->token_type, current_token->type);
                }
                
                // Recuperación de error: saltar token
                parser->current_token++;
            }
        } else {
            // Símbolo no terminal - usar tabla de parsing
            GrammarSymbol* terminal_symbol = find_terminal_by_token_type(parser->table, current_token->type);
            if (!terminal_symbol) {
                fprintf(stderr, "Error: Token no reconocido: %s\n", 
                       current_token->lexeme ? current_token->lexeme : get_token_name(current_token->type));
                parser->current_token++;
                continue;
            }
            
            int nonterm_idx = -1, term_idx = -1;
            
            // Encontrar índices en la tabla
            for (int i = 0; i < parser->table->nonterminal_count; i++) {
                if (parser->table->nonterminals[i] == top->symbol) {
                    nonterm_idx = i;
                    break;
                }
            }
            
            for (int i = 0; i < parser->table->terminal_count; i++) {
                if (parser->table->terminals[i] == terminal_symbol) {
                    term_idx = i;
                    break;
                }
            }
            
            if (nonterm_idx != -1 && term_idx != -1) {
                int production_index = parser->table->table[nonterm_idx][term_idx];
                
                if (production_index != -1) {
                    printf("DEBUG: Usando producción %d\n", production_index);
                    
                    // Encontrar la producción
                    Production* production = parser->grammar->productions;
                    for (int i = 0; i < production_index && production; i++) {
                        production = production->next;
                    }
                    
                    if (production) {
                        // Aplicar la producción
                        state.stack = stack_pop(state.stack); // Remover no terminal
                        
                        // Agregar símbolos del lado derecho en orden inverso
                        for (int i = production->right_side_count - 1; i >= 0; i--) {
                            state.stack = stack_push(state.stack, production->right_side[i]);
                        }
                        
                        // Aplicar acción semántica si existe
                        if (production->action) {
                            apply_production(&state, production);
                        }
                    }
                } else {
                    fprintf(stderr, "Error: No hay producción para %s con token %s\n",
                           top->symbol->name, 
                           current_token->lexeme ? current_token->lexeme : get_token_name(current_token->type));
                    
                    if (parser->error_context) {
                        parser_error_invalid_expression(parser, top->symbol->name);
                    }
                    
                    // Recuperación de error: remover no terminal
                    state.stack = stack_pop(state.stack);
                }
            } else {
                fprintf(stderr, "Error: Índices no encontrados en tabla de parsing\n");
                break;
            }
        }
    }
    
    // Verificar si el parsing fue exitoso
    if (parser->current_token >= parser->token_count && state.ast_stack_size > 0) {
        printf("DEBUG: Parsing completado exitosamente\n");
        ASTNode* result = ast_stack_pop(&state);
        
        // Limpiar memoria
        stack_destroy(state.stack);
        free(state.ast_stack);
        
        return result;
    } else {
        fprintf(stderr, "Error: Parsing incompleto o fallido\n");
        
        // Limpiar memoria
        stack_destroy(state.stack);
        free(state.ast_stack);
        
        return NULL;
    }
}

// Función wrapper para mantener compatibilidad
ASTNode* generated_parser_parse(GeneratedParser* parser) {
    return generated_parser_parse_improved(parser);
} 
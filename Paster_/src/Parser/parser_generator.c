#define _GNU_SOURCE
#include "parser_generator.h"
#include <ctype.h>
#include <string.h>
#include <strings.h>

// Mapeo de nombres de tokens a tipos
static int token_name_to_type(const char* name) {
    if (strcmp(name, "FUNCTION") == 0) return FUNCTION;
    if (strcmp(name, "LET") == 0) return LET;
    if (strcmp(name, "IN") == 0) return IN;
    if (strcmp(name, "IF") == 0) return IF;
    if (strcmp(name, "ELSE") == 0) return ELSE;
    if (strcmp(name, "WHILE") == 0) return WHILE;
    if (strcmp(name, "FOR") == 0) return FOR;
    if (strcmp(name, "RETURN") == 0) return RETURN;
    if (strcmp(name, "TYPE") == 0) return TYPE;
    if (strcmp(name, "IDENT") == 0) return IDENT;
    if (strcmp(name, "NUMBER") == 0) return NUMBER;
    if (strcmp(name, "STRING") == 0) return STRING;
    if (strcmp(name, "TRUE") == 0) return TRUE;
    if (strcmp(name, "FALSE") == 0) return FALSE;
    if (strcmp(name, "PLUS") == 0) return PLUS;
    if (strcmp(name, "MINUS") == 0) return MINUS;
    if (strcmp(name, "MULT") == 0) return MULT;
    if (strcmp(name, "DIV") == 0) return DIV;
    if (strcmp(name, "MOD") == 0) return MOD;
    if (strcmp(name, "POW") == 0) return POW;
    if (strcmp(name, "LE") == 0) return LE;
    if (strcmp(name, "GE") == 0) return GE;
    if (strcmp(name, "EQ") == 0) return EQ;
    if (strcmp(name, "NEQ") == 0) return NEQ;
    if (strcmp(name, "LESS_THAN") == 0) return LESS_THAN;
    if (strcmp(name, "GREATER_THAN") == 0) return GREATER_THAN;
    if (strcmp(name, "OR") == 0) return OR;
    if (strcmp(name, "AND") == 0) return AND;
    if (strcmp(name, "NOT") == 0) return NOT;
    if (strcmp(name, "ASSIGN") == 0) return ASSIGN;
    if (strcmp(name, "DEQUALS") == 0) return DEQUALS;
    if (strcmp(name, "LPAREN") == 0) return LPAREN;
    if (strcmp(name, "RPAREN") == 0) return RPAREN;
    if (strcmp(name, "LBRACE") == 0) return LBRACE;
    if (strcmp(name, "RBRACE") == 0) return RBRACE;
    if (strcmp(name, "LBRACKET") == 0) return LBRACKET;
    if (strcmp(name, "RBRACKET") == 0) return RBRACKET;
    if (strcmp(name, "COMMA") == 0) return COMMA;
    if (strcmp(name, "SEMICOLON") == 0) return SEMICOLON;
    if (strcmp(name, "DOT") == 0) return DOT;
    if (strcmp(name, "COLON") == 0) return COLON;
    if (strcmp(name, "EOF") == 0) return EOF;
    return -1;
}

// Función para crear un símbolo
GrammarSymbol* grammar_symbol_create(const char* name, GrammarSymbolType type, int token_type) {
    GrammarSymbol* symbol = malloc(sizeof(GrammarSymbol));
    if (!symbol) return NULL;
    
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->token_type = token_type;
    symbol->next = NULL;
    
    return symbol;
}

void grammar_symbol_destroy(GrammarSymbol* symbol) {
    if (symbol) {
        free(symbol->name);
        free(symbol);
    }
}

// Función para crear una producción
Production* production_create(GrammarSymbol* left_side, GrammarSymbol** right_side, 
                           int right_side_count, const char* action) {
    Production* production = malloc(sizeof(Production));
    if (!production) return NULL;
    
    production->left_side = left_side;
    production->right_side = right_side;
    production->right_side_count = right_side_count;
    production->action = action ? strdup(action) : NULL;
    production->next = NULL;
    
    return production;
}

void production_destroy(Production* production) {
    if (production) {
        free(production->action);
        free(production);
    }
}

// Función para cargar la gramática desde un archivo
Grammar* grammar_load_from_file(const char* filename) {
    printf("DEBUG: Abriendo archivo %s\n", filename);
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo %s\n", filename);
        return NULL;
    }
    
    printf("DEBUG: Archivo abierto exitosamente\n");
    
    Grammar* grammar = malloc(sizeof(Grammar));
    if (!grammar) {
        fprintf(stderr, "Error: No se pudo asignar memoria para la gramática\n");
        fclose(file);
        return NULL;
    }
    
    printf("DEBUG: Memoria asignada para gramática\n");
    
    grammar->symbols = NULL;
    grammar->productions = NULL;
    grammar->start_symbol = NULL;
    grammar->symbol_count = 0;
    grammar->production_count = 0;
    
    char line[1024];
    GrammarSymbol** symbol_map = NULL;
    int symbol_map_size = 0;
    
    printf("DEBUG: Iniciando primera pasada - recolectar símbolos\n");
    // Primera pasada: recolectar símbolos
    while (fgets(line, sizeof(line), file)) {
        char* trimmed = line;
        while (*trimmed && isspace(*trimmed)) trimmed++;
        if (*trimmed == '#' || *trimmed == '\0') continue;
        
        printf("DEBUG: Procesando línea: %s", line);
        
        if (strncmp(trimmed, "%token", 6) == 0) {
            printf("DEBUG: Encontrada declaración de tokens\n");
            char* token_list = trimmed + 6;
            char* token = strtok(token_list, " \t\n");
            while (token) {
                int token_type = token_name_to_type(token);
                if (token_type != -1) {
                    GrammarSymbol* symbol = grammar_symbol_create(token, GRAMMAR_SYMBOL_TERMINAL, token_type);
                    if (symbol) {
                        symbol->next = grammar->symbols;
                        grammar->symbols = symbol;
                        grammar->symbol_count++;
                        
                        // Agregar al mapa de símbolos
                        symbol_map = realloc(symbol_map, sizeof(GrammarSymbol*) * (symbol_map_size + 1));
                        if (symbol_map) {
                            symbol_map[symbol_map_size++] = symbol;
                        }
                    }
                }
                token = strtok(NULL, " \t\n");
            }
        }
    }
    
    printf("DEBUG: Iniciando segunda pasada - procesar reglas gramaticales\n");
    // Segunda pasada: procesar reglas gramaticales
    rewind(file);
    while (fgets(line, sizeof(line), file)) {
        char* trimmed = line;
        while (*trimmed && isspace(*trimmed)) trimmed++;
        if (*trimmed == '#' || *trimmed == '\0') continue;
        if (strncmp(trimmed, "%token", 6) == 0) continue;
        if (strncmp(trimmed, "%start", 6) == 0) continue;
        if (strncmp(trimmed, "%left", 5) == 0) continue;
        if (strncmp(trimmed, "%right", 6) == 0) continue;
        
        printf("DEBUG: Segunda pasada - línea: %s", line);
        
        // Buscar reglas de producción (formato: nonterminal ::= ...)
        char* arrow = strstr(trimmed, "::=");
        if (arrow) {
            printf("DEBUG: Encontrada regla de producción\n");
            char* left_side_name = malloc(strlen(trimmed) + 1);
            if (!left_side_name) {
                printf("DEBUG: Error de memoria\n");
                continue;
            }
            strcpy(left_side_name, trimmed);
            left_side_name[arrow - trimmed] = '\0';
            
            // Limpiar espacios
            char* clean_name = left_side_name;
            while (*clean_name && isspace(*clean_name)) clean_name++;
            
            printf("DEBUG: Buscando símbolo izquierdo: '%s'\n", clean_name);
            // Crear o encontrar el símbolo no terminal
            GrammarSymbol* left_symbol = NULL;
            for (int i = 0; i < symbol_map_size; i++) {
                if (symbol_map[i] && strcmp(symbol_map[i]->name, clean_name) == 0) {
                    left_symbol = symbol_map[i];
                    printf("DEBUG: Símbolo encontrado en mapa\n");
                    break;
                }
            }
            if (!left_symbol) {
                printf("DEBUG: Creando nuevo símbolo no terminal: %s\n", clean_name);
                left_symbol = grammar_symbol_create(clean_name, GRAMMAR_SYMBOL_NONTERMINAL, -1);
                if (left_symbol) {
                    left_symbol->next = grammar->symbols;
                    grammar->symbols = left_symbol;
                    grammar->symbol_count++;
                    
                    symbol_map = realloc(symbol_map, sizeof(GrammarSymbol*) * (symbol_map_size + 1));
                    if (symbol_map) {
                        symbol_map[symbol_map_size++] = left_symbol;
                        printf("DEBUG: Símbolo agregado al mapa\n");
                    }
                }
            }
            free(left_side_name);
            
            if (!grammar->start_symbol) {
                grammar->start_symbol = left_symbol;
            }
            
            // Procesar el lado derecho de la producción
            char* right_side = arrow + 3;
            while (*right_side && isspace(*right_side)) right_side++;
            
            // Dividir en alternativas (separadas por |)
            char* alt = strtok(right_side, "|");
            while (alt) {
                while (*alt && isspace(*alt)) alt++;
                
                GrammarSymbol** right_symbols = NULL;
                int right_count = 0;
                
                // Dividir la alternativa en símbolos
                char* alt_copy = strdup(alt);
                if (!alt_copy) {
                    printf("DEBUG: Error de memoria al copiar alternativa\n");
                    continue;
                }
                
                char* symbol_name = strtok(alt_copy, " \t\n");
                while (symbol_name) {
                    if (strcmp(symbol_name, "ε") != 0) {
                        // Buscar el símbolo
                        GrammarSymbol* symbol = NULL;
                        for (int i = 0; i < symbol_map_size; i++) {
                            if (symbol_map[i] && strcmp(symbol_map[i]->name, symbol_name) == 0) {
                                symbol = symbol_map[i];
                                break;
                            }
                        }
                        if (!symbol) {
                            // Crear nuevo símbolo no terminal
                            symbol = grammar_symbol_create(symbol_name, GRAMMAR_SYMBOL_NONTERMINAL, -1);
                            if (symbol) {
                                symbol->next = grammar->symbols;
                                grammar->symbols = symbol;
                                grammar->symbol_count++;
                                
                                symbol_map = realloc(symbol_map, sizeof(GrammarSymbol*) * (symbol_map_size + 1));
                                if (symbol_map) {
                                    symbol_map[symbol_map_size++] = symbol;
                                }
                            }
                        }
                        
                        if (symbol) {
                            right_symbols = realloc(right_symbols, sizeof(GrammarSymbol*) * (right_count + 1));
                            if (right_symbols) {
                                right_symbols[right_count++] = symbol;
                            }
                        }
                    }
                    symbol_name = strtok(NULL, " \t\n");
                }
                
                if ((right_count > 0 || strstr(alt, "ε") != NULL) && left_symbol) {
                    Production* production = production_create(left_symbol, right_symbols, right_count, NULL);
                    if (production) {
                        production->next = grammar->productions;
                        grammar->productions = production;
                        grammar->production_count++;
                    }
                }
                
                free(alt_copy);
                alt = strtok(NULL, "|");
            }
        }
    }
    
    free(symbol_map);
    fclose(file);
    return grammar;
}

void grammar_destroy(Grammar* grammar) {
    if (!grammar) return;
    
    // Destruir símbolos
    GrammarSymbol* symbol = grammar->symbols;
    while (symbol) {
        GrammarSymbol* next = symbol->next;
        grammar_symbol_destroy(symbol);
        symbol = next;
    }
    
    // Destruir producciones
    Production* production = grammar->productions;
    while (production) {
        Production* next = production->next;
        production_destroy(production);
        production = next;
    }
    
    free(grammar);
}

void grammar_print(Grammar* grammar) {
    if (!grammar) return;
    
    printf("Gramática:\n");
    printf("Símbolos (%d):\n", grammar->symbol_count);
    
    GrammarSymbol* symbol = grammar->symbols;
    while (symbol) {
        printf("  %s (%s)\n", symbol->name, 
               symbol->type == GRAMMAR_SYMBOL_TERMINAL ? "terminal" : "no-terminal");
        symbol = symbol->next;
    }
    
    printf("\nProducciones (%d):\n", grammar->production_count);
    Production* production = grammar->productions;
    while (production) {
        printf("  %s ::= ", production->left_side->name);
        for (int i = 0; i < production->right_side_count; i++) {
            printf("%s ", production->right_side[i]->name);
        }
        if (production->right_side_count == 0) {
            printf("ε");
        }
        printf("\n");
        production = production->next;
    }
}

// Función para calcular el conjunto FIRST de un símbolo
bool* calculate_first_set(Grammar* grammar, GrammarSymbol* symbol) {
    // Implementación simplificada - en una implementación real
    // esto sería más complejo con recursión y memoización
    bool* first_set = calloc(grammar->symbol_count, sizeof(bool));
    if (!first_set) return NULL;
    
    if (symbol->type == GRAMMAR_SYMBOL_TERMINAL) {
        // Para símbolos terminales, FIRST = {símbolo}
        if (symbol->token_type >= 0 && symbol->token_type < grammar->symbol_count) {
            first_set[symbol->token_type] = true;
        }
    } else {
        // Para símbolos no terminales, calcular basado en las producciones
        Production* production = grammar->productions;
        while (production) {
            if (production->left_side == symbol) {
                if (production->right_side_count == 0) {
                    // Producción ε - no agregamos nada por ahora
                    // En una implementación completa, esto agregaría ε al FIRST
                } else {
                    // Agregar FIRST del primer símbolo del lado derecho
                    GrammarSymbol* first_right = production->right_side[0];
                    if (first_right) {
                        bool* first_right_set = calculate_first_set(grammar, first_right);
                        if (first_right_set) {
                            for (int i = 0; i < grammar->symbol_count; i++) {
                                if (first_right_set[i]) {
                                    first_set[i] = true;
                                }
                            }
                            free(first_right_set);
                        }
                    }
                }
            }
            production = production->next;
        }
    }
    
    return first_set;
}

// Función para calcular el conjunto FOLLOW de un símbolo
bool* calculate_follow_set(Grammar* grammar, GrammarSymbol* symbol) {
    // Implementación simplificada
    bool* follow_set = calloc(grammar->symbol_count, sizeof(bool));
    if (!follow_set) return NULL;
    
    // Para el símbolo inicial, agregar EOF
    if (symbol == grammar->start_symbol) {
        follow_set[EOF] = true;
    }
    
    // Implementación básica - en una implementación real sería más compleja
    return follow_set;
}

// Función para generar la tabla de parsing LL(1)
ParsingTable* generate_parsing_table(Grammar* grammar) {
    if (!grammar) return NULL;
    
    ParsingTable* table = malloc(sizeof(ParsingTable));
    if (!table) return NULL;
    
    // Contar terminales y no terminales
    int terminal_count = 0;
    int nonterminal_count = 0;
    GrammarSymbol* symbol = grammar->symbols;
    while (symbol) {
        if (symbol->type == GRAMMAR_SYMBOL_TERMINAL) {
            terminal_count++;
        } else {
            nonterminal_count++;
        }
        symbol = symbol->next;
    }
    
    table->terminal_count = terminal_count;
    table->nonterminal_count = nonterminal_count;
    
    // Crear arrays de símbolos
    table->terminals = malloc(sizeof(Symbol*) * terminal_count);
    table->nonterminals = malloc(sizeof(Symbol*) * nonterminal_count);
    
    if (!table->terminals || !table->nonterminals) {
        parsing_table_destroy(table);
        return NULL;
    }
    
    // Llenar arrays de símbolos
    int term_idx = 0, nonterm_idx = 0;
    symbol = grammar->symbols;
    while (symbol) {
        if (symbol->type == GRAMMAR_SYMBOL_TERMINAL) {
            table->terminals[term_idx++] = symbol;
        } else {
            table->nonterminals[nonterm_idx++] = symbol;
        }
        symbol = symbol->next;
    }
    
    // Crear tabla de parsing
    table->table = malloc(sizeof(int*) * nonterminal_count);
    if (!table->table) {
        parsing_table_destroy(table);
        return NULL;
    }
    
    for (int i = 0; i < nonterminal_count; i++) {
        table->table[i] = malloc(sizeof(int) * terminal_count);
        if (!table->table[i]) {
            parsing_table_destroy(table);
            return NULL;
        }
        for (int j = 0; j < terminal_count; j++) {
            table->table[i][j] = -1; // -1 indica error
        }
    }
    
    // Llenar tabla usando FIRST y FOLLOW
    Production* production = grammar->productions;
    int production_index = 0;
    while (production) {
        GrammarSymbol* left = production->left_side;
        if (left) {
            bool* first_set = calculate_first_set(grammar, left);
            
            if (first_set) {
                for (int i = 0; i < terminal_count; i++) {
                    if (first_set[i]) {
                        // Encontrar índices en la tabla
                        int nonterm_idx = -1;
                        int term_idx = -1;
                        
                        for (int j = 0; j < nonterminal_count; j++) {
                            if (table->nonterminals[j] == left) {
                                nonterm_idx = j;
                                break;
                            }
                        }
                        
                        for (int j = 0; j < terminal_count; j++) {
                            if (table->terminals[j] && table->terminals[j]->token_type == i) {
                                term_idx = j;
                                break;
                            }
                        }
                        
                        if (nonterm_idx != -1 && term_idx != -1) {
                            table->table[nonterm_idx][term_idx] = production_index;
                        }
                    }
                }
                free(first_set);
            }
        }
        
        production = production->next;
        production_index++;
    }
    
    return table;
}

void parsing_table_destroy(ParsingTable* table) {
    if (!table) return;
    
    if (table->table) {
        for (int i = 0; i < table->nonterminal_count; i++) {
            free(table->table[i]);
        }
        free(table->table);
    }
    
    free(table->terminals);
    free(table->nonterminals);
    free(table);
}

void parsing_table_print(ParsingTable* table) {
    if (!table) return;
    
    printf("Tabla de Parsing LL(1):\n");
    printf("     ");
    for (int i = 0; i < table->terminal_count; i++) {
        printf("%-10s ", table->terminals[i]->name);
    }
    printf("\n");
    
    for (int i = 0; i < table->nonterminal_count; i++) {
        printf("%-10s ", table->nonterminals[i]->name);
        for (int j = 0; j < table->terminal_count; j++) {
            if (table->table[i][j] == -1) {
                printf("%-10s ", "error");
            } else {
                printf("%-10d ", table->table[i][j]);
            }
        }
        printf("\n");
    }
}

// Funciones para el parser generado
GeneratedParser* generated_parser_create(Token* tokens, int token_count, 
                                       ParsingTable* table, Grammar* grammar,
                                       ErrorContext* error_context) {
    GeneratedParser* parser = malloc(sizeof(GeneratedParser));
    if (!parser) return NULL;
    
    parser->table = table;
    parser->grammar = grammar;
    parser->tokens = tokens;
    parser->token_count = token_count;
    parser->current_token = 0;
    parser->error_context = error_context;
    
    return parser;
}

void generated_parser_destroy(GeneratedParser* parser) {
    if (parser) {
        free(parser);
    }
}

// Función principal de parsing usando la tabla generada
ASTNode* generated_parser_parse(GeneratedParser* parser) {
    if (!parser || !parser->tokens || parser->token_count == 0) {
        return NULL;
    }
    
    // Implementación básica del parser usando la tabla
    // En una implementación completa, esto usaría una pila para
    // realizar el parsing predictivo
    
    printf("Parsing con tabla generada automáticamente...\n");
    
    // Por ahora, retornamos NULL - la implementación completa
    // requeriría más desarrollo del algoritmo de parsing
    return NULL;
}

// Función para generar código C del parser
void generate_parser_code(Grammar* grammar, ParsingTable* table, 
                         const char* output_filename) {
    FILE* file = fopen(output_filename, "w");
    if (!file) {
        fprintf(stderr, "Error: No se pudo crear el archivo %s\n", output_filename);
        return;
    }
    
    fprintf(file, "// Parser generado automáticamente\n");
    fprintf(file, "#include \"parser_generator.h\"\n\n");
    
    fprintf(file, "// Tabla de parsing generada\n");
    fprintf(file, "static int parsing_table[%d][%d] = {\n", 
            table->nonterminal_count, table->terminal_count);
    
    for (int i = 0; i < table->nonterminal_count; i++) {
        fprintf(file, "    {");
        for (int j = 0; j < table->terminal_count; j++) {
            fprintf(file, "%d", table->table[i][j]);
            if (j < table->terminal_count - 1) fprintf(file, ", ");
        }
        fprintf(file, "}");
        if (i < table->nonterminal_count - 1) fprintf(file, ",");
        fprintf(file, "\n");
    }
    fprintf(file, "};\n\n");
    
    fprintf(file, "// Función de parsing generada\n");
    fprintf(file, "ASTNode* generated_parse(Token* tokens, int token_count, ErrorContext* error_context) {\n");
    fprintf(file, "    // Implementación del parser usando la tabla generada\n");
    fprintf(file, "    // Esta función sería generada automáticamente\n");
    fprintf(file, "    return NULL;\n");
    fprintf(file, "}\n");
    
    fclose(file);
} 
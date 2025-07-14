// src/main.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token/token.h"
#include "ast/ast.h"
#include "utils/print_utils.h"
#include "check/check_semantic.h" 
#include "llvm/codegen.h"
#include "lexer/theoretical_lexer.h"
#include "Parser/ll1_parser.h"
#include "Parser/derivation_tree.h"
#include "error/error.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <archivo_fuente>\n", argv[0]);
        return 1;
    }

    // Leer el archivo fuente para el contexto de errores
    FILE* source_file = fopen(argv[1], "r");
    if (!source_file) {
        fprintf(stderr, "Error: No se puede abrir el archivo '%s'\n", argv[1]);
        return 1;
    }
    
    // Obtener el tamaño del archivo
    fseek(source_file, 0, SEEK_END);
    long file_size = ftell(source_file);
    fseek(source_file, 0, SEEK_SET);
    
    // Leer el contenido del archivo
    char* source_code = malloc(file_size + 1);
    if (!source_code) {
        fclose(source_file);
        fprintf(stderr, "Error: No se puede asignar memoria para el archivo fuente\n");
        return 1;
    }
    
    fread(source_code, 1, file_size, source_file);
    source_code[file_size] = '\0';
    fclose(source_file);
    
    // Crear el contexto de errores
    ErrorContext* error_context = error_context_create(argv[1], source_code);
    if (!error_context) {
        fprintf(stderr, "Error: No se puede crear el contexto de errores\n");
        free(source_code);
        return 1;
    }

    // --- Análisis Léxico ---
    print_info("Ejecutando análisis léxico...");
    
    size_t token_count;
    Token** tokens = tokenize_file_with_errors(argv[1], &token_count, error_context);

    if (!tokens) {
        print_error("Error en el análisis léxico o archivo fuente vacío.");
        print_compiler_errors(error_context);
        error_context_destroy(error_context);
        free(source_code);
        return 1;
    }

    // Verificar si hay errores léxicos
    if (error_context->error_count > 0) {
        print_error("Errores léxicos encontrados:");
        print_compiler_errors(error_context);
        error_context_destroy(error_context);
        free(source_code);
        
        // Liberar tokens
        for (size_t i = 0; i < token_count; i++) {
            free_token(tokens[i]);
        }
        free(tokens);
        return 1;
    }

    // Imprimir todos los tokens generados
    printf("\n--- TOKENS GENERADOS ---\n");
    for (size_t i = 0; i < token_count; i++) {
        printf("Token %zu: tipo=%d (%s), lexema='%s', línea=%d, columna=%d\n", 
               i, tokens[i]->type, get_token_name(tokens[i]->type), 
               tokens[i]->lexeme ? tokens[i]->lexeme : "<NULL>",
               tokens[i]->line, tokens[i]->column);
    }
    printf("-----------------------\n\n");
    
    // Print all tokens before parsing
    printf("--- TOKEN STREAM ---\n");
    for (int i = 0; i < token_count; i++) {
        printf("Token %d: type=%d (%s), lexeme='%s', line=%d, column=%d\n", i, tokens[i]->type, get_token_name(tokens[i]->type), tokens[i]->lexeme, tokens[i]->line, tokens[i]->column);
    }
    printf("--------------------\n");
    
    // --- Análisis Sintáctico (Proceso Simplificado) ---
    
    // 1. Crear el contexto del parser
    print_info("Creando el contexto del parser...");
    
    // Aplanar el arreglo de punteros a Token a un arreglo de structs Token
    Token* flat_tokens = malloc(sizeof(Token) * token_count);
    for (size_t i = 0; i < token_count; i++) {
        flat_tokens[i] = *tokens[i];  // Copiar el struct, no el puntero
        // Duplicar el lexeme para evitar problemas de memoria
        if (tokens[i]->lexeme) {
            size_t len = strlen(tokens[i]->lexeme);
            flat_tokens[i].lexeme = malloc(len + 1);
            if (flat_tokens[i].lexeme) {
                strcpy(flat_tokens[i].lexeme, tokens[i]->lexeme);
            }
        } else {
            flat_tokens[i].lexeme = NULL;
        }
    }
    
    LL1Parser* parser_ctx = ll1_parser_create_with_errors(flat_tokens, (int)token_count, error_context);

    if (!parser_ctx) {
        print_error("Error al crear el contexto del parser");
        // Liberar tokens originales
        for (size_t i = 0; i < token_count; i++) {
            free_token(tokens[i]);
        }
        free(tokens);
        // Liberar los lexemes de flat_tokens en caso de error
        for (size_t i = 0; i < token_count; i++) {
            if (flat_tokens[i].lexeme) {
                free(flat_tokens[i].lexeme);
            }
        }
        free(flat_tokens);
        error_context_destroy(error_context);
        free(source_code);
        return 1;
    }

    // 2. Ejecutar el parser para obtener el AST directamente
    print_info("Ejecutando análisis sintáctico (generación de AST)...");
    Program* ast = ll1_parser_parse(parser_ctx);
    
    ll1_parser_destroy(parser_ctx);
    
    // Liberar tokens originales
    for (size_t i = 0; i < token_count; i++) {
        free_token(tokens[i]);
    }
    free(tokens);
    
    // Liberar los lexemes de flat_tokens
    for (size_t i = 0; i < token_count; i++) {
        if (flat_tokens[i].lexeme) {
            free(flat_tokens[i].lexeme);
        }
    }
    free(flat_tokens);

    if (ast == NULL) {
        print_error("Error en el análisis sintáctico (falló la generación de AST)");
        print_compiler_errors(error_context);
        error_context_destroy(error_context);
        free(source_code);
        return 1;
    }

    // Verificar si hay errores sintácticos
    if (error_context->error_count > 0) {
        print_error("Errores sintácticos encontrados:");
        print_compiler_errors(error_context);
        error_context_destroy(error_context);
        free(source_code);
        free_ast((ASTNode*)ast);
        return 1;
    }

    print_success("AST creado exitosamente.");
    
    // Mostrar el AST generado
    print_info("Mostrando AST generado:");
    printf("\n--- AST GENERADO ---\n");
    print_ast((ASTNode*)ast, 0);
    printf("-------------------\n\n");

    // Liberar recursos
    free_ast((ASTNode*)ast);
    error_context_destroy(error_context);
    free(source_code);
    return 0;
}


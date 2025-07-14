
// error.c

#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Funciones para el nuevo sistema de errores
ErrorContext* error_context_create(const char* file_name, const char* source_code) {
    ErrorContext* ctx = malloc(sizeof(ErrorContext));
    if (!ctx) return NULL;
    
    ctx->errors = NULL;
    ctx->error_count = 0;
    ctx->warning_count = 0;
    ctx->file_name = file_name ? strdup(file_name) : NULL;
    ctx->source_code = source_code ? strdup(source_code) : NULL;
    
    return ctx;
}

void error_context_destroy(ErrorContext* ctx) {
    if (!ctx) return;
    
    CompilerError* current = ctx->errors;
    while (current) {
        CompilerError* next = current->next;
        compiler_error_destroy(current);
        current = next;
    }
    
    free(ctx->file_name);
    free(ctx->source_code);
    free(ctx);
}

CompilerError* compiler_error_create(ErrorType type, const char* message, int line, int column, 
                                   const char* code_snippet, const char* file_name) {
    CompilerError* error = malloc(sizeof(CompilerError));
    if (!error) return NULL;
    
    error->type = type;
    error->message = message ? strdup(message) : NULL;
    error->line = line;
    error->column = column;
    error->code_snippet = code_snippet ? strdup(code_snippet) : NULL;
    error->file_name = file_name ? strdup(file_name) : NULL;
    error->next = NULL;
    
    return error;
}

void compiler_error_destroy(CompilerError* error) {
    if (!error) return;
    
    free(error->message);
    free(error->code_snippet);
    free(error->file_name);
    free(error);
}

void add_compiler_error(ErrorContext* ctx, ErrorType type, const char* message, int line, int column) {
    if (!ctx) return;
    
    char* code_snippet = get_code_snippet(ctx->source_code, line, column, 2);
    CompilerError* error = compiler_error_create(type, message, line, column, code_snippet, ctx->file_name);
    
    if (error) {
        error->next = ctx->errors;
        ctx->errors = error;
        ctx->error_count++;
    }
    
    free(code_snippet);
}

void add_compiler_error_with_token(ErrorContext* ctx, ErrorType type, const char* message, Token* token) {
    if (!ctx || !token) return;
    
    add_compiler_error(ctx, type, message, token->line, token->column);
}

void add_compiler_error_with_snippet(ErrorContext* ctx, ErrorType type, const char* message, 
                                   int line, int column, const char* code_snippet) {
    if (!ctx) return;
    
    CompilerError* error = compiler_error_create(type, message, line, column, code_snippet, ctx->file_name);
    
    if (error) {
        error->next = ctx->errors;
        ctx->errors = error;
        ctx->error_count++;
    }
}

const char* get_error_type_string(ErrorType type) {
    switch (type) {
        case ERROR_LEXICAL: return "LEXICAL";
        case ERROR_SYNTAX: return "SYNTAX";
        case ERROR_SEMANTIC: return "SEMANTIC";
        case ERROR_TYPE: return "TYPE";
        case ERROR_RUNTIME: return "RUNTIME";
        default: return "UNKNOWN";
    }
}

void print_single_error(CompilerError* error) {
    if (!error) return;
    
    const char* type_str = get_error_type_string(error->type);
    const char* color = (error->type == ERROR_SYNTAX) ? RED : YELLOW;
    
    printf("%sERROR %s:%s ", color, type_str, RESET);
    if (error->file_name) {
        printf("%s:%d:%d: ", error->file_name, error->line, error->column);
    } else {
        printf("line %d, column %d: ", error->line, error->column);
    }
    printf("%s\n", error->message);
    
    if (error->code_snippet) {
        printf("  %s\n", error->code_snippet);
        
        // Mostrar el puntero al error
        if (error->column > 0) {
            printf("  ");
            for (int i = 0; i < error->column - 1; i++) {
                printf(" ");
            }
            printf("%s^%s\n", RED, RESET);
        }
    }
    printf("\n");
}

void print_compiler_errors(ErrorContext* ctx) {
    if (!ctx || !ctx->errors) return;
    
    printf("\n%s=== COMPILATION ERRORS ===%s\n\n", RED, RESET);
    
    CompilerError* current = ctx->errors;
    while (current) {
        print_single_error(current);
        current = current->next;
    }
    
    printf("%sTotal errors: %d%s\n", RED, ctx->error_count, RESET);
}

char* get_line_at(const char* source_code, int line_number) {
    if (!source_code || line_number <= 0) return NULL;
    
    int current_line = 1;
    const char* line_start = source_code;
    
    while (current_line < line_number && *source_code) {
        if (*source_code == '\n') {
            current_line++;
            line_start = source_code + 1;
        }
        source_code++;
    }
    
    if (current_line != line_number) return NULL;
    
    // Encontrar el final de la línea
    const char* line_end = line_start;
    while (*line_end && *line_end != '\n') {
        line_end++;
    }
    
    // Calcular la longitud y copiar
    int line_length = line_end - line_start;
    char* line = malloc(line_length + 1);
    if (!line) return NULL;
    
    strncpy(line, line_start, line_length);
    line[line_length] = '\0';
    
    return line;
}

char* get_code_snippet(const char* source_code, int line, int column, int context_lines) {
    if (!source_code || line <= 0) return NULL;
    
    char* snippet = malloc(1024); // Tamaño inicial
    if (!snippet) return NULL;
    snippet[0] = '\0';
    
    int snippet_len = 0;
    int max_len = 1024;
    
    // Agregar líneas de contexto
    for (int i = line - context_lines; i <= line + context_lines; i++) {
        if (i <= 0) continue;
        
        char* line_content = get_line_at(source_code, i);
        if (!line_content) continue;
        
        // Agregar número de línea
        char line_header[32];
        snprintf(line_header, sizeof(line_header), "%d: ", i);
        
        int needed_len = snippet_len + strlen(line_header) + strlen(line_content) + 2;
        if (needed_len >= max_len) {
            max_len = needed_len + 1024;
            snippet = realloc(snippet, max_len);
            if (!snippet) {
                free(line_content);
                return NULL;
            }
        }
        
        strcat(snippet, line_header);
        strcat(snippet, line_content);
        strcat(snippet, "\n");
        
        snippet_len = strlen(snippet);
        free(line_content);
    }
    
    return snippet;
}

// Funciones legacy para compatibilidad
ERROR *add_error_structure(ERROR *e, char *s)
{
    ERROR *error = malloc(sizeof(ERROR));
    error->value = s;
    error->next = e;
    error->index = e ? (e->index + 1) : 1;
    error->len_value = strlen(s);

    return error;
}

ERROR *error_to_string(char **list, int len_list)
{
    ERROR *result = NULL;

    for (int i = 0; i < len_list; i++)
    {
        result = add_error_structure(result, list[i]);
    }

    return result;
}

void print_error_structure(ERROR *e)
{
    ERROR *errors = e;

    while (errors)
    {
        printf(RED "!!ERROR SEMANTICO: %d -> %s\n" RESET, errors->index, errors->value);
        errors = errors->next;
    }
}

void add_error(char ***array, int *count, const char *str)
{
    *array = realloc(*array, (*count + 1) * sizeof(char *));
    (*array)[*count] = strdup(str);
    (*count)++;
}

void message_semantic_error(ASTVisitor *v, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *str = NULL;
    vasprintf(&str, fmt, args);
    va_end(args);
    add_error(&(v->errors), &(v->error_count), str);
}

void free_semantic_error(ERROR *e)
{
    if(!e) return ;

    free_semantic_error(e->next);

    free(e->value);
    free(e);
}

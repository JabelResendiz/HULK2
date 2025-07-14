
#ifndef ERROR_H
#define ERROR_H

#include "../visitor/visitor.h"
#include "../token/token.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define RED     "\x1B[31m"
#define YELLOW  "\x1B[33m"
#define BLUE    "\x1B[34m"
#define RESET   "\x1B[0m"

typedef enum {
    ERROR_LEXICAL,
    ERROR_SYNTAX,
    ERROR_SEMANTIC,
    ERROR_TYPE,
    ERROR_RUNTIME
} ErrorType;

typedef struct {
    ErrorType type;
    char* message;
    int line;
    int column;
    char* code_snippet;
    char* file_name;
    struct CompilerError* next;
} CompilerError;

typedef struct {
    CompilerError* errors;
    int error_count;
    int warning_count;
    char* source_code;
    char* file_name;
} ErrorContext;

// Funciones para el nuevo sistema de errores
ErrorContext* error_context_create(const char* file_name, const char* source_code);
void error_context_destroy(ErrorContext* ctx);

CompilerError* compiler_error_create(ErrorType type, const char* message, int line, int column, 
                                   const char* code_snippet, const char* file_name);
void compiler_error_destroy(CompilerError* error);

void add_compiler_error(ErrorContext* ctx, ErrorType type, const char* message, int line, int column);
void add_compiler_error_with_token(ErrorContext* ctx, ErrorType type, const char* message, Token* token);
void add_compiler_error_with_snippet(ErrorContext* ctx, ErrorType type, const char* message, 
                                   int line, int column, const char* code_snippet);

void print_compiler_errors(ErrorContext* ctx);
void print_single_error(CompilerError* error);

// Funciones para obtener fragmentos de código
char* get_code_snippet(const char* source_code, int line, int column, int context_lines);
char* get_line_at(const char* source_code, int line_number);

// Funciones legacy para compatibilidad
typedef struct ERROR
{
    char* value;
    int len_value;
    int index;
    struct ERROR* next;
}ERROR;

ERROR* add_error_structure(ERROR* e,char *s);
ERROR* error_to_string(char** list, int len_list);
void print_error_structure(ERROR* e);
void add_error(char*** array,int* count,const char* str);
void message_semantic_error(ASTVisitor *v, const char* fmt,...);
void free_semantic_error(ERROR* e);

#endif
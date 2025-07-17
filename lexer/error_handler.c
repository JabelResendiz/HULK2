#include "error_handler.h"
#include <ctype.h>

// Funciones para errores del lexer
LexerError *create_lexer_error(LexerErrorType type, int line, int column, const char *message, const char *lexeme)
{
    LexerError *error = malloc(sizeof(LexerError));
    if (!error) return NULL;
    
    error->type = type;
    error->line = line;
    error->column = column;
    error->message = message ? strdup(message) : NULL;
    error->lexeme = lexeme ? strdup(lexeme) : NULL;
    
    return error;
}

void destroy_lexer_error(LexerError *error)
{
    if (!error) return;
    
    if (error->message) free(error->message);
    if (error->lexeme) free(error->lexeme);
    free(error);
}

void print_lexer_error(LexerError *error, const char *source_code)
{
    if (!error) return;
    
    printf("\n🚨 ERROR LEXER: %s\n", get_lexer_error_type_name(error->type));
    printf("📍 Ubicación: Línea %d, Columna %d\n", error->line, error->column);
    
    if (error->message) {
        printf("📝 Descripción: %s\n", error->message);
    }
    
    if (error->lexeme) {
        printf("🔍 Lexema: '%s'\n", error->lexeme);
    }
    
    // Mostrar contexto del error
    print_error_context(source_code, error->line, error->column, 2);
    printf("\n");
}

// Funciones para errores del parser
ParserError *create_parser_error(ParserErrorType type, int line, int column, const char *message, const char *expected, const char *found)
{
    ParserError *error = malloc(sizeof(ParserError));
    if (!error) return NULL;
    
    error->type = type;
    error->line = line;
    error->column = column;
    error->message = message ? strdup(message) : NULL;
    error->expected = expected ? strdup(expected) : NULL;
    error->found = found ? strdup(found) : NULL;
    
    return error;
}

void destroy_parser_error(ParserError *error)
{
    if (!error) return;
    
    if (error->message) free(error->message);
    if (error->expected) free(error->expected);
    if (error->found) free(error->found);
    free(error);
}

void print_parser_error(ParserError *error, const char *source_code)
{
    if (!error) return;
    
    printf("\n🚨 ERROR PARSER: %s\n", get_parser_error_type_name(error->type));
    printf("📍 Ubicación: Línea %d, Columna %d\n", error->line, error->column);
    
    if (error->message) {
        printf("📝 Descripción: %s\n", error->message);
    }
    
    if (error->expected && error->found) {
        printf("❌ Se esperaba: '%s', pero se encontró: '%s'\n", error->expected, error->found);
    } else if (error->expected) {
        printf("❌ Se esperaba: '%s'\n", error->expected);
    } else if (error->found) {
        printf("❌ Se encontró: '%s'\n", error->found);
    }
    
    // Mostrar contexto del error
    print_error_context(source_code, error->line, error->column, 2);
    printf("\n");
}

// Funciones auxiliares
const char *get_lexer_error_type_name(LexerErrorType type)
{
    switch (type) {
        case ERROR_UNKNOWN_CHARACTER:
            return "Carácter desconocido";
        case ERROR_UNTERMINATED_STRING:
            return "Cadena no terminada";
        case ERROR_UNTERMINATED_COMMENT:
            return "Comentario no terminado";
        case ERROR_INVALID_NUMBER:
            return "Número inválido";
        case ERROR_INVALID_ESCAPE_SEQUENCE:
            return "Secuencia de escape inválida";
        case ERROR_UNEXPECTED_EOF:
            return "Fin de archivo inesperado";
        default:
            return "Error desconocido";
    }
}

const char *get_parser_error_type_name(ParserErrorType type)
{
    switch (type) {
        case ERROR_UNEXPECTED_TOKEN:
            return "Token inesperado";
        case ERROR_MISSING_TOKEN:
            return "Token faltante";
        case ERROR_INVALID_PRODUCTION:
            return "Producción inválida";
        case ERROR_UNEXPECTED_EOF_PARSER:
            return "Fin de archivo inesperado";
        case ERROR_SYNTAX_ERROR:
            return "Error de sintaxis";
        default:
            return "Error desconocido";
    }
}

void print_error_context(const char *source_code, int line, int column, int context_lines)
{
    if (!source_code) return;
    
    printf("\n📄 Contexto del error:\n");
    
    // Contar líneas en el código fuente
    int total_lines = 1;
    for (int i = 0; source_code[i]; i++) {
        if (source_code[i] == '\n') total_lines++;
    }
    
    // Calcular rango de líneas a mostrar
    int start_line = line - context_lines;
    int end_line = line + context_lines;
    
    if (start_line < 1) start_line = 1;
    if (end_line > total_lines) end_line = total_lines;
    
    // Mostrar líneas de contexto
    int current_line = 1;
    int char_index = 0;
    
    while (source_code[char_index] && current_line <= end_line) {
        if (current_line >= start_line) {
            // Mostrar número de línea
            printf("%s%3d │ ", (current_line == line) ? ">>> " : "    ", current_line);
            
            // Mostrar contenido de la línea
            while (source_code[char_index] && source_code[char_index] != '\n') {
                printf("%c", source_code[char_index]);
                char_index++;
            }
            printf("\n");
            
            // Si es la línea del error, mostrar indicador de columna
            if (current_line == line) {
                printf("     │ ");
                for (int i = 1; i < column; i++) {
                    printf(" ");
                }
                printf("^ aquí\n");
            }
        } else {
            // Saltar hasta la línea de inicio
            while (source_code[char_index] && source_code[char_index] != '\n') {
                char_index++;
            }
        }
        
        if (source_code[char_index] == '\n') {
            char_index++;
            current_line++;
        }
    }
} 
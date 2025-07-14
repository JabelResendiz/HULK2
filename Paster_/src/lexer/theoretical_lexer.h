#ifndef THEORETICAL_LEXER_H
#define THEORETICAL_LEXER_H

#include "../token/token.h"
#include "../automata/nfa.h"
#include "../automata/dfa.h"
#include "../automata/nfa_to_dfa.h"
#include "../error/error.h"
#include <stdbool.h>
#include <stddef.h>

// Estructura para el lexer teórico
typedef struct {
    DFA* dfa;
    char* input;
    size_t input_length;
    size_t current_pos;
    int current_line;
    int current_column;
    ErrorContext* error_context;
} TheoreticalLexer;

// Lista de tokens
typedef struct TokenList {
    Token* token;
    struct TokenList* next;
} TokenList;

// Funciones principales del lexer
TheoreticalLexer* theoretical_lexer_create(void);
TheoreticalLexer* theoretical_lexer_create_with_input(const char* input);
TheoreticalLexer* theoretical_lexer_create_with_error_context(const char* input, ErrorContext* error_context);
void theoretical_lexer_destroy(TheoreticalLexer* lexer);

// Configuración
void theoretical_lexer_set_input(TheoreticalLexer* lexer, const char* input);
void theoretical_lexer_set_error_context(TheoreticalLexer* lexer, ErrorContext* error_context);
void theoretical_lexer_reset(TheoreticalLexer* lexer);

// Análisis léxico
Token* theoretical_lexer_get_next_token(TheoreticalLexer* lexer);
TokenList* theoretical_lexer_tokenize(TheoreticalLexer* lexer);

// Estado
bool theoretical_lexer_has_more_tokens(TheoreticalLexer* lexer);

// Funciones auxiliares para construir NFAs
NFA* build_number_nfa(void);
NFA* build_string_nfa(void);
NFA* build_identifier_nfa(void);
NFA* build_keyword_nfa(const char* keyword);
NFA* build_operator_nfa(const char* op);
NFA* build_punctuation_nfa(char punct);

// Construcción del NFA combinado y DFA
NFA* build_combined_nfa(void);
void build_dfa(TheoreticalLexer* lexer);

// Utilidades
void skip_whitespace(TheoreticalLexer* lexer);
void skip_comments(TheoreticalLexer* lexer);
TokenType get_token_type_from_state(int state);
TokenType get_token_type_from_lexeme(int state, const char* lexeme);

// Funciones para reconocimiento de tokens específicos
Token* recognize_specific_tokens(TheoreticalLexer* lexer, int start_line, int start_column);
Token* recognize_number(TheoreticalLexer* lexer, int start_line, int start_column);
Token* recognize_string(TheoreticalLexer* lexer, int start_line, int start_column);
Token* recognize_identifier(TheoreticalLexer* lexer, int start_line, int start_column);
Token* recognize_operator(TheoreticalLexer* lexer, int start_line, int start_column);
Token* recognize_punctuation(TheoreticalLexer* lexer, int start_line, int start_column);

// Funciones para manejo de listas de tokens
TokenList* token_list_create(Token* token);
void token_list_destroy(TokenList* list);
void token_list_add(TokenList** list, Token* token);

// Función de utilidad para tokenizar un archivo fuente y devolver un array de Token* y su cantidad
Token** tokenize_file(const char* filename, size_t* out_count);
Token** tokenize_file_with_errors(const char* filename, size_t* out_count, ErrorContext* error_context);

// Funciones para manejo de errores léxicos
void lexer_error_unknown_character(TheoreticalLexer* lexer, char character);
void lexer_error_unterminated_string(TheoreticalLexer* lexer, int start_line, int start_column);
void lexer_error_invalid_number(TheoreticalLexer* lexer, const char* number_str, int line, int column);

#endif // THEORETICAL_LEXER_H

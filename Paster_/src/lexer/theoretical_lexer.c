#include "theoretical_lexer.h"
#include "../automata/nfa_to_dfa.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>


// --- Prototipos de constructores de NFA ---
NFA* build_number_nfa();
NFA* build_identifier_nfa();
NFA* build_string_nfa();
NFA* build_operator_nfa(const char* op_str);

// --- Función principal para construir el DFA ---
void build_dfa(TheoreticalLexer* lexer) {
    if (!lexer) return;

    // 1. Construir NFAs para cada tipo de token
    NFA* number_nfa = build_number_nfa();
    NFA* ident_nfa = build_identifier_nfa();
    NFA* string_nfa = build_string_nfa();

    // Operadores
    NFA* arrow_nfa = build_operator_nfa("=>");
    NFA* dequals_nfa = build_operator_nfa(":=");
    NFA* concat_nfa = build_operator_nfa("@");
    NFA* dconcat_nfa = build_operator_nfa("@@");
    NFA* plus_nfa = build_operator_nfa("+");
    NFA* minus_nfa = build_operator_nfa("-");
    NFA* mult_nfa = build_operator_nfa("*");
    NFA* div_nfa = build_operator_nfa("/");
    NFA* mod_nfa = build_operator_nfa("%");
    NFA* pow_nfa = build_operator_nfa("**");
    NFA* le_nfa = build_operator_nfa("<=");
    NFA* ge_nfa = build_operator_nfa(">=");
    NFA* eq_nfa = build_operator_nfa("==");
    NFA* neq_nfa = build_operator_nfa("!=");
    NFA* lt_nfa = build_operator_nfa("<");
    NFA* gt_nfa = build_operator_nfa(">");
    NFA* or_nfa = build_operator_nfa("||");
    NFA* and_nfa = build_operator_nfa("&&");
    NFA* not_nfa = build_operator_nfa("!");
    NFA* assign_nfa = build_operator_nfa("=");
    // ... (añadir más operadores aquí)

    // 2. Unir todos los NFAs en uno solo
    NFA* combined = nfa_union(number_nfa, ident_nfa);
    combined = nfa_union(combined, string_nfa);
    combined = nfa_union(combined, arrow_nfa);
    combined = nfa_union(combined, dequals_nfa);
    combined = nfa_union(combined, concat_nfa);
    combined = nfa_union(combined, dconcat_nfa);
    combined = nfa_union(combined, plus_nfa);
    combined = nfa_union(combined, minus_nfa);
    combined = nfa_union(combined, mult_nfa);
    combined = nfa_union(combined, div_nfa);
    combined = nfa_union(combined, mod_nfa);
    combined = nfa_union(combined, pow_nfa);
    combined = nfa_union(combined, le_nfa);
    combined = nfa_union(combined, ge_nfa);
    combined = nfa_union(combined, eq_nfa);
    combined = nfa_union(combined, neq_nfa);
    combined = nfa_union(combined, lt_nfa);
    combined = nfa_union(combined, gt_nfa);
    combined = nfa_union(combined, or_nfa);
    combined = nfa_union(combined, and_nfa);
    combined = nfa_union(combined, not_nfa);
    combined = nfa_union(combined, assign_nfa);
    // ... (unir más NFAs aquí)

    // 3. Convertir el NFA combinado a un DFA
    lexer->dfa = nfa_to_dfa_convert(combined);

    // 4. Liberar los NFAs intermedios (nfa_to_dfa_convert debería manejar la memoria)
    nfa_destroy(combined);
}

// --- Implementaciones de los constructores de NFA ---

NFA* build_number_nfa() {
    NFA* digit = nfa_create_range('0', '9');
    NFA* digits = nfa_kleene_plus(digit);
    nfa_set_token_type_for_all_accepting(digits, NUMBER);
    return digits;
}

NFA* build_identifier_nfa() {
    NFA* letter_ = nfa_create_range('a', 'z');
    letter_ = nfa_union(letter_, nfa_create_range('A', 'Z'));
    letter_ = nfa_union(letter_, nfa_create_symbol('_'));

    NFA* letter_digit_ = nfa_union(letter_, nfa_create_range('0', '9'));
    NFA* letter_digit_star = nfa_kleene_star(letter_digit_);

    NFA* ident = nfa_concatenate(letter_, letter_digit_star);
    nfa_set_token_type_for_all_accepting(ident, IDENT);
    return ident;
}

NFA* build_string_nfa() {
    NFA* quote = nfa_create_symbol('"');
    NFA* not_quote = nfa_create_any_except("\"");
    NFA* not_quote_star = nfa_kleene_star(not_quote);

    NFA* final_nfa = nfa_concatenate(quote, not_quote_star);
    final_nfa = nfa_concatenate(final_nfa, nfa_create_symbol('"'));
    nfa_set_token_type_for_all_accepting(final_nfa, STRING);
    return final_nfa;
}

NFA* build_operator_nfa(const char* op_str) {
    if (!op_str || strlen(op_str) == 0) return NULL;

    NFA* op_nfa = nfa_create_symbol(op_str[0]);
    for (size_t i = 1; i < strlen(op_str); ++i) {
        op_nfa = nfa_concatenate(op_nfa, nfa_create_symbol(op_str[i]));
    }
    // Asignar el tipo de token correcto según el operador
    if (strcmp(op_str, ":=") == 0) nfa_set_token_type_for_all_accepting(op_nfa, ASSIGN_DESTRUCT);
    else if (strcmp(op_str, "@@") == 0) nfa_set_token_type_for_all_accepting(op_nfa, CONCAT_WS);
    else if (strcmp(op_str, "@") == 0) nfa_set_token_type_for_all_accepting(op_nfa, CONCAT);
    else if (strcmp(op_str, "+") == 0) nfa_set_token_type_for_all_accepting(op_nfa, PLUS);
    else if (strcmp(op_str, "-") == 0) nfa_set_token_type_for_all_accepting(op_nfa, MINUS);
    else if (strcmp(op_str, "*") == 0) nfa_set_token_type_for_all_accepting(op_nfa, MULT);
    else if (strcmp(op_str, "/") == 0) nfa_set_token_type_for_all_accepting(op_nfa, DIV);
    else if (strcmp(op_str, "%") == 0) nfa_set_token_type_for_all_accepting(op_nfa, MOD);
    else if (strcmp(op_str, "**") == 0 || strcmp(op_str, "^") == 0) nfa_set_token_type_for_all_accepting(op_nfa, POW);
    else if (strcmp(op_str, "<=") == 0) nfa_set_token_type_for_all_accepting(op_nfa, LE);
    else if (strcmp(op_str, ">=") == 0) nfa_set_token_type_for_all_accepting(op_nfa, GE);
    else if (strcmp(op_str, "==") == 0) nfa_set_token_type_for_all_accepting(op_nfa, EQ);
    else if (strcmp(op_str, "!=") == 0) nfa_set_token_type_for_all_accepting(op_nfa, NEQ);
    else if (strcmp(op_str, "<") == 0) nfa_set_token_type_for_all_accepting(op_nfa, LESS_THAN);
    else if (strcmp(op_str, ">") == 0) nfa_set_token_type_for_all_accepting(op_nfa, GREATER_THAN);
    else if (strcmp(op_str, "||") == 0) nfa_set_token_type_for_all_accepting(op_nfa, OR);
    else if (strcmp(op_str, "&&") == 0) nfa_set_token_type_for_all_accepting(op_nfa, AND);
    else if (strcmp(op_str, "!") == 0) nfa_set_token_type_for_all_accepting(op_nfa, NOT);
    else if (strcmp(op_str, "=") == 0) nfa_set_token_type_for_all_accepting(op_nfa, ASSIGN);
    else if (strcmp(op_str, "=>") == 0) nfa_set_token_type_for_all_accepting(op_nfa, ARROW);
    // Agrega más operadores según sea necesario
    return op_nfa;
}

TheoreticalLexer* theoretical_lexer_create(void) {
    TheoreticalLexer* lexer = (TheoreticalLexer*)malloc(sizeof(TheoreticalLexer));
    if (!lexer) return NULL;

    lexer->dfa = NULL;
    lexer->input = NULL;
    lexer->input_length = 0;
    lexer->current_pos = 0;
    lexer->current_line = 1;
    lexer->current_column = 1;

    build_dfa(lexer);

    return lexer;
}

TheoreticalLexer* theoretical_lexer_create_with_input(const char* input) {
    TheoreticalLexer* lexer = theoretical_lexer_create();
    if (!lexer) return NULL;

    theoretical_lexer_set_input(lexer, input);
    return lexer;
}

TheoreticalLexer* theoretical_lexer_create_with_error_context(const char* input, ErrorContext* error_context) {
    TheoreticalLexer* lexer = theoretical_lexer_create_with_input(input);
    if (!lexer) return NULL;
    
    lexer->error_context = error_context;
    return lexer;
}

void theoretical_lexer_set_error_context(TheoreticalLexer* lexer, ErrorContext* error_context) {
    if (!lexer) return;
    lexer->error_context = error_context;
}

void theoretical_lexer_destroy(TheoreticalLexer* lexer) {
    if (!lexer) return;

    if (lexer->dfa) {
        dfa_destroy(lexer->dfa);
    }
    if (lexer->input) {
        free(lexer->input);
    }
    free(lexer);
}

void theoretical_lexer_set_input(TheoreticalLexer* lexer, const char* input) {
    if (!lexer || !input) return;

    if (lexer->input) {
        free(lexer->input);
    }

    lexer->input_length = strlen(input);
    lexer->input = (char*)malloc(lexer->input_length + 1);
    if (lexer->input) {
        strcpy(lexer->input, input);
    }

    theoretical_lexer_reset(lexer);
}

void theoretical_lexer_reset(TheoreticalLexer* lexer) {
    if (!lexer) return;

    lexer->current_pos = 0;
    lexer->current_line = 1;
    lexer->current_column = 1;
}

bool theoretical_lexer_has_more_tokens(TheoreticalLexer* lexer) {
    if (!lexer || !lexer->input) return false;

    return lexer->current_pos < lexer->input_length;
}

void skip_whitespace(TheoreticalLexer* lexer) {
    if (!lexer || !lexer->input) return;

    while (lexer->current_pos < lexer->input_length) {
        char c = lexer->input[lexer->current_pos];
        if (c == ' ' || c == '\t') {
            lexer->current_pos++;
            lexer->current_column++;
        } else if (c == '\n') {
            lexer->current_pos++;
            lexer->current_line++;
            lexer->current_column = 1;
        } else if (c == '\r') {
            lexer->current_pos++;
            // Manejar \r\n
            if (lexer->current_pos < lexer->input_length && lexer->input[lexer->current_pos] == '\n') {
                lexer->current_pos++;
            }
            lexer->current_line++;
            lexer->current_column = 1;
        } else {
            break;
        }
    }
}

void skip_comments(TheoreticalLexer* lexer) {
    if (!lexer || !lexer->input) return;

    // Manejar comentarios //
    if (lexer->current_pos + 1 < lexer->input_length &&
        lexer->input[lexer->current_pos] == '/' &&
        lexer->input[lexer->current_pos + 1] == '/') {

        // Saltar hasta el final de la línea
        while (lexer->current_pos < lexer->input_length &&
               lexer->input[lexer->current_pos] != '\n') {
            lexer->current_pos++;
        }
    }
}

Token* theoretical_lexer_get_next_token(TheoreticalLexer* lexer) {
    if (!lexer || !lexer->input || !lexer->dfa) {
        return token_create(TOKEN_EOF, "", 0, 0);
    }

    // Saltar espacios en blanco y comentarios
    skip_whitespace(lexer);
    skip_comments(lexer);
    skip_whitespace(lexer);

    // Verificar EOF
    if (lexer->current_pos >= lexer->input_length) {
        return token_create(TOKEN_EOF, "", lexer->current_line, lexer->current_column);
    }

    int start_line = lexer->current_line;
    int start_column = lexer->current_column;
    size_t start_pos = lexer->current_pos;

    // Intentar reconocer tokens específicos primero
    Token* token = recognize_specific_tokens(lexer, start_line, start_column);
    if (token) {
        return token;
    }

    // Si no se reconoce ningún token específico, crear token de error
    char error_char[2] = {lexer->input[lexer->current_pos], '\0'};
    lexer->current_pos++;
    lexer->current_column++;

    return token_create(T_ERROR, error_char, start_line, start_column);
}

// Función auxiliar para reconocer tokens específicos
Token* recognize_specific_tokens(TheoreticalLexer* lexer, int start_line, int start_column) {
    size_t start_pos = lexer->current_pos;
    char c = lexer->input[lexer->current_pos];

    // printf("DEBUG: recognize_specific_tokens - char='%c' (ASCII: %d)\n", c, c);

    // Reconocer números
    if (isdigit(lexer->input[lexer->current_pos])) {
        // printf("DEBUG: Calling recognize_number\n");
        return recognize_number(lexer, start_line, start_column);
    }

    // Reconocer strings
    if (lexer->input[lexer->current_pos] == '"') {
        // printf("DEBUG: Calling recognize_string\n");
        return recognize_string(lexer, start_line, start_column);
    }

    // Reconocer identificadores y palabras clave
    if (isalpha(lexer->input[lexer->current_pos]) || lexer->input[lexer->current_pos] == '_') {
        // printf("DEBUG: Calling recognize_identifier\n");
        return recognize_identifier(lexer, start_line, start_column);
    }

    // Reconocer operadores
    // printf("DEBUG: Calling recognize_operator\n");
    Token* op_token = recognize_operator(lexer, start_line, start_column);
    if (op_token) {
        // printf("DEBUG: Operator token created with type %d\n", op_token->type);
        return op_token;
    }

    // Reconocer puntuación
    // printf("DEBUG: Calling recognize_punctuation\n");
    Token* punct_token = recognize_punctuation(lexer, start_line, start_column);
    if (punct_token) {
        // printf("DEBUG: Punctuation token created with type %d\n", punct_token->type);
        return punct_token;
    }

    // printf("DEBUG: No token recognized\n");
    return NULL;
}

Token* recognize_number(TheoreticalLexer* lexer, int start_line, int start_column) {
    size_t start_pos = lexer->current_pos;

    // Reconocer dígitos
    while (lexer->current_pos < lexer->input_length &&
           isdigit(lexer->input[lexer->current_pos])) {
        lexer->current_pos++;
        lexer->current_column++;
    }

    // Reconocer parte decimal
    if (lexer->current_pos < lexer->input_length &&
        lexer->input[lexer->current_pos] == '.') {
        lexer->current_pos++;
        lexer->current_column++;

        while (lexer->current_pos < lexer->input_length &&
               isdigit(lexer->input[lexer->current_pos])) {
            lexer->current_pos++;
            lexer->current_column++;
        }
    }

    // Crear lexema
    size_t length = lexer->current_pos - start_pos;
    char* lexeme = (char*)malloc(length + 1);
    if (!lexeme) return NULL;

    strncpy(lexeme, &lexer->input[start_pos], length);
    lexeme[length] = '\0';

    Token* token = token_create(NUMBER, lexeme, start_line, start_column);
    free(lexeme);

    return token;
}

Token* recognize_string(TheoreticalLexer* lexer, int start_line, int start_column) {
    size_t start_pos = lexer->current_pos;

    // Saltar comilla inicial
    lexer->current_pos++;
    lexer->current_column++;

    // Leer hasta la comilla de cierre
    while (lexer->current_pos < lexer->input_length) {
        char c = lexer->input[lexer->current_pos];

        if (c == '"') {
            // Comilla de cierre encontrada
            lexer->current_pos++;
            lexer->current_column++;
            break;
        } else if (c == '\\') {
            // Carácter de escape
            lexer->current_pos++;
            lexer->current_column++;
            if (lexer->current_pos < lexer->input_length) {
                lexer->current_pos++;
                lexer->current_column++;
            }
        } else if (c == '\n') {
            // String sin cerrar
            break;
        } else {
            lexer->current_pos++;
            lexer->current_column++;
        }
    }

    // Crear lexema
    size_t length = lexer->current_pos - start_pos;
    char* lexeme = (char*)malloc(length + 1);
    if (!lexeme) return NULL;

    strncpy(lexeme, &lexer->input[start_pos], length);
    lexeme[length] = '\0';

    Token* token = token_create(STRING, lexeme, start_line, start_column);
    free(lexeme);

    return token;
}

Token* recognize_identifier(TheoreticalLexer* lexer, int start_line, int start_column) {
    size_t start_pos = lexer->current_pos;

    // Primer carácter: letra o _
    lexer->current_pos++;
    lexer->current_column++;

    // Resto: letras, dígitos o _
    while (lexer->current_pos < lexer->input_length) {
        char c = lexer->input[lexer->current_pos];
        if (isalnum(c) || c == '_') {
            lexer->current_pos++;
            lexer->current_column++;
        } else {
            break;
        }
    }

    // Crear lexema
    size_t length = lexer->current_pos - start_pos;
    char* lexeme = (char*)malloc(length + 1);
    if (!lexeme) return NULL;

    strncpy(lexeme, &lexer->input[start_pos], length);
    lexeme[length] = '\0';

    // Verificar si es palabra clave
    TokenType type = get_keyword_type(lexeme);
    
    // Debug: imprimir información sobre el identificador reconocido
    printf("DEBUG: recognize_identifier - lexeme='%s', type=%d (%s)\n", 
           lexeme, type, get_token_name(type));

    Token* token = token_create(type, lexeme, start_line, start_column);
    free(lexeme);

    return token;
}

Token* recognize_operator(TheoreticalLexer* lexer, int start_line, int start_column) {
    char c1 = lexer->input[lexer->current_pos];
    char c2 = (lexer->current_pos + 1 < lexer->input_length) ?
              lexer->input[lexer->current_pos + 1] : '\0';



    // Operadores de dos caracteres
    if (c1 == '=' && c2 == '>') {
        // printf("DEBUG: Recognizing ARROW (=>)\n");
        lexer->current_pos += 2;
        lexer->current_column += 2;
        return token_create(ARROW, "=>", start_line, start_column);
    }
    if (c1 == ':' && c2 == '=') {
        // printf("DEBUG: Recognizing ASSIGN_DESTRUCT (:=)\n");
        lexer->current_pos += 2;
        lexer->current_column += 2;
        return token_create(ASSIGN_DESTRUCT, ":=", start_line, start_column);
    }
    if (c1 == '=' && c2 == '=') {
        // printf("DEBUG: Recognizing EQ (==)\n");
        lexer->current_pos += 2;
        lexer->current_column += 2;
        return token_create(EQ, "==", start_line, start_column);
    }
    if (c1 == '!' && c2 == '=') {
        // printf("DEBUG: Recognizing NEQ (!=)\n");
        lexer->current_pos += 2;
        lexer->current_column += 2;
        return token_create(NEQ, "!=", start_line, start_column);
    }
    if (c1 == '<' && c2 == '=') {
        // printf("DEBUG: Recognizing LE (<=)\n");
        lexer->current_pos += 2;
        lexer->current_column += 2;
        return token_create(LE, "<=", start_line, start_column);
    }
    if (c1 == '>' && c2 == '=') {
        // printf("DEBUG: Recognizing GE (>=)\n");
        lexer->current_pos += 2;
        lexer->current_column += 2;
        return token_create(GE, ">=", start_line, start_column);
    }
    if (c1 == '|' && c2 == '|') {
        // printf("DEBUG: Recognizing OR (||)\n");
        lexer->current_pos += 2;
        lexer->current_column += 2;
        return token_create(OR, "||", start_line, start_column);
    }
    if (c1 == '&' && c2 == '&') {
        // printf("DEBUG: Recognizing AND (&&)\n");
        lexer->current_pos += 2;
        lexer->current_column += 2;
        return token_create(AND, "&&", start_line, start_column);
    }
    if (c1 == '*' && c2 == '*') {
        // printf("DEBUG: Recognizing POW (**)\n");
        lexer->current_pos += 2;
        lexer->current_column += 2;
        return token_create(POW, "**", start_line, start_column);
    }
    if (c1 == '@' && c2 == '@') {
        // printf("DEBUG: Recognizing CONCAT_WS (@@)\n");
        lexer->current_pos += 2;
        lexer->current_column += 2;
        return token_create(CONCAT_WS, "@@", start_line, start_column);
    }
    // Verificar si es un comentario //
    if (c1 == '/' && c2 == '/') {
        // printf("DEBUG: Found comment (//), skipping\n");
        // No reconocer como operador, será manejado por skip_comments
        return NULL;
    }

    // Operadores de un carácter
    switch (c1) {
        case '+':
            // printf("DEBUG: Recognizing PLUS (+)\n");
            lexer->current_pos++;
            lexer->current_column++;
            return token_create(PLUS, "+", start_line, start_column);
        case '-':
            // printf("DEBUG: Recognizing MINUS (-)\n");
            lexer->current_pos++;
            lexer->current_column++;
            return token_create(MINUS, "-", start_line, start_column);
        case '*':
            // printf("DEBUG: Recognizing MULT (*)\n");
            lexer->current_pos++;
            lexer->current_column++;
            return token_create(MULT, "*", start_line, start_column);
        case '/':
            printf("DEBUG: Recognizing DIV (/)\n");
            lexer->current_pos++;
            lexer->current_column++;
            return token_create(DIV, "/", start_line, start_column);
        case '%':
            printf("DEBUG: Recognizing MOD (%%)\n");
            lexer->current_pos++;
            lexer->current_column++;
            return token_create(MOD, "%", start_line, start_column);
        case '^':
            printf("DEBUG: Recognizing POW (^)\n");
            lexer->current_pos++;
            lexer->current_column++;
            return token_create(POW, "^", start_line, start_column);
        case '>':
            printf("DEBUG: Recognizing GREATER_THAN (>)\n");
            lexer->current_pos++;
            lexer->current_column++;
            return token_create(GREATER_THAN, ">", start_line, start_column);
        case '<':
            printf("DEBUG: Recognizing LESS_THAN (<)\n");
            lexer->current_pos++;
            lexer->current_column++;
            return token_create(LESS_THAN, "<", start_line, start_column);
        case '=':
            printf("DEBUG: Recognizing ASSIGN (=)\n");
            lexer->current_pos++;
            lexer->current_column++;
            return token_create(ASSIGN, "=", start_line, start_column);
        case '@':
            printf("DEBUG: Recognizing CONCAT (@)\n");
            lexer->current_pos++;
            lexer->current_column++;
            return token_create(CONCAT, "@", start_line, start_column);
    }

    printf("DEBUG: No operator recognized for '%c'\n", c1);
    return NULL;
}

Token* recognize_punctuation(TheoreticalLexer* lexer, int start_line, int start_column) {
    char c = lexer->input[lexer->current_pos];

    printf("DEBUG: recognize_punctuation - c='%c'\n", c);

    switch (c) {
        case ',':
            printf("DEBUG: Recognizing COMMA (,)\n");
            lexer->current_pos++;
            lexer->current_column++;
            Token* comma_token = token_create(COMMA, ",", start_line, start_column);
            printf("DEBUG: Created COMMA token with type %d (%s)\n", 
                   comma_token->type, get_token_name(comma_token->type));
            return comma_token;
        case ';':
            printf("DEBUG: Recognizing SEMICOLON (;)\n");
            lexer->current_pos++;
            lexer->current_column++;
            return token_create(SEMICOLON, ";", start_line, start_column);
        case '.':
            printf("DEBUG: Recognizing DOT (.)\n");
            lexer->current_pos++;
            lexer->current_column++;
            return token_create(DOT, ".", start_line, start_column);
        case ':':
            printf("DEBUG: Recognizing COLON (:)\n");
            lexer->current_pos++;
            lexer->current_column++;
            return token_create(COLON, ":", start_line, start_column);
        case '(': 
            printf("DEBUG: Recognizing LPAREN (()\n");
            lexer->current_pos++;
            lexer->current_column++;
            Token* lparen_token = token_create(LPAREN, "(", start_line, start_column);
            printf("DEBUG: Created LPAREN token with type %d (%s)\n", 
                   lparen_token->type, get_token_name(lparen_token->type));
            return lparen_token;
        case ')':
            printf("DEBUG: Recognizing RPAREN ())\n");
            lexer->current_pos++;
            lexer->current_column++;
            Token* rparen_token = token_create(RPAREN, ")", start_line, start_column);
            printf("DEBUG: Created RPAREN token with type %d (%s)\n", 
                   rparen_token->type, get_token_name(rparen_token->type));
            return rparen_token;
        case '{':
            printf("DEBUG: Recognizing LBRACE ({)\n");
            lexer->current_pos++;
            lexer->current_column++;
            Token* lbrace_token = token_create(LBRACE, "{", start_line, start_column);
            printf("DEBUG: Created LBRACE token with type %d (%s)\n", 
                   lbrace_token->type, get_token_name(lbrace_token->type));
            return lbrace_token;
        case '}':
            printf("DEBUG: Recognizing RBRACE (})\n");
            lexer->current_pos++;
            lexer->current_column++;
            Token* rbrace_token = token_create(RBRACE, "}", start_line, start_column);
            printf("DEBUG: Created RBRACE token with type %d (%s)\n", 
                   rbrace_token->type, get_token_name(rbrace_token->type));
            return rbrace_token;
    }

    printf("DEBUG: No punctuation recognized for '%c'\n", c);
    return NULL;
}

TokenList* theoretical_lexer_tokenize(TheoreticalLexer* lexer) {
    if (!lexer) return NULL;

    TokenList* list = NULL;
    Token* token;

    while ((token = theoretical_lexer_get_next_token(lexer)) != NULL) {
        if (token->type == TOKEN_EOF) {
            token_list_add(&list, token);
            break;
        }
        token_list_add(&list, token);
    }

    return list;
}

// Funciones para manejo de listas de tokens
TokenList* token_list_create(Token* token) {
    TokenList* list = (TokenList*)malloc(sizeof(TokenList));
    if (!list) return NULL;

    list->token = token;
    list->next = NULL;

    return list;
}

void token_list_destroy(TokenList* list) {
    while (list) {
        TokenList* next = list->next;
        if (list->token) {
            token_destroy(list->token);
        }
        free(list);
        list = next;
    }
}

void token_list_add(TokenList** list, Token* token) {
    if (!list || !token) return;

    TokenList* new_node = token_list_create(token);
    if (!new_node) return;

    if (*list == NULL) {
        *list = new_node;
    } else {
        TokenList* current = *list;
        while (current->next) {
            current = current->next;
        }
        current->next = new_node;
    }
}

// Declaraciones de funciones auxiliares que faltan
Token* recognize_specific_tokens(TheoreticalLexer* lexer, int start_line, int start_column);
Token* recognize_number(TheoreticalLexer* lexer, int start_line, int start_column);
Token* recognize_string(TheoreticalLexer* lexer, int start_line, int start_column);
Token* recognize_identifier(TheoreticalLexer* lexer, int start_line, int start_column);
Token* recognize_operator(TheoreticalLexer* lexer, int start_line, int start_column);
Token* recognize_punctuation(TheoreticalLexer* lexer, int start_line, int start_column);

Token** tokenize_file_with_errors(const char* filename, size_t* out_count, ErrorContext* error_context) {
    *out_count = 0;
    FILE* f = fopen(filename, "rb");
    if (!f) {
        if (error_context) {
            add_compiler_error(error_context, ERROR_LEXICAL, 
                              "Cannot open file", 0, 0);
        }
        return NULL;
    }
    
    struct stat st; 
    fstat(fileno(f), &st);
    size_t len = st.st_size;
    char* buffer = (char*)malloc(len + 1);
    if (!buffer) { 
        fclose(f); 
        if (error_context) {
            add_compiler_error(error_context, ERROR_LEXICAL, 
                              "Cannot allocate memory for file", 0, 0);
        }
        return NULL; 
    }
    
    fread(buffer, 1, len, f); 
    buffer[len] = '\0'; 
    fclose(f);
    
    TheoreticalLexer* lexer = theoretical_lexer_create_with_error_context(buffer, error_context);
    free(buffer);
    if (!lexer) return NULL;
    
    TokenList* list = theoretical_lexer_tokenize(lexer);
    
    // Contar tokens
    size_t count = 0; 
    TokenList* tmp = list;
    while (tmp) { 
        count++; 
        tmp = tmp->next; 
    }
    
    Token** arr = (Token**)malloc(sizeof(Token*) * count);
    tmp = list; 
    size_t i = 0;
    while (tmp) { 
        arr[i++] = tmp->token; 
        TokenList* next = tmp->next; 
        free(tmp); 
        tmp = next; 
    }
    
    *out_count = count;
    theoretical_lexer_destroy(lexer);
    return arr;
}

Token** tokenize_file(const char* filename, size_t* out_count) {
    return tokenize_file_with_errors(filename, out_count, NULL);
}

// Funciones para manejo de errores léxicos
void lexer_error_unknown_character(TheoreticalLexer* lexer, char character) {
    if (!lexer || !lexer->error_context) return;
    
    char message[256];
    snprintf(message, sizeof(message), "Unknown character '%c'", character);
    
    add_compiler_error(lexer->error_context, ERROR_LEXICAL, message, 
                      lexer->current_line, lexer->current_column);
}

void lexer_error_unterminated_string(TheoreticalLexer* lexer, int start_line, int start_column) {
    if (!lexer || !lexer->error_context) return;
    
    add_compiler_error(lexer->error_context, ERROR_LEXICAL, 
                      "Unterminated string literal", start_line, start_column);
}

void lexer_error_invalid_number(TheoreticalLexer* lexer, const char* number_str, int line, int column) {
    if (!lexer || !lexer->error_context) return;
    
    char message[256];
    snprintf(message, sizeof(message), "Invalid number format '%s'", number_str);
    
    add_compiler_error(lexer->error_context, ERROR_LEXICAL, message, line, column);
}

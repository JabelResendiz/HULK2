#include "lexer_generator.h"
#include "patterns.h"
#include "regex_parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

Lexer *lexer_create(void)
{
    Lexer *lexer = malloc(sizeof(Lexer));
    if (!lexer)
        return NULL;

    lexer->dfa = dfa_create();
    lexer->input = NULL;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 1;

    lexer_init(lexer);
    return lexer;
}

void lexer_destroy(Lexer *lexer)
{
    if (!lexer)
        return;

    if (lexer->dfa)
    {
        dfa_destroy(lexer->dfa);
    }

    if (lexer->input)
    {
        free(lexer->input);
    }

    free(lexer);
}

void lexer_init(Lexer *lexer)
{
    if (!lexer)
        return;

    // Compilar patrones de tokens
    NFA *nfas[NUM_PATTERNS];
    for (int i = 0; i < NUM_PATTERNS; i++)
    {
        nfas[i] = regex_parse(patterns[i].pattern, patterns[i].is_literal);
        if (!nfas[i])
        {
            printf("[ERROR] El patrón %d ('%s') no pudo ser parseado.\n", i, patterns[i].pattern);
        }
        else
        {
            // Si el NFA no tiene estados finales, marca el último como final
            if (nfas[i]->num_final_states == 0 && nfas[i]->num_states > 0)
            {
                nfa_add_final_state(nfas[i], nfas[i]->states[nfas[i]->num_states - 1], patterns[i].type);
            }
            // Marcar todos los estados finales con el tipo de token correspondiente
            for (int j = 0; j < nfas[i]->num_final_states; j++)
            {
                nfas[i]->final_token_types[j] = patterns[i].type;
            }
        }
    }

    // Unir todos los NFAs
    NFA *combined_nfa = NULL;
    int combined_count = 0;
    for (int i = 0; i < NUM_PATTERNS; i++)
    {
        if (nfas[i])
        {
            if (!combined_nfa)
            {
                combined_nfa = nfas[i];
                nfas[i] = NULL; // Evitar doble free
                combined_count = 1;
            }
            else
            {
                NFA *temp = nfa_union(combined_nfa, nfas[i]);
                if (temp)
                {
                    // No liberar aquí, ya que los arrays están embebidos
                    combined_nfa = temp;
                    combined_count++;
                }
                else
                {
                    printf("[ERROR] Fallo la unión con patrón %d\n", i);
                }
                nfas[i] = NULL; // Evitar doble free
            }
        }
    }

    if (combined_nfa)
    {
        // Convertir a DFA
        DFA *dfa = nfa_to_dfa(combined_nfa);
        if (dfa)
        {
            if (lexer->dfa)
            {
                dfa_destroy(lexer->dfa);
            }
            lexer->dfa = dfa;
        }
        else
        {
            printf("[ERROR] Fallo la conversión de NFA a DFA.\n");
        }
        nfa_destroy(combined_nfa);
    }
    else
    {
        printf("[ERROR] No se pudo crear el NFA combinado.\n");
    }

    // Limpiar NFAs temporales
    for (int i = 0; i < NUM_PATTERNS; i++)
    {
        if (nfas[i])
        {
            nfa_destroy(nfas[i]);
        }
    }
}

Token *lexer_next_token(Lexer *lexer)
{
    if (!lexer->input || lexer->pos >= strlen(lexer->input))
    {
        return create_token(TOKEN_EOF, "EOF", lexer->line, lexer->column);
    }

    // Saltar espacios en blanco
    while (lexer->pos < strlen(lexer->input) &&
           isspace(lexer->input[lexer->pos]))
    {
        if (lexer->input[lexer->pos] == '\n')
        {
            lexer->line++;
            lexer->column = 1;
        }
        else
        {
            lexer->column++;
        }
        lexer->pos++;
    }

    if (lexer->pos >= strlen(lexer->input))
    {
        return create_token(TOKEN_EOF, "EOF", lexer->line, lexer->column);
    }

    // Reconocer token usando DFA con longest match mejorado
    int current_state = lexer->dfa->start_state;
    int start_pos = lexer->pos;
    int end_pos = lexer->pos;
    int last_accepting_pos = -1;
    TokenType last_accepting_type = TOKEN_ERROR;
    int best_priority = 999;

    // Buscar la coincidencia más larga con la prioridad más alta
    while (end_pos < strlen(lexer->input))
    {
        char symbol = lexer->input[end_pos];
        int next_state = dfa_next_state(lexer->dfa, current_state, symbol);

        if (next_state == -1)
            break; // No hay transición

        current_state = next_state;
        end_pos++;

        // Si es estado final, verificar si tiene mejor prioridad
        if (dfa_is_final(lexer->dfa, current_state))
        {
            TokenType current_token_type = dfa_get_token_type(lexer->dfa, current_state);
            int current_priority = get_token_priority(current_token_type);

            // Actualizar si encontramos una coincidencia más larga o con mejor prioridad
            if (end_pos > last_accepting_pos ||
                (end_pos == last_accepting_pos && current_priority < best_priority))
            {
                last_accepting_pos = end_pos;
                last_accepting_type = current_token_type;
                best_priority = current_priority;
            }
        }
    }

    // Si encontramos un token válido, crear el token
    if (last_accepting_pos > start_pos)
    {
        int token_length = last_accepting_pos - start_pos;
        char *lexeme = malloc(token_length + 1);
        strncpy(lexeme, lexer->input + start_pos, token_length);
        lexeme[token_length] = '\0';

        Token *token = create_token(last_accepting_type, lexeme, lexer->line, lexer->column);

        // Actualizar posición
        lexer->pos = last_accepting_pos;
        lexer->column += token_length;

        return token;
    }

    // Error: token no reconocido
    char error_char[2] = {lexer->input[lexer->pos], '\0'};
    Token *error_token = create_token(TOKEN_ERROR, error_char, lexer->line, lexer->column);
    lexer->pos++;
    lexer->column++;

    return error_token;
}

Token *lexer_peek_token(Lexer *lexer)
{
    if (!lexer)
        return NULL;

    // Guardar estado actual
    int saved_pos = lexer->pos;
    int saved_line = lexer->line;
    int saved_column = lexer->column;

    // Obtener siguiente token
    Token *token = lexer_next_token(lexer);

    // Restaurar estado
    lexer->pos = saved_pos;
    lexer->line = saved_line;
    lexer->column = saved_column;

    return token;
}

void lexer_set_input(Lexer *lexer, const char *input)
{
    if (!lexer)
        return;

    if (lexer->input)
    {
        free(lexer->input);
    }

    lexer->input = strdup(input);
    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 1;
}
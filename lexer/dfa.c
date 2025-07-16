#include "dfa.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Función para obtener la prioridad de un tipo de token
int get_token_priority(TokenType type)
{
    // Prioridad más alta = número más bajo
    switch (type)
    {
    // Palabras clave (prioridad más alta)
    case TOKEN_FUNCTION:
    case TOKEN_LET:
    case TOKEN_IN:
    case TOKEN_IF:
    case TOKEN_ELIF:
    case TOKEN_ELSE:
    case TOKEN_WHILE:
    case TOKEN_AS:
    case TOKEN_IS:
    case TOKEN_TYPE:
    case TOKEN_INHERITS:
    case TOKEN_NEW:
    case TOKEN_BASE:
    case TOKEN_FOR:
    case TOKEN_RANGE:
    case TOKEN_PI:
    case TOKEN_E:
        return 0;

    // Literales
    case TOKEN_STRING:
    case TOKEN_NUMBER:
    case TOKEN_BOOLEAN:
        return 1;

    // Operadores de asignación
    case TOKEN_PLUSEQUAL:
    case TOKEN_MINUSEQUAL:
    case TOKEN_TIMESEQUAL:
    case TOKEN_DIVEQUAL:
    case TOKEN_MODEQUAL:
    case TOKEN_POWEQUAL:
    case TOKEN_ANDEQUAL:
    case TOKEN_OREQUAL:
    case TOKEN_CONCATEQUAL:
    case TOKEN_DEQUALS:
    case TOKEN_EQUALSEQUALS:
    case TOKEN_NEQUALS:
    case TOKEN_EGREATER:
    case TOKEN_ELESS:
    case TOKEN_DCONCAT:
    case TOKEN_ARROW:
        return 2;

    // Operadores simples
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_TIMES:
    case TOKEN_DIVIDE:
    case TOKEN_MOD:
    case TOKEN_POWER:
    case TOKEN_AND:
    case TOKEN_OR:
    case TOKEN_NOT:
    case TOKEN_GREATER:
    case TOKEN_LESS:
    case TOKEN_EQUALS:
    case TOKEN_CONCAT:
    case TOKEN_QUESTION:
        return 3;

    // Delimitadores
    case TOKEN_LPAREN:
    case TOKEN_RPAREN:
    case TOKEN_LBRACKET:
    case TOKEN_RBRACKET:
    case TOKEN_SEMICOLON:
    case TOKEN_COMMA:
    case TOKEN_DOT:
    case TOKEN_COLON:
        return 4;

    // Identificadores (prioridad más baja)
    case TOKEN_ID:
        return 5;

    // Especiales
    case TOKEN_EOF:
    case TOKEN_ERROR:
    case TOKEN_COMMENT:
        return 6;
    default:
        return 6;
    }
}

DFA *dfa_create(void)
{
    DFA *dfa = malloc(sizeof(DFA));
    if (!dfa)
        return NULL;

    dfa->num_states = 0;
    dfa->start_state = 0;
    dfa->alphabet_size = 0;
    dfa->num_final_states = 0;

    // Inicializar matriz de transiciones con -1 (no hay transición)
    for (int i = 0; i < MAX_STATES; i++)
    {
        for (int j = 0; j < MAX_ALPHABET; j++)
        {
            dfa->transitions[i][j] = -1;
        }
    }

    return dfa;
}

void dfa_destroy(DFA *dfa)
{
    if (dfa)
    {
        free(dfa);
    }
}

void dfa_add_transition(DFA *dfa, int from, char symbol, int to)
{
    if (!dfa)
        return;

    dfa->transitions[from][(unsigned char)symbol] = to;
}

void dfa_add_final_state(DFA *dfa, int state, TokenType token_type)
{
    if (!dfa || dfa->num_final_states >= MAX_STATES)
        return;

    dfa->final_states[dfa->num_final_states] = state;
    dfa->final_token_types[dfa->num_final_states] = token_type;
    dfa->num_final_states++;
}

int dfa_next_state(DFA *dfa, int state, char symbol)
{
    if (!dfa || state < 0 || state >= dfa->num_states)
        return -1;

    return dfa->transitions[state][(unsigned char)symbol];
}

bool dfa_is_final(DFA *dfa, int state)
{
    if (!dfa)
        return false;

    for (int i = 0; i < dfa->num_final_states; i++)
    {
        if (dfa->final_states[i] == state)
        {
            return true;
        }
    }
    return false;
}

TokenType dfa_get_token_type(DFA *dfa, int state)
{
    if (!dfa)
        return TOKEN_ERROR;

    for (int i = 0; i < dfa->num_final_states; i++)
    {
        if (dfa->final_states[i] == state)
        {
            return dfa->final_token_types[i];
        }
    }
    return TOKEN_ERROR;
}

// Función auxiliar para calcular epsilon-clausura
void epsilon_closure(NFA *nfa, int state, bool *visited, int *closure, int *closure_size)
{
    if (visited[state])
        return;
    visited[state] = true;
    closure[*closure_size] = state;
    (*closure_size)++;

    for (int i = 0; i < nfa->num_transitions; i++)
    {
        Transition *t = &nfa->transitions[i];
        if (t->from == state && t->is_epsilon)
        {
            epsilon_closure(nfa, t->to, visited, closure, closure_size);
        }
    }
}

// Función auxiliar para calcular move
void move(NFA *nfa, int *states, int num_states, char symbol, int *result, int *result_size)
{
    *result_size = 0;

    for (int i = 0; i < num_states; i++)
    {
        for (int j = 0; j < nfa->num_transitions; j++)
        {
            Transition *t = &nfa->transitions[j];
            if (t->from == states[i] && t->symbol == symbol && !t->is_epsilon)
            {
                result[*result_size] = t->to;
                (*result_size)++;
            }
        }
    }
}

// Función auxiliar para verificar si dos conjuntos de estados son iguales
bool states_equal(int *states1, int size1, int *states2, int size2)
{
    if (size1 != size2)
        return false;

    for (int i = 0; i < size1; i++)
    {
        bool found = false;
        for (int j = 0; j < size2; j++)
        {
            if (states1[i] == states2[j])
            {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    return true;
}

// Función auxiliar para encontrar el índice de un conjunto de estados
int find_state_set(int *states, int size, int **all_states, int *all_sizes, int num_all_states)
{
    for (int i = 0; i < num_all_states; i++)
    {
        if (states_equal(states, size, all_states[i], all_sizes[i]))
        {
            return i;
        }
    }
    return -1;
}

DFA *nfa_to_dfa(NFA *nfa)
{
    if (!nfa)
        return NULL;

    DFA *dfa = dfa_create();
    if (!dfa)
        return NULL;

    // Calcular epsilon-clausura del estado inicial
    bool *visited = calloc(MAX_STATES, sizeof(bool));
    int *initial_closure = malloc(MAX_STATES * sizeof(int));
    int initial_closure_size = 0;

    epsilon_closure(nfa, nfa->start_state, visited, initial_closure, &initial_closure_size);

    // Inicializar arrays para el algoritmo de subconjuntos
    int **all_states = malloc(MAX_STATES * sizeof(int *));
    int *all_sizes = malloc(MAX_STATES * sizeof(int));
    int num_all_states = 0;

    // Agregar el estado inicial
    all_states[0] = malloc(initial_closure_size * sizeof(int));
    memcpy(all_states[0], initial_closure, initial_closure_size * sizeof(int));
    all_sizes[0] = initial_closure_size;
    num_all_states = 1;

    dfa->start_state = 0;
    dfa->states[0] = 0;
    dfa->num_states = 1;

    // Copiar alfabeto del NFA
    for (int i = 0; i < nfa->alphabet_size; i++)
    {
        dfa->alphabet[dfa->alphabet_size++] = nfa->alphabet[i];
    }

    // Procesar cada estado del DFA
    for (int i = 0; i < num_all_states; i++)
    {
        int *current_states = all_states[i];
        int current_size = all_sizes[i];

        // Verificar si es estado final y asignar el tipo de token con prioridad
        TokenType token_type = TOKEN_ERROR;
        int best_priority = 999; // Prioridad más alta (menor número = mayor prioridad)

        for (int j = 0; j < current_size; j++)
        {
            for (int k = 0; k < nfa->num_final_states; k++)
            {
                if (current_states[j] == nfa->final_states[k])
                {
                    TokenType current_token_type = nfa->final_token_types[k];

                    // Asignar prioridad basada en el tipo de token
                    // Palabras clave tienen prioridad más alta que identificadores
                    int priority = get_token_priority(current_token_type);

                    if (priority < best_priority)
                    {
                        best_priority = priority;
                        token_type = current_token_type;
                    }
                }
            }
        }

        if (token_type != TOKEN_ERROR)
        {
            dfa_add_final_state(dfa, i, token_type);
        }

        // Procesar cada símbolo del alfabeto
        for (int j = 0; j < dfa->alphabet_size; j++)
        {
            char symbol = dfa->alphabet[j];

            // Calcular move
            int *move_result = malloc(MAX_STATES * sizeof(int));
            int move_size = 0;
            move(nfa, current_states, current_size, symbol, move_result, &move_size);

            if (move_size > 0)
            {
                // Calcular epsilon-clausura del resultado del move
                memset(visited, 0, MAX_STATES * sizeof(bool));
                int *new_closure = malloc(MAX_STATES * sizeof(int));
                int new_closure_size = 0;

                for (int k = 0; k < move_size; k++)
                {
                    epsilon_closure(nfa, move_result[k], visited, new_closure, &new_closure_size);
                }

                // Verificar si este conjunto de estados ya existe
                int existing_index = find_state_set(new_closure, new_closure_size,
                                                    all_states, all_sizes, num_all_states);

                if (existing_index == -1)
                {
                    // Nuevo estado
                    all_states[num_all_states] = malloc(new_closure_size * sizeof(int));
                    memcpy(all_states[num_all_states], new_closure, new_closure_size * sizeof(int));
                    all_sizes[num_all_states] = new_closure_size;

                    dfa->states[dfa->num_states] = num_all_states;
                    dfa->num_states++;

                    dfa_add_transition(dfa, i, symbol, num_all_states);
                    num_all_states++;
                }
                else
                {
                    // Estado existente
                    dfa_add_transition(dfa, i, symbol, existing_index);
                }

                free(new_closure);
            }

            free(move_result);
        }
    }

    // Limpiar memoria
    for (int i = 0; i < num_all_states; i++)
    {
        free(all_states[i]);
    }
    free(all_states);
    free(all_sizes);
    free(visited);
    free(initial_closure);

    return dfa;
}
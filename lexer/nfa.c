#include "nfa.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

NFA *nfa_create(void)
{
    NFA *nfa = malloc(sizeof(NFA));
    if (!nfa)
        return NULL;

    nfa->num_states = 0;
    nfa->num_transitions = 0;
    nfa->start_state = 0;
    nfa->num_final_states = 0;
    nfa->alphabet_size = 0;

    return nfa;
}

void nfa_destroy(NFA *nfa)
{
    if (!nfa)
        return;
    free(nfa);
}

NFA *nfa_symbol(char symbol)
{
    NFA *nfa = nfa_create();
    if (!nfa)
        return NULL;

    nfa->start_state = 0;
    nfa->states[0] = 0;
    nfa->states[1] = 1;
    nfa->num_states = 2;

    nfa_add_transition(nfa, 0, symbol, 1, false);
    nfa_add_final_state(nfa, 1, TOKEN_ERROR); // El tipo se asignará en lexer_init

    // Agregar símbolo al alfabeto
    nfa->alphabet[nfa->alphabet_size++] = symbol;

    return nfa;
}

NFA *nfa_epsilon(void)
{
    NFA *nfa = nfa_create();
    if (!nfa)
        return NULL;

    nfa->start_state = 0;
    nfa->states[0] = 0;
    nfa->num_states = 1;

    nfa_add_final_state(nfa, 0, TOKEN_ERROR);

    return nfa;
}

NFA *nfa_empty(void)
{
    NFA *nfa = nfa_create();
    if (!nfa)
        return NULL;

    nfa->start_state = 0;
    nfa->states[0] = 0;
    nfa->num_states = 1;

    return nfa;
}

NFA *nfa_any(void)
{
    NFA *nfa = nfa_create();
    if (!nfa)
        return NULL;

    nfa->start_state = 0;
    nfa->states[0] = 0;
    nfa->states[1] = 1;
    nfa->num_states = 2;

    // Transición para cualquier carácter
    nfa_add_transition(nfa, 0, '.', 1, false);
    nfa_add_final_state(nfa, 1, TOKEN_ERROR); // El tipo se asignará en lexer_init

    return nfa;
}

NFA *nfa_range(char from, char to)
{
    NFA *nfa = nfa_create();
    if (!nfa)
        return NULL;

    nfa->start_state = 0;
    nfa->states[0] = 0;
    nfa->states[1] = 1;
    nfa->num_states = 2;

    // Agregar transiciones para el rango
    for (char c = from; c <= to; c++)
    {
        nfa_add_transition(nfa, 0, c, 1, false);
        nfa->alphabet[nfa->alphabet_size++] = c;
    }

    nfa_add_final_state(nfa, 1, TOKEN_ERROR); // El tipo se asignará en lexer_init

    return nfa;
}

NFA *nfa_union(NFA *a, NFA *b)
{
    if (!a || !b)
        return NULL;

    NFA *nfa = nfa_create();
    if (!nfa)
        return NULL;

    // Nuevo estado inicial
    nfa->start_state = 0;
    nfa->states[0] = 0;
    nfa->num_states = 1;

    // Copiar estados de A (desplazados por 1)
    for (int i = 0; i < a->num_states; i++)
    {
        nfa->states[nfa->num_states] = a->states[i] + 1;
        nfa->num_states++;
    }

    // Copiar estados de B (desplazados por a->num_states + 1)
    int b_offset = a->num_states + 1;
    for (int i = 0; i < b->num_states; i++)
    {
        nfa->states[nfa->num_states] = b->states[i] + b_offset;
        nfa->num_states++;
    }

    // Agregar transiciones epsilon del nuevo inicial a los iniciales de A y B
    nfa_add_transition(nfa, 0, '\0', a->start_state + 1, true);
    nfa_add_transition(nfa, 0, '\0', b->start_state + b_offset, true);

    // Copiar transiciones de A
    for (int i = 0; i < a->num_transitions; i++)
    {
        Transition t = a->transitions[i];
        nfa_add_transition(nfa, t.from + 1, t.symbol, t.to + 1, t.is_epsilon);
    }

    // Copiar transiciones de B
    for (int i = 0; i < b->num_transitions; i++)
    {
        Transition t = b->transitions[i];
        nfa_add_transition(nfa, t.from + b_offset, t.symbol, t.to + b_offset, t.is_epsilon);
    }

    // Copiar estados finales de A (prioridad alta)
    for (int i = 0; i < a->num_final_states; i++)
    {
        nfa_add_final_state(nfa, a->final_states[i] + 1, a->final_token_types[i]);
    }

    // Copiar estados finales de B (prioridad baja) solo si no hay conflicto
    for (int i = 0; i < b->num_final_states; i++)
    {
        int b_final_state = b->final_states[i] + b_offset;
        bool conflict = false;

        // Verificar si hay conflicto con estados finales de A
        for (int j = 0; j < a->num_final_states; j++)
        {
            if ((a->final_states[j] + 1) == b_final_state)
            {
                conflict = true;
                break;
            }
        }

        if (!conflict)
        {
            nfa_add_final_state(nfa, b_final_state, b->final_token_types[i]);
        }
    }

    // Combinar alfabetos
    for (int i = 0; i < a->alphabet_size; i++)
    {
        nfa->alphabet[nfa->alphabet_size++] = a->alphabet[i];
    }
    for (int i = 0; i < b->alphabet_size; i++)
    {
        // Evitar duplicados
        bool found = false;
        for (int j = 0; j < nfa->alphabet_size; j++)
        {
            if (nfa->alphabet[j] == b->alphabet[i])
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            nfa->alphabet[nfa->alphabet_size++] = b->alphabet[i];
        }
    }

    return nfa;
}

NFA *nfa_concat(NFA *a, NFA *b)
{
    if (!a || !b)
        return NULL;

    NFA *nfa = nfa_create();
    if (!nfa)
        return NULL;

    // Copiar estados de A
    for (int i = 0; i < a->num_states; i++)
    {
        nfa->states[nfa->num_states] = a->states[i];
        nfa->num_states++;
    }

    // Copiar estados de B (desplazados por a->num_states)
    for (int i = 0; i < b->num_states; i++)
    {
        nfa->states[nfa->num_states] = b->states[i] + a->num_states;
        nfa->num_states++;
    }

    nfa->start_state = a->start_state;

    // Copiar transiciones de A
    for (int i = 0; i < a->num_transitions; i++)
    {
        Transition t = a->transitions[i];
        nfa_add_transition(nfa, t.from, t.symbol, t.to, t.is_epsilon);
    }

    // Copiar transiciones de B
    for (int i = 0; i < b->num_transitions; i++)
    {
        Transition t = b->transitions[i];
        nfa_add_transition(nfa, t.from + a->num_states, t.symbol, t.to + a->num_states, t.is_epsilon);
    }

    // Agregar transiciones epsilon de estados finales de A al inicial de B
    for (int i = 0; i < a->num_final_states; i++)
    {
        nfa_add_transition(nfa, a->final_states[i], '\0', b->start_state + a->num_states, true);
    }

    // Solo los estados finales de B son finales en el resultado
    for (int i = 0; i < b->num_final_states; i++)
    {
        nfa_add_final_state(nfa, b->final_states[i] + a->num_states, b->final_token_types[i]);
    }

    // Combinar alfabetos
    for (int i = 0; i < a->alphabet_size; i++)
    {
        nfa->alphabet[nfa->alphabet_size++] = a->alphabet[i];
    }
    for (int i = 0; i < b->alphabet_size; i++)
    {
        nfa->alphabet[nfa->alphabet_size++] = b->alphabet[i];
    }

    return nfa;
}

NFA *nfa_closure(NFA *a)
{
    if (!a)
        return NULL;

    NFA *nfa = nfa_create();
    if (!nfa)
        return NULL;

    // Nuevo estado inicial
    nfa->start_state = 0;
    nfa->states[0] = 0;
    nfa->num_states = 1;

    // Copiar estados de A (desplazados por 1)
    for (int i = 0; i < a->num_states; i++)
    {
        nfa->states[nfa->num_states] = a->states[i] + 1;
        nfa->num_states++;
    }

    // Nuevo estado final
    nfa->states[nfa->num_states] = a->num_states + 1;
    nfa->num_states++;

    // Transición epsilon del nuevo inicial al inicial de A
    nfa_add_transition(nfa, 0, '\0', a->start_state + 1, true);

    // Transición epsilon del nuevo inicial al nuevo final
    nfa_add_transition(nfa, 0, '\0', a->num_states + 1, true);

    // Copiar transiciones de A
    for (int i = 0; i < a->num_transitions; i++)
    {
        Transition t = a->transitions[i];
        nfa_add_transition(nfa, t.from + 1, t.symbol, t.to + 1, t.is_epsilon);
    }

    // Transiciones epsilon de estados finales de A al inicial de A
    for (int i = 0; i < a->num_final_states; i++)
    {
        nfa_add_transition(nfa, a->final_states[i] + 1, '\0', a->start_state + 1, true);
    }

    // Transiciones epsilon de estados finales de A al nuevo final
    for (int i = 0; i < a->num_final_states; i++)
    {
        nfa_add_transition(nfa, a->final_states[i] + 1, '\0', a->num_states + 1, true);
    }

    // Solo el nuevo estado final es final, preservar el tipo de token original
    if (a->num_final_states > 0)
    {
        nfa_add_final_state(nfa, a->num_states + 1, a->final_token_types[0]);
    }
    else
    {
        nfa_add_final_state(nfa, a->num_states + 1, TOKEN_ERROR);
    }

    // Copiar alfabeto
    for (int i = 0; i < a->alphabet_size; i++)
    {
        nfa->alphabet[nfa->alphabet_size++] = a->alphabet[i];
    }

    return nfa;
}

void nfa_add_transition(NFA *nfa, int from, char symbol, int to, bool is_epsilon)
{
    if (!nfa || nfa->num_transitions >= MAX_TRANSITIONS)
        return;

    Transition *t = &nfa->transitions[nfa->num_transitions++];
    t->from = from;
    t->symbol = symbol;
    t->to = to;
    t->is_epsilon = is_epsilon;
}

void nfa_add_final_state(NFA *nfa, int state, TokenType token_type)
{
    if (!nfa || nfa->num_final_states >= MAX_STATES)
        return;

    nfa->final_states[nfa->num_final_states] = state;
    nfa->final_token_types[nfa->num_final_states] = token_type;
    nfa->num_final_states++;
}

bool nfa_is_final(NFA *nfa, int state)
{
    if (!nfa)
        return false;

    for (int i = 0; i < nfa->num_final_states; i++)
    {
        if (nfa->final_states[i] == state)
            return true;
    }
    return false;
}

bool nfa_evaluate(NFA *nfa, const char *input)
{
    if (!nfa || !input)
        return false;

    // Implementación simplificada para evaluación
    // En un lexer real, esto se usaría para testing
    return false;
}
#ifndef NFA_H
#define NFA_H

#include <stdbool.h>
#include "token.h"

#define MAX_STATES 1000
#define MAX_TRANSITIONS 1000

typedef struct
{
    int from;
    char symbol;
    int to;
    bool is_epsilon;
} Transition;

typedef struct
{
    int states[MAX_STATES];
    int num_states;
    Transition transitions[MAX_TRANSITIONS];
    int num_transitions;
    int start_state;
    int final_states[MAX_STATES];
    int num_final_states;
    TokenType final_token_types[MAX_STATES];
    char alphabet[256];
    int alphabet_size;
} NFA;

// Funciones NFA
NFA *nfa_create(void);
void nfa_destroy(NFA *nfa);
NFA *nfa_symbol(char symbol);
NFA *nfa_union(NFA *a, NFA *b);
NFA *nfa_concat(NFA *a, NFA *b);
NFA *nfa_closure(NFA *a);
NFA *nfa_range(char from, char to);
NFA *nfa_any(void);
NFA *nfa_epsilon(void);
NFA *nfa_empty(void);

// Funciones de utilidad
bool nfa_is_final(NFA *nfa, int state);
bool nfa_evaluate(NFA *nfa, const char *input);
void nfa_add_transition(NFA *nfa, int from, char symbol, int to, bool is_epsilon);
void nfa_add_final_state(NFA *nfa, int state, TokenType token_type);

#endif
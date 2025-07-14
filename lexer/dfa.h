#ifndef DFA_H
#define DFA_H

#include "nfa.h"
#include "token.h"

#define MAX_STATES 1000
#define MAX_ALPHABET 256

typedef struct
{
    int states[MAX_STATES];
    int num_states;
    int start_state;
    int alphabet[MAX_ALPHABET];
    int alphabet_size;
    int final_states[MAX_STATES];
    TokenType final_token_types[MAX_STATES];
    int num_final_states;
    int transitions[MAX_STATES][MAX_ALPHABET];
} DFA;

DFA *dfa_create(void);
void dfa_destroy(DFA *dfa);
void dfa_add_transition(DFA *dfa, int from, char symbol, int to);
void dfa_add_final_state(DFA *dfa, int state, TokenType token_type);
int dfa_next_state(DFA *dfa, int state, char symbol);
bool dfa_is_final(DFA *dfa, int state);
TokenType dfa_get_token_type(DFA *dfa, int state);
DFA *nfa_to_dfa(NFA *nfa);
int get_token_priority(TokenType type);

#endif
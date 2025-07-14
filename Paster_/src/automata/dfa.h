#ifndef DFA_H
#define DFA_H

#include <stdbool.h>

// Tabla de transiciones para DFA
typedef struct DFATransition {
    int from_state;
    char symbol;
    int to_state;
    struct DFATransition* next;
} DFATransition;

// Estructura del DFA
typedef struct {
    int num_states;
    int start_state;
    bool* accepting_states; // Array de booleanos
    DFATransition* transitions;
    int* token_types; // Array donde token_types[state] = tipo de token
} DFA;

// Funciones para DFA
DFA* dfa_create(int num_states);
void dfa_destroy(DFA* dfa);

// Operaciones básicas
void dfa_add_transition(DFA* dfa, int from, char symbol, int to);
void dfa_set_accepting(DFA* dfa, int state, bool accepting);
void dfa_set_start_state(DFA* dfa, int state);
void dfa_set_token_type(DFA* dfa, int state, int token_type);

// Simulación
int dfa_simulate(DFA* dfa, const char* input, int* last_accepting_state);
bool dfa_is_accepting(DFA* dfa, int state);
int dfa_get_token_type(DFA* dfa, int state);

// Getters
int dfa_get_num_states(DFA* dfa);
int dfa_get_start_state(DFA* dfa);

// Funciones auxiliares
int dfa_get_transition(DFA* dfa, int from_state, char symbol);

#endif // DFA_H

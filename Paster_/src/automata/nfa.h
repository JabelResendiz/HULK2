#ifndef NFA_H
#define NFA_H

#include <stdbool.h>
#include "../token/token.h"

// Estructuras para manejar conjuntos de estados
typedef struct {
    int* states;
    int count;
    int capacity;
} StateSet;

// Tabla de transiciones para NFA
typedef struct NFATransition {
    int from_state;
    char symbol;
    int to_state;
    struct NFATransition* next;
} NFATransition;

// Transiciones epsilon
typedef struct EpsilonTransition {
    int from_state;
    int to_state;
    struct EpsilonTransition* next;
} EpsilonTransition;

// Estructura del NFA
typedef struct {
    int num_states;
    int start_state;
    StateSet* accepting_states;
    NFATransition* transitions;
    EpsilonTransition* epsilon_transitions;
    int* token_types; // Array donde token_types[state] = tipo de token
} NFA;

// Funciones para StateSet
StateSet* state_set_create(void);
void state_set_destroy(StateSet* set);
bool state_set_add(StateSet* set, int state);
bool state_set_contains(StateSet* set, int state);
void state_set_clear(StateSet* set);

// Funciones para NFA
NFA* nfa_create(int num_states);
void nfa_destroy(NFA* nfa);

// Constructores de NFA básicos
NFA* nfa_create_symbol(char symbol);
NFA* nfa_create_range(char from, char to);
NFA* nfa_create_any_except(const char* except_chars);

// Operaciones sobre NFAs
NFA* nfa_concatenate(NFA* nfa1, NFA* nfa2);
NFA* nfa_union(NFA* nfa1, NFA* nfa2);
NFA* nfa_kleene_star(NFA* nfa);
NFA* nfa_kleene_plus(NFA* nfa);
NFA* nfa_optional(NFA* nfa);

// Funciones auxiliares
void nfa_add_transition(NFA* nfa, int from, char symbol, int to);
void nfa_add_epsilon_transition(NFA* nfa, int from, int to);
void nfa_set_accepting(NFA* nfa, int state, bool accepting);
void nfa_set_token_type(NFA* nfa, int state, int token_type);

// Asigna el tipo de token a todos los estados de aceptación del NFA
void nfa_set_token_type_for_all_accepting(NFA* nfa, TokenType type);


// Obtener información
StateSet* nfa_get_accepting_states(NFA* nfa);
int nfa_get_num_states(NFA* nfa);

#endif // NFA_H

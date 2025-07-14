#include "dfa.h"
#include <stdlib.h>
#include <string.h>

DFA* dfa_create(int num_states) {
    DFA* dfa = (DFA*)malloc(sizeof(DFA));
    if (!dfa) return NULL;

    dfa->num_states = num_states;
    dfa->start_state = 0;
    dfa->transitions = NULL;

    // Inicializar array de estados de aceptación
    dfa->accepting_states = (bool*)calloc(num_states, sizeof(bool));
    if (!dfa->accepting_states) {
        free(dfa);
        return NULL;
    }

    // Inicializar array de tipos de token
    dfa->token_types = (int*)malloc(num_states * sizeof(int));
    if (!dfa->token_types) {
        free(dfa->accepting_states);
        free(dfa);
        return NULL;
    }

    // Inicializar tipos de token a -1 (sin tipo)
    for (int i = 0; i < num_states; i++) {
        dfa->token_types[i] = -1;
    }

    return dfa;
}

void dfa_destroy(DFA* dfa) {
    if (!dfa) return;

    // Liberar transiciones
    DFATransition* trans = dfa->transitions;
    while (trans) {
        DFATransition* next = trans->next;
        free(trans);
        trans = next;
    }

    free(dfa->accepting_states);
    free(dfa->token_types);
    free(dfa);
}

void dfa_add_transition(DFA* dfa, int from, char symbol, int to) {
    if (!dfa || from < 0 || from >= dfa->num_states || to < 0 || to >= dfa->num_states) {
        return;
    }

    DFATransition* trans = (DFATransition*)malloc(sizeof(DFATransition));
    if (!trans) return;

    trans->from_state = from;
    trans->symbol = symbol;
    trans->to_state = to;
    trans->next = dfa->transitions;
    dfa->transitions = trans;
}

void dfa_set_accepting(DFA* dfa, int state, bool accepting) {
    if (!dfa || state < 0 || state >= dfa->num_states) return;

    dfa->accepting_states[state] = accepting;
}

void dfa_set_start_state(DFA* dfa, int state) {
    if (!dfa || state < 0 || state >= dfa->num_states) return;

    dfa->start_state = state;
}

void dfa_set_token_type(DFA* dfa, int state, int token_type) {
    if (!dfa || state < 0 || state >= dfa->num_states) return;

    dfa->token_types[state] = token_type;
}

int dfa_get_transition(DFA* dfa, int from_state, char symbol) {
    if (!dfa) return -1;

    DFATransition* trans = dfa->transitions;
    while (trans) {
        if (trans->from_state == from_state && trans->symbol == symbol) {
            return trans->to_state;
        }
        trans = trans->next;
    }

    return -1; // No hay transición
}

int dfa_simulate(DFA* dfa, const char* input, int* last_accepting_state) {
    if (!dfa || !input) return -1;

    int current_state = dfa->start_state;
    int last_accepting = -1;
    int last_accepting_pos = -1;

    if (last_accepting_state) {
        *last_accepting_state = -1;
    }

    for (int i = 0; input[i] != '\0'; i++) {
        // Verificar si el estado actual es de aceptación
        if (dfa->accepting_states[current_state]) {
            last_accepting = current_state;
            last_accepting_pos = i;
            if (last_accepting_state) {
                *last_accepting_state = current_state;
            }
        }

        // Buscar transición
        int next_state = dfa_get_transition(dfa, current_state, input[i]);
        if (next_state == -1) {
            // No hay transición válida
            break;
        }

        current_state = next_state;
    }

    // Verificar si el estado final es de aceptación
    if (dfa->accepting_states[current_state]) {
        last_accepting = current_state;
        last_accepting_pos = strlen(input);
        if (last_accepting_state) {
            *last_accepting_state = current_state;
        }
    }

    return last_accepting_pos;
}

bool dfa_is_accepting(DFA* dfa, int state) {
    if (!dfa || state < 0 || state >= dfa->num_states) return false;

    return dfa->accepting_states[state];
}

int dfa_get_token_type(DFA* dfa, int state) {
    if (!dfa || state < 0 || state >= dfa->num_states) return -1;

    return dfa->token_types[state];
}

int dfa_get_num_states(DFA* dfa) {
    return dfa ? dfa->num_states : 0;
}

int dfa_get_start_state(DFA* dfa) {
    return dfa ? dfa->start_state : 0;
}

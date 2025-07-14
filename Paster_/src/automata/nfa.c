#include "nfa.h"
#include <stdlib.h>
#include <string.h>

// Implementación de StateSet
StateSet* state_set_create(void) {
    StateSet* set = (StateSet*)malloc(sizeof(StateSet));
    if (!set) return NULL;

    set->capacity = 10;
    set->count = 0;
    set->states = (int*)malloc(set->capacity * sizeof(int));
    if (!set->states) {
        free(set);
        return NULL;
    }

    return set;
}

void state_set_destroy(StateSet* set) {
    if (set) {
        if (set->states) {
            free(set->states);
        }
        free(set);
    }
}

bool state_set_add(StateSet* set, int state) {
    if (!set) return false;

    // Verificar si ya existe
    if (state_set_contains(set, state)) {
        return true;
    }

    // Expandir si es necesario
    if (set->count >= set->capacity) {
        set->capacity *= 2;
        int* new_states = (int*)realloc(set->states, set->capacity * sizeof(int));
        if (!new_states) return false;
        set->states = new_states;
    }

    set->states[set->count++] = state;
    return true;
}

bool state_set_contains(StateSet* set, int state) {
    if (!set) return false;

    for (int i = 0; i < set->count; i++) {
        if (set->states[i] == state) {
            return true;
        }
    }
    return false;
}

void state_set_clear(StateSet* set) {
    if (set) {
        set->count = 0;
    }
}

// Implementación de NFA
NFA* nfa_create(int num_states) {
    NFA* nfa = (NFA*)malloc(sizeof(NFA));
    if (!nfa) return NULL;

    nfa->num_states = num_states;
    nfa->start_state = 0;
    nfa->accepting_states = state_set_create();
    nfa->transitions = NULL;
    nfa->epsilon_transitions = NULL;

    nfa->token_types = (int*)malloc(num_states * sizeof(int));
    if (!nfa->token_types) {
        state_set_destroy(nfa->accepting_states);
        free(nfa);
        return NULL;
    }

    // Inicializar tipos de token a -1 (sin tipo)
    for (int i = 0; i < num_states; i++) {
        nfa->token_types[i] = -1;
    }

    return nfa;
}

void nfa_destroy(NFA* nfa) {
    if (!nfa) return;

    // Liberar transiciones
    NFATransition* trans = nfa->transitions;
    while (trans) {
        NFATransition* next = trans->next;
        free(trans);
        trans = next;
    }

    // Liberar transiciones epsilon
    EpsilonTransition* eps = nfa->epsilon_transitions;
    while (eps) {
        EpsilonTransition* next = eps->next;
        free(eps);
        eps = next;
    }

    state_set_destroy(nfa->accepting_states);
    free(nfa->token_types);
    free(nfa);
}

NFA* nfa_create_symbol(char symbol) {
    NFA* nfa = nfa_create(2);
    if (!nfa) return NULL;

    nfa_add_transition(nfa, 0, symbol, 1);
    nfa_set_accepting(nfa, 1, true);

    return nfa;
}

NFA* nfa_create_range(char from, char to) {
    NFA* nfa = nfa_create(2);
    if (!nfa) return NULL;

    // Añadir transiciones para cada carácter en el rango
    for (char c = from; c <= to; c++) {
        nfa_add_transition(nfa, 0, c, 1);
    }
    nfa_set_accepting(nfa, 1, true);

    return nfa;
}

NFA* nfa_create_any_except(const char* except_chars) {
    NFA* nfa = nfa_create(2);
    if (!nfa) return NULL;

    // Añadir transiciones para todos los caracteres ASCII excepto los especificados
    for (int c = 1; c < 128; c++) {
        bool is_exception = false;
        if (except_chars) {
            for (int i = 0; except_chars[i] != '\0'; i++) {
                if (c == except_chars[i]) {
                    is_exception = true;
                    break;
                }
            }
        }
        if (!is_exception) {
            nfa_add_transition(nfa, 0, (char)c, 1);
        }
    }
    nfa_set_accepting(nfa, 1, true);

    return nfa;
}

void nfa_add_transition(NFA* nfa, int from, char symbol, int to) {
    if (!nfa) return;

    NFATransition* trans = (NFATransition*)malloc(sizeof(NFATransition));
    if (!trans) return;

    trans->from_state = from;
    trans->symbol = symbol;
    trans->to_state = to;
    trans->next = nfa->transitions;
    nfa->transitions = trans;
}

void nfa_add_epsilon_transition(NFA* nfa, int from, int to) {
    if (!nfa) return;

    EpsilonTransition* eps = (EpsilonTransition*)malloc(sizeof(EpsilonTransition));
    if (!eps) return;

    eps->from_state = from;
    eps->to_state = to;
    eps->next = nfa->epsilon_transitions;
    nfa->epsilon_transitions = eps;
}

void nfa_set_accepting(NFA* nfa, int state, bool accepting) {
    if (!nfa) return;

    if (accepting) {
        state_set_add(nfa->accepting_states, state);
    }
}

void nfa_set_token_type(NFA* nfa, int state, int token_type) {
    if (!nfa || state < 0 || state >= nfa->num_states) return;

    nfa->token_types[state] = token_type;
}

void nfa_set_token_type_for_all_accepting(NFA* nfa, TokenType type) {
    if (!nfa || !nfa->accepting_states) return;
    for (int i = 0; i < nfa->accepting_states->count; ++i) {
        int state = nfa->accepting_states->states[i];
        nfa_set_token_type(nfa, state, type);
    }
}

StateSet* nfa_get_accepting_states(NFA* nfa) {
    return nfa ? nfa->accepting_states : NULL;
}

int nfa_get_num_states(NFA* nfa) {
    return nfa ? nfa->num_states : 0;
}

// Operaciones complejas sobre NFAs - implementación básica
NFA* nfa_concatenate(NFA* nfa1, NFA* nfa2) {
    if (!nfa1 || !nfa2) return NULL;

    int total_states = nfa1->num_states + nfa2->num_states;
    NFA* result = nfa_create(total_states);
    if (!result) return NULL;

    // Copiar transiciones de nfa1
    NFATransition* trans = nfa1->transitions;
    while (trans) {
        nfa_add_transition(result, trans->from_state, trans->symbol, trans->to_state);
        trans = trans->next;
    }

    // Copiar transiciones de nfa2 (ajustando estados)
    trans = nfa2->transitions;
    while (trans) {
        nfa_add_transition(result, trans->from_state + nfa1->num_states,
                          trans->symbol, trans->to_state + nfa1->num_states);
        trans = trans->next;
    }

    // Conectar estados de aceptación de nfa1 con el estado inicial de nfa2
    for (int i = 0; i < nfa1->accepting_states->count; i++) {
        int accepting_state = nfa1->accepting_states->states[i];
        nfa_add_epsilon_transition(result, accepting_state, nfa1->num_states + nfa2->start_state);
    }

    // Los estados de aceptación son los de nfa2 (ajustados)
    for (int i = 0; i < nfa2->accepting_states->count; i++) {
        int accepting_state = nfa2->accepting_states->states[i];
        nfa_set_accepting(result, accepting_state + nfa1->num_states, true);
    }

    return result;
}

NFA* nfa_union(NFA* nfa1, NFA* nfa2) {
    if (!nfa1 || !nfa2) return NULL;

    int total_states = nfa1->num_states + nfa2->num_states + 2; // +2 para nuevo inicio y fin
    NFA* result = nfa_create(total_states);
    if (!result) return NULL;

    int new_start = 0;
    int new_final = total_states - 1;

    // Copiar transiciones de nfa1 (ajustando estados +1)
    NFATransition* trans = nfa1->transitions;
    while (trans) {
        nfa_add_transition(result, trans->from_state + 1, trans->symbol, trans->to_state + 1);
        trans = trans->next;
    }

    // Copiar transiciones de nfa2 (ajustando estados +1+nfa1->num_states)
    trans = nfa2->transitions;
    while (trans) {
        nfa_add_transition(result, trans->from_state + 1 + nfa1->num_states,
                          trans->symbol, trans->to_state + 1 + nfa1->num_states);
        trans = trans->next;
    }

    // Conectar nuevo inicio con los inicios de nfa1 y nfa2
    nfa_add_epsilon_transition(result, new_start, 1); // nfa1 start
    nfa_add_epsilon_transition(result, new_start, 1 + nfa1->num_states); // nfa2 start

    // Conectar estados de aceptación con el nuevo final
    for (int i = 0; i < nfa1->accepting_states->count; i++) {
        int accepting_state = nfa1->accepting_states->states[i];
        nfa_add_epsilon_transition(result, accepting_state + 1, new_final);
    }

    for (int i = 0; i < nfa2->accepting_states->count; i++) {
        int accepting_state = nfa2->accepting_states->states[i];
        nfa_add_epsilon_transition(result, accepting_state + 1 + nfa1->num_states, new_final);
    }

    nfa_set_accepting(result, new_final, true);

    return result;
}

NFA* nfa_kleene_star(NFA* nfa) {
    if (!nfa) return NULL;

    int total_states = nfa->num_states + 2;
    NFA* result = nfa_create(total_states);
    if (!result) return NULL;

    int new_start = 0;
    int new_final = total_states - 1;

    // Copiar transiciones (ajustando estados +1)
    NFATransition* trans = nfa->transitions;
    while (trans) {
        nfa_add_transition(result, trans->from_state + 1, trans->symbol, trans->to_state + 1);
        trans = trans->next;
    }

    // Epsilon transitions para Kleene star
    nfa_add_epsilon_transition(result, new_start, 1); // inicio -> nfa_start
    nfa_add_epsilon_transition(result, new_start, new_final); // inicio -> final (empty string)

    for (int i = 0; i < nfa->accepting_states->count; i++) {
        int accepting_state = nfa->accepting_states->states[i];
        nfa_add_epsilon_transition(result, accepting_state + 1, new_final); // nfa_final -> final
        nfa_add_epsilon_transition(result, accepting_state + 1, 1); // nfa_final -> nfa_start (repeat)
    }

    nfa_set_accepting(result, new_final, true);

    return result;
}

NFA* nfa_kleene_plus(NFA* nfa) {
    if (!nfa) return NULL;

    NFA* star = nfa_kleene_star(nfa);
    if (!star) return NULL;

    return nfa_concatenate(nfa, star);
}

NFA* nfa_optional(NFA* nfa) {
    if (!nfa) return NULL;

    int total_states = nfa->num_states + 2;
    NFA* result = nfa_create(total_states);
    if (!result) return NULL;

    int new_start = 0;
    int new_final = total_states - 1;

    // Copiar transiciones (ajustando estados +1)
    NFATransition* trans = nfa->transitions;
    while (trans) {
        nfa_add_transition(result, trans->from_state + 1, trans->symbol, trans->to_state + 1);
        trans = trans->next;
    }

    // Epsilon transitions para optional
    nfa_add_epsilon_transition(result, new_start, 1); // inicio -> nfa_start
    nfa_add_epsilon_transition(result, new_start, new_final); // inicio -> final (skip)

    for (int i = 0; i < nfa->accepting_states->count; i++) {
        int accepting_state = nfa->accepting_states->states[i];
        nfa_add_epsilon_transition(result, accepting_state + 1, new_final); // nfa_final -> final
    }

    nfa_set_accepting(result, new_final, true);

    return result;
}

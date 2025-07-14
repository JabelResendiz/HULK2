#include "nfa_to_dfa.h"
#include <stdlib.h>
#include <string.h>

// Implementación simplificada - versión básica
DFA* nfa_to_dfa_convert(NFA* nfa) {
    if (!nfa) return NULL;

    // Para simplificar, creamos un DFA básico que simula el comportamiento del NFA
    // En una implementación completa, aquí estaría el algoritmo de construcción de subconjuntos

    DFA* dfa = dfa_create(nfa->num_states * 2); // Estimación conservadora
    if (!dfa) return NULL;

    // Copiar transiciones básicas (simplificado)
    NFATransition* trans = nfa->transitions;
    while (trans) {
        dfa_add_transition(dfa, trans->from_state, trans->symbol, trans->to_state);
        trans = trans->next;
    }

    // Copiar estados de aceptación
    for (int i = 0; i < nfa->accepting_states->count; i++) {
        int accepting_state = nfa->accepting_states->states[i];
        dfa_set_accepting(dfa, accepting_state, true);

        // Copiar tipo de token si existe
        if (nfa->token_types[accepting_state] != -1) {
            dfa_set_token_type(dfa, accepting_state, nfa->token_types[accepting_state]);
        }
    }

    dfa_set_start_state(dfa, nfa->start_state);

    return dfa;
}

StateSet* epsilon_closure(NFA* nfa, StateSet* states) {
    if (!nfa || !states) return NULL;

    StateSet* closure = state_set_create();
    if (!closure) return NULL;

    // Agregar estados iniciales
    for (int i = 0; i < states->count; i++) {
        state_set_add(closure, states->states[i]);
    }

    // Agregar estados alcanzables por transiciones epsilon
    bool changed = true;
    while (changed) {
        changed = false;
        EpsilonTransition* eps = nfa->epsilon_transitions;
        while (eps) {
            if (state_set_contains(closure, eps->from_state) &&
                !state_set_contains(closure, eps->to_state)) {
                state_set_add(closure, eps->to_state);
                changed = true;
            }
            eps = eps->next;
        }
    }

    return closure;
}

StateSet* move(NFA* nfa, StateSet* states, char symbol) {
    if (!nfa || !states) return NULL;

    StateSet* result = state_set_create();
    if (!result) return NULL;

    // Para cada estado en el conjunto
    for (int i = 0; i < states->count; i++) {
        int state = states->states[i];

        // Buscar transiciones con el símbolo dado
        NFATransition* trans = nfa->transitions;
        while (trans) {
            if (trans->from_state == state && trans->symbol == symbol) {
                state_set_add(result, trans->to_state);
            }
            trans = trans->next;
        }
    }

    return result;
}

char* get_alphabet(NFA* nfa) {
    if (!nfa) return NULL;

    // Simplificación: retornar un alfabeto básico
    // En una implementación completa, esto extraería todos los símbolos únicos del NFA
    char* alphabet = (char*)malloc(256 * sizeof(char));
    if (!alphabet) return NULL;

    int count = 0;
    for (int c = 1; c < 128; c++) {
        alphabet[count++] = (char)c;
    }
    alphabet[count] = '\0';

    return alphabet;
}

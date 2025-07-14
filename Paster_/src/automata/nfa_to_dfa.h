#ifndef NFA_TO_DFA_H
#define NFA_TO_DFA_H

#include "nfa.h"
#include "dfa.h"

// Función principal para convertir NFA a DFA
DFA* nfa_to_dfa_convert(NFA* nfa);

// Funciones auxiliares para la conversión
StateSet* epsilon_closure(NFA* nfa, StateSet* states);
StateSet* move(NFA* nfa, StateSet* states, char symbol);
char* get_alphabet(NFA* nfa);

#endif // NFA_TO_DFA_H

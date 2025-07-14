

#ifndef LLVM_SCOPE_H
#define LLVM_SCOPE_H

#include <llvm-c/Core.h>
#include "../type_value/type.h"

/// @brief estructura para almacenear variables en un ambito
typedef struct ScopeVarEntry {
    char* name;               // Nombre de la variable (ej: "x", "count").
    LLVMValueRef alloca;      // Referencia a la instrucción LLVM que reserva memoria (`alloca`).
    struct ScopeVarEntry* next; // Puntero al siguiente nodo (para manejar colisiones en una lista enlazada).
} ScopeVarEntry;


/// @brief estrucutra para representar un scope 
typedef struct LLVMScope {
    ScopeVarEntry* entries;  // Lista de variables en este ámbito.
    struct LLVMScope* parent; // Puntero al ámbito padre (para anidamiento, como en funciones/bloques).
} LLVMScope;




void print_scope();

// Crea un nuevo ámbito hijo.
void push_scope(void);           

// Elimina el ámbito actual (vuelve al padre).
void pop_scope(void);                   

// Busca una variable en los ámbitos (actual y padres).
LLVMValueRef lookup_variable(const char* name); 

// Añade una variable al ámbito actual.
void declare_variable(const char* name, LLVMValueRef alloca); 

// Actualiza el valor de una variable.
void update_variable(const char* name, LLVMValueRef new_alloca); 


#endif
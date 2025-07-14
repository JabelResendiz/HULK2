
//llvm_scope.c

#include "llvm_scope.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// apuntar al ambito activo (el mas interno en al jerarquia anidada)
static LLVMScope* current_scope  = NULL;


void print_scope() {
    LLVMScope* scope = current_scope;
    int depth = 0;
    
    printf("Current Scope Hierarchy:\n");
    printf("------------------------\n");
    
    while (scope != NULL) {
        printf("Scope %d (Level: %d)\n", depth, depth);
        printf("-------------------\n");
        
        // obtiene las variables del scope
        ScopeVarEntry* entry = scope->entries;

        if (entry == NULL) {
            printf("  (empty)\n");
        } 
        
        else {
            while (entry != NULL) { // recorre la lisat de varaibles
                printf("  Variable: %-15s Alloca: %p\n", 
                      entry->name, (void*)entry->alloca);
                entry = entry->next;
            }
        }
        
        // sube al ambito padre
        scope = scope->parent;
        depth++;
        printf("\n");
    }
    
    printf("End of Scope Chain\n");
    printf("============\n");
}



void push_scope(void)
{
    LLVMScope* new_scope = malloc(sizeof(LLVMScope));
    new_scope->entries = NULL;
    new_scope->parent = current_scope;
    current_scope = new_scope;
}


void pop_scope(void)
{
    if(!current_scope) return;

    ScopeVarEntry* current = current_scope->entries;

    // Libera todas las variables del ámbito actual.
    while(current)
    {
        ScopeVarEntry* next = current->next;
        free(current->name);     
        free(current);          
        current = next;
    }

    // Elimina el ámbito y vuelve al padre.
    LLVMScope* parent = current_scope->parent;
    free(current_scope);
    current_scope = parent;
}

LLVMValueRef lookup_variable(const char* name) {
    LLVMScope* scope = current_scope;
    while (scope) {
        ScopeVarEntry* entry = scope->entries;
        while (entry) {
            if (strcmp(entry->name, name) == 0) {
                return entry->alloca;    // // Devuelve la referencia LLVM si encuentra la variable.
            }
            entry = entry->next;
        }

        // Sube al ámbito padre.
        scope = scope->parent;
    }

    // No encontró la variable.
    return NULL;
}



void declare_variable(const char* name, LLVMValueRef alloca) {
    ScopeVarEntry* entry = malloc(sizeof(ScopeVarEntry));
    entry->name = strdup(name);
    entry->alloca = alloca;
    entry->next = current_scope->entries;
    current_scope->entries = entry;
}

void update_variable(const char* name, LLVMValueRef new_alloca) {
    LLVMScope* scope = current_scope;
    while (scope) {
        ScopeVarEntry* entry = scope->entries;
        while (entry) {
            if (strcmp(entry->name, name) == 0) {
                entry->alloca = new_alloca;
                return;
            }
            entry = entry->next;
        }
        scope = scope->parent;
    }
}
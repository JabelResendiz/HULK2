#pragma once
#include <stddef.h>
#include <stdbool.h>
#include "token/token.h"

// Estructura para representar un nodo del árbol de derivación en C puro
typedef struct DerivationNode {
    char* symbol; // No terminal o terminal
    struct DerivationNode** children;
    size_t child_count;
    Token* token; // Solo para terminales (NULL si no es terminal)
    int line_number;
    int column_number;
} DerivationNode;

// Funciones para crear y manipular nodos
DerivationNode* derivation_node_create(const char* symbol);
DerivationNode* derivation_node_create_with_token(const char* symbol, Token* token);
void derivation_node_set_token(DerivationNode* node, Token* token);
const char* derivation_node_get_symbol(const DerivationNode* node);
void derivation_node_add_child(DerivationNode* parent, DerivationNode* child);
void derivation_node_print(const DerivationNode* node, int depth);
void derivation_node_destroy(DerivationNode* node);

// Tipo para puntero a nodo (equivalente a unique_ptr)
typedef DerivationNode* DerivationNodePtr;
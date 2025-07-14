#include "derivation_tree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

DerivationNode* derivation_node_create(const char* symbol) {
    DerivationNode* node = (DerivationNode*)malloc(sizeof(DerivationNode));
    if (!node) return NULL;
    
    node->symbol = strdup(symbol);
    node->children = NULL;
    node->child_count = 0;
    node->token = NULL;
    node->line_number = 0;
    node->column_number = 0;
    return node;
}

DerivationNode* derivation_node_create_with_token(const char* symbol, Token* token) {
    DerivationNode* node = derivation_node_create(symbol);
    if (!node) return NULL;
    
    node->token = token;
    if (token) {
        node->line_number = token->line;
        node->column_number = token->column;
    }
    return node;
}

void derivation_node_set_token(DerivationNode* node, Token* token) {
    if (!node || !token) return;
    node->token = token;
    node->line_number = token->line;
    node->column_number = token->column;
}

const char* derivation_node_get_symbol(const DerivationNode* node) {
    return node ? node->symbol : NULL;
}

void derivation_node_add_child(DerivationNode* parent, DerivationNode* child) {
    if (!parent || !child) return;
    
    parent->children = (DerivationNode**)realloc(parent->children, 
                                               sizeof(DerivationNode*) * (parent->child_count + 1));
    if (!parent->children) return;
    
    parent->children[parent->child_count++] = child;
}

void derivation_node_print(const DerivationNode* node, int depth) {
    if (!node) return;
    
    for (int i = 0; i < depth; ++i) {
        printf("  ");
    }
    printf("%s", node->symbol);
    
    if (node->token) {
        printf(" ('%s')", node->token->lexeme);
    }
    printf("\n");
    
    for (size_t i = 0; i < node->child_count; ++i) {
        derivation_node_print(node->children[i], depth + 1);
    }
}

void derivation_node_destroy(DerivationNode* node) {
    if (!node) return;
    
    free(node->symbol);
    
    for (size_t i = 0; i < node->child_count; ++i) {
        derivation_node_destroy(node->children[i]);
    }
    
    free(node->children);
    free(node);
} 
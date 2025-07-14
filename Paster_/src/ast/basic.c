

#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Added for strcpy and strlen

ASTNode *create_program_node(ASTNode **statements,
                             int count,
                             ASTNodeType type)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = type;
    node->scope = create_scope(NULL);
    node->env = create_env(NULL);
    node->data.program.statements = malloc(sizeof(ASTNode *) * 1000); // Usar 1000 como capacidad fija
    node->data.program.count = count;
    for (int i = 0; i < count; i++)
    {
        node->data.program.statements[i] = statements[i];
    }
    return node;
}

ASTNode *create_num_node(double value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = AST_NUM;
    node->scope = create_scope(NULL);
    node->env = create_env(NULL);
    node->computed_type = &TYPE_NUM;
    node->data.number_value = value;

    return node;
}

ASTNode *create_var_node(char *name, char *type, int param)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = AST_VAR;
    node->scope = create_scope(NULL);
    node->env = create_env(NULL);
    node->computed_type = &TYPE_OBJ;
    
    // Duplicar el nombre de la variable para evitar problemas de memoria
    if (name) {
        size_t len = strlen(name);
        node->data.var_name = malloc(len + 1);
        if (node->data.var_name) {
            strcpy(node->data.var_name, name);
        } else {
            node->data.var_name = NULL;
        }
    } else {
        node->data.var_name = NULL;
    }
    
    node->param = param;

    if(type)
        node->static_type = type;

    fprintf(stderr,"el nombre de la variable es %s y es un parametro %d\n", name,param);

    return node;
}

ASTNode *create_string_node(char *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = AST_STRING;
    node->scope = create_scope(NULL);
    node->env = create_env(NULL);
    node->computed_type = &TYPE_STRING;
    
    // Duplicar el valor del string para evitar problemas de memoria
    if (value) {
        size_t len = strlen(value);
        node->data.string_value = malloc(len + 1);
        if (node->data.string_value) {
            strcpy(node->data.string_value, value);
        } else {
            node->data.string_value = NULL;
        }
    } else {
        node->data.string_value = NULL;
    }

    return node;
}

ASTNode *create_boolean_node(char* value)
{
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = AST_BOOLEAN;
    node->scope = create_scope(NULL);
    node->env = create_env(NULL);
    node->computed_type = &TYPE_BOOLEAN;
    
    // Duplicar el valor booleano para evitar problemas de memoria
    if (value) {
        size_t len = strlen(value);
        node->data.string_value = malloc(len + 1);
        if (node->data.string_value) {
            strcpy(node->data.string_value, value);
        } else {
            node->data.string_value = NULL;
        }
    } else {
        node->data.string_value = NULL;
    }

    return node;
}

ASTNode* create_return_stmt(ASTNode* value)
{
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = AST_RETURN;
    node->scope = create_scope(NULL);
    node->env = create_env(NULL);
    node->computed_type = &TYPE_VOID;
    node->data.return_stmt.value = value;

    return node;
}
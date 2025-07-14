
#include "ast.h"
#include <stdlib.h>
#include <stdio.h>


ASTNode *create_type_node(char *name_type,
                          char *name_parent,
                          ASTNode **args,
                          int args_count,
                          ASTNode **p_args,
                          int p_args_count,
                          ASTNode **body,
                          int body_count,
                          int p_constructor)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = AST_TYPE;
    node->checked = 0;
    node->computed_type = &TYPE_VOID;
    node->scope = create_scope(NULL);
    node->env = create_env(NULL);
    node->data.typeDef.name_type = name_type;
    node->data.typeDef.name_parent = name_parent;
    node->data.typeDef.args_count = args_count;
    node->data.typeDef.p_args_count = p_args_count;
    node->data.typeDef.body_count = body_count;

    node->data.typeDef.p_args = malloc(sizeof(ASTNode *) * p_args_count);
    for (int i = 0; i < p_args_count; i++)
    {
        node->data.typeDef.p_args[i] = p_args[i];
    }

    node->data.typeDef.args = malloc(sizeof(ASTNode *) * args_count);
    for (int i = 0; i < args_count; i++)
    {
        node->data.typeDef.args[i] = args[i];
    }

    node->data.typeDef.body_elements = malloc(sizeof(ASTNode *) * body_count);
    for (int i = 0; i < body_count; i++)
    {
        node->data.typeDef.body_elements[i] = body[i];
    }

    node->data.typeDef.parent_instance = NULL;
    node->data.typeDef.id =0;
    node->data.typeDef.p_constructor = p_constructor;
    node->usages = NULL;

    
    return node;
}

// getX()=> self.x = x;
ASTNode *create_property_access_node(ASTNode *instance,
                                     ASTNode *property)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = AST_GETTER;
    node->scope = create_scope(NULL);
    node->env = create_env(NULL);
    node->data.binary_op.left = instance;
    node->data.binary_op.right = property;

    return node;
}

// new Point(2,3);
ASTNode *create_struct_instance_node(char *name_type,
                                     ASTNode **args,
                                     int args_count)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = AST_INSTANCE;
    node->scope = create_scope(NULL);
    node->env = create_env(NULL);
    node->data.typeDef.name_type = name_type;
    node->data.typeDef.args = malloc(sizeof(ASTNode *) * args_count);
    node->data.typeDef.args_count = args_count;

    for (int i = 0; i < args_count; i++)
    {
        node->data.typeDef.args[i] = args[i];
    }
    node->data.typeDef.parent_instance = NULL;
    node->usages = NULL;
    return node;
}


// SEETER
ASTNode *create_property_assignment_node(ASTNode *instance,
                                         ASTNode *property,
                                         ASTNode *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = AST_SETTER;
    node->computed_type = &TYPE_OBJ;
    node->scope = create_scope(NULL);
    node->env = create_env(NULL);
    node->data.setter.instance = instance;
    node->data.setter.property = property;
    node->data.setter.value = value;
    return node;
}

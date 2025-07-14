

#include "ast.h"
#include <stdlib.h>
#include <stdio.h>

ASTNode *create_conditional_node(ASTNode *cond,
                                 ASTNode *body,
                                 ASTNode *else_branch)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = AST_IF;
    node->computed_type = &TYPE_OBJ;
    node->scope = create_scope(NULL);
    node->env = create_env(NULL);
    node->data.conditional.cond = cond;
    node->data.conditional.body = body;
    node->data.conditional.else_branch = else_branch;
    node->usages = add_usages(body,NULL);
    node->usages = add_usages(else_branch,node->usages);

    return node;
}



ASTNode *create_while_node(ASTNode *cond, ASTNode *body)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = AST_WHILE;
    node->computed_type = &TYPE_OBJ;
    node->scope = create_scope(NULL);
    node->env = create_env(NULL);
    node->data.binary_op.left = cond;
    node->data.binary_op.right = body;
    node->usages = add_usages(body,NULL);

    fprintf(stderr, "estoy en el node de while");
    return node;
}
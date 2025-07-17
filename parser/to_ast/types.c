
#include "../cst_to_ast.h"

ASTNode *typeDef_to_ast(CSTNode *cst_node)
{
    for (int i = 0; i < cst_node->child_count; i++)
    {
        CSTNode *child = cst_node->children[i];
        if (!strcmp(child->symbol, "ID") && child->token)
        {
            if (!type_name)
            {
                type_name = strdup(child->token->value);
            }
            else if (!parent_name)
            {
                parent_name = strdup(child->token->value);
            }
        }
        else if (!strcmp(child->symbol, "ParamList"))
        {
            // Procesa parámetros del tipo
        }
        else if (!strcmp(child->symbol, "ParentParams"))
        {
            // Procesa parámetros del padre
        }
        else if (!strcmp(child->symbol, "Block") || !strcmp(child->symbol, "Body"))
        {
            body = block_to_ast(child);
        }
    }
}
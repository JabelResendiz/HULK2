

#include "usage.h"
#include <stdlib.h>
#include "../ast/ast.h"
#include <string.h>

// agrega el nodo al usage y enlaza con la que esta
Usage *add_usages(struct ASTNode *node, Usage *list)
{
    if (!list)
    {
        list = malloc(sizeof(Usage));
        list->first = NULL;
        list->count = 0;
    }

    UsageElemental *element = malloc(sizeof(UsageElemental));

    element->node = node;
    element->next = list->first;

    list->first = element;
    list->count++;

    return list;
}




Usage *find_type_by_method(Env *env, char *name)
{
    Usage *current = NULL;

    while (env)
    {
        int i = 0;
        EnvItem *item = env->start;

        while (i < env->env_count)
        {
            if (item->usages->type == AST_TYPE)
            {
                if (type_contains_method_in_context(item->usages, name))
                {
                    current = add_usages(item->usages, current);
                }
            }
            i++;
            item = item->next;
        }

        env = env->parent;
    }

    return current;
}

int type_contains_method_in_context(ASTNode *node, char *name)
{
    for (int i = 0; i < node->data.typeDef.body_count; i++)
    {
        ASTNode *def = node->data.typeDef.body_elements[i];
        if (def->type == AST_DECL_FUNC &&
            (!strcmp(def->data.func_node.name, name) ||
             !strcmp(def->data.func_node.name, generate_underscored_name(
                                                   node->data.typeDef.name_type, name))))
        {
            return 1;
        }
    }

    return 0;
}

Function *find_function_by_name_1(Scope *scope, char *name, int see)
{
    if (!scope)
    {
        return NULL;
    }

    Function *current = scope->functions->first;
    int i = 0;
    while (i < scope->functions->count)
    {
        if (!strcmp(current->name, name))
        {
            return current;
        }
        current = current->next;
        i++;
    }

    if (see && scope->parent)
    {
        return find_function_by_name_1(scope->parent, name, see);
    }

    return NULL;
}

int type_contains_method_in_scope(TypeValue *type, char *name, int see_parent)
{
    if (!type->def_node)
        return 0;

    char *tmp_name = generate_underscored_name(
        type->name, name);

    if (find_function_by_name_1(type->def_node->scope, tmp_name, 0))
        return 1;

    if (see_parent)
        return type_contains_method_in_scope(type->super_type, name, see_parent);

    return 0;
}



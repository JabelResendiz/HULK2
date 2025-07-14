

#include "unifiedIndex.h"
#include <stdlib.h>
#include <string.h>

#define foreach_valueList(elem, list) \
    for (UsageElemental *elem = (list) ? (list)->first : NULL; elem != NULL; elem = elem->next)

void free_unified_index(UnifiedIndex *u)
{
    if (!u)
        return;
    free_unified_index(u->next);
    free(u);
}

/// @brief intenta unifcar un operando con un cierto tipo
/// @param v
/// @param node el nodo a unificar
/// @param type el tipo unificador
/// @return
int try_unify_operand(ASTVisitor *v,
                      ASTNode *node,
                      TypeValue *type)
{
    if (type)
    {
        fprintf(stderr, "RESENDIZ\n");
    }

    int unified = 0;

    if (node->type == AST_VAR && node->param == 1)
    {
        fprintf(stderr, "Estoy en try_unify_operand\n");
        fprintf(stderr, "El valor COMPUTADO de node left es %s\n", node->computed_type->name);

        // busca le simbolo del parametro
        Symbol *s = lookup_parameter(node->scope, node->data.var_name);

        // si no existe el simbolo retorna 0
        if (!s)
            return 0;

        // Si el tipo del simbolo no se ha detectado o el simbolo es ancestro
        if (compare_types(s->type, &TYPE_GENERIC) ||
            ancestor_type(s->type, type))
        {
            // actualiza el tipo del simbolo y del nodo
            s->type = type;
            node->computed_type = type;
            fprintf(stderr, "El valor COMPUTADO de node left es %s\n", node->computed_type->name);

            // Propagar a sus usages
        }

        // si el type no es ancestro
        else if (!ancestor_type(type, s->type))
        {
            fprintf(stderr, "el tipo del nodo no se puede unficar con el tipo requerido\n");
            return 0;
        }

        unified = 1;
    }

    // si es una llamada a funcion y ademas su tipo no se ha comprobado aun
    else if (node->type == AST_CALL_FUNC && compare_types(node->computed_type, &TYPE_GENERIC))
    {
        fprintf(stderr, "estoy dentro de una llamada de funcion\n");

        // buscar en el contexto

        EnvItem *env_item = find_env_item(node->env, node->data.func_node.name, 0, 0);

        if (env_item)
        {
            env_item->computed_type = type;
            node->computed_type = type;
            unified = 1;
        }
    }

    // si es una condicional el nodo
    else if (node->type == AST_IF)
    {
        unified = try_unify_conditional(v, node, type);
    }

    // si hay derivaciones

    else if (node->usages)
    {
        fprintf(stderr, "Hay derivaciones en juego\n");

        foreach_valueList(der, node->usages)
        {
            ASTNode *use_node = der->node;

            if (use_node && compare_types(use_node->computed_type, &TYPE_GENERIC))
            {
                int u_member = try_unify_operand(v, use_node, type);

                if (u_member)
                {
                    use_node->computed_type = type;
                }

                unified |= u_member;
            }
        }

        if (unified)
        {
            if (node->type == AST_VAR)
            {
                Symbol *s = scope_lookup(node->scope, node->data.var_name);

                if (s)
                {
                    s->type = type;
                }
            }

            node->computed_type = type;
        }
    }
    fprintf(stderr, "En unify member el valor de unified es %d\n", unified);

    return unified;
}

/// @brief intentar unificar los tipos de left y right
/// @param v
/// @param left
/// @param right
/// @param op
/// @param op_name
/// @return
int try_unify_binary_op(ASTVisitor *v, ASTNode *left,
                        ASTNode *right, Operator op,
                        char *op_name)
{
    int unified = 0;
    int count = 0;
    OperatorRule *rule;

    if (right && compare_types(left->computed_type, &TYPE_GENERIC) && compare_types(right->computed_type, &TYPE_GENERIC))
    {
        count = find_rule(op, &rule, 0, NULL);

        if (count > 1)
        {
            return 0;
        }

        if (count > 0)
        {
            // si se encontro un operador valido

            // intenta que el nodo adopte el tipo dado de la regla
            int left_unified = try_unify_operand(v, left, rule->left_type);

            // intenta que el nodo adopte el tipo dado de la regla
            int right_unified = try_unify_operand(v, right, rule->right_type);

            // si ambos lograron unificar entonces retorna 3 sino 0
            unified = (left_unified && right_unified) ? 3 : 0;
        }
    }

    // SOLO SI EL LADO IZQUIERDO ES GENERIC

    else if (compare_types(left->computed_type, &TYPE_GENERIC))
    {
        fprintf(stderr, "1111111111111111111\n");
        count = find_rule(op, &rule, 1, right);
        fprintf(stderr, "el valor de count es %d\n", count);

        if (count > 1)
        {
            return 0;
        }

        if (count > 0)
        {
            int left_unified = try_unify_operand(v, left, rule->left_type);

            unified = (left_unified) ? 2 : 0;
        }

        else if (!compare_types(right->computed_type, &TYPE_ERROR))
        {
            fprintf(stderr, "Opeardores no pueden ser usados como lado derecho\n");
            exit(1);
        }
    }

    // SOLO SI EL LADO DERECHO ES GENERIC

    else if (right && compare_types(right->computed_type, &TYPE_GENERIC))
    {
        count = find_rule(op, &rule, 2, left);

        if (count > 1)
        {
            return 0;
        }

        if (count > 0)
        {
            int right_unified = try_unify_operand(v, right, rule->right_type);

            unified = (right_unified) ? 1 : 0;
        }

        else if (!compare_types(left->computed_type, &TYPE_ERROR))
        {
            fprintf(stderr, "Opeardores no pueden ser usados como lado dizquierod\n");
            exit(1);
        }
    }

    return unified;
}

/// @brief Unificafor de condicionales
/// @param v El vistor del AST que contiene el entorno actual de analisis
/// @param node El nodo es de tipo Condicional
/// @param type El tipo con el que se debe intentar unificar el tipo de retorno del condicional
/// @return
int try_unify_conditional(ASTVisitor *v,
                          ASTNode *node,
                          TypeValue *type)
{
    // si se intenta unficar con GENERIC, el condicional tomo como tipo de retorno OBJ
    if (compare_types(type, &TYPE_GENERIC))
    {
        node->computed_type = &TYPE_OBJ;
        // no se hizo una unificacion real
        return 0;
    }

    int a = try_unify_operand(v, node->data.conditional.body, type);
    int b = (node->data.conditional.else_branch)
                ? try_unify_operand(v, node->data.conditional.else_branch, type)
                : 0;

    // se intenta unificar el tipo con la rama verdadera
    // se hace lo mismo con la rama else_branch
    // se tiene exito si alguna de las dos ramas intenta unificar
    return a || b;
}

UnifiedIndex *try_unified_type(ASTVisitor *v,
                               ASTNode **args,
                               Scope *scope,
                               int arg_count,
                               const char *t_name,
                               EnvItem *item)
{
    int count = 0;
    UnifiedIndex *unified = NULL;

    TypeValue *t;

    while (scope)
    {
        if (scope->types)
        {
            Symbol *curr = scope->types;
            int i = 0;

            while (i < scope->t_count)
            {
                if (!strcmp(t_name, curr->name) &&
                    arg_count == curr->type->num_params)
                {
                    count++;
                    if (count > 1)
                        return NULL;

                    t = curr->type;
                }
                curr = curr->next;
                i++;
            }
        }

        scope = scope->parent;
    }

    if (count)
    {
        for (int i = 0; i < arg_count; i++)
        {
            if (compare_types(&TYPE_GENERIC, args[i]->computed_type))
            {
                if (try_unify_operand(v, args[i], t->argument_types[i]))
                {
                    unified = add_unified_index(unified, i);
                }
                else
                {
                    return NULL;
                }
            }
        }
    }

    else if (item && item->usages->data.typeDef.args_count == arg_count)
    {
        for (int i = 0; i < item->usages->data.typeDef.args_count; i++)
        {
            if (try_unify_operand(
                    v, item->usages->data.typeDef.args[i],
                    args[i]->computed_type))
            {
                unified = add_unified_index(unified, i);
            }
            else
            {
                return NULL;
            }
        }
    }

    return unified;
}

UnifiedIndex *try_unified_func(ASTVisitor *v,
                               ASTNode **args,
                               Scope *scope,
                               int arg_count,
                               const char *f_name,
                               EnvItem *item)
{
    int count = 0;
    UnifiedIndex *unified = NULL;
    Function *f;

    while (scope)
    {

        Function *curr = scope->functions->first;

        int i = 0;

        while (i < scope->functions->count)
        {
            if (!strcmp(f_name, curr->name) &&
                arg_count == curr->count_args)
            {
                count++;
                if (count > 1)
                    return NULL;
                f = curr;
            }
            curr = curr->next;
            i++;
        }

        scope = scope->parent;
    }

    if (count)
    {
        for (int i = 0; i < arg_count; i++)
        {
            if (compare_types(&TYPE_GENERIC, args[i]->computed_type))
            {
                if (try_unify_operand(v, args[i], f->args_types[i]))
                {
                    unified = add_unified_index(unified, i);
                }
                else
                {
                    return NULL;
                }
            }
        }
    }

    else if (item && item->usages->data.func_node.arg_count == arg_count)
    {
        for (int i = 0; i < item->usages->data.func_node.arg_count; i++)
        {
            if (try_unify_operand(v, item->usages->data.func_node.args[i],
                                  args[i]->computed_type))
            {
                unified = add_unified_index(unified, i);
            }

            else
                return NULL;
        }
    }

    return unified;
}

UnifiedIndex *add_unified_index(UnifiedIndex *list, int index)
{
    UnifiedIndex *e = (UnifiedIndex *)malloc(sizeof(UnifiedIndex));
    e->value = index;
    e->next = list;

    return e;
}

int try_unify_attr(ASTVisitor *v, ASTNode *node)
{
    ASTNode *instance = node->data.binary_op.left;
    ASTNode *member = node->data.binary_op.right;

    int unified = 0;

    if (member->type == AST_VAR)
        return 1;

    Usage *types = find_type_by_method(node->env, member->data.func_node.name);

    TypeValue *type = NULL;
    Symbol *struct_type = NULL;

    foreach_valueList(der, types)
    {
        ASTNode * dec = der->node;

        accept(v,dec);

        struct_type = find_type_scopes(node->scope,dec->data.typeDef.name_type);

        if(struct_type)
        {
            if(!type)
            {
                type = struct_type->type;
            }

            else{
                TypeValue* tmp = compute_lca(type,struct_type->type);

                if(type_contains_method_in_scope(tmp,member->data.func_node.name,1))
                {
                    type = tmp;
                }
                else return 0;
            }
        }
    }

    if(type)
    {
        unified = try_unify_operand(v,instance,type);

        if(unified)
        {
            instance->computed_type = type;
        }
    }

    return unified;
}
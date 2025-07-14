
// variable_semantic.c

#include "check_semantic.h"
#include "../ast/keyword.h"
#include "../scope/unifiedIndex.h"
#include <string.h>

#define foreach_valueList(elem, list) \
    for (UsageElemental *elem = (list) ? (list)->first : NULL; elem != NULL; elem = elem->next)

void visit_let(ASTVisitor *v, ASTNode *node)
{
    ASTNode **bindings = node->data.func_node.args;
    ASTNode *body = node->data.func_node.body;

    if (body) {
        propagate_env_scope(node, body);
    }

    // revisar los bindings del bloque let
    for (int i = 0; i < node->data.func_node.arg_count; i++)
    {
        if (bindings[i]) {
            propagate_env_scope(node, bindings[i]);
            accept(v, bindings[i]);
        }
    }

    // visitar el cuerpo del cuerpo
    if (body) {
        accept(v, body);
        // el tipo del bloque let es el tipo del cuerpo
        node->computed_type = resolve_node_type(body);
    } else {
        node->computed_type = &TYPE_VOID;
    }
}

void visit_assignment(ASTVisitor *v, ASTNode *node)
{
    ASTNode *left = node->data.binary_op.left;
    ASTNode *right = node->data.binary_op.right;

    if (match_keyword(left->data.var_name))
    {
        message_semantic_error(v, "La variable de nombre '%s' es una palabra clave, no puede ser usada para nombrar variables. line %d",
                               left->data.var_name, node->line);
    }

    propagate_env_scope(node, left);
    propagate_env_scope(node, right);

    // buscar el tipo definido (estatico de la variable)
    Symbol *static_types = find_type_scopes(node->scope, left->static_type);

    // flag para saber si luego habra que liberar memorai
    int flag = 0;

    // si el tipo no existe, intentara inferirlo en el entorno global
    // tipo el usuario a:T pero T aun no ha sido resuelto
    if (strcmp(left->static_type, "") && !static_types)
    {
        EnvItem *env_item = find_env_item(node->env, left->data.var_name, 1, 0);

        if (env_item)
        {
            // encontro el tipo en entorno, y asegura entregarlo al entorno actual
            accept(v, env_item->usages);

            // intentara buscarlo nuevamente el tipo en el scope
            static_types = find_type_scopes(node->scope, left->static_type);

            if (!static_types)
            {
                static_types = malloc(sizeof(Symbol));

                if (env_item->computed_type)
                {
                    static_types->name = env_item->computed_type->name;
                    static_types->type = env_item->computed_type;
                }

                flag = 1;
            }
        }

        // no se encontro en el contexto ERROR

        else
        {
            message_semantic_error(v, "La variable '%s' fue definida con tipo '%s' que no es valido. Linea %d",
                                   left->data.var_name, left->static_type, node->line);
        }
    }

    // se visita el lado derecho
    accept(v, right);

    // se busca resolver el tipo del lado
    TypeValue *inferred_type = resolve_node_type(right);

    // si el tipo aun es GENERIC , pero hay un tipo definido (static_type), se intenta unificar el tipo del valor con el esperado
    if (compare_types(inferred_type, &TYPE_GENERIC) &&
        static_types && try_unify_operand(v, right, static_types->type))
    {

        // se vuelve a revisar el nodo y buscar el tipo real
        accept(v, right);
        inferred_type = resolve_node_type(right);
    }

    // si existe un tipo asociado y no es ancestro del tipo inferido
    if (static_types && !ancestor_type(static_types->type, inferred_type))
    {
        // se reporta un error
        message_semantic_error(v, "La variable '%s' fue definida como '%s' pero su tipo inferido es '%s'. No son compatible. Linea %d",
                               left->data.var_name, left->static_type,
                               inferred_type->name, node->line);
    }

    // uso de destructores

    if (strcmp(left->static_type, "") && node->type == AST_DESTRUCTOR)
    {
        message_semantic_error(v, "La variable '%s' no puede ser de tipo anotado cuando es reasignada. Linea %d",
                               left->data.var_name, node->line);
    }
    // se usa el tipo definido como definitivo, sobreescribinedo la inferencia, para garantizar conssitencia
    if (static_types)
    {
        inferred_type = static_types->type;
    }

    Symbol *s = scope_lookup(node->scope, left->data.var_name);

    if (!s && node->type == AST_DESTRUCTOR)
    {
        message_semantic_error(v, "La variable '%s' necesita ser inicializada en un bloque 'let' antes de ser reasignada. Linea %d",
                               left->data.var_name, node->line);
    }

    // si es una declaracion let , se declara la var en el scope del padre
    else if (node->type == AST_ASSIGNMENT)
    {
        fprintf(stderr, "99999999999999\n");

        scope_add_symbol(node->scope->parent, left->data.var_name, inferred_type, right, 0);

        // se busac ahora el nuevo simbolo
        Symbol *new_s = scope_lookup(node->scope, left->data.var_name);

        // se agrega todas los usages del mismo
        new_s->usage = add_usages(right, new_s->usage);

        // finalmente se asegura que l tipo de retorno de right es el inferido
        right->computed_type = inferred_type;
    }

    // si hay compatiblidad de tipo (o tipo GENERIC) lo que implica que puedes especializarse
    else if (
        ancestor_type(s->type, &inferred_type) ||
        compare_types(inferred_type, &TYPE_GENERIC) ||
        compare_types(s->type, &TYPE_GENERIC))
    {
        // si el valor es GENERIC pero el simbolo tiene tipo real
        if (compare_types(inferred_type, &TYPE_GENERIC) &&
            !compare_types(s->type, &TYPE_GENERIC))
        {
            // se intenta unificar
            if (try_unify_operand(v, right, s->type))
            {
                accept(v, right);
                inferred_type = resolve_node_type(right);
            }
        }

        // si el simbolo es GENERIC pero el tipo no
        else if (compare_types(s->type, &TYPE_GENERIC) &&
                 !compare_types(inferred_type, &TYPE_GENERIC))
        {
            s->type = inferred_type;

            foreach_valueList(der, s->usage)
            {
                ASTNode *node = der->node;
                if (node && compare_types(node->computed_type, &TYPE_GENERIC))
                {
                    try_unify_operand(v, node, inferred_type);
                }
            }

            right->computed_type = inferred_type;
        }

        // se registran las derivaciones para analisis o futuras unificaciones
        s->usage = add_usages(right, s->usage);
        node->usages = add_usages(left, node->usages);
    }

    // si nada de lo anterior es posible entonces es un error de tipo
    else
    {
        message_semantic_error(v, "La variable '%s' fue inicializada como '%s' pero luego reasignada como '%s' . linea %d",
                               s->name, s->type->name, inferred_type->name, node->line);
    }

    right->computed_type = inferred_type;

    // si es tipo reasignacion
    if (node->type == AST_DESTRUCTOR)
    {
        node->computed_type = inferred_type;
    }
    // node->computed_type = inferred_type;

    if (flag)
        free(static_types);


}

void visit_variable(ASTVisitor *v, ASTNode *node)
{
    // Buscar la variable en el scope del padre
    Symbol *sym = scope_lookup(node->scope, node->data.var_name);

    if (sym)
    {
        // si la variable se encontro, entonces el bloque varible sera del tipo de la variable
        node->computed_type = sym->type;
        node->param = sym->param;
        node->usages = sym->usage;
        return;
    }

    // si no se encontro la variable en la tabla de simbolos del nodo
    else if (!compare_types(node->computed_type, &TYPE_ERROR))
    {
        // habra que buscar en el contexto semantico mas global
        fprintf(stderr, "\n333333333333\n");
        EnvItem *item = find_env_item(node->env, node->data.var_name, 0, 1);

        if (!item)
        {
            // si tampoco esta en el entorno
            node->computed_type = &TYPE_ERROR;
            message_semantic_error(v, "La variable '%s' no esta definida en el programa. Line %d",
                                   node->data.var_name, node->line);
        }
        else
        {
            // visitar todas las declaraciones
            accept(v, item->usages);
            // se marca el nodo como no revisado para su posterior revisado
            item->usages->checked = 0;
            // se espera que la varaible ya este registrado en el scope del nodo
            sym = scope_lookup(node->scope, node->data.var_name);
            node->computed_type = sym->type;
            node->param = sym->param;
            node->usages = sym->usage;
        }
    }
}

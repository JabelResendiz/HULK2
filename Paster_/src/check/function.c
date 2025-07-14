
// function.c

#include "check_semantic.h"
#include "../scope/scope.h"
#include "../ast/keyword.h"
#include "../scope/unifiedIndex.h"
#include <string.h>

void check_call_function(ASTVisitor *v, ASTNode *node, TypeValue *type)
{

    ASTNode **args = node->data.func_node.args;

    // 1. Verificamos los argumentos
    for (int i = 0; i < node->data.func_node.arg_count; i++)
    {
        if (args[i]) {
            propagate_env_scope(node, args[i]);
            accept(v, args[i]);
        }
    }

    // buscamos la funcion en el contexto global si es metodo de tipo

    EnvItem *env_item = !type ? find_env_item(node->env, node->data.func_node.name, 0, 0)
                              : lookup_type_member_recursive(type->def_node->env,
                                                             node->data.func_node.name, type, 1);

    if (env_item)
    {
        if (type)
        {
            check_dec_function(v, env_item->usages, type);
        }
        else
        {
            accept(v, env_item->usages);
            fprintf(stderr, ">>>>>>>>>>>>>>>>>>>>>\n");
            fprintf(stderr, "EL nodo %s y valor de retorno es %s\n", node->data.func_node.name, node->computed_type->name);
        }
    }
    /*
    AHST AQUI TODO BIEN
    */

    // 2. Buscar los tipos definidos en los scopes
    Scope *scope = type ? type->def_node->scope : node->scope;

    int arg_count = node->data.func_node.arg_count;

    UnifiedIndex *unified = try_unified_func(v, args, scope, arg_count, node->data.func_node.name, env_item);

    for (UnifiedIndex *curr = unified; curr; curr = curr->next)
    {
        if (args[curr->value]) {
            accept(v, args[curr->value]);
        }
    }

    fprintf(stderr, ">>>>>>>>>>>>>>>>>>>>>\n");
    fprintf(stderr, "2-EL nodo %s y valor de retorno es %s\n", node->data.func_node.name, node->computed_type->name);
    fprintf(stderr, ">>>>>>>>>>>>>>>>>>>>>\n");

    free_unified_index(unified);

    fprintf(stderr, "SERA AQUIIII?\n");
    // crear un array de tpyes de los argumentos
    TypeValue **types = resolve_nodes_type(args, arg_count);

    
    Function *f = malloc(sizeof(Function));

    f->name = node->data.func_node.name;
    f->count_args = arg_count;
    f->args_types = types;

    Function *dec = NULL;

    if (env_item)
    {
        TypeValue **dec_args_type = resolve_nodes_type(
            env_item->usages->data.func_node.args,
            env_item->usages->data.func_node.arg_count);

            fprintf(stderr, "O SERA AQUIIII?\n");
        dec = malloc(sizeof(Function));
        dec->name = env_item->usages->data.func_node.name;
        dec->count_args = env_item->usages->data.func_node.arg_count;
        dec->args_types = dec_args_type;
        dec->result_types = env_item->computed_type ? env_item->computed_type : &TYPE_GENERIC;

        fprintf(stderr, ">>>>>>>>>>>>>>>>>>>>>\n");
        fprintf(stderr, "3-EL nodo %s y valor de retorno es %s\n", node->data.func_node.name, node->computed_type->name);
        fprintf(stderr, ">>>>>>>>>>>>>>>>>>>>>\n");

        fprintf(stderr,"el tipo de retorno de dec  es %s\n",dec->result_types->name);
    }

    if(type)
    {
        fprintf(stderr,"EL TYPE NO ES NULO\n");

    }

        fprintf(stderr, "O SERA SERA AQUIIII?\n");
    FuncStructure *func_structure = type ? find_function_in_hierarchy(type, f, dec)
                                         : match_function_scope(node->scope, f, dec);
    fprintf(stderr,"el valor de function es %d\n",func_structure->state->is_match);

    fprintf(stderr, ">>>>>>>>>>>>>>>>>>>>>\n");
    fprintf(stderr, "4-EL nodo %s y valor de retorno es %s\n", node->data.func_node.name, node->computed_type->name);
    fprintf(stderr, ">>>>>>>>>>>>>>>>>>>>>\n");

    if (func_structure->function)
    {
        node->computed_type = func_structure->function->result_types;

        fprintf(stderr, "\nLa funcData es %s\n", func_structure->function->result_types->name);
    }


    fprintf(stderr, ">>>>>>>>>>>>>>>>>>>>>\n");
    fprintf(stderr, "5-EL nodo %s y valor de retorno es %s\n", node->data.func_node.name, node->computed_type->name);
    fprintf(stderr, ">>>>>>>>>>>>>>>>>>>>>\n");

    fprintf(stderr,"el valor de function es %d\n",func_structure->state->is_match);

    if (!func_structure->state->is_match)
    {
        fprintf(stderr, "Candela\n");
        if (!func_structure->state->name_matches)
        {
            node->computed_type = &TYPE_ERROR;
            fprintf(stderr, "ERROR\n");
            exit(1);
        }
        else if (!func_structure->state->parameters_matched)
        {
            fprintf(stderr, "ERROR\n");
            exit(1);
        }
        else
        {
            if (!strcmp(func_structure->state->second_type_name, "Error"))
                return;

            fprintf(stderr, "ERROR\n");
            exit(1);
        }
    }


    fprintf(stderr,"JBEL\n");
    free(func_structure->state);
    free(types);
    free(f);


}

void visit_call_function(ASTVisitor *v, ASTNode *node)
{
    return check_call_function(v, node, NULL);
}

void check_dec_function(ASTVisitor *v, ASTNode *node, TypeValue *type)
{
    // type is NULL
    fprintf(stderr, "ESTOY EN EL CHECK DEC FUNCTION function\n");

    if (node->checked)
    {
        return;
    }

    node->checked = 1;

    char *visitor_function = v->current_function;

    // actualiza nombre actual
    if (type)
    {
        v->current_function = node->data.func_node.name;
    }

    ASTNode **args = node->data.func_node.args;
    ASTNode *body = node->data.func_node.body;

    if (body) {
        propagate_env_scope(node, body);
    }

    // comprobar que no es palabra reservada
    if (match_keyword(node->data.func_node.name))
    {
        fprintf(stderr, "Es una palabra reservada el nombre de al funcion %s\n", node->data.func_node.name);
        exit(1);
    }

    // Detectar nombres repetidos de parametros

    for (int i = 0; i < node->data.func_node.arg_count; i++)
    {
        for (int j = i + 1; j < node->data.func_node.arg_count; j++)
        {
            if (args[i] && args[j] && !strcmp(args[i]->data.func_node.name, args[j]->data.func_node.name))
            {
                fprintf(stderr, "Simbolo es usado por dos arguments");
                v->current_function = visitor_function;
                exit(1);
            }
        }
    }

    // Procesar cada parametro

    for (int i = 0; i < node->data.func_node.arg_count; i++)
    {
        if (!args[i]) continue;
        
        // conectar scope
        propagate_env_scope(node, args[i]);

        
        // buscar el tipo declarado (por nombre)
        Symbol *arg_type = find_type_scopes(node->scope, args[i]->static_type);

       
        int flag = 0;

        // si no existe buscar en el contexto
        
    
        if (strcmp(args[i]->static_type, "") && !arg_type)
        {
            fprintf(stderr, "NO SE EJECUTA\n");

            EnvItem *env_item = find_env_item(node->env, args[i]->static_type, 1, 0);

            if (env_item)
            {
                accept(v, env_item->usages);

                arg_type = find_type_scopes(node->scope, args[i]->static_type);

                if (!arg_type)
                {
                    arg_type = malloc(sizeof(Symbol));
                    arg_type->name = env_item->computed_type->name;
                    arg_type->type = env_item->computed_type;

                    flag = 1;
                }
            }

            // si no se encuentra el entorno : ERROR

            else
            {
                fprintf(stderr, "ERROR EN LA BUSQUEDA DE ENTORNO\n");
                args[i]->computed_type = &TYPE_ERROR;
            }
        }

        
        // si no tiene tipo usar TYPE_GENERIC
        if (!strcmp(args[i]->static_type, ""))
        {
            args[i]->computed_type = &TYPE_GENERIC;
        }

        // sino setear el resultado
        else if (arg_type)
        {
            args[i]->computed_type = arg_type->type;

            
        }

       // fprintf(stderr,"JULIOOOO CABELLO\n");
        // se agrega el arguemnto al contexto
        scope_add_symbol(node->scope, args[i]->data.var_name, args[i]->computed_type, NULL, 1);

        if (flag)
            free(arg_type);
    }

    // Validar el cuerpo
    if (body) {
        accept(v, body);
    }

    fprintf(stderr, "SE ACABO DE PROCESAR EL CUERPO DE LA FUNCION\n");
    // buscar el entorno asicado al nombre de al funcion
    EnvItem *env_item = !type ? find_env_item(node->env, node->data.func_node.name, 0, 0)
                              : lookup_type_member_recursive(type->def_node->env,
                                                             node->data.func_node.name, type, 1);
    
    
                                                             if (env_item == NULL)
    {
        fprintf(stderr, "ACASO NO ENTIENDES\n");
    }
    fprintf(stderr, "SE BUSCO DE NUEVO LA FUNCION EN EL ENTORNO\n");
    // Infiere el tipo de retorno
    TypeValue *inferred_type_body = body ? resolve_node_type(body) : &TYPE_VOID;

    fprintf(stderr, "el tipo inferido del cuerpo de la funcion es : %s\n", inferred_type_body->name);

    // buscar el tipo de retorno de del nodo
    Symbol *static_type_node = find_type_scopes(node->scope, node->static_type);

    fprintf(stderr, "JABEL RESENDIZ AGUIRRE\n");
    // fprintf(stderr,"el tipo inferred_type es %s y el tipo static_type_node es %s\n: ",inferred_type_body->name,static_type_node->name);
    

    // si el tipo del cuerpo es GENERIC, pero existe un tipo definido y es posible unficar
    if (compare_types(inferred_type_body, &TYPE_GENERIC) &&
        static_type_node && body && try_unify_operand(v, body, static_type_node->type))
    {
        // vuelve a analizar el cuerpo para propagar el tipo unificado
        fprintf(stderr, "JABEL RESENDIZ AGUIRRE dentro\n");
        accept(v, body);
        inferred_type_body = static_type_node->type;
    }

    // si no hay tipo definido del nodo , pero hay uno en el entorno y hay conflicto con el inferido, ERROR
    if (!static_type_node)
    {
        if (env_item && env_item->computed_type)
        {
            fprintf(stderr, "CUBAAAA\n");
        }

        fprintf(stderr, "suiza\n");
    }

    fprintf(stderr, "mexico\n");

    if (!static_type_node && env_item && env_item->computed_type &&
        !ancestor_type(env_item->computed_type, inferred_type_body) &&
        !ancestor_type(env_item->computed_type, &TYPE_ERROR) &&
        !ancestor_type(env_item->computed_type, &TYPE_GENERIC) &&
        !ancestor_type(inferred_type_body, &TYPE_GENERIC) &&
        !ancestor_type(inferred_type_body, &TYPE_ERROR))
    {
        fprintf(stderr, "JABEL RESENDIZ AGUIRRE error\n");
        fprintf(stderr, "Es necesario lanzar un error aqui\n");
        exit(1);
    }
    fprintf(stderr, "JABEL RESENDIZ AGUIRRE final\n");
    // buscar el tipo de entorno anotado (si se escribio uno explicito)
    int flag = 0;

    // si
    if (strcmp(node->static_type, "") && !static_type_node)
    {
        fprintf(stderr, "NO SE EJECUTA\n");

        EnvItem *env_item = find_env_item(node->env, node->static_type, 1, 0);

        if (env_item)
        {
            accept(v, env_item->usages);

            static_type_node = find_type_scopes(node->scope, node->static_type);

            if (!static_type_node)
            {
                static_type_node = malloc(sizeof(Symbol));
                static_type_node->name = env_item->computed_type->name;
                static_type_node->type = env_item->computed_type;

                flag = 1;
            }
        }

        // no se encontro en el entorno

        else
        {
            fprintf(stderr, "EL tipo de retorno de la funcion no fue definido\n");
        }
    }

    
    // comprobacion de incompatiblidad entre tipos inferidos y el tipo definido
    if (static_type_node &&
        !ancestor_type(static_type_node->type, inferred_type_body) && !compare_types(inferred_type_body, &TYPE_ERROR))
    {
        fprintf(stderr, "ERRROR\n");
        exit(1);
    }

    if (static_type_node)
    {
        inferred_type_body = static_type_node->type;
    }

    // Ultimo intento de inferenci si el tipo aun es GENERIC

    if (compare_types(inferred_type_body, &TYPE_GENERIC))
    {
        fprintf(stderr, "~~~~~~~~~~NO SE EJECUTA~~~~~~~~~~~~~~\n");
        if (body) {
            accept(v, body);
        }
        if (compare_types(inferred_type_body, &TYPE_GENERIC))
        {
            fprintf(stderr, "ERROR DE NUEVO\n");
            exit(1);
        }
    }

    // Declaracion final de la fucnion

    Function *func = find_function_by_name(node->scope, node->data.func_node.name);
    TypeValue **args_type = resolve_nodes_type(args, node->data.func_node.arg_count);


   
    
    if (!func)
    {
        for (int i = 0; i < node->data.func_node.arg_count; i++)
        {
            if (!args[i]) continue;
            
            // busca el simbolo que representa el parametro en el scope
            Symbol *param = lookup_parameter(node->scope, args[i]->data.var_name);

            fprintf(stderr, "el tipo del paramtror es %s\n", param->type->name);

            if (compare_types(param->type, &TYPE_GENERIC))
            {
                fprintf(stderr, "CLASE CANDELAAAAAA\n");
                if (body) {
                    accept(v, body);
                }
                if (compare_types(param->type, &TYPE_GENERIC))
                {
                    fprintf(stderr, "ERROR NUEVAMNET\n");
                    exit(1);
                }
            }
            // si fue anotado y definido
            Symbol *def_type = find_type_scopes(node->scope, args[i]->static_type);

            // si el tipo esta definido x:Int
            if (def_type)
                args_type[i] = def_type->type;

            // si fue inferido del cuerpo
            else
                args_type[i] = param->type;

            node->data.func_node.args[i]->computed_type = args_type[i];
        }

        // agregar la funcion al scope

        scope_add_function(node->scope->parent, node->data.func_node.arg_count,
                           args_type, inferred_type_body, node->data.func_node.name);

        Function* r = node->scope->parent->functions->first;

        fprintf(stderr,"!!!!!!!!!!!!!!!!!!!! %s\n",r->name);
    }

    // actualizar el tipo de retorno en el entorno
    if (env_item)
    {
        env_item->computed_type = inferred_type_body;
    }

    // restaurar el nombre de la funcion actual
    v->current_function = visitor_function;

    fprintf(stderr, "SE ACABO DE PROCESAR LA FUNCION\n");
}

void visit_dec_function(ASTVisitor *v, ASTNode *node)
{
    return check_dec_function(v, node, NULL);
}


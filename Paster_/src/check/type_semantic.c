

// type_semantic.c

#include "check_semantic.h"
#include "../scope/inheritance.h"
#include "../scope/unifiedIndex.h"
#include "../ast/keyword.h"
#include <stdio.h>
#include <string.h>

#define foreach_valueList(elem, list) \
    for (UsageElemental *elem = (list) ? (list)->first : NULL; elem != NULL; elem = elem->next)

InheritanceChain *ich_list;

void visit_type_dec(ASTVisitor *v, ASTNode *node)
{
    fprintf(stderr, "ESTOY EN LA DECLARACION DE UN TIPO\n");
    fprintf(stderr, "EL node.checked es %d\n", node->checked);

    // Deteccion de Herencia Circular
    // Previene si el padre esta en la cadena de herencia
    if (find_type_in_inheritance_chain(node->data.typeDef.name_parent, ich_list))
    {
        fprintf(stderr, "Hay un error aqui\n");

        ich_list = free_inheritance_chain(ich_list);
        return;
    }

    if (node->checked)
    {
        fprintf(stderr, "YA SE INSPECCIONO\n");
        // si ya se inspecciono el nodo
        return;
    }

    node->checked = 1;

    // guarda el tipo actual del visitor
    TypeValue *visitor_type = v->current_type;

    // recoger el ambito del padre
    Scope *parent_scope = node->scope->parent;

    // actualizar la lista MRO del padre en la lista
    ich_list = extend_inheritance_chain(ich_list, node->data.typeDef.name_parent);

    // obtiene los parametros del tipo
    ASTNode **params = node->data.typeDef.args;

    // obtiene el conjutno de definciones del tipo
    ASTNode **definitions = node->data.typeDef.body_elements;

    /// Detectar parametros duplicados en la declaracion
    for (int i = 0; i < node->data.typeDef.args_count - 1; i++)
    {
        for (int j = i + 1; j < node->data.typeDef.args_count; j++)
        {
            if (!strcmp(
                    params[i]->data.var_name,
                    params[j]->data.var_name))
            {
                fprintf(stderr, "Hay un error claro");
                return;
            }
        }
    }

    // Procesa los parametro del tipo
    for (int i = 0; i < node->data.typeDef.args_count; i++)
    {
        // configuracion del ambito y context
        propagate_env_scope(node, params[i]);

        // buscamos el tipo del parametro
        Symbol *param_type = find_type_scopes(node->scope, params[i]->static_type);
        int flag = 0;

        // si el tipo esta definido y no se haya encontrado un tipo definido
        if (strcmp(params[i]->static_type, "") && !param_type)
        {

            // buscar en el contexto
            EnvItem *env_item = find_env_item(
                node->env, params[i]->static_type, 1, 0);

            // si fue encontrado
            if (env_item)
            {

                if (!env_item->usages->checked)
                {
                    // procesa su declaracion
                    accept(v, env_item->usages);

                    // reintenta buscar el tipo ahora que deberia esta definido
                    param_type = find_type_scopes(node->scope, params[i]->static_type);
                }

                else if (env_item->computed_type)
                {
                    // se crea un simbolo temporal
                    param_type = malloc(sizeof(Symbol));
                    param_type->name = env_item->computed_type->name;
                    param_type->type = env_item->computed_type;
                    flag = 1;
                }
                // si el tipo sigue sin andar definido
                if (!param_type)
                {
                    fprintf(stderr, "HAY UN ERROR\n");
                }
            }

            // si no se encontro en el contexto : ERROR
            else
            {
                fprintf(stderr, "ALGP SUCEDE\n");
                params[i]->computed_type = &TYPE_ERROR;
            }
        }

        // si el tipo no esta definido
        else if (strcmp(params[i]->static_type, "") && compare_types(param_type->type, &TYPE_VOID))
        {
            fprintf(stderr, "HAY UNERROR\n");
            params[i]->computed_type = &TYPE_ERROR;
        }

        // Anotar los parametros como ANY si ellos no estan anotados
        if (!strcmp(params[i]->static_type, ""))
        {
            params[i]->computed_type = &TYPE_GENERIC;
        }
        else if (param_type)
        {
            params[i]->computed_type = param_type->type;
        }

        // se declara el simbolo en el scope
        scope_add_symbol(
            node->scope, params[i]->data.var_name,
            params[i]->computed_type, NULL, 1);

        // se libera la memoria temporal
        if (flag)
            free(param_type);
    }

    // busqueda del tipo del padre
    Symbol *parent_info = find_type_scopes(node->scope, node->data.typeDef.name_parent);
    TypeValue *parent_type = &TYPE_OBJ;

    // si existe el nombre de padre y no se haya encontrado en el ambito actual
    if (strcmp(node->data.typeDef.name_parent, "") &&
        !parent_info)

    {
        // buscar en el contexto
        EnvItem *env_item = find_env_item(
            node->env,
            node->data.typeDef.name_parent, 1, 0);

        // si se encontro en el ambito
        if (env_item)
        {
            int aux = 0;

            if (!env_item->usages->checked)
            {
                // se procesa la declaracion del padre
                accept(v, env_item->usages);

                // se reintenta buscar el tipo del padre en el ambito
                parent_info = find_type_scopes(node->scope, node->data.typeDef.name_parent);
            }

            else if (env_item->computed_type)
            {
                parent_type = env_item->computed_type;
                aux = 1;
            }

            if (!parent_info && !aux)
            {
                fprintf(stderr, "Algo pasa\n");
                node->checked = 0;
                ich_list = free_inheritance_chain(ich_list);
                return;
            }

            else if (!aux)
            {
                parent_type = parent_info->type;
            }
        }
    }

    // si existe el nombre del padre y es un tipo del inicial del programa : ERROR
    else if (
        strcmp(node->data.typeDef.name_parent, "") &&
        is_builtin_type(parent_info->type))
    {
        fprintf(stderr, "HAY UN ERROR GRANDE\n");
        exit(1);
    }

    else if (strcmp(node->data.typeDef.name_parent, ""))
    {
        parent_type = parent_info->type;
    }

    // si el parent no es de ningun tipo inicial y el hijo no tiene parametros propios
    if (!is_builtin_type(parent_type) && !node->data.typeDef.p_constructor)
    {
        // copia la cantidad de parametros del padre al hijo
        node->data.typeDef.args_count = parent_type->num_params;
        node->data.typeDef.args = malloc(sizeof(ASTNode *) * parent_type->num_params);

        // copia cada paametro del padre al hijo ,actualizando la linea a la del hijo
        for (int i = 0; i < parent_type->num_params; i++)
        {
            node->data.typeDef.args[i] = parent_type->def_node->data.typeDef.args[i];
            node->data.typeDef.args[i]->line = node->line;
        }

        // establece los arguemntos del tipo
        params = node->data.typeDef.args;

        // por cada parametro propaga el contexto y el padre y alo agrega ala tabla de simbolos
        for (int i = 0; i < node->data.typeDef.args_count; i++)
        {
            propagate_env_scope(node, params[i]);

            scope_add_symbol(
                node->scope, params[i]->data.var_name,
                params[i]->computed_type, NULL, 1);
        }
    }

    // si el parent no es de nigun tipo incial pero el hijo tiene sus propios parametros
    else if (!is_builtin_type(parent_type))
    {

        // crea una instancia temporal del tipo padre con los argumentos proporicionados
        ASTNode *parent = create_struct_instance_node(
            parent_type->name,
            node->data.typeDef.p_args,
            node->data.typeDef.p_args_count);

        // pasa los argumentos
        propagate_env_scope(node, parent);

        parent->line = node->line;
        // vistia la instancia padre para resovler tipos
        accept(v, parent);

        // libera la instancia temporal despues de usar
        free_ast(parent);
    }

    ich_list = free_inheritance_chain(ich_list);

    // crea el nuevo tipo
    TypeValue *this = create_type(node->data.typeDef.name_type, parent_type, NULL, 0, node);

    // busca en el contexto del padre, su nombre
    EnvItem *env_item = find_env_item(node->env->parent, this->name, 1, 0);
    // marca su tipo de retorno
    env_item->computed_type = this;
    // marca el tipo del visitor a this
    v->current_type = this;

    fprintf(stderr, "5-JABEL RESEMDIZ8998\n");

    // por cada insruccion(definicion) dentro
    for (int i = 0; i < node->data.typeDef.body_count; i++)
    {
        ASTNode *child = definitions[i];

        // propaga el contexto y el ambito
        propagate_env_scope(node, child);

        if (!register_type_member_in_env(node->env, child, this->name))
        {
            fprintf(stderr, "CNADELA\n");
            char *name = child->type == AST_DECL_FUNC ? child->data.func_node.name : child->data.binary_op.left->data.var_name;

            fprintf(stderr, "HAY UNERRROR\n");
            exit(1);
        }

        fprintf(stderr, "CNADELA23\n");
    }

    fprintf(stderr, "6-JABEL RESEMDIZ\n");

    for (int i = 0; i < node->data.typeDef.body_count; i++)
    {
        ASTNode *current = definitions[i];
        if (current->type == AST_ASSIGNMENT)
        {
            accept(v, current);
        }
    }

    fprintf(stderr, "7-JABEL RESEMDIZ\n");

    // Hidding parameters for method declarations
    for (int i = 0; i < node->data.typeDef.args_count; i++)
    {
        Symbol *sym = lookup_parameter(
            node->scope, node->data.typeDef.args[i]->data.var_name);
        sym->is_type_param = 1;
    }

    scope_add_symbol(node->scope, "self", this, NULL, 0);
    fprintf(stderr, "8-JABEL RESEMDIZ\n");

    for (int i = 0; i < node->data.typeDef.body_count; i++)
    {
        ASTNode *current = definitions[i];
        if (current->type == AST_DECL_FUNC)
        {

            check_dec_function(v, current, this);

            // fprintf(stderr, "1111111111111111111111111111EL tipo de la primer funcion del scope es %s Y nombre %s\n", node->scope->functions->first->args_types[0]->name, node->scope->functions->first->name);
        }
    }

    fprintf(stderr, "9-JABEL RESEMDIZ\n");

    // Symbol* actual = node->scope->symbols;

    // while(actual)
    // {
    //     fprintf(stderr,"MI nombre es %s y soy parametro %d\n",actual->name, actual->param);
    //     actual = actual->next;
    // }
    // exit(1);

    // Returning changes made to parameters
    for (int i = 0; i < node->data.typeDef.args_count; i++)
    {
        Symbol *sym = lookup_parameter(
            node->scope, node->data.typeDef.args[i]->data.var_name);
        sym->is_type_param = 0;
    }

    // exit(1);
    //  obtener los tipos de los argumentos pasados  por argumento
    TypeValue **args_type = resolve_nodes_type(params, node->data.typeDef.args_count);

    fprintf(stderr, "hay %d simbolos\n", node->scope->s_count);
    // exit(1);

    fprintf(stderr, "9-JABEL RESEMDIZ\n");

    // recorrer los argumentos y analizar sus tipos
    for (int i = 0; i < node->data.typeDef.args_count; i++)
    {
        fprintf(stderr, "La variable que voy a buscar es %s\n", params[i]->data.var_name);

        // obtener el simbolo del paramtro dentro del scope del type
        Symbol *param = lookup_parameter(node->scope, params[i]->data.var_name);

        // exit(1);

        fprintf(stderr, "10-JABEL RESEMDIZ\n");

        // si el tipo del parametro es GENERIC
        if (compare_types(param->type, &TYPE_GENERIC))
        {
            fprintf(stderr, "11-JABEL RESEMDIZ\n");

            // Visitar las definiciones del tipo(cuerpo de la clase)
            for (int i = 0; i < node->data.typeDef.body_count; i++)
            {
                fprintf(stderr, "11-JABEL RESEMDIZ\n");

                accept(v, definitions[i]);
            }

            // si sigue sin inferirse el tipo del parametro : ERROR
            if (compare_types(param->type, &TYPE_GENERIC))
            {
                fprintf(stderr, "ERROR NO SE PUEDE INFERIR EL TIPO DEL PARAMETRO");
                exit(1);
            }
        }
        fprintf(stderr, "12-JABEL RESEMDIZ\n");

        // el tipo asociado no es GENERIC o fue inferido
        Symbol *def_type = find_type_scopes(node->scope, params[i]->static_type);
        args_type[i] = def_type ? def_type->type : param->type;
        node->data.typeDef.args[i]->computed_type = args_type[i];
    }
    fprintf(stderr, "13-JABEL RESEMDIZ\n");

    if (parent_type && !is_builtin_type(parent_type))
    {
        ASTNode *parent_instance = !node->data.typeDef.p_constructor ? create_struct_instance_node(parent_type->name, node->data.typeDef.args,
                                                                                                   node->data.typeDef.args_count)
                                                                     : create_struct_instance_node(parent_type->name, node->data.typeDef.p_args,
                                                                                                   node->data.typeDef.p_args_count);

        parent_instance->data.typeDef.parent_instance = parent_type->def_node->data.typeDef.parent_instance;
        parent_instance->computed_type = parent_type;
        node->data.typeDef.parent_instance = parent_instance;
    }

    node->data.typeDef.id = ++v->id;
    this->def_node = node;
    this->num_params = node->data.typeDef.args_count;
    this->argument_types = args_type;
    scope_add_type(node->scope->parent, this);
    ich_list = free_inheritance_chain(ich_list);
    v->current_type = visitor_type;
}

void visit_type_instance(ASTVisitor *v, ASTNode *node)
{
    fprintf(stderr, "EstAMOS EN EL TYPE INSTANCE\n");

    ASTNode **args = node->data.typeDef.args;

    for (int i = 0; i < node->data.typeDef.args_count; i++)
    {
        propagate_env_scope(node, args[i]);

        accept(v, args[i]);
    }

    if (match_keyword(node->data.typeDef.name_type))
    {
        node->computed_type = &TYPE_ERROR;
        fprintf(stderr, "candela\n");
        exit(1);
        return;
    }
    // buscar el tipo en el contexto
    EnvItem *env_item = find_env_item(
        node->env, node->data.typeDef.name_type, 1, 0);

    // if(env_item->computed_type!= NULL)
    // {
    //     fprintf(stderr,"CANDELAA\n");
    //     exit(1);
    // }
    // fprintf(stderr,"exit(1)\n");
    // exit(1);

    // si la encuentra ,vista su decalracion para este bien procesado antes de usarla
    if (env_item)
    {
        accept(v, env_item->usages);
    }

    // la estrucuta que nos permite buscar todos los nodos que logran unificar
    UnifiedIndex *unified = try_unified_type(v, args, node->scope, node->data.typeDef.args_count,
                                             node->data.typeDef.name_type, env_item);

    for (UnifiedIndex *curr = unified; curr != NULL; curr = curr->next)
    {
        accept(v, args[curr->value]);
    }

    free_unified_index(unified);

    // obtiene el tipo computado de cada argumento pasado al constructor
    TypeValue **args_types = resolve_nodes_type(args, node->data.typeDef.args_count);

    Function *function = malloc(sizeof(Function));
    function->name = node->data.typeDef.name_type;
    function->count_args = node->data.typeDef.args_count;
    function->args_types = args_types;

    Function *dec = NULL;

    if (env_item && env_item->computed_type)
    {

        TypeValue **dec_args_types = resolve_nodes_type(
            env_item->usages->data.typeDef.args,
            env_item->usages->data.typeDef.args_count);

        dec = malloc(sizeof(Function));
        dec->name = env_item->usages->data.typeDef.name_type;
        dec->count_args = env_item->usages->data.typeDef.args_count;
        dec->args_types = dec_args_types;
        dec->result_types = env_item->computed_type;
    }
    else if (env_item)
    {

        node->computed_type = &TYPE_ERROR;
        fprintf(stderr, "ERROR\n");
        exit(1);
    }

    FuncStructure *funcData = find_type_data(node->scope, function, dec);

    fprintf(stderr, "JABEL RESENDIZ\n");

    if (funcData->function && funcData->function->result_types)
    {
        node->computed_type = funcData->function->result_types;
        node->data.typeDef.parent_instance = funcData->function->result_types->def_node->data.typeDef.parent_instance;
    }
    else if (funcData->function)
    {
        node->computed_type = create_type(
            node->data.typeDef.name_type, NULL, NULL, 0, env_item->usages);
    }

    if (!funcData->state->is_match)
    {
        if (!funcData->state->name_matches)
        {
            node->computed_type = &TYPE_ERROR;
            fprintf(stderr, "ERROR\n");
            exit(1);
        }
        else if (!funcData->state->parameters_matched)
        {
            fprintf(stderr, "ERROR\n");
            exit(1);
        }
        else
        {
            if (!strcmp(funcData->state->second_type_name, "Error"))
                return;

            fprintf(stderr, "ERROR\n");
            exit(1);
        }
    }

    free(funcData->state);
    free(args_types);
    free(function);
}

void visit_getter(ASTVisitor *v, ASTNode *node)
{
    fprintf(stderr, "ESTAMOS EN EL GETTER DE UNA FUNCION\n");

    // el objetio del cual se quiere acceder a un aributo o metodo
    ASTNode *instance = node->data.binary_op.left;

    // el atributo o metodo que se quiere acceders
    ASTNode *member = node->data.binary_op.right;

    // propagar el scope y el entorno
    propagate_env_scope(node, instance);
    propagate_env_scope(node, member);

    // visitar el instancia y determinar su tipo
    accept(v, instance);
    TypeValue *instance_type = resolve_node_type(instance);

    fprintf(stderr, "Instance_type es de tipo %s\n", instance_type->name);

    // si no se puede determinar el tipo de la instancia
    if (compare_types(instance_type, &TYPE_ERROR))
    {
        fprintf(stderr, "INSTANCE_TYPE ES UN ERROR\n");

        node->computed_type = &TYPE_ERROR;
        return;
    }
    else if (compare_types(instance_type, &TYPE_GENERIC))
    {
        fprintf(stderr, "INSTANCE_TYPE ES UN GENERIC\n");

        if (!try_unify_attr(v, node))
        {
            node->computed_type = &TYPE_ERROR;
            return;
        }
        else
        {
            accept(v, instance); // visit again if unified
            instance_type = resolve_node_type(instance);
        }
    }

    fprintf(stderr, "TODO BIEN\n");

    // si se intenta acceder a un atributo directamente desde fuera del tipo
    // no usndo "self" y el atributo no es accesible , ERROR
    if (member->type == AST_VAR && ((
                                        instance->type == AST_VAR &&
                                        strcmp(instance->data.var_name, "self")

                                            ) ||
                                    instance->type != AST_VAR))
    {
        fprintf(stderr, "ERRROR GRANDEEEE\n");
        node->computed_type = &TYPE_ERROR;
        exit(1);
    }

    // si el miemebro es una variable

    else if (member->type == AST_VAR)
    {
        // _(tipo de la instancia)_(nombre de la variable)
        member->data.var_name = generate_underscored_name(instance_type->name, member->data.var_name);

        // buscar atributo en la tabla de simbolo
        Symbol *sym = lookup_type_attr(instance_type, member->data.var_name);

        if (sym)
        {
            member->computed_type = sym->type;
            member->param = sym->param;
            member->usages = sym->usage;
        }

        else
        {
            EnvItem *env_item = lookup_type_member_recursive(
                instance_type->def_node->env,
                member->data.var_name,
                instance_type, 0);

            if (!env_item)
            {
                // ERROR
                fprintf(stderr, "EERRORR\n");
                member->computed_type = &TYPE_ERROR;
                exit(1);
            }

            else
            {
                accept(v, env_item->usages);

                sym = lookup_type_attr(instance_type, member->data.var_name);
                member->computed_type = sym->type;
                member->param = sym->param;
                member->usages = sym->usage;
            }
        }
    }

    fprintf(stderr, "Candela reverenda\n");

    // si es una llamada de funcion y no es un error
    if (member->type == AST_CALL_FUNC && !compare_types(instance_type, &TYPE_ERROR))
    {
        Symbol *new_symb = find_type_scopes(node->scope, instance_type->name);

        if (!new_symb)
        {
            // fprintf(stderr,"Candela reverenda\n");
            EnvItem *new_item = find_env_item(node->env, instance_type->name, 1, 0);

            if (new_item)
            {
                accept(v, new_item->usages);
                instance_type = new_item->computed_type;
            }

            else
            {
                node->computed_type = &TYPE_ERROR;
                if (!compare_types(instance_type, &TYPE_ERROR))
                {
                    fprintf(stderr, "HAY UN ERROR\n");
                    exit(1);
                }
            }
        }
        else if (is_builtin_type(instance_type))
        {
            node->computed_type = &TYPE_ERROR;

            if (!compare_types(instance_type, &TYPE_ERROR))
            {
                fprintf(stderr, "Hay un errror\n");
                exit(1);
            }
        }
        else
        {
            instance_type = new_symb->type;
        }

        fprintf(stderr, "Candela reverenda\n");

        member->data.func_node.name = generate_underscored_name(instance_type->name, member->data.func_node.name);

        check_call_function(v, member, instance_type);
    }

    fprintf(stderr, "Candela reverenda\n");

    node->computed_type = resolve_node_type(member);
    node->usages = add_usages(member, node->usages);
}

void visit_setter(ASTVisitor *v, ASTNode *node)
{

    fprintf(stderr, "ESTAMOS EN EL SETTER DE UNA FUNCION\n");

    ASTNode *instance = node->data.setter.instance;
    ASTNode *property = node->data.setter.property;
    ASTNode *value = node->data.setter.value;


    propagate_env_scope(node, value);
    propagate_env_scope(node, instance);
    propagate_env_scope(node, property);

   
    
    accept(v, instance);
     
    TypeValue *instance_type = resolve_node_type(instance);


    fprintf(stderr, "Candela reverenda\n");
    
    if (compare_types(instance_type, &TYPE_ERROR))
    {
        fprintf(stderr, "INStance_Type es un ERROR\n");

        node->computed_type = &TYPE_ERROR;
        return;
    }
   
    if ((instance->type == AST_VAR &&
         strcmp(instance->data.var_name, "self")) ||
        instance->type != AST_VAR)
    {
        fprintf(stderr, "ERROR ASKER \n");
        node->computed_type = &TYPE_ERROR;
        return;
    }

    
    // prefijar el atributo con el tipo, por ejemplo name -> _User_name
    property->data.var_name = generate_underscored_name(instance_type->name, property->data.var_name);

    // buscar el simbolo del atributo
    Symbol *s = lookup_type_attr(instance_type, property->data.var_name);

    fprintf(stderr, "TODO BIEN EN EL SETTER 1\n");

    // si esta el simbolo del atributo
    
    
    if (s)
    {
        fprintf(stderr, "ESTA EN EL SCOPE\n");
        // si esta en el scope
        property->computed_type = s->type;
        property->param = s->param;
        property->usages = s->usage;
    }

    else
    {
        // no se enecontro en el scope, buscar en el contexto

        EnvItem *env_item = lookup_type_member_recursive(instance_type->def_node->env, property->data.var_name, instance_type, 0);

        if (env_item)
        {
            accept(v, env_item->usages);
            s = lookup_type_attr(instance_type, property->data.var_name);

            property->computed_type = s->type;
            property->param = s->param;
            property->usages = s->usage;
        }

        else
        {
            // error no se encontro en el entorno
            fprintf(stderr, "No se encontro en el entorno\n");
            property->computed_type = &TYPE_ERROR;
            exit(1);
        }
    }

    // se visita el valor
    fprintf(stderr, "DEJAME VER ALGO\n");

    
    accept(v, value);
    fprintf(stderr, "DEJAME VER ALGO\n");

    
    TypeValue *inferred_type = resolve_node_type(value);

    fprintf(stderr, "DEJAME VER ALGO\n");

    if (s)
    {
        fprintf(stderr, "SE METIO AQUI\n");
        // compatibilidad de tipos
        if (ancestor_type(s->type, inferred_type) ||
            compare_types(inferred_type, &TYPE_GENERIC) ||
            compare_types(s->type, &TYPE_GENERIC))
        {

            fprintf(stderr, "SE METIO AQUI \n");
            if (compare_types(inferred_type, &TYPE_GENERIC) &&
                !compare_types(s->type, &TYPE_GENERIC))
            {
                if (try_unify_operand(v, value, s->type))
                {
                    accept(v, value);
                    inferred_type = s->type;
                }
            }

            else if (compare_types(s->type, &TYPE_GENERIC) &&
                     !compare_types(inferred_type, &TYPE_GENERIC))
            {
                s->type = inferred_type;

                foreach_valueList(der, s->usage)
                {
                    ASTNode *f = der->node;
                    if (f && compare_types(f->computed_type, &TYPE_GENERIC))
                    {
                        try_unify_operand(v, f, inferred_type);
                    }
                }
            }

            s->usage = add_usages(value, s->usage);
        }

        // no son compatibles: ERROR
        else
        {
            fprintf(stderr, "Hay un problema de compatibilidad\n");
            exit(1);
        }

        fprintf(stderr, "TODO BIEN EN LE SETTER 2\n");
    }

    node->computed_type = resolve_node_type(value);
    node->usages = add_usages(value, node->usages);
    node->usages = add_usages(property, node->usages);
}
